#include "../fishtank.h"

bool MenuMain::exec(State &state){
	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.count=1;
	background.frame=0;

	button_play.init(-3.0f,1.0f,"play");
	button_quit.init(3.0,1.0f,"quit");

	while(state.process()){
		if(button_play.process(state.pointer)){
			if(!state.menu.play.exec(state))
				return false;
			if(state.match.connected())
				return true;
		}
		if(button_quit.process(state.pointer)){
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
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.draw(background);

	// title text
	glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-2.0f,"FISH TANK!");

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_play.render(renderer);
	button_quit.render(renderer);

	// button label
	glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	button_play.render_text(renderer);
	button_quit.render_text(renderer);
}
