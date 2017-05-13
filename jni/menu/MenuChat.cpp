#include "../fishtank.h"

static const float SCROLLPANE_BOTTOM=1.0f;
static const float NEW_LINE_OFFSET=0.7f;

bool MenuChat::exec(State &state){
	background.init_background(state.renderer);

	button_say.init(-4.0f,3.1f,"Say...");
	button_back.init(button_say.x+BUTTON_WIDTH+0.3f,3.1f,"Back");

	scrolltop=SCROLLPANE_BOTTOM-(state.chat.size()*NEW_LINE_OFFSET)-0.5f;
	drag=false;
	offset=0.0f;

	while(state.process()){
		// get new chat messages
		bool new_message=false;
		int beforesize=state.chat.size();
		state.match.recv_data(state);
		if(state.chat.size()!=beforesize)
			new_message=true;

		// buttons
		if(button_say.process(state.pointer)){
			std::string chat;
			if(!state.menu.input.exec(state,"Type your message or don't",&chat))
				return false;

			if(chat.length()!=0){
				state.match.send_chat(chat);
				ChatMessage cm(state.name.c_str(),chat.c_str());
			}
		}
		if(button_back.process(state.pointer)||state.back){
			state.back=false;
			return true;
		}

		// scrolling mechanism
		if(state.chat.size()>0){
			// scroll to the bottom on new chat
			if(new_message&&scrolltop+(state.chat.size()*NEW_LINE_OFFSET)>SCROLLPANE_BOTTOM&&scrolltop+((state.chat.size()-2)*NEW_LINE_OFFSET)<SCROLLPANE_BOTTOM)
				scrolltop=SCROLLPANE_BOTTOM-(state.chat.size()*NEW_LINE_OFFSET)-0.5f;

			if(state.pointer[0].active){
				if(drag){
					// already scrolling
					scrolltop=state.pointer[0].y-offset;
				}
				else{
					// set the anchor
					if(state.pointer[0].y<SCROLLPANE_BOTTOM){
						drag=true;
						offset=state.pointer[0].y-scrolltop;
					}
				}
			}
			else
				drag=false;

			// check if within scrolling limits
			const float scrollbottom=scrolltop+(state.chat.size()*NEW_LINE_OFFSET);
			if(scrolltop+NEW_LINE_OFFSET+0.1f>SCROLLPANE_BOTTOM)
				scrolltop=SCROLLPANE_BOTTOM-NEW_LINE_OFFSET-0.1f;
			if(scrollbottom-NEW_LINE_OFFSET<state.renderer.view.top)
				scrolltop=state.renderer.view.top-(state.chat.size()*NEW_LINE_OFFSET)+NEW_LINE_OFFSET;
		}

		render(state.renderer,state.chat);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuChat::render(const Renderer &renderer,const std::vector<ChatMessage> &chat_list)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// messages
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	float line=scrolltop;
	const float LEFT=-6.0f;
	for(const ChatMessage &cm:chat_list){
		if(line+0.6f>SCROLLPANE_BOTTOM)
			break;
		std::string m=cm.from+": "+cm.msg;
		drawtext(renderer.font.main,LEFT,line,m.c_str());
		line+=NEW_LINE_OFFSET;
	}

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_say.render(renderer);
	button_back.render(renderer);

	// button text
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	button_say.render_text(renderer);
	button_back.render_text(renderer);
}
