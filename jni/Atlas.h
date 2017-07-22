#ifndef ATLAS_H
#define ATLAS_H

#include <vector>

class Atlas{
public:
	Atlas();
	bool load(AAssetManager*,const char*);
	void unload();
	unsigned texture()const{return object;}
	// never store or save result
	// lifetime of return is lifetime of Atlas object
	const float *coords(int i)const{return &tex_coords[i*4];}
	const char *error()const{return last_error;}

private:
	std::vector<float> tex_coords;
	unsigned object; // texture object;
	const char *last_error;
};

#endif // ATLAS_H
