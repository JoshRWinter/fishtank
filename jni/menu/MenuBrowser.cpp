#include <ctype.h>
#include "../fishtank.h"

const float BLOB_SIZE=1.0f;
const float LARGE_BLOB_SIZE=1.3f;

bool MenuBrowser::exec(State &state, const std::vector<ServerConfig> &servers){
	list = &servers;

	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.frame=0;
	background.count=1;

	direct.init(-6.2f,3.1f,"Connect");
	cancel.init(direct.x+BUTTON_WIDTH+0.3f,3.1f,"Cancel");

	while(state.process()){
		// connect the player
		if(direct.process(state.pointer)){
		}

		// cancel
		if(cancel.process(state.pointer)||state.back){
			state.back=false;
			return true;
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuBrowser::render(const Renderer &renderer)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-3.75f,"Server Browser");

	// list
	for(const ServerConfig &server:*list){
		drawtextcentered(renderer.font.main, 0.0f, -1.0f, server.name.c_str());
	}

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	direct.render(renderer);
	cancel.render(renderer);

	// button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	direct.render_text(renderer);
	cancel.render_text(renderer);
}
