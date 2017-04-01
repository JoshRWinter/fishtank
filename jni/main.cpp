#include "object/object.h"
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
		state->reset();
		break;
	case APP_CMD_RESUME:
		hidenavbars(&state->jni);
		break;
	}
}

int32_t inputproc(android_app *app,AInputEvent *event){
	return false;
}

extern "C" void android_main(android_app *app){
	logcat("--- BEGIN NEW LOG ---");
	app_dummy();
	State state;
	state.app=app;
	app->userData=&state;
	app->onInputEvent=inputproc;
	app->onAppCmd=cmdproc;
	init_jni(app,&state.jni);

	while(state.process()&&state.core()){
		state.render();
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	};

	term_jni(&state.jni);
	logcat("--- END LOG ---");
}
