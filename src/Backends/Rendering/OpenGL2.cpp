#include "../Rendering.h"

#include <stddef.h>
#include <stdlib.h>

#include <GL/glew.h>
#include "SDL.h"

#include "../../WindowsWrapper.h"

#include "../../Font.h"

typedef struct Backend_Surface
{
	GLuint texture_id;
	unsigned int width;
	unsigned int height;
	unsigned char *pixels;
} Backend_Surface;

typedef struct Backend_Glyph
{
	GLuint texture_id;
	unsigned int width;
	unsigned int height;
} Backend_Glyph;

static SDL_Window *window;
static SDL_GLContext context;

static GLuint program_texture;
static GLuint program_colour_fill;
static GLuint program_glyph;

static GLint uniform_colour_fill_colour;
static GLint uniform_glyph_colour;

static GLuint framebuffer_id;
static GLfloat vertex_buffer[4][2];
static GLfloat texture_coordinate_buffer[4][2];

static Backend_Surface framebuffer_surface;

static const GLchar *vertex_shader_plain = " \
#version 120\n \
void main() \
{ \
	gl_Position = gl_ModelViewMatrix * gl_Vertex; \
} \
";

static const GLchar *vertex_shader_texture = " \
#version 120\n \
void main() \
{ \
	gl_TexCoord[0] = gl_MultiTexCoord0; \
	gl_Position = gl_ModelViewMatrix * gl_Vertex; \
} \
";

static const GLchar *fragment_shader_texture = " \
#version 120\n \
uniform sampler2D tex; \
void main() \
{ \
	gl_FragColor = texture2D(tex, gl_TexCoord[0].st); \
} \
";

static const GLchar *fragment_shader_colour_fill = " \
#version 120\n \
uniform vec4 colour; \
void main() \
{ \
	gl_FragColor = colour; \
} \
";

static const GLchar *fragment_shader_glyph = " \
#version 120\n \
uniform sampler2D tex; \
uniform vec4 colour; \
void main() \
{ \
	gl_FragColor = colour * texture2D(tex, gl_TexCoord[0].st); \
} \
";

static GLuint CompileShader(const char *vertex_shader_source, const char *fragment_shader_source)
{
	GLint shader_status;

	GLuint program_id = glCreateProgram();

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
		return 0;

	glAttachShader(program_id, vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
		return 0;

	glAttachShader(program_id, fragment_shader);

	glLinkProgram(program_id);

	glGetProgramiv(program_id, GL_LINK_STATUS, &shader_status);
	if (shader_status != GL_TRUE)
		return 0;

	return program_id;
}

static void SetRenderTarget(Backend_Surface *surface)
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, surface->texture_id, 0);

	glViewport(0, 0, surface->width, surface->height);

	glLoadIdentity();
	glOrtho(0.0, surface->width, 0.0, surface->height, 1.0, -1.0);
}

SDL_Window* Backend_CreateWindow(const char *title, int width, int height)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	return SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
}

BOOL Backend_Init(SDL_Window *p_window, unsigned int internal_screen_width, unsigned int internal_screen_height, BOOL vsync)
{
	window = p_window;

	context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(vsync);

	if (glewInit() != GLEW_OK)
		return FALSE;

	// Check if the platform supports OpenGL 2.1
	if (!GLEW_VERSION_2_1)
		return FALSE;

	// Check for framebuffer object extension (is part of the core spec in OpenGL 3.0, but not 2.1)
	if (!GLEW_EXT_framebuffer_object)
		return FALSE;

	// We're using pre-multiplied alpha so we can blend onto textures that have their own alpha
	// http://apoorvaj.io/alpha-compositing-opengl-blending-and-premultiplied-alpha.html
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertex_buffer);
	glTexCoordPointer(2, GL_FLOAT, 0, texture_coordinate_buffer);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Set up our shaders
	program_texture = CompileShader(vertex_shader_texture, fragment_shader_texture);
	program_colour_fill = CompileShader(vertex_shader_plain, fragment_shader_colour_fill);
	program_glyph = CompileShader(vertex_shader_texture, fragment_shader_glyph);

	if (program_texture == 0 || program_colour_fill == 0 || program_glyph == 0)
		printf("Failed to compile shaders\n");

	// Get shader uniforms
	uniform_colour_fill_colour = glGetUniformLocation(program_colour_fill, "colour");
	uniform_glyph_colour = glGetUniformLocation(program_glyph, "colour");

	// Set up framebuffer (used for surface-to-surface blitting)
	glGenFramebuffersEXT(1, &framebuffer_id);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_id);

	// Set up framebuffer screen texture (used for screen-to-surface blitting)
	glGenTextures(1, &framebuffer_surface.texture_id);
	glBindTexture(GL_TEXTURE_2D, framebuffer_surface.texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, internal_screen_width, internal_screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	framebuffer_surface.width = internal_screen_width;
	framebuffer_surface.height = internal_screen_height;

	return TRUE;
}

