#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include "mapi.h"


int main(void)
{
	if (!mapi_init())
		return EXIT_FAILURE;

	const GLfloat paddle_vel = 5.5f;
	struct vec2 ball_vel = { 8.5f, 8.5f };

	const GLfloat mod[3] = { 0.00015, 0.00011, 0.00018 };
	bool up[3] = { true, true, true };
	GLfloat c[3] = { 0.0, 0.0, 0.0 };

	struct quad objects[] = {
		{ { 11.5f, 64 }, { 11.5f, 300  }, { 0, 0, 255 } },
		{ { 11.5f, 64 }, { 800 - 11.5f, 300 }, { 255, 0, 0 } },
		{ { 8.5f,  8.5f  }, { 400, 300 }, { 0, 255, 0 } }
	};

	struct quad* const player = &objects[0];
	struct quad* const enemy = &objects[1];
	struct quad* const ball = &objects[2];

	while (mapi_proc_events()) {
		mapi_render_clear(c[0], c[1], c[2], 1.0);

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

		/* update positions */
		if (mapi_is_key_pressed(MAPI_KEY_UP))
			player->pos.y -= paddle_vel;
		else if (mapi_is_key_pressed(MAPI_KEY_DOWN))
			player->pos.y += paddle_vel;

		ball->pos.x += ball_vel.x;
		ball->pos.y += ball_vel.y;

		if (ball->pos.x >= 800 || ball->pos.x <= 0) {
			ball_vel.x = -ball_vel.x;
			ball->pos.x = 400;
			ball->pos.y = 300;
		} else if (ball->pos.y >= 600 || ball->pos.y <= 0) {
			ball_vel.y = -ball_vel.y;
		}
	
		/* check collisions */
		const int16_t player_right = player->pos.x + player->size.x;
		const int16_t enemy_left = enemy->pos.x - enemy->size.x;
		const int16_t ball_left = ball->pos.x - ball->size.x;
		const int16_t ball_right = ball->pos.x + ball->size.x;
		const struct quad* paddle;

		if (ball_left < player_right)
			paddle = player;	
		else if (ball_right > enemy_left)
			paddle = enemy;
		else
			paddle = NULL;

		if (paddle != NULL) {
			const int16_t ptop = paddle->pos.y - paddle->size.y;
			const int16_t pbottom = paddle->pos.y + paddle->size.y;
			const int16_t btop = ball->pos.y - ball->size.y;
			const int16_t bbottom = ball->pos.y + ball->size.y;
			if (btop <= pbottom && bbottom >= ptop) {
				ball_vel.x = -ball_vel.x;
				if (ball->pos.y > paddle->pos.y)
					ball_vel.y = abs(ball_vel.y);
				else
					ball_vel.y = -abs(ball_vel.y);
			}
		}

		mapi_render_begin();
		mapi_render_quads(objects, sizeof(objects)/sizeof(objects[0]));
		mapi_render_flush();
		mapi_render_frame();
	}

	mapi_term();
	return EXIT_SUCCESS;
}

