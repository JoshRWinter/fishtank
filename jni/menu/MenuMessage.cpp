#include "../fishtank.h"

bool MenuMessage::exec(State &state,const char *msg,const char *header){
	this->msg=msg;
	this->header=header;

	button_ok.init(-BUTTON_WIDTH/2.0f,3.1f,"OK");
	background.init_background(state.renderer);

	while(state.process()){
		// ok button
		if(button_ok.process(state.pointer)||state.back){
			state.back=false;
			return true;
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuMessage::render(const Renderer &renderer)const{
	// background
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	renderer.uidraw(background);

	// button
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_ok.render(renderer);

	// button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	button_ok.render_text(renderer);

	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	// main msg
	drawtextcentered(renderer.font.button,0.0f,-2.55f,msg);

	// header text
	if(header!=NULL){
		glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
		drawtextcentered(renderer.font.main,0.0f,-4.0f,header);
	}
}
