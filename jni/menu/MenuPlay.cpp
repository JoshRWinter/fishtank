#include "../fishtank.h"

bool MenuPlay::exec(State &state){
	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.frame=0;
	background.count=1;

	button_name.init(-5.0f,1.5f,"Change\nName");
	button_connect.init(-0.0f,1.5f,"Connect");
	button_back.init(4.0f,1.5f,"Back");

	name=&state.name;

	while(state.process()){
		// change the player's name
		if(button_name.process(state.pointer)){
			std::string tmp;
			if(!state.menu.input.exec(state,"New Name?",&tmp))
				return false;
			if(tmp.length()>0)
				state.name=tmp;
		}

		// connect the player
		if(button_connect.process(state.pointer)){
			std::string ip_address="kosh.nku.edu";
			if(!state.menu.input.exec(state,"Enter an IPv4, IPv6 ip address,\nor a DNS resolvable domain name",&ip_address))
				return false;
			if(ip_address.length()>0){
				if(!state.menu.connect.exec(state,ip_address))
					return false;
				if(state.match.connected())
					return true;
			}
		}

		// cancel
		if(button_back.process(state.pointer)){
			return true;
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuPlay::render(const Renderer &renderer)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.draw(background);

	// text
	glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	char msg[MSG_LIMIT+30];
	sprintf(msg,"Your name: %s",name->c_str());
	drawtext(renderer.font.main,-2.0f,0.0f,msg);

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_name.render(renderer);
	button_connect.render(renderer);
	button_back.render(renderer);

	// button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	button_name.render_text(renderer);
	button_connect.render_text(renderer);
	button_back.render_text(renderer);
}
