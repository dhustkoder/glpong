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
	uint16_t x, y;
};

struct quad {
	struct vec2 size;
	struct vec2 pos;
};


static inline bool mapi_is_key_pressed(const enum MAPI_Key key)
{
	extern bool mapi_keys[MAPI_KEY_NKEYS];
	return mapi_keys[key];
}


bool mapi_init(void);
void mapi_term(void);
bool mapi_proc_events(void);
void mapi_clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void mapi_draw_quad(const struct quad* quad);
void mapi_render_frame(void);


#endif