void Backend_Deinit(void)
{
	glDeleteTextures(1, &framebuffer_surface.texture_id);
	glDeleteFramebuffersEXT(1, &framebuffer_id);
	glDeleteProgram(program_glyph);
	glDeleteProgram(program_colour_fill);
	glDeleteProgram(program_texture);
	SDL_GL_DeleteContext(context);
}

void Backend_DrawScreen(void)
{
	// This would be a good time to use a custom shader to divide the pixels by
	// their alpha, to undo the premultiplied alpha stuff, but the framebuffer
	// is pretty much guaranteed to be fully opaque, and X / 1 == X, so it'd be
	// a waste of processing power.

	// Fit the screen to the window
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	float fit_width, fit_height;

	if ((float)framebuffer_surface.width / framebuffer_surface.height > (float)window_width / window_height)
	{
		fit_width = 1.0f;
		fit_height = ((float)framebuffer_surface.height / framebuffer_surface.width) / ((float)window_height / window_width);
	}
	else
	{
		fit_width = ((float)framebuffer_surface.width / framebuffer_surface.height) / ((float)window_width / window_height);
		fit_height = 1.0f;
	}

	glUseProgram(program_texture);

	glDisable(GL_BLEND);

	// Target actual screen, and not our framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glViewport(0, 0, window_width, window_height);

	glLoadIdentity();
	glOrtho(0.0, window_width, 0.0, window_height, 1.0, -1.0);

	// Draw framebuffer to screen
	glPushMatrix();
	glLoadIdentity();

	glBindTexture(GL_TEXTURE_2D, framebuffer_surface.texture_id);

	texture_coordinate_buffer[0][0] = 0.0f;
	texture_coordinate_buffer[0][1] = 1.0f;
	texture_coordinate_buffer[1][0] = 1.0f;
	texture_coordinate_buffer[1][1] = 1.0f;
	texture_coordinate_buffer[2][0] = 1.0f;
	texture_coordinate_buffer[2][1] = 0.0f;
	texture_coordinate_buffer[3][0] = 0.0f;
	texture_coordinate_buffer[3][1] = 0.0f;

	vertex_buffer[0][0] = -fit_width;
	vertex_buffer[0][1] = -fit_height;
	vertex_buffer[1][0] = fit_width;
	vertex_buffer[1][1] = -fit_height;
	vertex_buffer[2][0] = fit_width;
	vertex_buffer[2][1] = fit_height;
	vertex_buffer[3][0] = -fit_width;
	vertex_buffer[3][1] = fit_height;

	glDrawArrays(GL_QUADS, 0, 4);

	glPopMatrix();

	SDL_GL_SwapWindow(window);

	// According to https://www.khronos.org/opengl/wiki/Common_Mistakes#Swap_Buffers
	// the buffer should always be cleared
	glClear(GL_COLOR_BUFFER_BIT);

	// Switch back to our framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_id);
}

