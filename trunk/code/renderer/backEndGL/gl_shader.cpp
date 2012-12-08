/*
============================================================================
Copyright (C) 2012 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// gl_shader.cpp - GLSL shaders for openGL backend
#include "gl_shader.h"
#include <shared/array.h>
#include <api/coreAPI.h>
#include <api/vfsAPI.h>

bool GL_CompileShaderProgram(GLuint handle, const char *source) {
	glShaderSourceARB(handle, 1,(const GLcharARB**) &source, 0);
	glCompileShaderARB(handle);
	int status;
	glGetObjectParameterivARB(handle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(status == 0) {
		int logLen = 0;
		glGetObjectParameterivARB(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
		if(logLen > 0) {
			char *tmp = (char*)malloc(logLen+1);
			glGetInfoLogARB(handle, logLen, 0, tmp);
			g_core->RedWarning(tmp);
			free(tmp);
		}
		return true; // error
	}
	return false;
}

bool GL_LoadFileTextToString(str &out, const char *fname) {
	char *fileData;
	int len = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open
	}
	out = fileData;
	g_vfs->FS_FreeFile(fileData);
	return false;
}
arraySTD_c<glShader_c*> gl_shaders;
static glShader_c *GL_FindShader(const char *baseName) {
	for(u32 i = 0; i < gl_shaders.size(); i++) {
		glShader_c *s = gl_shaders[i];
		if(!stricmp(baseName,s->getName())) {
			return s;
		}
	}
	return 0;
}
glShader_c *GL_RegisterShader(const char *baseName) {
	// see if the shader is already loaded
	glShader_c *ret = GL_FindShader(baseName);
	if(ret) {
		if(ret->isValid())
			return ret;
		return 0;
	}
	// if not, try to load it
	str vertFile = "glsl/";
	vertFile.append(baseName);
	vertFile.append(".vert");
	str fragFile = "glsl/";
	fragFile.append(baseName);
	fragFile.append(".frag");
	ret = new glShader_c;
	ret->name = baseName;
	gl_shaders.push_back(ret);
	if(g_vfs->FS_FileExists(fragFile) == false) {
		g_core->RedWarning("GL_RegisterShader: file %s does not exist\n",fragFile.c_str());
		return 0;
	}
	if(g_vfs->FS_FileExists(vertFile) == false) {
		g_core->RedWarning("GL_RegisterShader: file %s does not exist\n",vertFile.c_str());
		return 0;
	}
	str vertexSource;
	if(GL_LoadFileTextToString(vertexSource,vertFile)) {
		g_core->RedWarning("GL_RegisterShader: cannot open %s for reading\n",vertFile.c_str());
		return 0;
	}
	str fragmentSource;
	if(GL_LoadFileTextToString(fragmentSource,fragFile)) {
		g_core->RedWarning("GL_RegisterShader: cannot open %s for reading\n",fragFile.c_str());
		return 0;
	}
	// load separate programs
	// vertex program (.vert file)
	GLuint vertexProgram = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	if(GL_CompileShaderProgram(vertexProgram,vertexSource)) {
		glDeleteShader(vertexProgram);
		return 0;
	}
	// fragment program (.frag file)
	GLuint fragmentProgram = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	if(GL_CompileShaderProgram(fragmentProgram,fragmentSource)) {
		glDeleteShader(vertexProgram);
		glDeleteShader(fragmentProgram);
		return 0;
	}	
	// link vertex and fragment programs to create final shader
	GLhandleARB shader = glCreateProgramObjectARB();
	glAttachObjectARB(shader,vertexProgram);
	glAttachObjectARB(shader,fragmentProgram);
	glLinkProgramARB(shader);
	// check for errors
	int status;
	glGetObjectParameterivARB(shader, GL_OBJECT_LINK_STATUS_ARB, &status);
	if(status == 0) {
		int logLen = 0;
		glGetObjectParameterivARB(shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);
		if(logLen > 0) {
			char *tmp = (char*)malloc(logLen+1);
			glGetInfoLogARB(shader, logLen, 0, tmp);
			g_core->RedWarning(tmp);
			free(tmp);
		}
		glDeleteShader(vertexProgram);
		glDeleteShader(fragmentProgram);
		glDeleteProgram(shader);
		return 0;
	}
	ret->handle = shader;
	// precache uniform locations
	ret->sColorMap = glGetUniformLocation(shader,"colorMap");
	ret->uLightOrigin = glGetUniformLocation(shader,"u_lightOrigin");
	ret->uLightRadius = glGetUniformLocation(shader,"u_lightRadius");
	return ret;
}
void GL_ShutdownHLSLShaders() {
	for(u32 i = 0; i < gl_shaders.size(); i++) {
		delete gl_shaders[i];
	}
	gl_shaders.clear();
}