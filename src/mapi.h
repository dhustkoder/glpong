#ifndef GLPONG_MAPI_H_
#define GLPONG_MAPI_H_
#include <stdbool.h>


bool mapi_init(void);
void mapi_term(void);
bool mapi_proc_events(void);
void mapi_frame_done(void);

#endif
