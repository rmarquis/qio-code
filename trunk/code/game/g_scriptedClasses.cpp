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
// g_scriptedClasses.cpp
#include "g_local.h"
#include "g_scriptedClasses.h"
#include <api/coreAPI.h>
#include <shared/hashTableTemplate.h>

static hashTableTemplateExt_c<scriptedClass_c> g_scriptedClasses;

const scriptedClass_c *G_FindScriptedClassDef(const char *className) {
	return g_scriptedClasses.getEntry(className);
}
u32 G_GetNumKnownScriptedClassDefs() {
	return g_scriptedClasses.size();
}
void G_ShutdownScriptedClasses() {
	for(u32 i = 0; i < g_scriptedClasses.size(); i++) {
		delete g_scriptedClasses[i];
	}
	g_scriptedClasses.clear();
}

//
// experimental C-style structs syntax parser
// used for Q3 items support
//
class cFieldDef_c {
	u16 varType; // int, char, itemType_t
	u16 pointerLevel; // number of '*'
	short arraySize; // [2], [4] (0 means no array)
	u16 nameIndex; // field name
public:
	void setArraySize(short newArraySize) {
		arraySize = newArraySize;
	}
	void setName(u32 newNameIndex) {
		nameIndex = newNameIndex;
	}
	void setPointerLevel(u32 newPointerLevel) {
		pointerLevel = newPointerLevel;
	}
	void setVarTypeName(u32 newFieldType) {
		varType = newFieldType;
	}
	bool isArray() const {
		if(arraySize == 0)
			return false;
		return true;
	}
	u32 getType() const {
		return varType;
	}
	u32 getArraySize() const {
		return arraySize;
	}
	u32 getName() const {
		return nameIndex;
	}
};
class cStructDef_c {
	u32 structName;
	arraySTD_c<cFieldDef_c> fields;
public:
	void setName(u32 newStructName) {
		this->structName = newStructName;
	}
	void addField(const cFieldDef_c &newField) {
		fields.push_back(newField);
	}
	u32 getName() const {
		return structName;
	}
	u32 getNumFields() const {
		return fields.size();
	}
	const cFieldDef_c &getField(u32 fieldNum) const {
		return fields[fieldNum];
	}
	bool findFieldWithName(u32 fieldName, u32 &localIndex) const {
		for(u32 i = 0; i < fields.size(); i++) {
			if(fields[i].getName() == fieldName) {
				localIndex = i;
				return true;
			}
		}
		localIndex = 0xffffffff;
		return false;
	}
};
class cFieldValue_c {
	str value;
	arraySTD_c<cFieldValue_c> *values;
public:
	cFieldValue_c() {
		values = 0;
	}
	cFieldValue_c(const cFieldValue_c &other) {
		if(other.values) {
			this->values = new arraySTD_c<cFieldValue_c>;
			*this->values = *other.values;
		} else {
			this->values = 0;
		}
		this->value = other.value;
	}
	~cFieldValue_c() {
		if(values) {
			delete values;
		}
	}
	void setSingleValue(const char *newValStr) {
		value = newValStr;
		if(values) {
			delete values;
			values = 0;
		}
	}
	void initArray(u32 newArraySize) {
		if(values) {
			delete values;
		}
		values = new arraySTD_c<cFieldValue_c>;
		values->resize(newArraySize);
	}
	void setArrayField(u32 fieldNum, const char *str) {
		(*values)[fieldNum].setSingleValue(str);
	}
	const char *getSingleValue() const {
		return value;
	}
	const char *getArrayValue(u32 localIndex) const {
		if(values == 0)
			return 0;
		if(values->size() <= localIndex)
			return 0;
		return (*values)[localIndex].getSingleValue();
	}
	bool isArray() const {
		if(values)
			return true;
		return false;
	}
};
class cStructInstance_c {
	const cStructDef_c *myStructDef;
	arraySTD_c<cFieldValue_c> fieldValues; 
public:
	cStructInstance_c() {
		myStructDef = 0;
	}
	void setStructDef(const cStructDef_c *sd) {
		myStructDef = sd;
		if(sd == 0) {
			g_core->RedWarning("cStructInstance_c::setStructDef: NULL structDef argument passed\n");
		} else {
			fieldValues.resize(sd->getNumFields());
		}
	}
	void setFieldValue(u32 localFieldIndex, const cFieldValue_c &nv) {
		fieldValues[localFieldIndex] = nv;
	}
	cFieldValue_c &getFieldValue(u32 localFieldIndex) {
		return fieldValues[localFieldIndex];
	}
	const cFieldValue_c &getFieldValue(u32 localFieldIndex) const {
		return fieldValues[localFieldIndex];
	}
	const cFieldDef_c &getFieldDef(u32 localFieldIndex) const {
		return myStructDef->getField(localFieldIndex);
	}
	const cFieldValue_c *findFieldValue(u32 fieldName) const {
		u32 localIndex;
		if(myStructDef->findFieldWithName(fieldName, localIndex)) {
			return &fieldValues[localIndex];
		}
		return 0;
	}
	u32 getNumFields() const {
		return fieldValues.size();
	}
};
class cStructInstanceArray_c {
	u32 name;
	arraySTD_c<cStructInstance_c> data;
public:
	cStructInstance_c &allocNextStructInstance() {
		return data.pushBack();
	}
	void setName(u32 newName) {
		this->name = newName;
	}
	u32 getName() const {
		return name;
	}
	u32 size() const {
		return data.size();
	}
	const cStructInstance_c *getStructInstance(u32 localIndex) const {
		return &data[localIndex];
	}
};
struct typedefStruct_s {
	u32 name;
	cStructDef_c *structDef;
};
#include <shared/stringHashTable.h>
class cOutData_c {
	stringRegister_c stringNames;
	arraySTD_c<cStructDef_c *> structDefs;
	arraySTD_c<typedefStruct_s> typedefStructs;
	arraySTD_c<cStructInstanceArray_c*> globalStructArrays;
public:
	cOutData_c() {

	}
	~cOutData_c() {
		for(u32 i = 0; i < structDefs.size(); i++) {
			delete structDefs[i];
			structDefs[i] = 0;
		}
		structDefs.size();
	}
	u32 registerString(const char *str) {
		return stringNames.registerString(str);
	}
	bool findString(const char *str, u32 &outIndex) const {
		return stringNames.findString(str,outIndex);
	}
	const char *getString(u32 index) const {
		return stringNames.getString(index);
	}
	void addStructDef(cStructDef_c *newStructDef) {
		structDefs.push_back(newStructDef);
	}
	void addTypedefStruct(const char *typedefName, cStructDef_c *structPtr) {
		typedefStruct_s ts;
		ts.name = registerString(typedefName);
		ts.structDef = structPtr;
		typedefStructs.push_back(ts);
	}
	const cStructDef_c *findStruct(u32 typeName) const {
		for(u32 i = 0; i < structDefs.size(); i++) {
			const cStructDef_c *s = structDefs[i];
			if(s->getName() == typeName) {
				return s;
			}
		}
		for(u32 i = 0; i < typedefStructs.size(); i++) {
			const typedefStruct_s &ts = typedefStructs[i];
			if(ts.name == typeName) {
				return ts.structDef;
			}
		}
		return 0;
	}
	void addStructInstanceArray(cStructInstanceArray_c *sia) {
		globalStructArrays.push_back(sia);
	}
	const cStructInstanceArray_c *findGlobalStructInstancesArray(const char *name) const {
		u32 nameIndex;
		if(findString(name,nameIndex) == false) {
			return 0;
		}
		for(u32 i = 0; i < globalStructArrays.size(); i++) {
			const cStructInstanceArray_c *a = globalStructArrays[i];
			if(a->getName() == nameIndex) {
				return a;
			}
		}
		return 0;
	}
};
class cStyleStructuresParser_c {
	const char *p;
	cOutData_c *outData;

