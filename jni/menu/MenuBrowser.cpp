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

	direct.init(-BUTTON_WIDTH - 0.15f,3.1f,"Connect To");
	cancel.init(direct.x+BUTTON_WIDTH+0.3f,3.1f,"Cancel");

	anchor = 0.0f;
	trueanchor = 0.0f;
	scrolly = 0.0f;
	touching = false;
	tapzone = false;

	while(state.process()){
		if(state.pointer[0].active){
			if(!touching){
				tapzone = true;
				touching = true;
				anchor = state.pointer[0].y - scrolly;
				trueanchor = state.pointer[0].y;
			}
			else{
				scrolly = state.pointer[0].y - anchor;
				if(fabsf(trueanchor - state.pointer[0].y) > 0.5f){
					tapzone = false;
				}
			}
		}
		else{
			if(touching && tapzone){
				float yoff = -2.0f + scrolly;
				for(int i = 0; i < servers.size(); ++i){
					if(state.pointer[0].y > yoff && state.pointer[0].y < yoff + 1.625f){
						state.connect_to = servers[i].ip;
						if(!state.menu.connect.exec(state, state.connect_to))
							return false;
						if(state.match.connected())
							return true;
					}

					yoff += 1.625 + 0.1f;
				}
			}

			touching = false;
		}

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
	const char *const header = list->size() ? "Server Browser" : "(No Servers)";
	drawtextcentered(renderer.font.main,0.0f,-3.75f,header);

	// list
	float yoff = -2.0f + scrolly;
	for(const ServerConfig &server:*list){
		UIBase selector;
		selector.w = 15.5f;
		selector.h = 1.625f;
		selector.x = -selector.w / 2.0f;
		selector.y = yoff;
		selector.rot = 0.0f;
		selector.frame = 0;
		selector.count = 1;
		selector.texture = UITID_SERVER;

		glBindTexture(GL_TEXTURE_2D, renderer.uiassets.texture[UITID_SERVER].object);
		glUniform4f(renderer.uniform.rgba, 1.0f, 1.0f, 1.0f, 1.0f);
		renderer.uidraw(selector);
		glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
		glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
		const std::string name = "Name: " + server.name;
		drawtext(renderer.font.button, renderer.view.left + 0.4f, yoff, name.c_str());
		const std::string loc = "Location: " + server.location;
		drawtext(renderer.font.button, renderer.view.left + 0.4f, yoff + 0.5f, loc.c_str());
		const std::string ip = "IP Address: " + server.ip;
		drawtext(renderer.font.button, renderer.view.left + 0.4f, yoff + 1.0f, ip.c_str());

		yoff += selector.h + 0.1f;

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
