#include <time.h>
#include "object/object.h"
#include "fishtank.h"

bool State::core(){
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
		drawtext(renderer.font.main,renderer.view.left+0.1f,renderer.view.top+0.1f,fps_string);
	}
#endif // SHOW_FPS
}

State::State(){
	running=false;

	// background
	background.x=renderer.view.left;
	background.y=renderer.view.top;
	background.w=renderer.view.right*2.0f;
	background.h=renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.count=1;
	background.frame=0;
}

void State::reset(){
}
