#ifndef FISHTANK_H
#define FISHTANK_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include <string>
#include <vector>
#include "glesutil.h"

#define SHOW_FPS
#define DATAPATH "/data/data/joshwinter.fishtank/files"

struct Renderer;
struct State;
#include "network.h"
#include "fishnet.h"
#include "object.h"
#include "Match.h"
#include "menu.h"

// gameplay textures
#define TID_BACKGROUND 0
#define TID_TANK 1

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
	void write_config();
	bool read_config();

	Renderer renderer;

	bool running;
	bool show_menu; // show the main menu
	std::string name; // the player's name
	std::string connect_to; // connecting to address ...
	Match match;

	struct{
		MenuMain main;
		MenuPlay play;
		MenuConnect connect;
		MenuInput input;
	}menu;

	crosshair pointer[2];
	android_app *app;
	jni_info jni;

	apack aassets; // sound effects
	slesenv *soundengine;

	// ui buttons
	struct{
		ButtonBasic left,right,down,up,fire,aim_left,aim_right;
	}input;

	// objects
	Base background;
	std::vector<Player> player_list;
};

int32_t inputproc(android_app*,AInputEvent*);
void cmdproc(android_app*,int32_t);

#endif // FISHTANK_H
