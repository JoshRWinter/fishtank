#include "../fishtank.h"

bool MenuPause::exec(State &state){
	background.init_background(state.renderer);
	black_background=background;

	button_settings.init(-6.2f,3.1f,"Settings");
	button_quit.init(button_settings.x+BUTTON_WIDTH+0.3f,3.1f,"Quit");
	button_back.init(button_quit.x+BUTTON_WIDTH+0.3f,3.1f,"Resume");

	while(state.process()){
		// buttons
		if(button_settings.process(state.pointer)){
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
	glUniform4f(renderer.uniform.rgba,0.2f,0.5f,0.65f,0.6f);
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

	// header text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-3.0f,"PAUSED");
}
