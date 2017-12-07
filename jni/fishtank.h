#ifndef FISHTANK_H
#define FISHTANK_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android_native_app_glue.h>
#include <string>
#include <vector>
#include "glesutil.h"

// #define SHOW_FPS
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
#define RGB_CYAN_R 0.1289f
#define RGB_CYAN_G 0.6045f
#define RGB_CYAN_B 0.60f
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
struct stat;
#include "network.h"
#include "fishnet.h"
#include "object.h"
#include "Match.h"
#include "menu.h"

// gameplay textures
#define TID_BACKDROP_1 0
#define TID_BACKDROP_2 1
#define TID_BACKDROP_3 2
#define TID_BACKDROP_4 3
#define TID_GROUND 4
#define TID_UPPER_GRADIENT 5
#define TID_TANK 6
#define TID_TURRET 7

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
#define UITID_SKULL 12
#define UITID_HIGHLIGHT 13
#define UITID_SERVER 14

// gameplay atlas
#define AID_ARTILLERY 0
#define AID_BEACON_1 1
#define AID_BEACON_2 2
#define AID_DEADFISH 3
#define AID_MINE_CHAIN 4
#define AID_MINE 5
#define AID_PARTICLE_BUBBLE 6
#define AID_PARTICLE_PLATFORM_1 7
#define AID_PARTICLE_PLATFORM_2 8
#define AID_PARTICLE_PLAYER_1 9
#define AID_PARTICLE_PLAYER_2 10
#define AID_PARTICLE_PLAYER_3 11
#define AID_PARTICLE_PLAYER_4 12
#define AID_PLATFORM_1 13
#define AID_PLATFORM_2 14
#define AID_PLATFORM_3 15
#define AID_PLATFORM_DMG 16
#define AID_SHELL 17
#define AID_TANK_BASE 18
#define AID_TANK_DMG 19
#define AID_TURRET 20
#define AID_GRASS1 21

// sound effects
#define SID_SILENCE 0
#define SID_MENU_THEME 1
#define SID_GAMEPLAY_THEME_1 2
#define SID_GAMEPLAY_THEME_2 3
#define SID_PLATFORM_IMPACT 4
#define SID_PARTICLE_PLAYER_IMPACT 5
#define SID_CANNON 6
#define SID_SHELL_PLAYER_IMPACT 7
#define SID_PLATFORM_DESTROY 8
#define SID_MINE_EXPLOSION 9
#define SID_PLAYER_EXPLODE 10
#define SID_MINE_CHAIN_SNAP 11
#define SID_BEACON_FIRE 12
#define SID_BEACON_BOUNCE 13
#define SID_BUBBLES 14
#define SID_ENGINE 15
#define SID_ENGINE_HALT 16
#define SID_CHAT 17

#define FIREPOWER_INCREMENT 0.008f
#define TIMER_CHATPANE 400.0f
#define SOUND_RANGE 2.0f

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

struct stat{
	std::string name;
	int match_victories;
	int victories;
	int deaths;
	bool dead;
	int points;
	int id;
};

struct Renderer{
	Renderer();
	void init(android_app&);
	void term();
	void draw(const Base&,const Atlas* =NULL)const;
	void uidraw(const UIBase&)const;

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
	Atlas atlas; // gameplay atlas
};

struct State{
	State();
	void reset();
	void init(android_app&);
	void term();
	bool core();
	void render()const;
	bool process()const;
	void write_config();
	bool read_config();
	void play_music();
	void process_music();
	static void fill_color(int,float*,float*,float*);

	Renderer renderer;

	float speed; // time delta
	bool running,back,pause_menu;
	bool show_menu; // show the main menu
	bool strike_mode; // next shot will be artillery strike
	std::string name; // the player's name
	std::string connect_to; // connecting to address ...
	Match match; // manages the game
	float firepower,final_firepower,final_strikepower;
	float timer_chatpane; // show the chat when active
	int colorid; // my colorid
	std::vector<stat> scoreboard;

	// configuration
	Config config;

	// gameplay music
	struct{
		int track;
		int id;
		int check_timer;
	}music;

	// menus
	struct{
		MenuMain main;
		MenuConfig config;
		MenuPlay play;
		MenuBrowserConnect browser_connect;
		MenuBrowser browser;
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
		ButtonBasic left,right,up_l,up_r,fire,cycle,aim_left,aim_right,strike;
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
	std::vector<Grass> grass_list;
};

int32_t inputproc(android_app*,AInputEvent*);
void cmdproc(android_app*,int32_t);
void sound_config_fn(const struct sl_entity_position*,const struct sl_entity_position*,float*,float*);

#endif // FISHTANK_H
