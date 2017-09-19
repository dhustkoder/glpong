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

	struct quad quads[] = {
		{ { 32, 32 }, { 400, 300 }, { 0, 255, 0 } },
		{ { 32, 32 }, { 400, 300 }, { 255, 0, 0 } }
	};

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
			quads[0].pos.y -= 6;
		else if (mapi_is_key_pressed(MAPI_KEY_DOWN))
			quads[0].pos.y += 6;

		if (mapi_is_key_pressed(MAPI_KEY_LEFT))
			quads[0].pos.x -= 6;
		else if (mapi_is_key_pressed(MAPI_KEY_RIGHT))
			quads[0].pos.x += 6;
	
		mapi_render_begin();
		mapi_render_quads(quads, 2);
		mapi_render_flush();

		mapi_render_frame();
	}

	mapi_term();
	return EXIT_SUCCESS;
}