	bool skipWhiteSpaces();
	const char *getToken(str &out, const char *stopCharSet = 0);
	bool parseTypedef();
	bool parseStructDef(class cStructDef_c **outNewStructPtr = 0);
	bool parseField(cFieldDef_c &newField);
	bool parseVar();
public:
	cStyleStructuresParser_c();
	~cStyleStructuresParser_c();
	bool parseText(const char *rawTextData);

	const cOutData_c *getConstResults() const {
		return outData;
	}
};
cStyleStructuresParser_c::cStyleStructuresParser_c() {
	outData = 0;
	p = 0;
}
cStyleStructuresParser_c::~cStyleStructuresParser_c() {
	if(outData) {
		delete outData;
	}
}
const char *cStyleStructuresParser_c::getToken(str &out, const char *stopCharSet) {
	if(skipWhiteSpaces()) {
		return 0; // EOF hit
	}
	if(*p == '"') {
		bool bHaveToRepeat;
		do {
			p++;
			const char *start = p;
			while(*p != '"') {
				if(*p == 0) {
					break;
				}
				p++;
			}
			str tmp;
			tmp.setFromTo(start,p);
			p++;
			out.append(tmp);
			// merge string that split into multiple lines
			const char *test = p;
			while(G_isWS(*test)) {
				test++;
			}
			if(*test == '"') {
				bHaveToRepeat = true;
				p = test;
			} else {
				bHaveToRepeat = false;
			}
		} while(bHaveToRepeat);
	} else {
		u32 stopCharSetLen;
		if(stopCharSet) {
			stopCharSetLen = strlen(stopCharSet);
		} else {
			stopCharSetLen = 0;
		}
		const char *start = p;
		while(*p) {
			if(G_isWS(*p)) {
				break;
			}
			if(stopCharSet) {
				bool bFound = false;
				for(u32 i = 0; i < stopCharSetLen; i++) {
					if(*p == stopCharSet[i]) {
						bFound = true;
						break;
					}
				}
				if(bFound) {
					break; // stop
				}
			}
			p++;
		}
		out.setFromTo(start,p);
	}
	return out;
}
bool cStyleStructuresParser_c::skipWhiteSpaces() {
	while(G_isWS(*p)) {
		if(*p == 0) {
			return true; // EOF hit
		}
		p++;
	}
	return false; // no error
}
bool cStyleStructuresParser_c::parseVar() {
	cFieldDef_c fieldDef;
	parseField(fieldDef);
	const char *varTypeName = outData->getString(fieldDef.getType());
	if(*p == ';') {
		return false;
	}
	if(*p != '=') {
		g_core->RedWarning("Expected '='\n");
		return true;
	}
	p++; // skip '='
	if(fieldDef.isArray()) {
		// TODO: handle simple types here as well
		skipWhiteSpaces();
		if(*p != '{') {
			return true;	
		}
		const cStructDef_c *structDef = outData->findStruct(fieldDef.getType());
		if(structDef == 0) {
			g_core->RedWarning("cStyleStructuresParser_c::parseVar(): unknown var type %s\n",varTypeName);
			// skip braced block
			int level = 1;
			while(level) {
				if(*p == 0) {
					g_core->RedWarning("Unexpected end of file hit while skipping curly braced block\n");
					break;
				}
				if(*p == '{') {
					level++;
				} else if(*p == '}') {
					level--;
				}
				p++;
			}
			return true; // error
		} else {
			cStructInstanceArray_c *newAr = new cStructInstanceArray_c;
			outData->addStructInstanceArray(newAr);
			newAr->setName(fieldDef.getName());
			p++; // skip '{'
			skipWhiteSpaces();
			while(*p != '}') {
				cStructInstance_c &data = newAr->allocNextStructInstance();
				if(*p != '{') {
					return true;
				}
				p++; // skip '{'
				data.setStructDef(structDef);
				for(u32 i = 0; i < structDef->getNumFields(); i++) {
					if(*p == '}') {
						// allow some fields to be left unitialized
						break;
					}
					const cFieldDef_c &fd = structDef->getField(i);
					cFieldValue_c &fVal = data.getFieldValue(i);
					if(fd.isArray()) {
						skipWhiteSpaces();
						if(*p != '{') {
							return true;
						}
						p++; // skip '{'
						fVal.initArray(fd.getArraySize());
						for(u32 j = 0; j < fd.getArraySize(); j++) {
							str atok;
							getToken(atok,",}");
							fVal.setArrayField(j,atok);
							skipWhiteSpaces();
							if(*p == ',') {
								p++; // skip ','
							}
						}
						skipWhiteSpaces();
						if(*p != '}') {
							return true;
						}
						p++; // skip '}'
					} else {
						str tok;
						getToken(tok,",}");
						fVal.setSingleValue(tok);

						u32 fieldNameIndex = fd.getName();
						const char *fieldName = outData->getString(fieldNameIndex);
						printf("Field \"%s\", value \"%s\"\n",fieldName,tok.c_str());
					}
					skipWhiteSpaces();
					if(*p == ',') {
						p++; // skip ','
					}
				}
				if(*p != '}') {
					return true;
				}
				p++; // skip '}'
				skipWhiteSpaces();
				if(*p == ',') {
					p++; // skip ','
				}
				skipWhiteSpaces();
			}
		}
	}
	if(*p == '}') {
		p++;
		skipWhiteSpaces();
		if(*p == ';') {
			p++;
			skipWhiteSpaces();
		}
	}
	return false; // no error
}
bool cStyleStructuresParser_c::parseField(cFieldDef_c &newField) {
	skipWhiteSpaces();
	str fieldType;
	// don't include '*' (pointer character) in field name!!!
	getToken(fieldType,"*");
	// count '*' and skip whitespaces
	u32 pointerLevel = 0;
	while(1) {
		if(*p == '*') {
			pointerLevel++;
		} else if(G_isWS(*p) == false) {
			break;
		}	
		p++;
	}
	// read fieldName
	str fieldName;
	getToken(fieldName,";[");
	skipWhiteSpaces();
	short arraySize = 0;
	if(*p == '[') {
		p++; // skip '['
		// that's an array
		skipWhiteSpaces();
		if(*p == ']') {
			// autodetect array size
			arraySize = -2;
		} else {
			str arraySizeStr;
			getToken(arraySizeStr,"]");
			arraySize = atoi(arraySizeStr);
		}
		skipWhiteSpaces();
		if(*p != ']') {
			g_core->RedWarning(" ']' missing after array size\n");
		} else {
			p++; // skip ']'
		}
		skipWhiteSpaces();
	}
	if(*p != ';') {
		//g_core->RedWarning(" ';' missing after field\n");
	} else {
		p++;
	}
	skipWhiteSpaces();

	newField.setArraySize(arraySize);
	newField.setName(outData->registerString(fieldName));
	newField.setPointerLevel(pointerLevel);
	newField.setVarTypeName(outData->registerString(fieldType));
	return false; // no error
}
bool cStyleStructuresParser_c::parseStructDef(cStructDef_c **outNewStructPtr) {
	skipWhiteSpaces();
	str structName = "unnamedStructure";
	if(*p != '{') {
		getToken(structName);
		skipWhiteSpaces();
	}
	if(*p != '{') {
		g_core->RedWarning("cStyleStructuresParser_c::parseStructDef: expected '{' to follow struct %s\n",structName);
		return true;
	}
	p++; // skip '{'
	cStructDef_c *newStructDef = new cStructDef_c;
	newStructDef->setName(outData->registerString(structName));

	// parse struct fields
	while(*p != '}') {
		cFieldDef_c newField;
		parseField(newField);
		newStructDef->addField(newField);
	}
	p++; // skip '}'
	outData->addStructDef(newStructDef);
	if(outNewStructPtr) {
		*outNewStructPtr = newStructDef;
	}
	return false;
}
bool cStyleStructuresParser_c::parseTypedef() {
	skipWhiteSpaces();
	cStructDef_c *newStructPtr = 0;
	if(!Q_stricmpn(p,"struct",6) && G_isWS(p[6])) {
		p += 6;
		parseStructDef(&newStructPtr);
	} else {
		g_core->Print("Only typedef struct supported..\n");
	}
	skipWhiteSpaces();
	str typedefName;
	getToken(typedefName,";");
	skipWhiteSpaces();
	if(*p == ';') {
		p++;
	}
	if(newStructPtr) {
		outData->addTypedefStruct(typedefName,newStructPtr);
	}
	return false;
}
bool cStyleStructuresParser_c::parseText(const char *rawTextData) {
	p = rawTextData;
	outData = new cOutData_c;
	while(*p) {
		skipWhiteSpaces();
		if(!Q_stricmpn(p,"typedef",7) && G_isWS(p[7])) {
			p += 7;
			parseTypedef();
		} else if(!Q_stricmpn(p,"struct",6) && G_isWS(p[6])) {
			p += 6;
			parseStructDef();
		} else {
			parseVar();
		}
	}
	return false;
}

