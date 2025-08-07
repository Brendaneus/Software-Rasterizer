// SDL context handler
// TODO List: loadPixelsToBuffer
#pragma once
#include <SDL.h>
#include "vector_math.h"

namespace window
{
	SDL_Window *gWindow = NULL;
	SDL_Surface *gScreenSurface = NULL;
	SDL_Surface *gBufferSurface = NULL;

	// Init SDL subsystem and create window
	void init(const int &width, const int &height)
	{
		// Init SDL video subsystem
		SDL_Init( SDL_INIT_VIDEO );

		// Create window
		gWindow = SDL_CreateWindow( "Software Rasterizer",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width, height,
			SDL_WINDOW_SHOWN );

		// Get window surface
		gScreenSurface = SDL_GetWindowSurface( gWindow );

		// Create buffer surface for holding pixel data
		gBufferSurface = SDL_CreateRGBSurfaceWithFormat( 0, width, height, 32, SDL_PIXELFORMAT_RGBA32 );
		SDL_SetSurfaceBlendMode( gBufferSurface, SDL_BLENDMODE_NONE );
	}

	// Loads rendered image to buffer surface
	// TODO: Find out why image appears upside down without fix
	void loadPixelsToBuffer( const std::vector<std::vector<float3>> &pixelColors )
	{
		uint32_t *pixels = (uint32_t *)gBufferSurface->pixels;
		int height = pixelColors.size();
		int width = pixelColors[0].size();
		for ( int y = 0; y < height; y++ ) {
			int yOffset = (y * width);
			for ( int x = 0; x < width; x++ ) {
				// HACK: Load pixels bottom-up instead of top-down
				pixels[yOffset + x] = pixelColors[y][x].toUint32();
			}
		}
	}

	// Updates the window with image in buffer surface
	void drawToWindow()
	{
		SDL_BlitSurface( gBufferSurface, NULL, gScreenSurface, NULL );
		SDL_UpdateWindowSurface( gWindow );
	}

	// Destroy window and shut down SDL
	void close()
	{
		// Deallocate buffer
		 SDL_FreeSurface( gBufferSurface );
		 gBufferSurface = NULL;

		// Destroy Window
		SDL_DestroyWindow( gWindow );
		gWindow = NULL;

		// Quit SDL subsystem
		SDL_Quit();
	}
}