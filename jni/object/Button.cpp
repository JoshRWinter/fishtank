#include "../fishtank.h"

void Button::init(float xpos,float ypos,const char *button_label){
	x=xpos;
	y=ypos;
	w=BUTTON_WIDTH;
	h=BUTTON_HEIGHT;
	rot=0.0f;
	count=1;
	frame=0;
	label=button_label;
	active=false;
}

bool Button::process(const crosshair *ch,float tolerance){
	if(pointing(ch[0],tolerance)){
		if(ch[0].active)
			active=true;
		else if(active){
			active=false;
			return true;
		}
	}
	else
		active=false;
	return false;
}

void Button::render(const Renderer &renderer)const{
	if(active)
		glUniform4f(renderer.uniform.rgba,0.0f,0.5f,0.8f,1.0f);
	else
		glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	renderer.draw(*this);
}

void Button::render_text(const Renderer &renderer)const{
	if(active)
		glUniform4f(renderer.uniform.rgba,0.0f,0.5f,0.8f,1.0f);
	else
		glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	drawtextcentered(renderer.font.button,x+(BUTTON_WIDTH/2.0f),y+(BUTTON_HEIGHT/2.0f)-0.3f,label);
}

void ButtonSmall::init(float xpos,float ypos,const char *button_label){
	x=xpos;
	y=ypos;
	w=BUTTON_SMALL_SIZE;
	h=BUTTON_SMALL_SIZE;
	label=button_label;
	rot=0.0f;
	count=1;
	frame=0;
	active=false;
}

void ButtonSmall::render_text(const Renderer &renderer)const{
	if(active)
		glUniform4f(renderer.uniform.rgba,0.0f,0.5f,0.8f,1.0f);
	else
		glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	drawtextcentered(renderer.font.button_small,x+(BUTTON_SMALL_SIZE/2.0f),y+(BUTTON_SMALL_SIZE/2.0f)-0.2f,label);
}

void ButtonBasic::init(float xpos,float ypos,float size,const char *name){
	x=xpos;
	y=ypos;
	active=false;
	label=name;
	w=size;
	h=size;
	rot=0.0f;
	frame=0;
	count=1;
}

void ButtonBasic::render_text(const Renderer &renderer)const{
	drawtextcentered(renderer.font.button_small,x+(w/2.0f),y+(h/2.0f)-(renderer.font.button_small->fontsize/2.0f),label);
}
