#include <time.h>
#include "../fishtank.h"

bool MenuConnect::exec(State &state,const std::string &ip){
	background.init_background(state.renderer);

	socket_tcp &tcp=state.match.get_tcp();
	button_cancel.init(-BUTTON_WIDTH/2.0f,3.1f,"Cancel");
	button_ready.init(-BUTTON_WIDTH/2.0f,3.1f,"Ready!");
	address=ip;
	connection_state=CONN_STATE_TRYING;
	int initial_time=time(NULL);
	if(!tcp.setup(ip,TCP_PORT)){
		tcp.close();
		connection_state=CONN_STATE_DEAD;
	}
	tcp.get_name(connected_address);

	while(state.process()){
		if(connection_state!=CONN_STATE_READY){
			if(button_cancel.process(state.pointer)||state.back){
				state.back=false;
				tcp.close();
				return true;
			}
		}
		else{
			if(button_ready.process(state.pointer)){
				state.pointer[0].active=false;
				state.pointer[0].y=0.0f;
				state.reset();
				return true;
			}
		}

		// process socket
		if(connection_state==CONN_STATE_TRYING){
			if(time(NULL)-initial_time>=CONNECTION_TIMEOUT){
				connection_state=CONN_STATE_TIMEOUT;
				tcp.close();
			}
			bool result=tcp.connect(); // non blocking
			if(result){
				// see if the client is accepted
				uint8_t accepted=0;
				tcp.recv(&accepted,sizeof(uint8_t));
				if(accepted){
					connection_state=CONN_STATE_READY;
					state.match.initialize(state);
				}
				else{
					tcp.close();
					connection_state=CONN_STATE_KICKED;
				}
			}
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuConnect::render(const Renderer &renderer)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// main text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	char connecting_to[MSG_LIMIT+250];
	switch(connection_state){
	case CONN_STATE_TRYING:
		sprintf(connecting_to,"Connecting to\n%s",address.c_str());
		break;
	case CONN_STATE_DEAD:
		sprintf(connecting_to,"Could not resolve\n\"%s\"\nMake sure it is a valid IP address or domain name",address.c_str());
		break;
	case CONN_STATE_READY:
		sprintf(connecting_to,"%s\nConnected and ready",connected_address.c_str());
		break;
	case CONN_STATE_TIMEOUT:
		sprintf(connecting_to,"TIMEOUT:\nCould not connect to\n%s",address.c_str());
		break;
	case CONN_STATE_KICKED:
		sprintf(connecting_to,"ERROR:\nServer at capacity");
		break;
	}
	drawtextcentered(renderer.font.main,0.0f,-3.0f,connecting_to);

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	if(connection_state==CONN_STATE_TRYING||connection_state==CONN_STATE_TIMEOUT)
		button_cancel.render(renderer);
	else
		button_ready.render(renderer);

	// button labels
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	if(connection_state!=CONN_STATE_READY)
		button_cancel.render_text(renderer);
	else
		button_ready.render_text(renderer);
}
