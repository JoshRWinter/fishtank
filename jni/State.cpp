#include <time.h>
#include "fishtank.h"

bool State::core(){
	if(show_menu){
		show_menu=false;
		if(!menu.main.exec(*this))
			return false;
	}

	match.recv_data(*this);
	match.send_data(*this);
	// see if an error happened with the above operations
	if(!match.connected()){
		logcat("error, disconnected");
		show_menu=true;
	}

	// process ui buttons
	const float UI_TOLERANCE=-0.25f;
	input.left.process(pointer,UI_TOLERANCE);
	input.right.process(pointer,UI_TOLERANCE);
	input.up.process(pointer,UI_TOLERANCE);
	input.down.process(pointer,UI_TOLERANCE);
	input.fire.process(pointer,UI_TOLERANCE);
	input.aim_left.process(pointer,UI_TOLERANCE);
	input.aim_right.process(pointer,UI_TOLERANCE);

	return true;
}

bool State::process()const{
	int events;
	android_poll_source *source;
	while(ALooper_pollAll(running?0:-1,NULL,&events,(void**)&source)>=0){
		if(source)
			source->process(app,source);
		if(app->destroyRequested)
			return false;
	}
	return true;
}

void State::render()const{
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	// draw backdround
	glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_BACKGROUND].object);
	renderer.draw(background);

	// draw ui buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON_SMALL].object);
	input.left.render(renderer);
	input.right.render(renderer);
	input.up.render(renderer);
	input.down.render(renderer);
	input.fire.render(renderer);
	input.aim_left.render(renderer);
	input.aim_right.render(renderer);
	// ui button text
	glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
	input.left.render_text(renderer);
	input.right.render_text(renderer);
	input.up.render_text(renderer);
	input.down.render_text(renderer);
	input.fire.render_text(renderer);
	input.aim_left.render_text(renderer);
	input.aim_right.render_text(renderer);

	// fps
#ifdef SHOW_FPS
	{
		static char fps_string[36];
		static int last_time,fps;
		int current_time=time(NULL);
		if(current_time!=last_time){
			last_time=current_time;
			sprintf(fps_string,"[fishtank] fps: %d",fps);
			fps=0;
		}
		else
			++fps;

		glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
		glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
		drawtext(renderer.font.main,renderer.view.left+0.1f,renderer.view.top+0.1f,fps_string);
	}
#endif // SHOW_FPS
}

State::State(){
	name="Basil Fawlty";
	running=false;
	show_menu=true;
	memset(pointer,0,sizeof(crosshair)*2);

	// background
	background.x=renderer.view.left;
	background.y=renderer.view.top;
	background.w=renderer.view.right*2.0f;
	background.h=renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.count=1;
	background.frame=0;

	// ui buttons
	const float DPAD_SIZE=0.8f;
	const float FIRE_BUTTON_SIZE=1.5f;
	input.left.init(-7.0f,2.25f,DPAD_SIZE,"L");
	input.right.init(-4.5f,2.25f,DPAD_SIZE,"R");
	input.down.init(-5.75f,3.2f,DPAD_SIZE,"D");
	input.up.init(-5.75f,1.3f,DPAD_SIZE,"U");
	input.fire.init(5.0f,2.0f,FIRE_BUTTON_SIZE,"FIRE");
	input.aim_left.init(3.9f,2.5f,DPAD_SIZE,"L");
	input.aim_right.init(6.75f,2.5f,DPAD_SIZE,"R");
}

void State::reset(){
}
