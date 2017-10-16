#ifndef GLPONG_MAPI_H_
#define GLPONG_MAPI_H_
#include <stdint.h>
#include <stdbool.h>
#include <GL/gl.h>


enum MAPI_Key {
	MAPI_KEY_UP,
	MAPI_KEY_DOWN,
	MAPI_KEY_LEFT,
	MAPI_KEY_RIGHT,
	MAPI_KEY_NKEYS
};


struct vec2 {
	GLfloat x, y;	
};

struct vec3 {
	GLfloat x, y, z;
};

struct vec4 {
	GLfloat x, y, z, w;
};

struct mat4 {
	GLfloat data[4][4];
};

struct color {
	uint8_t r, g, b;
};


typedef struct vec2 vec2_t;
typedef struct vec3 vec3_t;
typedef struct vec4 vec4_t;
typedef struct mat4 mat4_t;
typedef struct color color_t;


struct quad {
	vec2_t size;
	vec2_t pos;
	color_t color;
};


static inline bool mapi_is_key_pressed(const enum MAPI_Key key)
{
	extern bool mapi_keys[MAPI_KEY_NKEYS];
	return mapi_keys[key];
}


bool mapi_init(void);
void mapi_term(void);
bool mapi_proc_events(void);

/* graphic */
void mapi_render_clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void mapi_render_begin(void);
void mapi_render_quads(const struct quad* quads, int size);
void mapi_render_flush(void);
void mapi_render_frame(void);

/* sound */
bool mapi_load_music_files(const char* const* const filepaths, int cnt);
bool mapi_load_sound_files(const char* const* const filepaths, int cnt);
void mapi_play_music(int id);
void mapi_play_sound(int id);
bool mapi_music_playing(void);


#endif

