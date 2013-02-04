#ifndef __RFSURFSFLAGSARRAY_H__
#define __RFSURFSFLAGSARRAY_H__

#include <shared/array.h>
#include <shared/flags32.h>

class rfSurfsFlagsArray_t : public arraySTD_c<flags32_c> {
public:
	const flags32_c &getFlags(u32 idx) const {
		if(this->size() <= idx) {
			return flags32_c();
		}
		return (*this)[idx];
	}
};


#endif // __RFSURFSFLAGSARRAY_H__
