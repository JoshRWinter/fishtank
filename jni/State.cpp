#include "fishtank.h"

bool State::core(){
	if(show_menu){
		show_menu=false;
		if(!menu.main.exec(*this))
			return false;
	}

	// get network updates
	match.recv_data(*this);
	// send network updates
	match.send_data(*this);
	// see if an error happened with the above operations
	if(!match.connected()){
		logcat("error, disconnected");
		show_menu=true;
	}

	// process players
	Player::process(*this);

	// process shells
	Shell::process(*this);

	// process particle shells
	ParticleShell::process(*this);

	// process particle platforms
	ParticlePlatform::process(*this);

	// process particle players
	ParticlePlayer::process(*this);

	// process ui buttons
	const float UI_TOLERANCE=-0.25f;
	input.left.process(pointer,UI_TOLERANCE);
	input.right.process(pointer,UI_TOLERANCE);
	input.up.process(pointer,UI_TOLERANCE);
	input.aim_left.process(pointer,UI_TOLERANCE);
	input.aim_right.process(pointer,UI_TOLERANCE);
	if(input.fire.process(pointer,UI_TOLERANCE)){
		const float minimum=0.35f;
		if(firepower<minimum)
			firepower=minimum;
		final_firepower=firepower;
		firepower=0.0f;
	}
	else if(input.fire.active){
		firepower+=FIREPOWER_INCREMENT*speed;
		if(firepower>1.0f)
			firepower=1.0f;
	}
	else
		firepower=0.0f;

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
	renderer.uidraw(background);

	// draw shells
	if(shell_list.size()!=0)
		Shell::render(renderer,shell_list);

	// draw players
	Player::render(renderer,player_list);

	// draw particles players
	ParticlePlayer::render(renderer,particle_player_list);

	// draw particle platforms
	ParticlePlatform::render(renderer,particle_platform_list);

	// draw platforms
	Platform::render(renderer,platform_list);

	// draw particle shells
	if(particle_shell_list.size()!=0)
		ParticleShell::render(renderer,particle_shell_list);

	// firepower indicator
	if(firepower>0.0f){
		glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_FIREPOWER_INDICATOR].object);
		float s=FIRE_BUTTON_SIZE*firepower;
		Base fpi={input.fire.x+(FIRE_BUTTON_SIZE/2.0f)-(s/2.0f),input.fire.y+(FIRE_BUTTON_SIZE/2.0f)-(s/2.0f),s,s,0.0f,0,1};
		renderer.uidraw(fpi);
	}
	// draw ui buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON_SMALL].object);
	input.left.render(renderer);
	input.right.render(renderer);
	input.up.render(renderer);
	input.fire.render(renderer);
	input.aim_left.render(renderer);
	input.aim_right.render(renderer);
	// ui button text
	glUniform4f(renderer.uniform.rgba,0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.button_small->atlas);
	input.left.render_text(renderer);
	input.right.render_text(renderer);
	input.up.render_text(renderer);
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
	if(!read_config()){
		name="basil fawlty";
		connect_to="";
	}

	speed=1.0f;
	running=false;
	show_menu=true;
	memset(pointer,0,sizeof(crosshair)*2);
	firepower=0.0f;
	final_firepower=0.0f;
	colorid=randomint(COLOR_RED,COLOR_PURPLE);

	// players
	Player dummy;
	for(int i=0;i<MAX_PLAYERS;++i){
		player_list.push_back(dummy);
	}

	// background
	background.x=renderer.view.left;
	background.y=renderer.view.top;
	background.w=renderer.view.right*2.0f;
	background.h=renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.count=1;
	background.frame=0;

	// ui buttons
	const float DPAD_SIZE=1.0f;
	input.left.init(-7.0f,2.25f,DPAD_SIZE,"L");
	input.right.init(-4.5f,2.25f,DPAD_SIZE,"R");
	input.up.init(-5.75f,1.3f,DPAD_SIZE,"U");
	input.fire.init(4.625f,2.0f,FIRE_BUTTON_SIZE,"FIRE");
	input.aim_left.init(3.3f,2.5f,DPAD_SIZE,"L");
	input.aim_right.init(6.75f,2.5f,DPAD_SIZE,"R");
}

void State::reset(){
	// clear shells
	for(Shell *shell:shell_list)
		delete shell;
	shell_list.clear();
	// platforms are cleared in Match::get_level_config
	// clear particle shells
	for(ParticleShell *p:particle_shell_list)
		delete p;
	particle_shell_list.clear();
	// clear particle platforms
	for(ParticlePlatform *p:particle_platform_list)
		delete p;
	particle_platform_list.clear();
	// clear particle players
	for(ParticlePlayer *p:particle_player_list)
		delete p;
	particle_player_list.clear();
}

bool State::read_config(){
	FILE *file=fopen(DATAPATH"/00","rb");

	if(!file)
		return false;
	char connect_to_tmp[MSG_LIMIT+1];
	char name_tmp[MSG_LIMIT+1];
	fread(connect_to_tmp,1,MSG_LIMIT+1,file);
	fread(name_tmp,1,MSG_LIMIT+1,file);
	name=name_tmp;
	connect_to=connect_to_tmp;

	fclose(file);
	return true;
}

void State::write_config(){
	FILE *file=fopen(DATAPATH"/00","wb");
	if(!file){
		logcat("could not open " DATAPATH "/00 for writing");
		return;
	}

	char connect_to_tmp[MSG_LIMIT+1];
	char name_tmp[MSG_LIMIT+1];
	strncpy(connect_to_tmp,connect_to.c_str(),MSG_LIMIT+1);
	strncpy(name_tmp,name.c_str(),MSG_LIMIT+1);
	fwrite(connect_to_tmp,1,MSG_LIMIT+1,file);
	fwrite(name_tmp,1,MSG_LIMIT+1,file);

	fclose(file);
}

void State::fill_color(int id,float *r,float *g,float *b){
	switch(id){
	default:
	case COLOR_RED:
		*r=RGB_RED_R;*g=RGB_RED_G;*b=RGB_RED_B;
		break;
	case COLOR_BLUE:
		*r=RGB_BLUE_R;*g=RGB_BLUE_G;*b=RGB_BLUE_B;
		break;
	case COLOR_CYAN:
		*r=RGB_CYAN_R;*g=RGB_CYAN_G;*b=RGB_CYAN_B;
		break;
	case COLOR_GREEN:
		*r=RGB_GREEN_R;*g=RGB_GREEN_G;*b=RGB_GREEN_B;
		break;
	case COLOR_PURPLE:
		*r=RGB_PURPLE_R;*g=RGB_PURPLE_G;*b=RGB_PURPLE_B;
		break;
	}
}
