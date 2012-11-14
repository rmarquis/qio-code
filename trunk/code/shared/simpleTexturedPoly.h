#ifndef __SIMPLETEXTUREDPOLY_H__
#define __SIMPLETEXTUREDPOLY_H__

#include "simpleVert.h"
#include "array.h"

struct simplePoly_s {
	class mtrAPI_i *material;
	arraySTD_c<simpleVert_s> verts;
};

#endif // __SIMPLETEXTUREDPOLY_H__

