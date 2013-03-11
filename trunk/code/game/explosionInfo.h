#ifndef __EXPLOSIONINFO_H__
#define __EXPLOSIONINFO_H__

#include <shared/str.h>

struct explosionInfo_s {
	float radius; // this is the radius of physical explosion effect
	float force;
	// clientside explosion effect def
	float spriteRadius; // this is the radius of explosion sprite
	str materialName;
	str explosionMark;
	float markRadius;

	explosionInfo_s() {
		radius = 0.f;
		force = 0.f;
		spriteRadius = 0.f;
		markRadius = 0.f;
	}
};

#endif // __EXPLOSIONINFO_H__