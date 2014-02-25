/*
Yet Another CHip-8 emulaTor
Created by Jacob White
See LICENSE file for license information
*/

#include "sdlhelper.h"

//logs an SDL error to an output stream
void logSDLError(std::ostream &os, const std::string &message)
{
	os << message << " error: " << SDL_GetError() << std::endl;
}

//logs an SDL_mixer error to an output stream
void logMIXError(std::ostream &os, const std::string &message)
{
	os << message << " error: " << Mix_GetError() << std::endl;
}

//set an individual pixel of a SDL_Renderer
void setRendererPixel(SDL_Renderer *renderer, int x, int y, int r, int g, int b, int a)
{
	SDL_Rect pixel;
	pixel.x = x;
	pixel.y = y;
	pixel.w = 1;
	pixel.h = 1;
	
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, &pixel);
}

void setRendererScaledPixel(SDL_Renderer *renderer, int scale, int x, int y, int r, int g, int b, int a)
{
	SDL_Rect pixel;
	pixel.x = x * scale;
	pixel.y = y * scale;
	pixel.w = scale;
	pixel.h = scale;
	
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, &pixel);
}

void getTexturePixelRGB(SDL_Texture *texture, int x, int y, Uint8 *r, Uint8 *g, Uint8 *b)
{
	void *pixels;
	int pitch;
	Uint32 format;

	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	SDL_QueryTexture(texture, &format, NULL, NULL, NULL);
	SDL_PixelFormat *fmt = SDL_AllocFormat(format);
	Uint8 *temp = (Uint8*)pixels;
	SDL_GetRGB(temp[(y * pitch) + x], fmt, r, g, b);

	SDL_UnlockTexture(texture);
}

void setTexturePixel(SDL_Texture *texture, int x, int y, int r, int g, int b, int a)
{
	void *pixels;
	int pitch;
	Uint32 format;

	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	SDL_QueryTexture(texture, &format, NULL, NULL, NULL);
	SDL_PixelFormat *fmt = SDL_AllocFormat(format);
	Uint8 *temp = (Uint8*)pixels;
	temp[(y * pitch) + x] = (Uint8)SDL_MapRGBA(fmt, r, g, b, a);

	SDL_UnlockTexture(texture);
}

void setTextureScaledPixel(SDL_Texture *texture, int scale, int x, int y, int r, int g, int b, int a)
{
	void *pixels;
	int pitch;
	Uint32 format;

	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	SDL_QueryTexture(texture, &format, NULL, NULL, NULL);
	SDL_PixelFormat *fmt = SDL_AllocFormat(format);
	Uint8 *temp = (Uint8*)pixels;

	for(int i = 0; i < scale; i++)
	{
		for(int j = 0; j < scale; j++)
		{
			temp[(((y * scale) + j) * pitch) + (x * scale) + i] = (Uint8)SDL_MapRGBA(fmt, r, g, b, a);
		}
	}

	SDL_UnlockTexture(texture);
}
