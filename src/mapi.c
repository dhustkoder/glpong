#include <assert.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include "mapi.h"


/* video / renderer */
static struct vertex_data {
	GLfloat x, y;
	GLfloat r, g, b;
} *vertex_data = NULL;

static int vertex_cnt = 0;

static SDL_Window* window = NULL;
static SDL_GLContext context = 0;
static GLuint shader_id = 0;
static GLuint vs_id = 0;
static GLuint fs_id = 0;
static GLuint vao = 0;
static GLuint vbo = 0;

/* input */
bool mapi_keys[MAPI_KEY_NKEYS];



static void shader_init(void)
{
	const GLchar* const vs_src =
	"#version 130\n"
	"in vec2 pos;\n"
	"in vec3 rgb;\n"
	"out vec4 frag_color;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(-1.0 + (pos.x / 800.0),\n"
	"	                    1.0 - (pos.y / 600.0), 0.0, 1.0);\n"
	"	frag_color = vec4(rgb / 255.0, 1.0);\n"
	"}\n";
	const GLchar* const fs_src =
	"#version 130\n"
	"in vec4 frag_color;\n"
	"out vec4 outcolor;\n"
	"void main()\n"
	"{\n"
	"	outcolor = frag_color;\n"
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

	const GLint pos_attrib = glGetAttribLocation(shader_id, "pos");
	const GLint rgb_attrib = glGetAttribLocation(shader_id, "rgb");
	glEnableVertexAttribArray(pos_attrib);
	glEnableVertexAttribArray(rgb_attrib);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_TRUE,
	                      sizeof(struct vertex_data), NULL);
	glVertexAttribPointer(rgb_attrib, 3, GL_FLOAT, GL_TRUE,
	                      sizeof(struct vertex_data),
			      (void*)(sizeof(GLfloat) * 2));
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 1024,
	             NULL, GL_DYNAMIC_DRAW);

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
	bool k = true;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT: return false;
		case SDL_KEYUP: k = false; /* fallthrough */
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_UP: mapi_keys[MAPI_KEY_UP] = k; break;
			case SDLK_DOWN: mapi_keys[MAPI_KEY_DOWN] = k; break;
			case SDLK_LEFT: mapi_keys[MAPI_KEY_LEFT] = k; break;
			case SDLK_RIGHT: mapi_keys[MAPI_KEY_RIGHT] = k; break;
			}
			break;
		}
	}

	return true;
}

void mapi_render_clear(const GLfloat r, const GLfloat g, const GLfloat b, const GLfloat a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void mapi_render_begin(void)
{
	vertex_data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void mapi_render_quads(const struct quad* const quads, const int size)
{
	vertex_cnt += size;
	for (int i = 0; i < size; ++i) {
		const GLfloat px = quads[i].pos.x;
		const GLfloat py = quads[i].pos.y;
		const GLfloat sx = quads[i].size.x;
		const GLfloat sy = quads[i].size.y;
		const GLfloat r = quads[i].color.r;
		const GLfloat g = quads[i].color.g;
		const GLfloat b = quads[i].color.b;

		vertex_data->x = px + sx;
		vertex_data->y = py + sy;
		vertex_data->r = r;
		vertex_data->g = g;
		vertex_data->b = b;
		++vertex_data;

		vertex_data->x = px + sx;
		vertex_data->y = py - sy;
		vertex_data->r = r;
		vertex_data->g = g;
		vertex_data->b = b;
		++vertex_data;

		vertex_data->x = px - sx;
		vertex_data->y = py - sy;
		vertex_data->r = r;
		vertex_data->g = g;
		vertex_data->b = b;
		++vertex_data;

		vertex_data->x = px - sx;
		vertex_data->y = py + sy;
		vertex_data->r = r;
		vertex_data->g = g;
		vertex_data->b = b;
		++vertex_data;
	}
}

void mapi_render_flush(void)
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glDrawArrays(GL_QUADS, 0, vertex_cnt * 4);
	vertex_cnt = 0;
}

void mapi_render_frame(void)
{
	SDL_GL_SwapWindow(window);
}

