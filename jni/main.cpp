#include <time.h>
#include <signal.h>
#include "fishtank.h"

void cmdproc(android_app *app,int32_t cmd){
	State *state=(State*)app->userData;
	switch(cmd){
	case APP_CMD_INIT_WINDOW:
		state->running=true;
		state->renderer.init(*app);
		state->init(*app);
		break;
	case APP_CMD_TERM_WINDOW:
		state->running=false;
		state->renderer.term();
		state->term();
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
	else if(type==AINPUT_EVENT_TYPE_KEY){
		int32_t code=AKeyEvent_getKeyCode(event);
		int32_t action=AKeyEvent_getAction(event);
		if(code==AKEYCODE_BACK&&action==AKEY_EVENT_ACTION_UP){
			state->back=true;
			return true;
		}
	}
	return false;
}

void sound_config_fn(const struct sl_entity_position *listener,const struct sl_entity_position *source,float *stereo,float *attenuation){
	// set the stereo position
	*stereo=(source->x-listener->x)/10.0f;

	// set the attenuation
	float dist=distance(listener->x,source->x,listener->y,source->y);
	float atten;
	if(dist<SOUND_RANGE)
		atten=1.0f;
	else{
		atten=1.0f-((dist-SOUND_RANGE)/10.0f);
		// clamp
		if(atten<0.0f)
			atten=0.0f;
		else if(atten>1.0f)
			atten=1.0f;
	}
	*attenuation=atten;
}

extern "C" void android_main(android_app *app){
	logcat("--- BEGIN NEW LOG ---");
	srand48(time(NULL));
	State state;
	state.reset();
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
