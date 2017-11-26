#include <ctype.h>
#include "../network.h"
#include "../fishtank.h"

const float BLOB_SIZE=1.0f;
const float LARGE_BLOB_SIZE=1.3f;

static int current;
static const int TIMEOUT = 4;

static bool recv_net(net::tcp &tcp, void *buffer, unsigned size){
	int read = 0;
	while(read != size){
		if(time(NULL) - current > TIMEOUT){
			memset(buffer, 0, size);
			return false;
		}

		read += tcp.recv_nonblock((char*)buffer + read, size - read);
		if(tcp.error())
			return false;
	}

	return true;
}

// get a string from socket <tcp>
static std::string get_string(net::tcp &tcp){
	uint32_t count = 0;
	if(!recv_net(tcp, &count, sizeof(count)))
		return "";

	std::vector<char> data(count + 1);
	if(!recv_net(tcp, &data[0], count))
		return "";

	data[count] = 0;
	return {&data[0]};
}

bool MenuBrowserConnect::exec(State &state){
	background.x=state.renderer.view.left;
	background.y=state.renderer.view.top;
	background.w=state.renderer.view.right*2.0f;
	background.h=state.renderer.view.bottom*2.0f;
	background.rot=0.0f;
	background.frame=0;
	background.count=1;

	servers.clear();

	cancel.init(-BUTTON_WIDTH/2.0f,3.1f,"Cancel");

	enum class connstate{
		NO, // not connected
		TELL, // tell server connection type
		RECV // wait for browser list
	}conn = connstate::NO;

	current = time(NULL);
	net::tcp tcp(MASTER, MASTER_PORT);
	uint64_t servercount = 0;

	while(state.process()){
		if(time(NULL) - current > TIMEOUT){
			if(!state.menu.message.exec(state, "You may still try a \"Direct Connect\"","Could not retrieve Server Listing"))
				return false;
			return state.menu.browser.exec(state, std::vector<ServerConfig>());
		}

		// network
		switch(conn){
		case connstate::NO:
		{
			bool connected = tcp.connect();
			if(connected)
				conn = connstate::TELL;
		}
			break;
		case connstate::TELL:
		{
			// tell the master server this is a listing request
			const uint8_t type = 0;
			tcp.send_block(&type, sizeof(type));
			if(!recv_net(tcp, &servercount, sizeof(servercount)))
				return true;
			else
				conn = connstate::RECV;
			break;
		}
		case connstate::RECV:
		{
			if(servercount-- == 0){
				return state.menu.browser.exec(state, servers);
			}
			std::string ip = get_string(tcp);
			std::string name = get_string(tcp);
			std::string location = get_string(tcp);
			uint8_t count = 0;
			tcp.recv_block(&count, sizeof(count));
			servers.push_back({ip, name, location, count});
			break;
		}
		}

		// cancel
		if(cancel.process(state.pointer)||state.back){
			state.back = false;
			return state.menu.browser.exec(state, std::vector<ServerConfig>());
		}

		render(state.renderer);
		eglSwapBuffers(state.renderer.display,state.renderer.surface);
	}

	return false;
}

void MenuBrowserConnect::render(const Renderer &renderer)const{
	// background
	glUniform4f(renderer.uniform.rgba,1.0f,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BACKGROUND].object);
	renderer.uidraw(background);

	// text
	glUniform4f(renderer.uniform.rgba,TEXT_COLOR,1.0f);
	glBindTexture(GL_TEXTURE_2D,renderer.font.main->atlas);
	drawtextcentered(renderer.font.main,0.0f,-1.75f,"Loading Server Listing...");

	// buttons
	glBindTexture(GL_TEXTURE_2D,renderer.uiassets.texture[UITID_BUTTON].object);
	cancel.render(renderer);

	// button text
	glBindTexture(GL_TEXTURE_2D,renderer.font.button->atlas);
	glUniform4f(renderer.uniform.rgba,BUTTON_TEXT_COLOR,1.0f);
	cancel.render_text(renderer);
}
