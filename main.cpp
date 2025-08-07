#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "rasterizer.h"
#include "window_handler.h"

using namespace vec;

const int WIDTH = 800; // Datatype should be changed to prevent overflow when cast to float
const int HEIGHT = 600; // Powers of 2 are faster

void run( RenderTarget &target, Scene &scene)
{
	// Set up frametime clock
	typedef std::chrono::high_resolution_clock time;
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
	typedef std::chrono::duration<float, std::milli> duration;
	time_point timeStart;
	time_point timeEnd;
	duration timeElapsed;
	int fps;

	int mouse_x = 0;
	int mouse_y = 0;
	float x_coord = 0;
	float y_coord = 0;

	// Run until something quits the window
	bool running = true;
	SDL_Event event;
	while ( running ) {
		// Get time at start of frame
		timeStart = std::chrono::high_resolution_clock::now();

		// Poll window continuously and respond on event
		while ( SDL_PollEvent( &event ) != 0 ) {
			switch ( event.type ) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_MOUSEMOTION:
				mouse_x = event.motion.x;
				mouse_y = event.motion.y;
				// Map mouse location to world coordinates
				x_coord = ((float)mouse_x - WIDTH / 2) / WIDTH * 5;
				y_coord = -((float)mouse_y - HEIGHT / 2) / HEIGHT * 5;
				break;
			}
		}

		// Animate objects
		scene.getModel( "Suzanne" ).transform.rotatePitch( 1.0f );
		scene.getModel( "Suzanne" ).transform.setPosition( { x_coord, y_coord, 5.0f } );
		scene.getModel( "cube" ).transform.rotateYaw( 1.0f );
		scene.getModel( "cube2" ).transform.rotateYaw( -1.0f );

		// Reset buffers before rendering
		clearImage( target ); // 0-3ms

		// Render whole scene at once
		renderMany( target, scene.data() ); // 16-19ms

		// Load rendered image to buffer
		window::loadPixelsToBuffer(target.colorBuffer);
		
		// Display buffer thru window
		window::drawToWindow();

		// Get time at end of frame and measure duration elapsed
		timeEnd = time::now();
		timeElapsed = timeEnd - timeStart;
		fps = 1000 / timeElapsed.count();
		//printf( "Frametime: %2dms\n", (int)timeElapsed.count() );
		printf( "FPS: %3d\n", fps );
	}
}

int main( int argc, char **argv )
{
	// TODO: Allow multiple models of each obj file

	// Generate models for each obj file in folder
	std::map<const char *, Model> modelLibrary = Scene::generateLibrary( { "cube", "Suzanne" } );
	// HACK: Rotate 180deg and push everything back 5 units to appear right-side-up and in front of camera
	for ( auto &[name, model] : modelLibrary ) {
		model.transform.setOrientation( { 0.0f, 180.0f, 0.0f } );
		model.transform.setPosition( { 0.0f, 0.0f, 5.0f } );
	}

	// HACK: Duplicate existing model in library under another name
	Model cube2 = modelLibrary.at( "cube" );
	modelLibrary.insert( { "cube2", cube2 } );

	// Position models
	modelLibrary.at( "cube" ).transform.translate( { -2.0f, +0.5f, +2.0f } );
	modelLibrary.at( "cube" ).transform.rotateYaw( -30.0f );
	modelLibrary.at( "cube2" ).transform.translate( { +2.0f, +0.5f, +2.0f } );
	modelLibrary.at( "cube2" ).transform.rotateYaw( +30.0f );

	// Wrap models into scene
	Scene scene( modelLibrary );

	// Create target buffer to render onto
	RenderTarget renderTarget( WIDTH, HEIGHT );

	// Set up SDL
	window::init( WIDTH, HEIGHT );

	// Run SDL loop
	run( renderTarget, scene );

	// Quit SDL
	window::close();

	return 0;
}