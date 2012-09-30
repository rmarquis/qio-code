#ifndef __R_API_H__
#define __R_API_H__

class rApi_i {
public:
	virtual void setupViewer(const class vec3_c &camPos, const vec3_c &camAngles) = 0;

	//virtual enum cullResult_e cull(const class bounds_c &b) const = 0;
	//virtual enum cullResult_e cullLocal(const class bounds_c &b) const = 0;
	virtual void drawScene() = 0;

	virtual void loadWorldMap(const char *mapName) = 0;

};


#endif // __R_API_H__
