#pragma once

#include <stddef.h>

#include "SDL.h"

typedef struct FontObject FontObject;

FontObject* LoadFont(unsigned int cell_width, unsigned int cell_height, char *font_filename);
void DrawText(FontObject *font_object, SDL_Renderer *renderer, SDL_Texture *texture, int x, int y, unsigned long colour, const char *string, size_t string_length);
void UnloadFont(FontObject *font_object);