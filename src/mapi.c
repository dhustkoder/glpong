#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "mapi.h"


#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif


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

/* sound */
static Mix_Chunk** sounds = NULL;
static Mix_Music** musics = NULL;
static int sounds_size = 0;
static int musics_size = 0;


static void* aux_load_wav(const char* const file)
{
	return Mix_LoadWAV_RW(SDL_RWFromFile(file, "rb"), 1);
}

static void mix_free_files(void(* const mix_freefun)(void*),
                           void** const buffer,
                           int* const buffer_size)
{
	const int size = *buffer_size;
	for (int i = 0; i < size; ++i)
		mix_freefun(buffer[i]);
	free(buffer);
	*buffer_size = 0;
}

static void mapi_free_music_files(void)
{
	mix_free_files((void(*)(void*))Mix_FreeMusic, (void**)musics, &musics_size);
}

static void mapi_free_sound_files(void)
{
	mix_free_files((void(*)(void*))Mix_FreeChunk, (void**)sounds, &sounds_size);
}

static bool mix_load_files(const char* const* const filepaths,
                           const int cnt,
                           void*(* const mix_loadfun)(const char* file),
                           void(* const free_files)(void),
                           void*** const buffer,
                           int* const buffer_size)
{
	if (*buffer_size > 0)
		free_files();

	(*buffer) = malloc(sizeof(void*) * cnt);
	assert((*buffer) != NULL);

	int i;
	for (i = 0; i < cnt; ++i) {
		if (((*buffer)[i] = mix_loadfun(filepaths[i])) == NULL)
			break;
	}

	*buffer_size = i;
	if (*buffer_size < cnt) {
		free_files();
		return false;
	}

	return true;
}



static GLfloat radians(const GLfloat degrees)
{
	return degrees * (M_PI / 180.f);	
}

static mat4_t mat4_identity(const GLfloat i)
{
	mat4_t mat = {{
		{ i, 0, 0, 0 },
		{ 0, i, 0, 0 },
		{ 0, 0, i, 0 },
		{ 0, 0, 0, i }
	}};

	return mat;
}

static void mat4_translate(const GLfloat x, const GLfloat y, const GLfloat z,
                           mat4_t* const mat)
{
	GLfloat* const data = &mat->data[0][0];
	data[12] = data[12] + ((x * data[0]) + (y * data[4]) + (z * data[8]));
	data[13] = data[13] + ((x * data[1]) + (y * data[5]) + (z * data[9]));
	data[14] = data[14] + ((x * data[2]) + (y * data[6]) + (z * data[10]));
	data[15] = data[15] + ((x * data[3]) + (y * data[7]) + (z * data[11]));
}

static mat4_t mat4_ortho(const GLfloat left, const GLfloat right,
                         const GLfloat bottom, const GLfloat top,
			 const GLfloat near, const GLfloat far)
{
	mat4_t mat = mat4_identity(1);
	mat.data[0][0] = 2.f / (right - left);
	mat.data[1][1] = 2.f / (top - bottom);
	mat.data[2][2] = -2.f / (far - near);
	mat.data[3][0] = -(right + left) / (right - left);
	mat.data[3][1] = -(top  + bottom) / (top - bottom);
	mat.data[3][2] = -(far + near) / (far - near);
	return mat;
}

static mat4_t mat4_persp(const GLfloat fovy, const GLfloat aspect,
                         const GLfloat near, const GLfloat far)
{
	const GLfloat range = tan(radians(fovy / 2.f)) * near;
	const GLfloat left = -range * aspect;
	const GLfloat right = range * aspect;
	const GLfloat bottom = -range;
	const GLfloat top = range;

	mat4_t mat = mat4_identity(0);
	mat.data[0][0] = (2.f * near) / (right - left);
	mat.data[1][1] = (2.f * near) / (top - bottom);
	mat.data[2][2] = -(far + near) / (far - near);
	mat.data[2][3] = -1.f;
	mat.data[3][2] = -(2.f * far * near) / (far * near);

	return mat;

}


static void shader_init(void)
{
	const GLchar* const vs_src =
	"#version 130\n"
	"in vec2 pos;\n"
	"in vec3 rgb;\n"
	"out vec4 frag_color;\n"
	"uniform mat4 model = mat4(1.0);\n"
	"uniform mat4 view = mat4(1.0);\n"
	"uniform mat4 projection = mat4(1.0);\n"
	"void main()\n"
	"{\n"
	"	gl_Position = projection * view * model * vec4(pos, 0.0, 1.0);\n"
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

	const GLint proj_attrib = glGetUniformLocation(shader_id, "projection");
	mat4_t projection = mat4_ortho(0, 800, 600, 0, -1, 1);
	glUniformMatrix4fv(proj_attrib, 1, GL_FALSE, (GLfloat*) &projection);
}

static void shader_term(void)
{
	glDetachShader(shader_id, vs_id);
	glDetachShader(shader_id, fs_id);
	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
	glDeleteProgram(shader_id);
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

		vertex_data->x = px - sx;
		vertex_data->y = py - sy;
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

		vertex_data->x = px + sx;
		vertex_data->y = py + sy;
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

bool mapi_load_music_files(const char* const* const filepaths, const int cnt)
{
	return mix_load_files(filepaths, cnt,
                              (void*(*)(const char*))Mix_LoadMUS,
                              mapi_free_music_files,
                              (void***)&musics, &musics_size);

}

bool mapi_load_sound_files(const char* const* const filepaths, const int cnt)
{
	return mix_load_files(filepaths, cnt,
                              (void*(*)(const char*))aux_load_wav,
                              mapi_free_sound_files,
                              (void***)&sounds, &sounds_size);

}

void mapi_play_music(const int id)
{
	assert(id < musics_size);
	if (Mix_PlayMusic(musics[id], 0) != 0) {
		fprintf(stderr, "Couldn't play music \'%d\': %s\n",
		        id, SDL_GetError());
	}
}

void mapi_play_sound(const int id)
{
	assert(id < sounds_size);
	if (Mix_PlayChannel(-1, sounds[id], 0) != 0) {
		fprintf(stderr, "Couldn't play sound \'%d\': %s\n",
		        id, SDL_GetError());
	}
}

bool mapi_music_playing(void)
{
	return Mix_PlayingMusic() != 0;
}


bool mapi_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "SDL2 Error: %s", SDL_GetError());
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) != 0)
		goto Lsdlquit;

	window = SDL_CreateWindow("glpong",
	                          SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
			          800, 600, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
		goto Lcloseaudio;
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
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(GLfloat) * 1024,
	             NULL, GL_DYNAMIC_DRAW);

	glViewport(0, 0, 800, 600);

	shader_init();
	return true;

Ldeletecontext:
	SDL_GL_DeleteContext(context);
Ldestroywindow:
	SDL_DestroyWindow(window);
Lcloseaudio:
	Mix_CloseAudio();
Lsdlquit:
	SDL_Quit();

	return false;
}

void mapi_term(void)
{
	mapi_free_music_files();
	mapi_free_sound_files();
	shader_term();
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_CloseAudio();
	SDL_Quit();
}

