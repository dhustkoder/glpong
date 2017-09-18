#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "mapi.h"


SDL_Window* mapi_window;
SDL_GLContext mapi_context;


bool mapi_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "SDL2 Error: %s", SDL_GetError());
		return false;
	}

	mapi_window = SDL_CreateWindow("glpong",
	                          SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
			          800, 600, SDL_WINDOW_OPENGL);
	if (mapi_window == NULL) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		goto Lsdlquit;
	}
	
	mapi_context = SDL_GL_CreateContext(mapi_window);
	if (mapi_context == 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		goto Ldestroywindow;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

	GLenum err;
	if ((err = glewInit()) != GLEW_OK) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		goto Ldeletecontext;
	}

	return true;

Ldeletecontext:
	SDL_GL_DeleteContext(mapi_context);
Ldestroywindow:
	SDL_DestroyWindow(mapi_window);
Lsdlquit:
	SDL_Quit();

	return false;
}

void mapi_term(void)
{
	SDL_GL_DeleteContext(mapi_context);
	SDL_DestroyWindow(mapi_window);
	SDL_Quit();
}

bool mapi_proc_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT: return false;
		}
	}

	return true;
}

void mapi_frame_done(void)
{
	SDL_GL_SwapWindow(mapi_window);
}

