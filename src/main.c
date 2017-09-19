#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <GL/glew.h>
#include "mapi.h"


int main(void)
{
	if (!mapi_init())
		return EXIT_FAILURE;

	const GLfloat mod[3] = { 0.0015, 0.0033, 0.0047 };
	bool up[3] = { true, true, true };
	GLfloat c[3] = { 0.0, 0.0, 0.0 };

	struct quad quad = { 300, 300, -300, 0 };

	while (mapi_proc_events()) {
		mapi_clear(c[0], c[1], c[2], 1.0);

		for (int i = 0; i < 3; ++i) {
			if (up[i]) {
				c[i] += mod[i];
				if (c[i] > 1.0) {
					c[i] = 1.0;
					up[i] = false;
				}
			} else {
				c[i] -= mod[i];
				if (c[i] < 0.0) {
					c[i] = 0.0;
					up[i] = true;
				}
			}
		}

		mapi_draw_quad(&quad);
		mapi_render_frame();
	}

	mapi_term();
	return EXIT_SUCCESS;
}

