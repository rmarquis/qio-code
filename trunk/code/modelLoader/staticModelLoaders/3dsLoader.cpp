/*
============================================================================
Copyright (C) 2016 V.

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
// 3dsLoader.cpp - .3ds file format loader. Gothic models file format.
#include <shared/array.h>
#include <shared/str.h>
#include <shared/readStream.h>
#include <shared/hashTableTemplate.h>
#include <math/aabb.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include <api/coreAPI.h>
#include <api/staticModelCreatorAPI.h>
#include <fileFormats/3ds_file_format.h>

vec3_c MOD_Read3DSColor(class readStream_c &s, struct g3dsChunk_s &parentChunk) {
	vec3_c ret;
	u32 end = s.getPos() + parentChunk.len-6;
	while(end > s.getPos()) {
		g3dsChunk_s c;
		s.readData(&c,sizeof(c));
		if(c.ident == 0x0010) {
			ret.x = s.readFloat();
			ret.y = s.readFloat();
			ret.z = s.readFloat();
		} else if(c.ident == 0x0011) {
			ret.x = float(s.readByte())/255.f;
			ret.y = float(s.readByte())/255.f;
			ret.z = float(s.readByte())/255.f;
		} else {
			g_core->Print("skelModel_c::MOD_Read3DSColor: Unknown chunk id - %i\n",c.ident);
		}

	}
	return ret;
}
float MOD_Read3DSPercent(class readStream_c &s, struct g3dsChunk_s &parentChunk) {
	float ret;
	u32 end = s.getPos() + parentChunk.len-6;
	while(end > s.getPos()) {
		g3dsChunk_s c;
		s.readData(&c,sizeof(c));
		if(c.ident == 0x0030) {
			ret = s.readU16();
		} else if(c.ident == 0x0031) {
			ret = s.readFloat();
		} else {
			g_core->Print("skelModel_c::MOD_Read3DSPercent: Unknown chunk id - %i\n",c.ident);
		}

	}
	return ret;
}
struct tmp3DSMaterial_s {
	str name;
	str diffuseName;
	str qioMaterial;

	tmp3DSMaterial_s *hashNext;

	const char *getName() const {
		return name;
	}
	tmp3DSMaterial_s *getHashNext() {
		return hashNext;
	}
	void setHashNext(tmp3DSMaterial_s*n) {
		hashNext = n;
	}
};
bool MOD_Load3DS(const char *fname, class staticModelCreatorAPI_i *out) {
	readStream_c s;
	if(s.loadFromFile(fname)) {
		g_core->RedWarning("MOD_Load3DS: cannot open %s\n",fname);
		return true;
	}	
	const g3dsChunk_s *h = (const g3dsChunk_s *)s.getDataPtr();
	if(h->ident != IDENT_3DS) {
		g_core->RedWarning("MOD_Load3DS: %s has bad ident %i, should be %i\n",fname,h->ident,IDENT_3DS);
		return true;
	}
	aabb allPointsBounds;
	str dir = fname;
	dir.toDir();
	hashTableTemplateExt_c<tmp3DSMaterial_s> readMats;
	s.skipBytes(sizeof(g3dsChunk_s));
	g3dsChunk_s c;
	int version;
	float oneUnit = 1.f;
	u32 headerEnd = s.getPos() + h->len - 6;
	while(s.getPos() < headerEnd) {
		s.readData(&c,sizeof(c));
		if(c.ident == 0x0002) {
			version = s.readU32();
			g_core->Print("MOD_Load3DS: version %i\n",version);
		} else if(c.ident == 0x3D3D) {
			u32 meshVersion;
			u32 editorChunkEnd = s.getPos() + c.len - 6;
			while(editorChunkEnd > s.getPos()) {
				g3dsChunk_s ec;
				s.readData(&ec,sizeof(ec));
				u32 ecChunkEnd = s.getPos() + ec.len - 6;
				if(ec.ident == 0x3D3E) {
					meshVersion = s.readU32();
					//g_core->Print("mesh version chunk - v %i\n",meshVersion);
				} else if(ec.ident == 0xAFFF) {
					//g_core->Print("material block chunk - %i bytes at %i\n",ec.len,s.getPos());
					str lastMatName;
					str lastDiffMapName;
					vec3_c lastDiffColor;
					bool bHasColor = false;
					u32 materialBlockEnd = s.getPos() + ec.len - 6;
					while(materialBlockEnd > s.getPos()) {
						g3dsChunk_s mc;
						s.readData(&mc,sizeof(mc));	
						u32 mcEnd = s.getPos() + mc.len - 6;
						if(mc.ident == 0xA000) {
							const char *matName = s.readByteString();
							g_core->Print("Mat name %s\n",matName);
							lastMatName = matName;
						} else if(mc.ident == 0xA020) {
							//g_core->Print("diffuse color\n");
							lastDiffColor = MOD_Read3DSColor(s,mc);
							bHasColor = true;
						} else if(mc.ident == 0xA200) {
							//g_core->Print("diffuse map\n");
							u32 diffuseMapEnd = s.getPos() + mc.len - 6;
							while(diffuseMapEnd > s.getPos()) {
								g3dsChunk_s dmc;
								s.readData(&dmc,sizeof(dmc));	
								u32 dmcEnd = s.getPos() + dmc.len - 6;
								if(dmc.ident == 0xA300) {
									const char *diffMapName = s.readByteString();
									g_core->Print("diffMapName %s\n",diffMapName);
									lastDiffMapName = diffMapName;
								} else {
									g_core->Print("Unknown diffuse map chunk %i\n",dmc.ident);
								}
								if(s.getPos() != dmcEnd) {
									s.setPos(dmcEnd);
								}
							}
						} else if(mc.ident == 0xA010) {
							//g_core->Print("ambColor map\n");
							MOD_Read3DSColor(s,mc);
						} else if(mc.ident == 0xA030) {
							//g_core->Print("specColor\n");
							MOD_Read3DSColor(s,mc);
						} else if(mc.ident == 0xA040) {
							//g_core->Print("shininessPercent\n");
							MOD_Read3DSPercent(s,mc);
						} else if(mc.ident == 0xA041) {
						//	g_core->Print("shininessStrengthPercent\n");
							MOD_Read3DSPercent(s,mc);
						} else if(mc.ident == 0xA050) {
							//g_core->Print("transparencyPercent\n");
							MOD_Read3DSPercent(s,mc);
						} else if(mc.ident == 0xA052) {
							///g_core->Print("transparencyFalloffPercent\n");
							MOD_Read3DSPercent(s,mc);
						} else if(mc.ident == 0xA053) {
							//g_core->Print("reflectionBlurPercent\n");
							MOD_Read3DSPercent(s,mc);
						} else {
							g_core->Print("unknown chunk in material block - %i\n", mc.ident);
						}
						if(s.getPos() != mcEnd) {
							s.setPos(mcEnd);
						}
					}
					if(lastMatName.length() && lastDiffMapName.length()) {
						g_core->Print("3ds mat pair: %s, %s\n",lastMatName.c_str(),lastDiffMapName.c_str());
						tmp3DSMaterial_s *n = new tmp3DSMaterial_s;
						n->name = lastMatName;
						n->diffuseName = lastDiffMapName;
						n->qioMaterial = dir;
						n->qioMaterial.append(n->diffuseName);
						readMats.addObject(n);
					} else if(bHasColor && 0) {
						g_core->Print("3ds mat pair: %s, color %f %f %f\n",lastMatName.c_str(),lastDiffColor.x,lastDiffColor.y,lastDiffColor.z);
						tmp3DSMaterial_s *n = new tmp3DSMaterial_s;
						n->name = lastMatName;
						//n->diffuseName = g_ms->registerColorMat(lastDiffColor)->getName();
						readMats.addObject(n);
					} else {
						g_core->Print("Incomplete 3ds material %s..\n",lastMatName.c_str());
					}
				} else if(ec.ident == 0x0100) {
					oneUnit = s.readFloat();
					//g_core->Print("one unit chunk - %f\n",oneUnit);
				} else if(ec.ident == 0x4000) {
					const char *objFname = s.readByteString();
					//g_core->Print("boject blockchunk - %s\n",objFname);
					u32 bObjectEnd = s.getPos() + ec.len - 6;
					while(s.getPos() < bObjectEnd) {
						g3dsChunk_s boc;
						s.readData(&boc,sizeof(boc));	
						if(boc.ident == 0x4010) { // objecthidden
						//	g_core->Print("Object hidden\n");
						} else if(boc.ident == 0x4100) { // mesh
						//	g_core->Print("Reading mesh!\n");
							arraySTD_c<u32> indices;
							arraySTD_c<vec3_c> xyzs;
							arraySTD_c<vec2_c> tcs;
						//	skelSurface_c *lastSF = 0;
							//sf.setMaterial(lastDiffMapName);
							u32 bMeshEnd = s.getPos() + boc.len - 6;
							while(s.getPos() < bMeshEnd) {
								g3dsChunk_s meshc;
								const float *xyzAt = 0;
								const float *tcAt = 0;
								s.readData(&meshc,sizeof(meshc));
								u32 meshChunkEnd = s.getPos() + meshc.len - 6;
								if(meshc.ident == 0x4110) {
									u16 numVerts = s.readU16();
									xyzAt = s.getDataPtrAsFloat();
								//	g_core->Print("NumVerts: %i\n",numVerts);
									xyzs.resize(numVerts);
									for(u32 i = 0; i < numVerts; i++) {
										vec3_c xyz;
										xyz.x = s.readFloat();
										xyz.y = s.readFloat();
										xyz.z = s.readFloat();
										xyzs[i] = xyz;
										allPointsBounds.addPoint(xyz);
										//g_core->Print("Vert %i of %i - %f %f %f\n",i,numVerts,xyz.x,xyz.y,xyz.z);
									}
								} else if(meshc.ident == 0x4140) {
									u16 numTexCoords = s.readU16();
									tcAt = s.getDataPtrAsFloat();
									tcs.resize(numTexCoords);
								//	g_core->Print("numTexCoords: %i\n",numTexCoords);
									if(xyzs.size()) {
										if(xyzs.size() != numTexCoords) {
											g_core->Print("MOD_Load3DS: Texcoords count: %i, vertex count: %i\n",numTexCoords,xyzs.size());
										//	return true; // error
										}
									}
									for(u32 i = 0; i < numTexCoords; i++) {
										vec2_c tc;
										tc.x = s.readFloat();
										tc.y = s.readFloat();
										//g_core->Print("TC %i of %i - %f %f\n",i,numTexCoords,tc.x,tc.y);
										tcs[i] = tc;
									}
								} else if(meshc.ident == 0x4120) {
									u32 triDataEnd = s.getPos() + meshc.len - 6;

									u16 numFaces = s.readU16();
									indices.resize(numFaces*3);
									//g_core->Print("NumFAces: %i\n",numFaces);
									for(u32 i = 0; i < numFaces; i++) {
										u16 v0 = s.readU16();
										u16 v1 = s.readU16();
										u16 v2 = s.readU16();

										indices[i*3+0] = v0;
										indices[i*3+1] = v1;
										indices[i*3+2] = v2;

										u16 flags = s.readU16();
									//	g_core->Print("Face %i of %i - %i %i %i flags %i\n",i,numFaces,v0,v1,v2,flags);
									}

									while(s.getPos() < triDataEnd) {
										g3dsChunk_s tric;
										s.readData(&tric,sizeof(tric));	
										u32 tricEnd = s.getPos() + tric.len - 6;
										if(tric.ident == 0x4130) {
											const char *mtlName = s.readByteString();
											// HACK - todo - per tri materials
											tmp3DSMaterial_s *f = readMats.getEntry(mtlName);
											u16 numEntries = s.readU16();
											
											str qioMatName;
											if(f) {
												qioMatName = f->qioMaterial;
											} else {
												qioMatName = "noMaterial";
											}
											if(tcs.size() == 0) {
												g_core->RedWarning("3DS surface without texcoords - numTris %i, mtlName %s\n",numEntries,mtlName);
											}
											//g_core->Print("mtllist - numEtnries %i, mtlName %s\n",numEntries,mtlName);
											for(u32 i = 0; i < numEntries; i++) {
												u16 faceIndex = s.readU16();
												u32 v0 = indices[faceIndex*3+0];
												u32 v1 = indices[faceIndex*3+1];
												u32 v2 = indices[faceIndex*3+2];
												simpleVert_s sv0;
												sv0.xyz = xyzs[v0];
												simpleVert_s sv1;
												sv1.xyz = xyzs[v1];
												simpleVert_s sv2;
												sv2.xyz = xyzs[v2];
												if(tcs.size()) {
													sv2.tc = tcs[v2];
													sv0.tc = tcs[v0];
													sv1.tc = tcs[v1];
												} else {
													sv2.tc = vec2_c(0,0);
													sv0.tc = vec2_c(0,0);
													sv1.tc = vec2_c(0,0);
												}
												out->addTriangle(qioMatName,sv2,sv1,sv0);
											//	g_core->Print("mtlist faceIndex - %i\n",faceIndex);
											}
										} else if(tric.ident == 0x4150) {
											//g_core->Print("smoothsn\n");
											for(u32 i = 0; i < numFaces; i++) {
												u32 sm = s.readU32();
												//g_core->Print("smooth for face %i - %i\n",i,sm);
											}
										//	g_core->Print("Smooth groups read\n");
										} else {
									//		g_core->Print("unkown fchunk id %i in tris block\n",tric.ident);
										}
										if(s.getPos() != tricEnd) {
											s.setPos(tricEnd);
										}
									}
								} else {
									g_core->Print("Unknown mesh chunktype %i\n",meshc.ident);
								}
								if(s.getPos() != meshChunkEnd) {
									s.setPos(meshChunkEnd);
								}
							}
						}
					}
				} else {
					g_core->Print("Unknown editor chunktype %i\n",ec.ident);
				}
				if(s.getPos() != ecChunkEnd) {
					s.setPos(ecChunkEnd);
				}
			}
		}
	}
	vec3_c sizes = allPointsBounds.getSizes();
	g_core->Print("3DS model %s sizes %f %f %f\n",fname,sizes.x,sizes.y,sizes.z);
	for(u32 i = 0; i < readMats.size(); i++) {
		delete readMats[i];
	}
	return false;
}
