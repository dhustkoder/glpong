#include <assert.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "mapi.h"

/* video */
static SDL_Window* window = NULL;
static SDL_GLContext context = 0;
static GLuint shader_id = 0;
static GLuint vs_id = 0;
static GLuint fs_id = 0;
static GLint pos_attrib = 0;
static GLuint vao = 0;
static GLuint vbo = 0;


static void shader_init(void)
{
	const GLchar* const vs_src =
	"#version 130\n"
	"in vec2 position;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(position, 0.0, 1.0);\n"
	"}\n";
	const GLchar* const fs_src =
	"#version 130\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

	shader_id = glCreateProgram();
	vs_id = glCreateShader(GL_VERTEX_SHADER);
	fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	assert(shader_id != 0 && vs_id != 0 && fs_id != 0);

	const GLuint ids[] = { vs_id, fs_id };
	const GLchar* const srcs[] = { vs_src, fs_src };
	GLchar err_buffer[512];

	for (int i = 0; i < 2; ++i) {
		glShaderSource(ids[i], 1, &srcs[i], NULL);
		glCompileShader(ids[i]);

		GLint success;
		glGetShaderiv(ids[i], GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE) {
			glGetShaderInfoLog(ids[i], 512, NULL, err_buffer);
			puts(err_buffer);
			assert(false);
		}

		glAttachShader(shader_id, ids[i]);
	}

	glLinkProgram(shader_id);
	glUseProgram(shader_id);

	pos_attrib = glGetAttribLocation(shader_id, "position");
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pos_attrib);
}

static void shader_term(void)
{
	glDetachShader(shader_id, vs_id);
	glDetachShader(shader_id, fs_id);
	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
	glDeleteProgram(shader_id);
}


bool mapi_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "SDL2 Error: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("glpong",
	                          SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
			          800, 600, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		goto Lsdlquit;
	}
	
	context = SDL_GL_CreateContext(window);
	if (context == 0) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		goto Ldestroywindow;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(1);

	GLenum err;
	if ((err = glewInit()) != GLEW_OK) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		goto Ldeletecontext;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glViewport(0, 0, 800, 600);

	shader_init();
	return true;

Ldeletecontext:
	SDL_GL_DeleteContext(context);
Ldestroywindow:
	SDL_DestroyWindow(window);
Lsdlquit:
	SDL_Quit();

	return false;
}

void mapi_term(void)
{
	shader_term();
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
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

void mapi_clear(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mapi_draw_quad(const struct quad* const quad)
{
	typedef struct vec2f { GLfloat x; GLfloat y; } vec2f;
	const GLfloat middle_x = quad->origin_x / 800.f;
	const GLfloat middle_y = quad->origin_y / 600.f;
	const GLfloat x = quad->size_x / 800.f;
	const GLfloat y = quad->size_y / 600.f;
	const vec2f v0 = { middle_x + x, middle_y + y };
	const vec2f v1 = { middle_x + x, middle_y - y };
	const vec2f v2 = { middle_x - x, middle_y - y };
	const vec2f v3 = { middle_x - x, middle_y + y };
	const vec2f v4 = { middle_x - x, middle_y - y };

	const vec2f vertex[] = {
		v0, v1, v2,
		v0, v3, v4
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STREAM_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void mapi_render_frame(void)
{
	SDL_GL_SwapWindow(window);
}

