#include <android/log.h>
#include <android/sensor.h>
#include <GLES2/gl2ext.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define GLESUTIL_DEBUG

#ifdef GLESUTIL_DEBUG
#define logcat(...) ((void)__android_log_print(ANDROID_LOG_INFO, "winter", __VA_ARGS__))
#else
#define logcat(...)
#endif

#define GET_POINTER_INDEX(action) ((action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)>>AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT)
#define onein(n) (randomint(0,n-1)==0)

#ifdef __cplusplus
extern "C" {
#endif

extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays;
struct packtexture{
	unsigned object;
	unsigned size;
	float pinch[4];
};
struct pack{
	struct packtexture *texture;
	unsigned count;
};
struct apacksound{
	unsigned size,targetsize;
	short *buffer;
	struct apack *parent;
};
struct apack{
	struct apacksound *sound;
	char count;
	pthread_t thread;
	pthread_mutex_t mutex;
};
typedef struct{
	float fontsize;
	void *kern;
	unsigned atlas;
}ftfont;
struct audioplayer;
typedef struct{
	SLObjectItf engine,outputmix;
	SLEngineItf engineinterface;
	struct audioplayer *audioplayerlist;
	int enabled;
	int sound_count;
	int last_id;
}slesenv;
struct audioplayer{
	SLObjectItf playerobject;
	SLPlayItf playerinterface;
	SLVolumeItf volumeinterface;
	SLAndroidSimpleBufferQueueItf playerbufferqueue;
	slesenv *engine;
	struct apacksound *sound;
	int loop,destroy,initial;
	int id;
	struct audioplayer *next;
};
struct crosshair{
	int active;
	float x,y;
};
#define DIMS_LAND 0
#define DIMS_PORT 1
struct device{
	int w,h;
};
struct jni_info{
	JNIEnv *env;
	JavaVM *vm;
	jobject clazz;
	jobject sys_svc;

	int hasvb;
	jclass vb_svc;
	jmethodID vbmethod;

	jmethodID MethodGetWindow;
	jobject lWindow;
	jclass cWindow;
	jclass cView;
	jmethodID MethodGetDecorView;
	jobject lDecorView;
	jmethodID MethodSetSystemUiVisibility;
};
struct accel_info{
	struct android_app *app;
	ASensorManager *manager;
	const ASensor *sensor;
	ASensorEventQueue *queue;
	float x,y,z;
};
int loadpack(struct pack *asset,AAssetManager *mgr,const char *filename,const char *mode);
unsigned char *convert_power_of_two(unsigned char *bytedata,int *size,int width,int height);
void destroypack(struct pack *asset);
void loadapack(struct apack *target,AAssetManager *mgr,const char *filename);
void destroyapack(struct apack *target);
int32_t retrieve_touchscreen_input(AInputEvent *event,struct crosshair *pointer,int iscreenwidth,int iscreenheight,float fscreenwidth,float fscreenheight);
void getdims(struct device*,ANativeWindow*,int);
void initextensions();
void initortho(float *matrix,float left,float right,float bottom,float top,float znear,float zfar);
unsigned initshaders(const char *Vertex,const char *Fragment);
void set_ftfont_params(int iscreenwidth,int iscreenheight,float fscreenwidth,float fscreenheight,unsigned ftvector,unsigned ftsize,unsigned ft_texcoords);
ftfont *create_ftfont(struct AAssetManager *mgr,float fontsize,const char *facename);
void destroy_ftfont(ftfont *font);
float textlen(ftfont *font,const char *text);
float textheight(ftfont *font,const char *text);
void drawtext(ftfont *font,float xpos,float ypos,const char *output);
void drawtextcentered(ftfont *font,float xpos,float ypos,const char *output);
slesenv *initOpenSL();
void termOpenSL(slesenv *soundengine);
int playsound(slesenv *engine,struct apacksound *sound,float intensity,int loop);
void setsoundintensity(slesenv *engine,int id,float intensity);
int checksound(slesenv *engine,int id);
void stopsound(slesenv *engine,int id);
void stopallsounds(slesenv *engine);
void disablesound(slesenv *engine);
void enablesound(slesenv *engine);
int randomint(int low,int high);
float zerof(float *val,float step);
float targetf(float *val,float step,float target);
float align(float *rot,float step,float target);
unsigned screenshot(int w,int h,int darken);
unsigned screenshotblur(int w,int h,int resize,int intensity);
void get_nano_time(long long*);
void init_jni(struct android_app *app,struct jni_info *jni_info);
void vibratedevice(struct jni_info* jni_info,int mills);
void hidenavbars(struct jni_info *jni_info);
void term_jni(struct jni_info *jni_info);
void init_accel(struct android_app *app,struct accel_info *accel_info);
void handle_accel(struct accel_info *accel_info);
void enable_accel(struct accel_info *accel_info);
void disable_accel(struct accel_info *accel_info);
void term_accel(struct accel_info *accel_indf);

#ifdef __cplusplus
}
#endif
