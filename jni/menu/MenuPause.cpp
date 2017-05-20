#include "../fishtank.h"

bool MenuPause::exec(State &state){
	background.init_background(state.renderer);
	black_background=background;

	button_settings.init(-6.2f,3.1f,"Settings");
	button_quit.init(button_settings.x+BUTTON_WIDTH+0.3f,3.1f,"Quit");
	button_back.init(button_quit.x+BUTTON_WIDTH+0.3f,3.1f,"Resume");

	// get the scoreboard from the server
	state.scoreboard.clear();
	state.match.request_scoreboard();
	scoreboard=&state.scoreboard;

	while(state.process()){
		if(!state.match.connected())
			return true;

		// buttons
		if(button_settings.process(state.pointer)){
			if(!state.menu.config.exec(state))
				return false;
		}
		if(button_quit.process(state.pointer)){
			state.match.quit();
			state.show_menu=true;
			return true;
		}
		if(button_back.process(state.pointer)||state.back){
			state.back=false;
			return true;
		}

		state.core();
		state.render();
		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuPause::render(const Renderer &renderer)const{
	// black background
	glUniform4f(renderer.uniform.rgba,0.2f,0.5f,0.65f,0.8f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_FULL_WHITE].object);
	renderer.uidraw(black_background);

	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND_TRANSPARENT].object);
	renderer.uidraw(background);

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_settings.render(renderer);
	button_quit.render(renderer);
	button_back.render(renderer);

	// button text
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	button_settings.render_text(renderer);
	button_quit.render_text(renderer);
	button_back.render_text(renderer);

	// stats
	float yoffset=-1.5f;
	const float NAME_OFFSET=-7.0f;
	const float MV_OFFSET=-1.0f;
	const float OOO_OFFSET=2.5f;
	const float D_OFFSET=6.0f;
	// stats header
	glUniform4f(renderer.uniform.rgba,0.8f,0.8f,0.8f,1.0f);
	drawtext(renderer.font.button,NAME_OFFSET,yoffset-0.8f,"Name");
	drawtextcentered(renderer.font.button,MV_OFFSET+0.1f,yoffset-0.8f,"Victories");
	drawtextcentered(renderer.font.button,OOO_OFFSET+0.1f,yoffset-0.8f,"Kills");
	drawtextcentered(renderer.font.button,D_OFFSET+0.1f,yoffset-0.8f,"Deaths");
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	for(const stat &s:*scoreboard){
		// convert to string
		char str_mv[16];
		char str_ooo[16];
		char str_d[16];
		sprintf(str_mv,"%d",s.match_victories);
		sprintf(str_ooo,"%d",s.victories);
		sprintf(str_d,"%d",s.deaths);

		drawtext(renderer.font.button,NAME_OFFSET,yoffset,s.name.c_str());
		drawtext(renderer.font.button,MV_OFFSET,yoffset,str_mv);
		drawtext(renderer.font.button,OOO_OFFSET,yoffset,str_ooo);
		drawtext(renderer.font.button,D_OFFSET,yoffset,str_d);

		yoffset+=0.6f;
	}

	// header text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-3.75f,"PAUSED");
}