Backend_Surface* Backend_CreateSurface(unsigned int width, unsigned int height)
{
	Backend_Surface *surface = (Backend_Surface*)malloc(sizeof(Backend_Surface));

	if (surface == NULL)
		return NULL;

	glGenTextures(1, &surface->texture_id);
	glBindTexture(GL_TEXTURE_2D, surface->texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	surface->width = width;
	surface->height = height;

	return surface;
}

void Backend_FreeSurface(Backend_Surface *surface)
{
	if (surface == NULL)
		return;

	glDeleteTextures(1, &surface->texture_id);
	free(surface);
}

unsigned char* Backend_Lock(Backend_Surface *surface, unsigned int *pitch)
{
	if (surface == NULL)
		return NULL;

	surface->pixels = (unsigned char*)malloc(surface->width * surface->height * 4);
	*pitch = surface->width * 4;

	return surface->pixels;
}

void Backend_Unlock(Backend_Surface *surface)
{
	if (surface == NULL)
		return;

	// Pre-multiply the colour channels with the alpha, so blending works correctly
	unsigned char *pixels = surface->pixels;

	for (unsigned int y = 0; y < surface->height; ++y)
	{
		for (unsigned int x = 0; x < surface->width; ++x)
		{
			pixels[0] = (pixels[0] * pixels[3]) / 0xFF;
			pixels[1] = (pixels[1] * pixels[3]) / 0xFF;
			pixels[2] = (pixels[2] * pixels[3]) / 0xFF;
			pixels += 4;
		}
	}

	glBindTexture(GL_TEXTURE_2D, surface->texture_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->width, surface->height, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	free(surface->pixels);
}

static void BlitCommon(Backend_Surface *source_surface, const RECT *rect, long x, long y, BOOL alpha_blend)
{
	if (rect->right - rect->left < 0 || rect->bottom - rect->top < 0)
		return;

	glUseProgram(program_texture);

	if (alpha_blend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, source_surface->texture_id);

	const GLfloat texture_left = (GLfloat)rect->left / (GLfloat)source_surface->width;
	const GLfloat texture_right = (GLfloat)rect->right / (GLfloat)source_surface->width;
	const GLfloat texture_top = (GLfloat)rect->top / (GLfloat)source_surface->height;
	const GLfloat texture_bottom = (GLfloat)rect->bottom / (GLfloat)source_surface->height;

	const GLfloat vertex_left = (GLfloat)x;
	const GLfloat vertex_right = (GLfloat)x + (rect->right - rect->left);
	const GLfloat vertex_top = (GLfloat)y;
	const GLfloat vertex_bottom = (GLfloat)y + (rect->bottom - rect->top);

	texture_coordinate_buffer[0][0] = texture_left;
	texture_coordinate_buffer[0][1] = texture_top;
	texture_coordinate_buffer[1][0] = texture_right;
	texture_coordinate_buffer[1][1] = texture_top;
	texture_coordinate_buffer[2][0] = texture_right;
	texture_coordinate_buffer[2][1] = texture_bottom;
	texture_coordinate_buffer[3][0] = texture_left;
	texture_coordinate_buffer[3][1] = texture_bottom;

	vertex_buffer[0][0] = vertex_left;
	vertex_buffer[0][1] = vertex_top;
	vertex_buffer[1][0] = vertex_right;
	vertex_buffer[1][1] = vertex_top;
	vertex_buffer[2][0] = vertex_right;
	vertex_buffer[2][1] = vertex_bottom;
	vertex_buffer[3][0] = vertex_left;
	vertex_buffer[3][1] = vertex_bottom;

	glDrawArrays(GL_QUADS, 0, 4);
}

void Backend_Blit(Backend_Surface *source_surface, const RECT *rect, Backend_Surface *destination_surface, long x, long y, BOOL alpha_blend)
{
	if (source_surface == NULL || destination_surface == NULL)
		return;

	// Point our framebuffer to the destination texture
	SetRenderTarget(destination_surface);

	BlitCommon(source_surface, rect, x, y, alpha_blend);
}

void Backend_BlitToScreen(Backend_Surface *source_surface, const RECT *rect, long x, long y, BOOL alpha_blend)
{
	if (source_surface == NULL)
		return;

	// Point our framebuffer to the screen texture
	SetRenderTarget(&framebuffer_surface);

	BlitCommon(source_surface, rect, x, y, alpha_blend);
}

static void ColourFillCommon(const RECT *rect, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	if (rect->right - rect->left < 0 || rect->bottom - rect->top < 0)
		return;

	glUseProgram(program_colour_fill);

	glDisable(GL_BLEND);

	glUniform4f(uniform_colour_fill_colour, red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);

	vertex_buffer[0][0] = (GLfloat)rect->left;
	vertex_buffer[0][1] = (GLfloat)rect->top;
	vertex_buffer[1][0] = (GLfloat)rect->right;
	vertex_buffer[1][1] = (GLfloat)rect->top;
	vertex_buffer[2][0] = (GLfloat)rect->right;
	vertex_buffer[2][1] = (GLfloat)rect->bottom;
	vertex_buffer[3][0] = (GLfloat)rect->left;
	vertex_buffer[3][1] = (GLfloat)rect->bottom;

	glDrawArrays(GL_QUADS, 0, 4);
}

void Backend_ColourFill(Backend_Surface *surface, const RECT *rect, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	if (surface == NULL)
		return;

	// Point our framebuffer to the destination texture
	SetRenderTarget(surface);

	ColourFillCommon(rect, red, green, blue, alpha);
}

void Backend_ColourFillToScreen(const RECT *rect, unsigned char red, unsigned char green, unsigned char blue)
{
	// Point our framebuffer to the screen texture
	SetRenderTarget(&framebuffer_surface);

	ColourFillCommon(rect, red, green, blue, 0xFF);
}

void Backend_ScreenToSurface(Backend_Surface *surface, const RECT *rect)
{
	if (surface == NULL)
		return;

	// Point our framebuffer to the destination texture
	SetRenderTarget(surface);

	BlitCommon(&framebuffer_surface, rect, rect->left, rect->top, FALSE);
}

Backend_Glyph* Backend_LoadGlyph(const unsigned char *pixels, unsigned int width, unsigned int height, int pitch, unsigned short total_greys, unsigned char pixel_mode)
{
	Backend_Glyph *glyph = (Backend_Glyph*)malloc(sizeof(Backend_Glyph));

	if (glyph == NULL)
		return NULL;

	const int destination_pitch = ((width * 2) + 3) & ~3;	// Round up to the nearest 4 (OpenGL needs this)

	unsigned char *buffer = (unsigned char*)malloc(destination_pitch * height);

	if (buffer == NULL)
	{
		free(glyph);
		return NULL;
	}

	switch (pixel_mode)
	{
		case FONT_PIXEL_MODE_GRAY:
			for (unsigned int y = 0; y < height; ++y)
			{
				const unsigned char *source_pointer = pixels + y * pitch;
				unsigned char *destination_pointer = buffer + y * destination_pitch;

				for (unsigned int x = 0; x < width; ++x)
				{
					const unsigned char alpha = (unsigned char)(pow((double)*source_pointer++ / (total_greys - 1), 1.0 / 1.8) * 255.0);

					*destination_pointer++ = alpha;
					*destination_pointer++ = alpha;
				}
			}

			break;

		case FONT_PIXEL_MODE_MONO:
			for (unsigned int y = 0; y < height; ++y)
			{
				const unsigned char *source_pointer = pixels + y * pitch;
				unsigned char *destination_pointer = buffer + y * destination_pitch;

				for (unsigned int x = 0; x < width; ++x)
				{
					const unsigned char alpha = *source_pointer++ ? 0xFF : 0;

					*destination_pointer++ = alpha;
					*destination_pointer++ = alpha;
				}
			}

			break;
	}

	glGenTextures(1, &glyph->texture_id);
	glBindTexture(GL_TEXTURE_2D, glyph->texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glyph->width = width;
	glyph->height = height;

	free(buffer);

	return glyph;
}

void Backend_UnloadGlyph(Backend_Glyph *glyph)
{
	if (glyph == NULL)
		return;

	glDeleteTextures(1, &glyph->texture_id);
	free(glyph);
}

static void DrawGlyphCommon(Backend_Glyph *glyph, long x, long y, const unsigned char *colours)
{
	glUseProgram(program_glyph);

	glEnable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, glyph->texture_id);

	const GLfloat vertex_left = (GLfloat)x;
	const GLfloat vertex_right = (GLfloat)x + glyph->width;
	const GLfloat vertex_top = (GLfloat)y;
	const GLfloat vertex_bottom = (GLfloat)y + glyph->height;

	glUniform4f(uniform_glyph_colour, colours[0] / 255.0f, colours[1] / 255.0f, colours[2] / 255.0f, 1.0f);

	texture_coordinate_buffer[0][0] = 0.0f;
	texture_coordinate_buffer[0][1] = 0.0f;
	texture_coordinate_buffer[1][0] = 1.0f;
	texture_coordinate_buffer[1][1] = 0.0f;
	texture_coordinate_buffer[2][0] = 1.0f;
	texture_coordinate_buffer[2][1] = 1.0f;
	texture_coordinate_buffer[3][0] = 0.0f;
	texture_coordinate_buffer[3][1] = 1.0f;

	vertex_buffer[0][0] = vertex_left;
	vertex_buffer[0][1] = vertex_top;
	vertex_buffer[1][0] = vertex_right;
	vertex_buffer[1][1] = vertex_top;
	vertex_buffer[2][0] = vertex_right;
	vertex_buffer[2][1] = vertex_bottom;
	vertex_buffer[3][0] = vertex_left;
	vertex_buffer[3][1] = vertex_bottom;

	glDrawArrays(GL_QUADS, 0, 4);
}

void Backend_DrawGlyph(Backend_Surface *surface, Backend_Glyph *glyph, long x, long y, const unsigned char *colours)
{
	if (glyph == NULL || surface == NULL)
		return;

	// Point our framebuffer to the destination texture
	SetRenderTarget(surface);

	DrawGlyphCommon(glyph, x, y, colours);
}

void Backend_DrawGlyphToScreen(Backend_Glyph *glyph, long x, long y, const unsigned char *colours)
{
	if (glyph == NULL)
		return;

	// Point our framebuffer to the screen texture
	SetRenderTarget(&framebuffer_surface);

	DrawGlyphCommon(glyph, x, y, colours);
}

void Backend_HandleDeviceLoss(void)
{
	// No problem for us
}

void Backend_HandleWindowResize(void)
{
	// No problem for us
}
