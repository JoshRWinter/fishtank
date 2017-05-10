#include <time.h>
#include <signal.h>
#include "fishtank.h"

void cmdproc(android_app *app,int32_t cmd){
	State *state=(State*)app->userData;
	switch(cmd){
	case APP_CMD_INIT_WINDOW:
		state->running=true;
		state->renderer.init(*app);
		break;
	case APP_CMD_TERM_WINDOW:
		state->running=false;
		state->renderer.term();
		break;
	case APP_CMD_DESTROY:
		state->write_config();
		state->reset();
		break;
	case APP_CMD_RESUME:
		hidenavbars(&state->jni);
		break;
	}
}

int32_t inputproc(android_app *app,AInputEvent *event){
	State *state=(State*)app->userData;
	int32_t type=AInputEvent_getType(event);
	if(type==AINPUT_EVENT_TYPE_MOTION)
		return retrieve_touchscreen_input(event,state->pointer,state->renderer.dev.w,state->renderer.dev.h,state->renderer.view.right*2.0f,state->renderer.view.bottom*2.0f);
	return false;
}

extern "C" void android_main(android_app *app){
	logcat("--- BEGIN NEW LOG ---");
	app_dummy();
	srand48(time(NULL));
	State state;
	state.app=app;
	app->userData=&state;
	app->onInputEvent=inputproc;
	app->onAppCmd=cmdproc;
	init_jni(app,&state.jni);

	long long last_time;
	get_nano_time(&last_time);
	while(state.process()&&state.core()){
		state.render();
		eglSwapBuffers(state.renderer.display,state.renderer.surface);

		// update state.speed, the time delta
		long long current_time,diff;
		get_nano_time(&current_time);
		diff=current_time-last_time;
		state.speed=diff/16666600.0f;
		last_time=current_time;
		if(state.speed>4.0f)
			state.speed=4.0f;
	}

	term_jni(&state.jni);
	logcat("--- END LOG ---");
}
