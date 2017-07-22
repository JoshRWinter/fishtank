#include <zlib.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#include <android/asset_manager.h>

#include "Atlas.h"

Atlas::Atlas(){
	object=0;
	last_error="atlas: no error";
}

bool Atlas::load(AAssetManager *mgr,const char *name){
	AAsset *asset=AAssetManager_open(mgr,name,AASSET_MODE_UNKNOWN);
	if(!asset){
		last_error="atlas: couldn't open file";
		return false;
	}

	// find size of file
	int filesize=AAsset_seek(asset,0,SEEK_END);
	AAsset_seek(asset,0,SEEK_SET);

	// check magic
	unsigned char magic[3]={0,0,0};
	AAsset_read(asset,magic,3);
	bool pass=magic[0]=='J'&&magic[1]=='R'&&magic[2]=='W';
	if(!pass){
		AAsset_close(asset);
		last_error="atlas: didn't pass magic";
		return false;
	}

	// read uncompressed size
	uLongf uncomp_size=0;
	AAsset_read(asset,&uncomp_size,sizeof(uLongf));
	if(uncomp_size==0){
		AAsset_close(asset);
		last_error="atlas: uncomp_size was zero";
		return false;
	}

	// read file into memory
	const int compressed_size=filesize-3-sizeof(unsigned int);
	unsigned char *compressed=new unsigned char[compressed_size];
	if(compressed_size!=AAsset_read(asset,compressed,compressed_size)){
		AAsset_close(asset);
		delete[] compressed;
		return false;
	}
	AAsset_close(asset);

	// allocate array for uncompressed data
	unsigned char *uncompressed=new unsigned char[uncomp_size];

	// uncompress with zlib
	int z_result=uncompress(uncompressed,&uncomp_size,compressed,compressed_size);
	delete[] compressed;
	if(z_result!=Z_OK){
		last_error="atlas: decompression failed";
		delete[] uncompressed;
		return false;
	}

	// make sure uncompressed chunk is at least as big as the initial header
	const int expected_header_length=2+2+2;
	if(uncomp_size<expected_header_length){
		delete[] uncompressed;
		last_error="atlas: failed expected header length check";
		return false;
	}

	// read number of textures present in atlas data
	unsigned short texture_count;
	memcpy(&texture_count,uncompressed,2);

	// read atlas bitmap dimensions
	unsigned short canvas_width;
	unsigned short canvas_height;
	memcpy(&canvas_width,uncompressed+2,2);
	memcpy(&canvas_height,uncompressed+4,2);

	// comprehensive size consistency check
	const int expected_length=2+2+2+(texture_count*sizeof(unsigned short)*4)+(canvas_width*canvas_height*4);
	if(expected_length!=uncomp_size){
		delete[] uncompressed;
		last_error="atlas: corrupted chunk";
		return false;
	}

	// read coordinate data for all textures
	for(int i=0;i<texture_count;++i){
		unsigned short xpos,ypos,width,height;
		memcpy(&xpos,uncompressed+2+2+2+(i*sizeof(unsigned short)*4)+0,2);
		memcpy(&ypos,uncompressed+2+2+2+(i*sizeof(unsigned short)*4)+2,2);
		memcpy(&width,uncompressed+2+2+2+(i*sizeof(unsigned short)*4)+4,2);
		memcpy(&height,uncompressed+2+2+2+(i*sizeof(unsigned short)*4)+6,2);

		// convert to texture coordinates
		tex_coords.push_back((float)xpos/canvas_width);
		tex_coords.push_back((float)(xpos+width)/canvas_width);
		tex_coords.push_back((float)ypos/canvas_height);
		tex_coords.push_back((float)(ypos+height)/canvas_height);
	}

	// texturize for opengl
	unsigned char *bmp_start=uncompressed+2+2+2+(texture_count*sizeof(unsigned short)*4);
	glGenTextures(1,&object);
	glBindTexture(GL_TEXTURE_2D,object);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,canvas_width,canvas_height,0,GL_RGBA,GL_UNSIGNED_BYTE,bmp_start);

	// not needed anymore
	delete[] uncompressed;

	return true;
}

void Atlas::unload(){
	glDeleteTextures(1,&object);
	tex_coords.clear();
}
