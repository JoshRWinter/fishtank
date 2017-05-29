#include "../fishtank.h"

bool MenuConfig::exec(State &state){
	config=&state.config;
	background.init_background(state.renderer);

	button_music.init(1.0f,-2.25f,"Music");
	button_sounds.init(1.0f,-1.0f,"Sounds");
	button_vibrate.init(1.0f,0.25f,"Vibration");
	button_about.init(-4.25f,3.1f,"Aboot");
	button_back.init(button_about.x+BUTTON_WIDTH+0.3f,3.1f,"Back");

	const char *about_text=
	"~ Fishtank ~\n"
	"Programming and Art by Josh Winter\n"
	"Source for game and server software at\n"
	"https://bitbucket.org/JoshRWinter/fishtank"
	;

	while(state.process()){
		// buttons
		if(button_music.process(state.pointer)){
			state.config.music=!state.config.music;
		}
		if(button_sounds.process(state.pointer)){
			state.config.sounds=!state.config.sounds;
			if(state.config.sounds)
				enablesound(state.soundengine);
			else
				disablesound(state.soundengine);
		}
		if(button_vibrate.process(state.pointer)){
			state.config.vibrate=!state.config.vibrate;
		}
		if(button_about.process(state.pointer)){
			if(!state.menu.message.exec(state,about_text,"Aboot"))
				return false;
		}
		if(button_back.process(state.pointer)||state.back){
			state.back=false;
			return true;
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuConfig::render(const Renderer &renderer)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_music.render(renderer);
	button_sounds.render(renderer);
	button_vibrate.render(renderer);
	button_about.render(renderer);
	button_back.render(renderer);

	// button text
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	button_music.render_text(renderer);
	button_sounds.render_text(renderer);
	button_vibrate.render_text(renderer);
	button_about.render_text(renderer);
	button_back.render_text(renderer);

	// config string
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	std::string music=config->music?"Yes":"No";
	std::string sounds=config->sounds?"Yes":"No";
	std::string vibrate=config->vibrate?"Yes":"No";
	std::string cfg="Music: "+music+"\nSounds: "+sounds+"\nVibration: "+vibrate;
	drawtext(renderer.font.button,-4.75f,-1.5f,cfg.c_str());

	// header text
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-4.0f,"Configuration");
}
