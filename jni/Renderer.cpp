#include "fishtank.h"

extern const char *vertexshader,*fragmentshader;

void Renderer::init(android_app &app){
	getdims(&dev,app.window,DIMS_LAND);
	screen.w=dev.w>1280?1280:1024;
	screen.h=dev.h>720?720:576;

	initextensions(); // opengl extensions

	display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display,NULL,NULL);

	EGLConfig config;
	int configcount;
	int config_attributes[]={EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_NONE};
	eglChooseConfig(display,config_attributes,&config,1,&configcount);
	ANativeWindow_setBuffersGeometry(app.window,screen.w,screen.h,0); // set framebuffer dimensions

	surface=eglCreateWindowSurface(display,config,app.window,NULL);
	int context_attributes[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
	context=eglCreateContext(display,config,NULL,context_attributes);
	eglMakeCurrent(display,surface,surface,context);

	// shaders and uniforms
	program=initshaders(vertexshader,fragmentshader);
	glUseProgram(program);
	uniform.vector=glGetUniformLocation(program,"vector");
	uniform.size=glGetUniformLocation(program,"size");
	uniform.rot=glGetUniformLocation(program,"rot");
	uniform.texcoords=glGetUniformLocation(program,"texcoords");
	uniform.projection=glGetUniformLocation(program,"projection");
	uniform.rgba=glGetUniformLocation(program,"rgba");
	float matrix[16];
	initortho(matrix,view.left,view.right,view.bottom,view.top,-1.0f,1.0f);
	glUniformMatrix4fv(uniform.projection,1,false,matrix);

	// vaos and vbos
	glGenVertexArrays(1,&vao);
	glGenBuffers(1,&vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	const float verts[]={-0.5f,-0.5f,-0.5f,0.5f,0.5f,-0.5f,0.5f,0.5f};
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8,verts,GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,false,0,NULL);

	// opengl state settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.2f,0.5f,0.65f,1.f);

	// gameplay textures
	if(!loadpack(&assets,app.activity->assetManager,"assets",NULL))
		logcat("texture init error");
	// ui textures
	if(!loadpack(&uiassets,app.activity->assetManager,"uiassets",NULL))
		logcat("ui texture init error");
	// atlas
	if(!atlas.load(app.activity->assetManager,"atlas"))
		logcat("%s",atlas.error());

	// fonts
	set_ftfont_params(screen.w,screen.h,view.right*2.0f,view.bottom*2.0f,uniform.vector,uniform.size,uniform.texcoords);
	font.main=create_ftfont(app.activity->assetManager,0.6f,"arial.ttf");
	font.button=create_ftfont(app.activity->assetManager,0.5f,"arial.ttf");
	font.button_small=create_ftfont(app.activity->assetManager,0.4f,"arial.ttf");
}

void Renderer::term(){
	destroy_ftfont(font.main);
	destroy_ftfont(font.button);
	destroy_ftfont(font.button_small);
	destroypack(&assets);
	destroypack(&uiassets);
	atlas.unload();
	glDeleteBuffers(1,&vbo);
	glDeleteVertexArrays(1,&vao);
	glDeleteProgram(program);
	eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
	eglDestroyContext(display,context);
	eglDestroySurface(display,context);
	eglTerminate(display);
}

void Renderer::draw(const Base &b,const Atlas *atlas)const{
	static const float default_coords[]={0.0f,1.0f,0.0f,1.0f};
	const float *coords=default_coords;
	if(atlas)
		coords=atlas->coords(b.texture);

	float new_x=b.x-(player_x+(PLAYER_WIDTH/2.0f));
	float new_y=b.y-(player_y+(PLAYER_HEIGHT/2.0f));

	glUniform4fv(uniform.texcoords,1,coords);
	glUniform2f(uniform.vector,new_x,new_y);
	glUniform2f(uniform.size,b.w,b.h);
	glUniform1f(uniform.rot,b.rot);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void Renderer::uidraw(const UIBase &b)const{
	const float size=1.0f/b.count;
	const float left=b.frame*size;
	const float right=left+size;

	glUniform4f(uniform.texcoords,left,right,0.0f,1.0f);
	glUniform2f(uniform.vector,b.x,b.y);
	glUniform2f(uniform.size,b.w,b.h);
	glUniform1f(uniform.rot,b.rot);

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

Renderer::Renderer(){
	view.left=VIEW_LEFT;
	view.right=VIEW_RIGHT;
	view.bottom=VIEW_BOTTOM;
	view.top=VIEW_TOP;
}
