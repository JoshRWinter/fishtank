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
	backspace.init(6.0f,-0.7f,"BS");
	enter.init(6.0f,3.0f,"OK");
	cancel.init(-7.0f,3.0f,"nvm");

	const float START_XOFFSET=-6.4f;
	const float START_YOFFSET=-1.25f;
	float xoffset=START_XOFFSET;
	float yoffset=START_YOFFSET;

	// backdrop
	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.count=1;
	background.frame=0;

	// set numerals
	yoffset=START_YOFFSET;
	xoffset=START_XOFFSET;
	for(int i=0;i<10;++i){
		numeric[i].init(xoffset,yoffset,numerals[i]);
		xoffset+=BUTTON_SMALL_SIZE+0.2f;
	}
	yoffset+=BUTTON_SMALL_SIZE+0.1f;
	xoffset=START_XOFFSET;

	// set alphas and : and .
	for(int i=0;i<28;++i){
		alpha[i].init(xoffset,yoffset,letters[i]);
		if(letters[i][0]=='P'){
			xoffset=START_XOFFSET+0.5f;
			yoffset+=BUTTON_SMALL_SIZE+0.025f;
		}
		else if(letters[i][0]==':'){
			xoffset=START_XOFFSET+1.0f;
			yoffset+=BUTTON_SMALL_SIZE+0.025f;
		}
		else
			xoffset+=BUTTON_SMALL_SIZE+0.2f;
	}

	while(state.process()){
		// process buttons
		const float BUTTON_TOLERANCE=-0.15f;
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
		if(backspace.process(state.pointer,BUTTON_TOLERANCE))
			if(text->length()>0)
				text->pop_back();
		if(enter.process(state.pointer,BUTTON_TOLERANCE))
			return true;
		if(cancel.process(state.pointer,BUTTON_TOLERANCE)){
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
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// render buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON_SMALL].object);
	for(int i=0;i<28;++i)
		alpha[i].render(renderer);
	for(int i=0;i<10;++i)
		numeric[i].render(renderer);
	space.render(renderer);
	backspace.render(renderer);
	enter.render(renderer);
	cancel.render(renderer);

	// render button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
	for(int i=0;i<28;++i)
		alpha[i].render_text(renderer);
	for(int i=0;i<10;++i)
		numeric[i].render_text(renderer);
	space.render_text(renderer);
	backspace.render_text(renderer);
	enter.render_text(renderer);
	cancel.render_text(renderer);

	glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	// render what's in the string
	drawtextcentered(renderer.font.button_small,0.0f,-2.5f,(*text+"|").c_str());
	// render the character limit
	char char_limit_string[18];
	sprintf(char_limit_string,"%d/%d",text->length(),MSG_LIMIT);
	drawtextcentered(renderer.font.button_small,0.0f,-1.8f,char_limit_string);
	// draw the header text
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-4.0f,header);
}
