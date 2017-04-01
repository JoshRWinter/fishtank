#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include "glesutil.h"

#define TID_BACKGROUND 0

struct Renderer{
	Renderer();
	void init(android_app&);
	void term();
	void draw(const Base&,bool)const;
	void draw(const Base&)const;

	device dev; // screen resolution
	device screen; // framebuffer resolution

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;

	unsigned vbo,vao,program;

	struct{int vector,size,texcoords,rot,projection,rgba;}uniform;
	struct{ftfont *main;}font;

	pack assets; // gameplay textures
	pack uiassets; // ui textures

	struct{float left,right,bottom,top;}view;
};

struct State{
	State();
	void reset();
	bool core();
	void render()const;
	bool process()const;

	Renderer renderer;

	bool running;

	android_app *app;
	jni_info jni;

	apack aassets; // sound effects
	slesenv *soundengine;

	// objects
	Base background;
};

void process(android_app*);
int32_t inputproc(android_app*,AInputEvent*);
void cmdproc(android_app*,int32_t);
