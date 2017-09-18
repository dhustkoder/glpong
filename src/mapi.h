#ifndef GLPONG_MAPI_H_
#define GLPONG_MAPI_H_
#include <stdbool.h>


bool mapi_init(void);
void mapi_term(void);
bool mapi_proc_events(void);
void mapi_clear(float r, float g, float b, float a);
void mapi_draw(void);

#endif

