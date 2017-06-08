#include <ctype.h>
#include "../fishtank.h"

bool MenuInput::exec(State &state,const char *header_text,std::string *menu_text){
	text=menu_text;
	header=header_text;
	const char *letters[]={"Q","W","E","R","T","Y","U","I","O","P",
						"A","S","D","F","G","H","J","K","L",":",
						"Z","X","C","V","B","N","M","."};
	const char *numerals[]={"1","2","3","4","5","6","7","8","9","0"};
	space.init(-BUTTON_SMALL_SIZE/2.0f,3.3f,"Sp.");
	backspace.init(6.1f,-0.6f,"BS");
	clear.init(4.9f,2.1f,"CE");
	enter.init(6.0f,3.0f,"OK");
	cancel.init(-7.0f,3.0f,"nvm");

	const float START_XOFFSET=-7.2f;
	const float START_YOFFSET=-1.25f;
	float xoffset=START_XOFFSET;
	float yoffset=START_YOFFSET;

	// backdrop
	background.init_background(state.renderer);

	// set numerals
	yoffset=START_YOFFSET;
	xoffset=START_XOFFSET;
	for(int i=0;i<10;++i){
		numeric[i].init(xoffset,yoffset,numerals[i]);
		xoffset+=BUTTON_SMALL_SIZE+0.3f;
	}
	yoffset+=BUTTON_SMALL_SIZE+0.15f;
	xoffset=START_XOFFSET;

	// set alphas and : and .
	for(int i=0;i<28;++i){
		alpha[i].init(xoffset,yoffset,letters[i]);
		if(letters[i][0]=='P'){
			xoffset=START_XOFFSET+0.65f;
			yoffset+=BUTTON_SMALL_SIZE+0.025f;
		}
		else if(letters[i][0]==':'){
			xoffset=START_XOFFSET+1.35f;
			yoffset+=BUTTON_SMALL_SIZE+0.025f;
		}
		else
			xoffset+=BUTTON_SMALL_SIZE+0.3f;
	}

	while(state.process()){
		// process buttons
		const float BUTTON_TOLERANCE=-0.05f;
		for(int i=0;i<28;++i){
			if(alpha[i].process(state.pointer,BUTTON_TOLERANCE))
				if(text->length()!=MSG_LIMIT)
					text->push_back(tolower(alpha[i].label[0]));
		}
		for(int i=0;i<10;++i){
			if(numeric[i].process(state.pointer,BUTTON_TOLERANCE))
				if(text->length()!=MSG_LIMIT)
					text->push_back(numeric[i].label[0]);
		}
		if(space.process(state.pointer,BUTTON_TOLERANCE))
			if(text->length()!=MSG_LIMIT)
				text->push_back(' ');
		if(backspace.process(state.pointer,BUTTON_TOLERANCE)){
			if(text->length()>0)
				text->pop_back();
		}
		if(clear.process(state.pointer,BUTTON_TOLERANCE))
			text->clear();
		if(enter.process(state.pointer,BUTTON_TOLERANCE))
			return true;
		if(cancel.process(state.pointer,BUTTON_TOLERANCE)||state.back){
			state.back=false;
			text->clear();
			return true;
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuInput::render(const Renderer &renderer)const{
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_KEYBOARD_BACKGROUND].object);
	renderer.uidraw(background);

	// render buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON_SMALL].object);
	for(int i=0;i<28;++i)
		alpha[i].render(renderer);
	for(int i=0;i<10;++i)
		numeric[i].render(renderer);
	space.render(renderer);
	backspace.render(renderer);
	clear.render(renderer);
	enter.render(renderer);
	cancel.render(renderer);

	// render button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	for(int i=0;i<28;++i)
		alpha[i].render_text(renderer);
	for(int i=0;i<10;++i)
		numeric[i].render_text(renderer);
	space.render_text(renderer);
	backspace.render_text(renderer);
	clear.render_text(renderer);
	enter.render_text(renderer);
	cancel.render_text(renderer);

	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	// render what's in the string
	drawtextcentered(renderer.font.button_small,0.0f,-2.5f,(*text+"|").c_str());
	// render the character limit
	glUniform4f(renderer.uniform.rgba,0.4f,0.4f,0.4f,1.0f);
	char char_limit_string[18];
	sprintf(char_limit_string,"%d/%d",text->length(),MSG_LIMIT);
	drawtextcentered(renderer.font.button_small,6.95f,-1.4f,char_limit_string);
	// draw the header text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-4.3f,header);
}
