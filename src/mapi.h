#ifndef GLPONG_MAPI_H_
#define GLPONG_MAPI_H_
#include <stdbool.h>
#include <GL/gl.h>

struct quad {
	int16_t size_x;
	int16_t size_y;
	int16_t origin_x;
	int16_t origin_y;
};

bool mapi_init(void);
void mapi_term(void);
bool mapi_proc_events(void);
void mapi_clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void mapi_draw_quad(const struct quad* quad);
void mapi_render_frame(void);

#endif

