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
// aseLoader.cpp
#include <assert.h>
#include <shared/parser.h>
#include <shared/array.h>
#include <shared/autoCvar.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include <api/coreAPI.h>
#include <api/staticModelCreatorAPI.h>

static aCvar_c ase_verboseLoading("ase_verboseLoading","0");
static aCvar_c ase_printUnknownTokens("ase_printUnknownTokens","0");

// bitmap names are stored with prefixes used by developers
// we might need to convert them to paths relative to game directory
static const char *ase_d3Prefix = "//doom3/base/";
static u32 ase_d3PrefixLen = strlen(ase_d3Prefix);

static const char *ase_preyPrefix = "C:/Prey/base/";
static u32 ase_preyPrefixLen = strlen(ase_preyPrefix);

static const char *ase_prey2Prefix = "D:/Prey/base/";
static u32 ase_prey2PrefixLen = strlen(ase_prey2Prefix);

static const char *ase_q4Prefix = "C:/Ritual/Q4ritual/game/q4base/";
static u32 ase_q4PrefixLen = strlen(ase_q4Prefix);

static const char *ase_q4Prefix2 = "//q4base/";
static u32 ase_q4PrefixLen2 = strlen(ase_q4Prefix2);

struct aseMaterial_s {
	str matName;
	str bitmapName;
	const char *getGameBitMatName() const {
		if(bitmapName.length() == 0) {
			return matName;
		}
		// Prey prefix
		if(!_strnicmp(ase_preyPrefix,bitmapName,ase_preyPrefixLen)) {
			return bitmapName.c_str() + ase_preyPrefixLen;
		}
		// Prey prefix2
		if(!_strnicmp(ase_prey2Prefix,bitmapName,ase_prey2PrefixLen)) {
			return bitmapName.c_str() + ase_prey2PrefixLen;
		}
		// Quake4 prefix
		if(!_strnicmp(ase_q4Prefix,bitmapName,ase_q4PrefixLen)) {
			return bitmapName.c_str() + ase_q4PrefixLen;
		}
		// D3 prefix 1
		if(!_strnicmp(ase_d3Prefix,bitmapName,ase_d3PrefixLen)) {
			return bitmapName.c_str() + ase_d3PrefixLen;
		}
		// Quake4 prefix 2
		if(!_strnicmp(ase_q4Prefix2,bitmapName,ase_q4PrefixLen2)) {
			return bitmapName.c_str() + ase_q4PrefixLen2;
		}
		return bitmapName;
	}
};

struct rTriangle32_s {
	u32 indexes[3];
};

