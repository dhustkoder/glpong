#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include "mapi.h"


int main(void)
{
	if (!mapi_init())
		return EXIT_FAILURE;

	while (mapi_proc_events()) {
		glClearColor(0.5, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		mapi_frame_done();
	}

	mapi_term();
	return EXIT_SUCCESS;
}

