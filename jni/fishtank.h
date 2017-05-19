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

#define RGB_RED RGB_RED_R,RGB_RED_G,RGB_RED_B,1.0f
#define RGB_BLUE RGB_BLUE_R,RGB_BLUE_G,RGB_BLUE_B,1.0f
#define RGB_CYAN RGB_CYAN_R,RGB_CYAN_G,RGB_CYAN_B,1.0f
#define RGB_GREEN RGB_GREEN_R,RGB_GREEN_G,RGB_GREEN_B,1.0f
#define RGB_PURPLE RGB_PURPLE_R,RGB_PURPLE_G,RGB_PURPLE_B,1.0f

#define RGB_RED_R 0.66f
#define RGB_RED_G 0.21f
#define RGB_RED_B 0.25f
#define RGB_BLUE_R 0.25f
#define RGB_BLUE_G 0.32f
#define RGB_BLUE_B 0.85f
#define RGB_CYAN_R 0.2289f
#define RGB_CYAN_G 0.5445f
#define RGB_CYAN_B 0.64f
#define RGB_GREEN_R 0.35f
#define RGB_GREEN_G 0.6f
#define RGB_GREEN_B 0.42f
#define RGB_PURPLE_R 0.6f
#define RGB_PURPLE_G 0.275f
#define RGB_PURPLE_B 0.55f

struct Renderer;
struct State;
struct ChatMessage;
struct Config;
#include "network.h"
#include "fishnet.h"
#include "object.h"
#include "Match.h"
#include "menu.h"

// gameplay textures
#define TID_BACKDROP 0
#define TID_TANK 1
#define TID_TURRET 2
#define TID_SHELL 3
#define TID_PLATFORM 4
#define TID_PARTICLE_PLATFORM 5
#define TID_PARTICLE_PLAYER 6
#define TID_PARTICLE_BUBBLE 7
#define TID_DEAD_FISH 8
#define TID_BEACON 9
#define TID_ARTILLERY 10
#define TID_GROUND 11
#define TID_MINE 12
#define TID_MINE_CHAIN 13

// ui textures
#define UITID_FISHTANK 0
#define UITID_BACKGROUND 1
#define UITID_KEYBOARD_BACKGROUND 2
#define UITID_BUTTON 3
#define UITID_BUTTON_SMALL 4
#define UITID_FIREPOWER_INDICATOR 5
#define UITID_COLORBLOB 6
#define UITID_UIBUTTON 7
#define UITID_FBUTTON 8
#define UITID_BACKGROUND_TRANSPARENT 9
#define UITID_FULL_WHITE 10
#define UITID_BUTTON_AS 11

#define FIREPOWER_INCREMENT 0.008f
#define TIMER_CHATPANE 400.0f

struct ChatMessage{
	ChatMessage(const char *name,const char *content):
		from(name),msg(content){}

	std::string from;
	std::string msg;
};
struct ServerMessage:ChatMessage{
	ServerMessage(const char *name,const char *content):
		ChatMessage(name,content),timer(350.0f){}

	float timer;
};

struct Config{
	bool music,sounds,vibrate;
};

struct Renderer{
	Renderer();
	void init(android_app&);
	void term();
	void draw(const Base&)const;
	void uidraw(const Base&)const;

	device dev; // screen resolution
	device screen; // framebuffer resolution

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;

	unsigned vbo,vao,program;
	float player_x,player_y;

	struct{int vector,size,texcoords,rot,projection,rgba;}uniform;
	struct{ftfont *main,*button,*button_small;}font;
	struct{float left,right,bottom,top;}view;

	pack assets; // gameplay textures
	pack uiassets; // ui textures
};

struct State{
	State();
	void reset();
	bool core();
	void render()const;
	bool process()const;
	void write_config();
	bool read_config();
	static void fill_color(int,float*,float*,float*);

	Renderer renderer;

	float speed; // time delta
	bool running,back,pause_menu;
	bool show_menu; // show the main menu
	bool strike_mode; // next shot will be artillery strike
	std::string name; // the player's name
	std::string connect_to; // connecting to address ...
	Match match; // manages the game
	float firepower,final_firepower,final_strokepower;
	float timer_chatpane; // show the chat when active
	int colorid; // my colorid

	// configuration
	Config config;

	// menus
	struct{
		MenuMain main;
		MenuConfig config;
		MenuPlay play;
		MenuConnect connect;
		MenuInput input;
		MenuPause pause;
		MenuChat chat;
		MenuMessage message;
	}menu;

	crosshair pointer[2];
	android_app *app;
	jni_info jni;

	apack aassets; // sound effects
	slesenv *soundengine;

	// ui buttons
	struct{
		ButtonBasic left,right,up_l,up_r,fire,aim_left,aim_right,strike;
	}input;

	// chat message
	std::vector<ChatMessage> chat;
	std::vector<ServerMessage> announcement; // messages from the server

	// objects
	Base backdrop,ground;
	std::vector<Player> player_list;
	std::vector<Platform> platform_list;
	std::vector<Mine> mine_list;
	std::vector<Shell*> shell_list;
	std::vector<Artillery*> arty_list;
	std::vector<ParticleShell*> particle_shell_list;
	std::vector<ParticlePlatform*> particle_platform_list;
	std::vector<ParticlePlayer*> particle_player_list;
	std::vector<ParticleBubble*> particle_bubble_list;
	std::vector<DeadFish*> dead_fish_list;
};

int32_t inputproc(android_app*,AInputEvent*);
void cmdproc(android_app*,int32_t);

#endif // FISHTANK_H
