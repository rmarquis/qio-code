#include <api/moduleManagerAPI.h>
#include <api/iFaceMgrAPI.h>
#include <shared/str.h>
#include <shared/array.h>
#include "../sys/sys_loadlib.h"

class moduleIMPL_c : public moduleAPI_i {
	friend class moduleManagerIMPL_c;
	void *handle;
	str name;
	//str fullPath;
};

typedef void (*shareAPIsFunc_t)(class iFaceMgrAPI_i *iFaceMgr);

class moduleManagerIMPL_c : public moduleManagerAPI_i {
	arraySTD_c<moduleIMPL_c*> modules;

	moduleIMPL_c *findModuleInternal(const char *moduleName) {
		for(u32 i = 0; i < modules.size(); i++) {
			moduleIMPL_c *m = modules[i];
			if(!stricmp(moduleName,m->name)) {
				return m;
			}
		}
		return 0;
	}
	class moduleAPI_i *load(const char *moduleName) {
		moduleAPI_i *loaded = findModuleInternal(moduleName);
		if(loaded) {
			Com_Printf("moduleManagerIMPL_c::load: %s already loaded!\n",moduleName);
			return loaded;
		}

		void *h = Sys_LoadDll(moduleName,true);
		if(h == 0) {
			Com_Printf("moduleManagerIMPL_c::load: LoadLibrary failed for %s\n",moduleName);
			return 0;
		}
		shareAPIsFunc_t shareAPIsFunc = (shareAPIsFunc_t)Sys_LoadFunction(h,"vmMain");
		if(shareAPIsFunc == 0) {
			Com_Printf("moduleManagerIMPL_c::load: cannot find ShareAPIs func in %s\n",moduleName);
			return 0;
		}
		shareAPIsFunc(g_iFaceMan);
		moduleIMPL_c *nm = new moduleIMPL_c;
		nm->name = moduleName;
		//nm->fullPath = fullPath;
		modules.push_back(nm);
		return nm;
	}
	void unload(moduleAPI_i **mPtr) {
		if(mPtr == 0 || *mPtr == 0) {
			
			return;
		}
		moduleIMPL_c *m = (moduleIMPL_c*)(*mPtr);
		if(modules.isOnList(m) == false) {
			// this should never happen
			Com_Printf("moduleManagerIMPL_c::load: %x is not a valid module ptr!\n",m);
			return;
		}
		modules.remove(m);
		Sys_UnloadLibrary(m->handle);
		delete m;
		*mPtr = 0;
	}
};

static moduleManagerIMPL_c g_staticModuleManager;
moduleManagerAPI_i *g_moduleMgr = &g_staticModuleManager;


