#ifndef FISHTANK_H
#define FISHTANK_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include "glesutil.h"

struct Renderer;
struct State;
#include "object.h"
#include "menu.h"

#define SHOW_FPS

// gameplay textures
#define TID_BACKGROUND 0

// ui textures
#define UITID_BACKGROUND 0
#define UITID_BUTTON 1
#define UITID_BUTTON_SMALL 2

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
	struct{ftfont *main,*button,*button_small;}font;

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
	bool show_menu; // show the main menu
	std::string name; // the player's name

	struct{
		MenuMain main;
		MenuPlay play;
		MenuInput input;
	}menu;

	crosshair pointer[2];
	android_app *app;
	jni_info jni;

	apack aassets; // sound effects
	slesenv *soundengine;

	// objects
	Base background;
};

int32_t inputproc(android_app*,AInputEvent*);
void cmdproc(android_app*,int32_t);

#endif // FISHTANK_H
