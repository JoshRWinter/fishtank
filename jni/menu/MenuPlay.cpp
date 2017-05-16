#include <ctype.h>
#include "../fishtank.h"

const float BLOB_SIZE=1.0f;
const float LARGE_BLOB_SIZE=1.3f;

bool MenuPlay::exec(State &state){
	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.frame=0;
	background.count=1;

	button_name.init(-6.2f,3.1f,"Name");
	button_connect.init(button_name.x+BUTTON_WIDTH+0.3f,3.1f,"Connect");
	button_back.init(button_connect.x+BUTTON_WIDTH+0.3f,3.1f,"Back");

	// color blobs that the player can click on to change colors
	float xpos=-4.05f;
	const float ypos=0.0f;
	const float offset=1.8f;
	for(int i=0;i<5;++i){
		blob[i].x=xpos;
		blob[i].y=ypos;
		blob[i].w=BLOB_SIZE;
		blob[i].h=blob[i].w;
		blob[i].rot=0.0f;
		blob[i].frame=0;
		blob[i].count=1;

		xpos+=offset;
	}

	colorid=&state.colorid;
	name=&state.name;

	while(state.process()){
		// change the player's name
		if(button_name.process(state.pointer)){
			std::string tmp;
			if(!state.menu.input.exec(state,"New Name?",&tmp))
				return false;
			if(tmp.length()>0)
				state.name=tmp;
		}

		// connect the player
		if(button_connect.process(state.pointer)){
			std::string addr=state.connect_to;
			if(!state.menu.input.exec(state,"Connect to a server",&addr))
				return false;
			if(addr.length()>0){
				state.connect_to=addr;
				if(!state.menu.connect.exec(state,state.connect_to))
					return false;
				if(state.match.connected())
					return true;
			}
		}

		// choose a color
		if(state.pointer[0].active){
			for(int i=0;i<5;++i){
				if(blob[i].pointing(state.pointer[0]))
					state.colorid=i+1;
			}
		}

		// cancel
		if(button_back.process(state.pointer)||state.back){
			state.back=false;
			return true;
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuPlay::render(const Renderer &renderer)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// color blobs
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_COLORBLOB].object);
	for(int i=0;i<5;++i){
		float r,g,b;
		State::fill_color(i+1,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);

		Base color_blob=blob[i];
		if(*colorid==i+1){
			color_blob.w=LARGE_BLOB_SIZE;
			color_blob.h=LARGE_BLOB_SIZE;
			color_blob.x-=(LARGE_BLOB_SIZE-BLOB_SIZE)/2.0f;
			color_blob.y-=(LARGE_BLOB_SIZE-BLOB_SIZE)/2.0f;
		}

		renderer.uidraw(color_blob);
	}

	// text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	char msg[MSG_LIMIT+30];
	sprintf(msg,"%s",name->c_str());
	for(int i=0;i<strlen(msg);++i)msg[i]=toupper(msg[i]);
	drawtextcentered(renderer.font.main,0.0f,-3.75f,msg);

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	button_name.render(renderer);
	button_connect.render(renderer);
	button_back.render(renderer);

	// button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	button_name.render_text(renderer);
	button_connect.render_text(renderer);
	button_back.render_text(renderer);

	// player
	{
		float r,g,b;
		State::fill_color(*colorid,&r,&g,&b);
		glUniform4f(renderer.uniform.rgba,r,g,b,1.0f);
		Player player;
		player.x=-(PLAYER_WIDTH/2.0f);
		player.y-=2.0f;
		player.turret.rot=2.85f;
		player.turret.x=-0.8f;
		player.turret.y=-2.0f;
		player.count=2;
		player.frame=0;
		// turret
		glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TURRET].object);
		renderer.uidraw(player.turret);
		// player
		glBindTexture(GL_TEXTURE_2D,renderer.assets.texture[TID_TANK].object);
		renderer.uidraw(player);
	}
}