bool MOD_LoadASE(const char *fname, staticModelCreatorAPI_i *out) {
	parser_c p;
	if(p.openFile(fname)) {
		g_core->RedWarning("MOD_LoadASE: cannot open %s\n",fname);
		return true;
	}

	int materialCount = -1;
	int matNum;
	int asciiExportVersion = -1;
	int inGroup = 0;
	str comment;
	str groupName;
	arraySTD_c<u32> surfacesToTexture;
	arraySTD_c<aseMaterial_s> aseMaterials;
	while(p.atEOF() == false) {
		if(p.atWord("*3DSMAX_ASCIIEXPORT")) {
			asciiExportVersion = p.getInteger();
		} else if(p.atWord("*COMMENT")) {
			comment = p.getToken();
		} else if(p.atWord("*SCENE")) {
			p.skipCurlyBracedBlock();
		} else if(p.atWord("*MATERIAL_LIST")) {
			if(p.atWord("{") == false) {
				g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MATERIAL_LIST\", found %s in line %i of ASE file %s\n",
					p.getToken(),p.getCurrentLineNumber(),fname);
				return true; // error
			}
			while(p.atWord("}") == false) {
				if(p.atWord("*MATERIAL_COUNT")) {
					materialCount = p.getInteger();
					aseMaterials.resize(materialCount);
					matNum = -1;
				} else if(p.atWord("*MATERIAL")) {
					int checkIndex = p.getInteger();
					matNum++;
					if(checkIndex != matNum) {
						g_core->RedWarning("MOD_LoadASE: expected material index %i, found %s in line %i of ASE file %s\n",
							checkIndex,p.getToken(),p.getCurrentLineNumber(),fname);
						return true;
					}
					aseMaterial_s &aseMat = aseMaterials[matNum];
					if(p.atWord("{") == false) {
						g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MATERIAL %i\", found %s in line %i of ASE file %s\n",
							checkIndex,p.getToken(),p.getCurrentLineNumber(),fname);
						return true; // error
					}
					while(p.atWord("}") == false) {
						float val3[3];
						if(p.atWord("*MATERIAL_NAME")) {
							aseMat.matName = p.getToken();
						} else if(p.atWord("*MATERIAL_CLASS")) {
							str matClass = p.getToken();
						} else if(p.atWord("*MATERIAL_AMBIENT")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*MATERIAL_DIFFUSE")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*MATERIAL_SPECULAR")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*MATERIAL_SHINE")) {
							float shine = p.getFloat();
						} else if(p.atWord("*MATERIAL_SHINESTRENGTH")) {
							float shineStrength = p.getFloat();
						} else if(p.atWord("*MATERIAL_TRANSPARENCY")) {
							float trans = p.getFloat();
						} else if(p.atWord("*MATERIAL_WIRESIZE")) {
							float wiresize = p.getFloat();
						} else if(p.atWord("*MATERIAL_SHADING")) {
							str shading = p.getToken();
						} else if(p.atWord("*MATERIAL_XP_FALLOFF")) {
							float xpFallOff = p.getFloat();
						} else if(p.atWord("*MATERIAL_SELFILLUM")) {
							float selFillum = p.getFloat();
						} else if(p.atWord("*MATERIAL_FALLOFF")) {
							float fallOff = p.getFloat();
						} else if(p.atWord("*MATERIAL_XP_TYPE")) {
							str xpType = p.getToken();
						} else if(p.atWord("*MAP_DIFFUSE")) {
							if(p.atWord("{") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MAP_DIFFUSE\", found %s in line %i of ASE file %s\n",
									checkIndex,p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
							while(p.atWord("}") == false) {
								if(p.atWord("*MAP_NAME")) {
									str diffuseMapName = p.getToken();
								} else if(p.atWord("*MAP_CLASS")) {
									str mapClassName = p.getToken();
								} else if(p.atWord("*MAP_SUBNO")) {
									int mapSubNo = p.getInteger();
								} else if(p.atWord("*MAP_AMOUNT")) {
									float mapAmount = p.getFloat();
								} else if(p.atWord("*BITMAP")) {
									aseMat.bitmapName = p.getToken();
									aseMat.bitmapName.backSlashesToSlashes();
									aseMat.bitmapName.stripExtension();
									if(ase_verboseLoading.getInt()) {
										g_core->RedWarning("Bitmap of material %i - %s\n",matNum,aseMat.bitmapName.c_str());
									}
								} else if(p.atWord("*MAP_TYPE")) {
									str bitMapType = p.getToken();
								} else if(p.atWord("*UVW_U_OFFSET")) {
									float uOfs = p.getFloat();
								} else if(p.atWord("*UVW_V_OFFSET")) {
									float vOfs = p.getFloat();
								} else if(p.atWord("*UVW_U_TILING")) {
									float uTil = p.getFloat();
								} else if(p.atWord("*UVW_V_TILING")) {
									float vTil = p.getFloat();
								} else if(p.atWord("*UVW_ANGLE")) {
									float uvwAngle = p.getFloat();
								} else if(p.atWord("*UVW_BLUR")) {
									float uvBlur = p.getFloat();
								} else if(p.atWord("*UVW_BLUR_OFFSET")) {
									float uvBlurOffset = p.getFloat();
								} else if(p.atWord("*UVW_NOUSE_AMT")) {
									// "nouse" ? not a "noise" ? why is it spelled that way in Prey's ase file?
									float uvwNouseAMT = p.getFloat();
								} else if(p.atWord("*UVW_NOISE_SIZE")) {
									float uvwNoiseSize = p.getFloat();
								} else if(p.atWord("*UVW_NOISE_LEVEL")) {
									int uvwNoiseLevel = p.getInteger();
								} else if(p.atWord("*UVW_NOISE_PHASE")) {
									float uvwNoisePhase = p.getFloat();
								} else if(p.atWord("*BITMAP_FILTER")) {
									str bitMapFilter = p.getToken();
								} else if(p.atWord("{")) {
									p.skipCurlyBracedBlock(false);
									if(ase_printUnknownTokens.getInt()) {
										g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
									}
								} else {
									const char *unknownToken = p.getToken();
									if(ase_printUnknownTokens.getInt()) {
										g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
									}
								}
							}
						} else if(p.atWord("{")) {
							p.skipCurlyBracedBlock(false);
							if(ase_printUnknownTokens.getInt()) {
								g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
							}
						} else {
							const char *unknownToken = p.getToken();
							if(ase_printUnknownTokens.getInt()) {
								g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
							}
						}
					}
				} else if(p.atWord("{")) {
					p.skipCurlyBracedBlock(false);
					if(ase_printUnknownTokens.getInt()) {
						g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
					}
				} else {
					const char *unknownToken = p.getToken();				
					if(ase_printUnknownTokens.getInt()) {
						g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
					}
				}
			}
		} else if(p.atWord("*GROUP")) {
			if(inGroup) {
				g_core->RedWarning("MOD_LoadASE: WARNING: group inside a group at line %i of file %s\n",p.getCurrentLineNumber(),fname);
			}
			groupName = p.getToken();
			inGroup++;
			if(p.atWord("{") == false) {
				g_core->RedWarning("MOD_LoadASE: expected '{' to follow group %s at line %i of file %s, found %s\n",
					groupName.c_str(),p.getCurrentLineNumber(),fname,p.getToken());
				return true; // error
			}
		} else if(p.atWord("*GEOMOBJECT")) {	
			// NOTE: "*GEOMOBJECT" keyword sometimes appears in "*GROUPS"'s
			if(p.atWord("{") == false) {
				g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*GEOMOBJECT\", found %s in line %i of ASE file %s\n",
					p.getToken(),p.getCurrentLineNumber(),fname);
				return true; // error
			}
			while(p.atWord("}") == false) {
				if(p.atWord("*NODE_NAME")) {
					str nodeName = p.getToken();
				} else if(p.atWord("*NODE_PARENT")) {
					str nodeParent = p.getToken();
				} else if(p.atWord("*NODE_TM")) {
					if(p.atWord("{") == false) {
						g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*NODE_TM\", found %s in line %i of ASE file %s\n",
							p.getToken(),p.getCurrentLineNumber(),fname);
						return true; // error
					}
					float val3[3];
					while(p.atWord("}") == false) {
						if(p.atWord("*NODE_NAME")) {
							str nodeName = p.getToken();
						} else if(p.atWord("*INHERIT_POS")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*INHERIT_ROT")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*INHERIT_SCL")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_ROW0")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_ROW1")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_ROW2")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_ROW3")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_POS")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_ROTAXIS")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_ROTANGLE")) {
							float rotAngle = p.getFloat();
						} else if(p.atWord("*TM_SCALE")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_SCALEAXIS")) {
							p.getFloatMat(val3,3);
						} else if(p.atWord("*TM_SCALEAXISANG")) {
							float scaleAxisAng = p.getFloat();
						} else if(p.atWord("{")) {
							p.skipCurlyBracedBlock(false);
							if(ase_printUnknownTokens.getInt()) {
								g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
							}
						} else {
							const char *unknownToken = p.getToken();
							if(ase_printUnknownTokens.getInt()) {
								g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
							}
						}
					}	
				} else if(p.atWord("*MESH")) {
					int meshTimeValue = -1;
					int meshVertCount = -1;
					int meshTVertCount = -1;
					int meshCVertCount = -1;
					int meshFaceCount = -1;
					int meshCFaceCount = -1;
					int meshTVFaceCount = -1;
					arraySTD_c<vec3_c> vXYZs;
					arraySTD_c<vec3_c> texVerts;
					arraySTD_c<rTriangle32_s> vFaces;
					arraySTD_c<rTriangle32_s> texFaces;
					if(p.atWord("{") == false) {
						g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MESH\", found %s in line %i of ASE file %s\n",
							p.getToken(),p.getCurrentLineNumber(),fname);
						return true; // error
					}
					while(p.atWord("}") == false) {
						if(p.atWord("*TIMEVALUE")) {
							meshTimeValue = p.getInteger();
						} else if(p.atWord("*MESH_NUMVERTEX")) {
							meshVertCount = p.getInteger();
						} else if(p.atWord("*MESH_NUMFACES")) {
							meshFaceCount = p.getInteger();
						} else if(p.atWord("*MESH_NUMCFACES")) {
							meshCFaceCount = p.getInteger();
							// Number of color faces. Should always be identical to *MESH_NUMFACES
						} else if(p.atWord("*MESH_NUMTVFACES")) {
							meshTVFaceCount = p.getInteger();
						} else if(p.atWord("*MESH_NUMTVERTEX")) {
							meshTVertCount = p.getInteger();
						} else if(p.atWord("*MESH_NUMCVERTEX")) {
							meshCVertCount = p.getInteger();
						} else if(p.atWord("*MESH_VERTEX_LIST")) {
							if(p.atWord("{") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MESH_VERTEX_LIST\", found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
							vXYZs.resize(meshVertCount);
							for(u32 j = 0; j < meshVertCount; j++) {
								if(p.atWord("*MESH_VERTEX") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"MESH_VERTEX\" to follow vert %i of \"*MESH_VERTEX_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								u32 checkVertIndex = p.getInteger();
								if(checkVertIndex != j) {
									g_core->RedWarning("MOD_LoadASE: expected vertex index %i to follow \"*MESH_VERTEX_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								vec3_c &iVertXYZ = vXYZs[j];
								p.getFloatMat(iVertXYZ,3);
								//g_core->RedWarning("Vert %i of %i - %f %f %f\n",j,meshVertCount,iVertXYZ.x,iVertXYZ.y,iVertXYZ.z);
							}
							if(p.atWord("}") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '}' after \"*MESH_VERTEX_LIST\" block, found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
						} else if(p.atWord("*MESH_FACE_LIST")) {
							if(p.atWord("{") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MESH_FACE_LIST\", found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
							vFaces.resize(meshFaceCount);
							rTriangle32_s *tri = vFaces.getArray();
							for(u32 j = 0; j < meshFaceCount; j++, tri++) {
								if(p.atWord("*MESH_FACE") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"MESH_FACE\" to follow face %i of \"*MESH_FACE_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								str faceIndexStr = p.getToken();
								faceIndexStr.stripTrailing(":");
								u32 checkFaceIndex = atoi(faceIndexStr);
								if(checkFaceIndex != j) {
									g_core->RedWarning("MOD_LoadASE: expected face index %i to follow \"*MESH_FACE_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
						//	*MESH_FACE    0:    A:   14 B:   13 C:   12 AB:    1 BC:    1 CA:    0	 *MESH_SMOOTHING 1 	*MESH_MTLID 5--
								if(p.atWord("A:") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"A:\" to follow face %i of \"*MESH_FACE_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								tri->indexes[0] = p.getInteger();
								if(p.atWord("B:") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"B:\" to follow face %i of \"*MESH_FACE_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								tri->indexes[1] = p.getInteger();
								if(p.atWord("C:") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"C:\" to follow face %i of \"*MESH_FACE_LIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								tri->indexes[2] = p.getInteger();

								p.skipLine(); // ignore rest of data for now
							}
							if(p.atWord("}") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '}' after \"*MESH_FACE_LIST\" block, found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
						} else if(p.atWord("*MESH_TVERTLIST")) {
							if(p.atWord("{") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MESH_TVERTLIST\", found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
							texVerts.resize(meshTVertCount);
							for(u32 j = 0; j < meshTVertCount; j++) {
								if(p.atWord("*MESH_TVERT") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"MESH_TVERT\" to follow vert %i of \"*MESH_TVERTLIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								u32 checkVertIndex = p.getInteger();
								if(checkVertIndex != j) {
									g_core->RedWarning("MOD_LoadASE: expected vertex index %i to follow \"*MESH_TVERTLIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								// *MESH_TVERT 0	0.0000	0.0000	0.7482
								vec3_c &texVert = texVerts[j];
								p.getFloatMat(texVert,3);
								//g_core->RedWarning("TEXVERT %i of %i - %f %f %f\n",j,meshTVertCount,texVert.x,texVert.y,texVert.z);
							}
							if(p.atWord("}") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '}' after \"*MESH_TVERTLIST\" block, found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}	
						} else if(p.atWord("*MESH_TFACELIST")) {
							if(p.atWord("{") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '{' to follow \"*MESH_TFACELIST\", found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}
							texFaces.resize(meshTVFaceCount);
							rTriangle32_s *tvFace = texFaces.getArray();
							for(u32 j = 0; j < meshTVFaceCount; j++, tvFace++) {
								if(p.atWord("*MESH_TFACE") == false) {
									g_core->RedWarning("MOD_LoadASE: expected \"MESH_TFACE\" to follow vert %i of \"*MESH_TFACELIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
								u32 checkFaceIndex = p.getInteger();
								if(checkFaceIndex != j) {
									g_core->RedWarning("MOD_LoadASE: expected texFace index %i to follow \"*MESH_TFACELIST\", found %s in line %i of ASE file %s\n",
										j,p.getToken(),p.getCurrentLineNumber(),fname);
									return true; // error
								}
			
								tvFace->indexes[0] = p.getInteger();
								tvFace->indexes[1] = p.getInteger();
								tvFace->indexes[2] = p.getInteger();

								//g_core->RedWarning("TFace %i of %i - %i %i %i\n",j,meshTVFaceCount,tvFace->indexes[0],tvFace->indexes[1],tvFace->indexes[2]);
							}
							if(p.atWord("}") == false) {
								g_core->RedWarning("MOD_LoadASE: expected '}' after \"*MESH_TVERTLIST\" block, found %s in line %i of ASE file %s\n",
									p.getToken(),p.getCurrentLineNumber(),fname);
								return true; // error
							}						
						} else if(p.atWord("{")) {
							p.skipCurlyBracedBlock(false);
							if(ase_printUnknownTokens.getInt()) {
								g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
							}
						} else {
							const char *unknownToken = p.getToken();
							if(ase_printUnknownTokens.getInt()) {
								g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
							}
						}
					}	
					if(ase_verboseLoading.getInt()) {
						g_core->Print("Mesh had %i verts, %i faces, %i texVerts, %i colorVerts, %i colorFaces, %i tvFaces\n",
							meshVertCount,meshFaceCount,meshTVertCount,meshCVertCount,meshCFaceCount,meshTVFaceCount);
					}
					// compile mesh
					if(meshVertCount == 0) {

					} else {
						//rSurface_c &newSF = this->surfs.pushBack();
						if(meshTVFaceCount == 0 || meshTVertCount == 0) {
							g_core->RedWarning("MOD_LoadASE: surface has no texCoords\n");
							//newSF.setIndexes32((u32*)vFaces.getArray(),vFaces.size()*3);
							//newSF.setVOrgs(vXYZs.getArray(),vXYZs.size());
						} else {
							u32 newSurfNum = out->getNumSurfs();
							surfacesToTexture.push_back(newSurfNum);
							rTriangle32_s *xyzFace = vFaces.getArray();
							rTriangle32_s *texFace = texFaces.getArray();
							assert(texFaces.size() == vFaces.size());
							for(u32 j = 0; j < texFaces.size(); j++, texFace++, xyzFace++) {
								//vec3_c *triXYZVerts[3];
								//vec3_c *triTexVerts[3];
								//for(u32 k = 0; k < 3; k++) {
								//	triXYZVerts[k] = &vXYZs[xyzFace->indexes[k]];
								//	triTexVerts[k] = &texVerts[tv->indexes[k]];
								//}
								simpleVert_s verts[3];
								for(u32 k = 0; k < 3; k++) {
									verts[k].xyz = vXYZs[xyzFace->indexes[k]];
									verts[k].tc = &texVerts[texFace->indexes[k]].x;
									// negate the Y texture coordinate
									// It fixes texcoords of ASE models from ZeroBarier Q4 mod,
									// (test with clock.ase and the policesuv.ase)
									// I'm not sure if it's needed for all of the ASE models.
									verts[k].tc.y *= -1;
								}
								out->addTriangleToSF(newSurfNum,verts[2],verts[1],verts[0]);
							}
						}
					}
				} else if(p.atWord("*PROP_MOTIONBLUR")) {
					p.getInteger();
				} else if(p.atWord("*PROP_CASTSHADOW")) {
					p.getInteger();
				} else if(p.atWord("*PROP_RECVSHADOW")) {
					p.getInteger();
				} else if(p.atWord("*MATERIAL_REF")) {
					int refMatIndex = p.getInteger();
					if(refMatIndex >= 0 && refMatIndex < aseMaterials.size()) {
						aseMaterial_s &aseMat = aseMaterials[refMatIndex];
						const char *gameMatName = aseMat.getGameBitMatName();
						if(ase_verboseLoading.getInt()) {
							g_core->Print("using aseMaterial_s with matname field \"%s\" and bitmap field \"%s\"\n",aseMat.matName.c_str(),aseMat.bitmapName.c_str());
						}
						//if(MAT_IsMatNameDefinedInMaterialText(aseMat.matName)) 
						if(0) {
							out->setSurfsMaterial(surfacesToTexture.getArray(),surfacesToTexture.size(),aseMat.matName);
						} else {
							out->setSurfsMaterial(surfacesToTexture.getArray(),surfacesToTexture.size(),gameMatName);
						}
						surfacesToTexture.clear();
					} else {
						g_core->RedWarning("MOD_LoadASE: MATERIAL_REF index %i at line %i of file %s is out of range <0,%i)\n",
							refMatIndex, p.getCurrentLineNumber(),fname,aseMaterials.size());
					}
				} else if(p.atWord("{")) {
					p.skipCurlyBracedBlock(false);
					if(ase_printUnknownTokens.getInt()) {
						g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
					}
				} else {
					if(p.atEOF()) {				
						g_core->RedWarning("MOD_LoadASE: unexpected end of file %s\n",fname);
						break;
					}
					const char *unknownToken = p.getToken();
					if(ase_printUnknownTokens.getInt()) {
						g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
					}
				}
			}
		} else if(p.atWord("}")) {
			if(inGroup) {
				inGroup--;
			} else {
				g_core->RedWarning("MOD_LoadASE: skipping unexpected '{' at line %i of file %s\n",p.getCurrentLineNumber(),fname);
			}
		// skip unknown blocks of data and tokens
		} else if(p.atWord("{")) {
			p.skipCurlyBracedBlock(false);
			if(ase_printUnknownTokens.getInt()) {
				g_core->RedWarning("MOD_LoadASE: skipping braced block of data at line %i of file %s\n",p.getCurrentLineNumber(),fname);
			}
		} else {
			const char *unknownToken = p.getToken();
			if(ase_printUnknownTokens.getInt()) {
				g_core->RedWarning("MOD_LoadASE: skipping unknown token %s at line %i of file %s\n",unknownToken,p.getCurrentLineNumber(),fname);
			}
		}
	}
	return false; // no error
}