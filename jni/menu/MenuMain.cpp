#include "../fishtank.h"

bool MenuMain::exec(State &state){
	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.count=1;
	background.frame=0;

	button_config.init(-6.25f,3.1,"Settings");
	button_play.init(button_config.x+BUTTON_WIDTH+0.3f,3.1f,"Play");
	button_quit.init(button_play.x+BUTTON_WIDTH+0.3f,3.1f,"Quit");

	while(state.process()){
		if(button_config.process(state.pointer)){
			if(!state.menu.config.exec(state))
				return false;
		}
		if(button_play.process(state.pointer)){
			if(!state.menu.play.exec(state))
				return false;
			if(state.match.connected())
				return true;
		}
		if(button_quit.process(state.pointer)||state.back){
			state.back=false;
			ANativeActivity_finish(state.app->activity);
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuMain::render(const Renderer &renderer)const{
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	// background
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_FISHTANK].object);
	renderer.uidraw(background);

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_config.render(renderer);
	button_play.render(renderer);
	button_quit.render(renderer);

	// button label
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	button_config.render_text(renderer);
	button_play.render_text(renderer);
	button_quit.render_text(renderer);
}
