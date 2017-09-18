#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>



int main(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL2 Error: %s", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Window* window = SDL_CreateWindow("glpong",
	                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			     800, 600, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		goto Lsdlquit;
		return EXIT_FAILURE;
	}
	
	SDL_GLContext context = SDL_GL_CreateContext(window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);
	glewInit();


	SDL_Event event;
	bool rungame = true;
	while (rungame) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				rungame = false;
				break;
			}
		}

		glClearColor(0.5, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
Lsdlquit:
	SDL_Quit();
	return EXIT_SUCCESS;
}

