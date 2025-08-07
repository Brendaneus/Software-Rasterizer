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
				// Map mouse location to world coordinates (invert Y)
				x_coord = ((float)mouse_x - WIDTH / 2) / WIDTH * 5;
				y_coord = -((float)mouse_y - HEIGHT / 2) / HEIGHT * 5;
				break;
			}
		}

		// Animate objects
		scene.getModel( "Suzanne" ).transform.rotatePitch( 1.0f );
		scene.getModel( "Suzanne" ).transform.setPosition( { x_coord, y_coord, -4.0f } );
		scene.getModel( "cube" ).transform.rotateYaw( 1.0f );
		scene.getModel( "cube2" ).transform.rotateYaw( -1.0f );
		scene.getModel( "quad" ).transform.rotateYaw( 1.0f );

		// Reset buffers before rendering
		clearImage( target );

		// Render whole scene at once
		renderMany( target, scene.data() );

		// Load rendered image to buffer
		window::loadPixelsToBuffer(target.colorBuffer);
		
		// Display buffer thru window
		window::drawToWindow();

		// Get time at end of frame and measure duration elapsed
		timeEnd = time::now();
		timeElapsed = timeEnd - timeStart;
		fps = 1000 / timeElapsed.count();
		printf( "FPS: %3d\n", fps );
	}
	// HACK: Keep window open on last frame
	SDL_Event e; bool quit = false; while ( quit == false ) { while ( SDL_PollEvent( &e ) ) { if ( e.type == SDL_QUIT ) quit = true; } }
}

int main( int argc, char **argv )
{
	// TODO: Allow multiple models of each obj file

	// Generate models for each obj file in folder
	std::map<const char *, Model> modelLibrary = Scene::generateLibrary( { "quad", "cube", "Suzanne" } );

	// HACK: Push everything back 5 units to appear in front of camera
	for ( auto &[name, model] : modelLibrary ) {
		model.transform.setPosition( { 0.0f, 0.0f, -5.0f } );
	}

	// HACK: Duplicate existing model in library under another name
	Model cube2 = modelLibrary.at( "cube" );
	modelLibrary.insert( { "cube2", cube2 } );

	// Position models
	modelLibrary.at( "cube" ).transform.translate( { -2.5f, +1.5f, -1.0f } );
	modelLibrary.at( "cube" ).transform.rotateYaw( -30.0f );
	modelLibrary.at( "cube2" ).transform.translate( { +2.5f, +1.5f, -1.0f } );
	modelLibrary.at( "cube2" ).transform.rotateYaw( +30.0f );
	modelLibrary.at( "quad" ).transform.setScale( 5.0f );
	modelLibrary.at( "quad" ).transform.translate( { +0.0f, -3.0f, -2.5f } );
	modelLibrary.at( "quad" ).transform.rotatePitch( +90.0f );

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