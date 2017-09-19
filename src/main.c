#include <stdlib.h>
#include <stdio.h>
#include "mapi.h"


int main(void)
{
	if (!mapi_init())
		return EXIT_FAILURE;

	const GLfloat mod[3] = { 0.0015, 0.0033, 0.0047 };
	bool up[3] = { true, true, true };
	GLfloat c[3] = { 0.0, 0.0, 0.0 };

	struct quad quad = { { 32, 32 }, { 400, 300 } };

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

		if (mapi_is_key_pressed(MAPI_KEY_UP))
			quad.pos.y -= 6;
		else if (mapi_is_key_pressed(MAPI_KEY_DOWN))
			quad.pos.y += 6;

		if (mapi_is_key_pressed(MAPI_KEY_LEFT))
			quad.pos.x -= 6;
		else if (mapi_is_key_pressed(MAPI_KEY_RIGHT))
			quad.pos.x += 6;

		mapi_draw_quad(&quad);
		mapi_render_frame();
	}

	mapi_term();
	return EXIT_SUCCESS;
}

