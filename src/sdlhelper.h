/*
Yet Another CHip-8 emulaTor
Created by Jacob White
See LICENSE file for license information
*/

#ifndef SDL_HELPER_H
#define SDL_HELPER_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

void logSDLError(std::ostream &os, const std::string &message);

void logMIXError(std::ostream &os, const std::string &message);

void setRendererPixel(SDL_Renderer *renderer, int x, int y, int r, int g, int b, int a=255);

void setRendererScaledPixel(SDL_Renderer *renderer, int scale, int x, int y, int r, int g, int b, int a=255);

void getTexturePixelRGB(SDL_Texture *texture, int x, int y, Uint8 *r, Uint8 *g, Uint8 *b);

void setTexturePixel(SDL_Texture *texture, int x, int y, int r, int g, int b, int a=255);

void setTextureScaledPixel(SDL_Texture *texture, int scale, int x, int y, int r, int g, int b, int a=255);

#endif