#include <shared/cStylePreprocessor.h>
// instead of having Quake3 item classnames, models, and parameters hardcoded,
// we're parsing them every time on game module startup from their original definitinos
void G_LoadQuake3ItemDefs() {
	cStylePreprocessor_c pp;
	pp.addDefine("MISSIONPACK");
	pp.preprocessFile("srcItemDefs/quake3Items.c");
	cStyleStructuresParser_c cp;
	cp.parseText(pp.getResultConstChar());

	const cOutData_c *d = cp.getConstResults();
	const cStructInstanceArray_c *bg_items = d->findGlobalStructInstancesArray("bg_itemlist");
	if(bg_items) {
		u32 world_modelStrIndex;
		bool bFoundWorldModelStr = d->findString("world_model",world_modelStrIndex);

		u32 giTypeStrIndex;
		if(d->findString("giType",giTypeStrIndex)) {
			for(u32 i = 0; i < bg_items->size(); i++) {
				const cStructInstance_c *si = bg_items->getStructInstance(i);
				scriptedClass_c *newClass = new scriptedClass_c;
				const cFieldValue_c *giTypeField = si->findFieldValue(giTypeStrIndex);
				if(giTypeField) {
					const char *giTypeStr = giTypeField->getSingleValue();
					if(!stricmp(giTypeStr,"IT_HEALTH")) {
						newClass->setBaseClass("ModelEntity");//"ItemHealth");
					} else if(!stricmp(giTypeStr,"IT_ARMOR")) {
						newClass->setBaseClass("ModelEntity");//"ItemArmor");
					} else if(!stricmp(giTypeStr,"IT_WEAPON")) {
						//newClass->setBaseClass("ModelEntity");
						newClass->setBaseClass("Q3Weapon");
						//if(bFoundWorldModelStr) {
						//	const cFieldValue_c *worldModelField = si->findFieldValue(world_modelStrIndex);
						//	if(worldModelField) {
						//		str worldModel0 = worldModelField->getArrayValue();
						//		worldModel0.stripExtension();
						//		worldModel0.append("_hand.md3");
						//		newClass->setKeyValue("model_view",worldModel0);
						//	}
						//}
					} else {
						newClass->setBaseClass("ModelEntity");
					}
				} else {
					newClass->setBaseClass("ModelEntity");
				}
				for(u32 j = 0; j < si->getNumFields(); j++) {
					const cFieldDef_c &key = si->getFieldDef(j);
					const cFieldValue_c &val = si->getFieldValue(j);
					u32 keyNameIndex = key.getName();
					const char *keyName = d->getString(keyNameIndex);
					if(val.isArray()) {
						const char *valStr = val.getArrayValue(0);
						newClass->setKeyValue(keyName,valStr);
					} else {
						const char *valStr = val.getSingleValue();
						newClass->setKeyValue(keyName,valStr);
					}
				}
				newClass->setKeyValue("bUseRModelToCreateDynamicCVXShape","1");
				g_scriptedClasses.addObject(newClass);
			}
		}
	}
}
void G_InitScriptedClasses() {
	G_LoadQuake3ItemDefs();
}
