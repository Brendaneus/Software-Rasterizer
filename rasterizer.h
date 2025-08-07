// Draws scene elements to buffers using vector math
#pragma once
#include <optional>
#include <vector>
#include <algorithm>
#include <limits>
#include "scene.h"
#include "vector_math.h"

using namespace vec;
using f_limits = std::numeric_limits<float>;


// Image buffer for rasterization
class RenderTarget
{
private:
	int width_;
	int height_;
	float2 dimensions_;

public:
	std::vector<std::vector<float3>> colorBuffer;
	std::vector<std::vector<float>> depthBuffer;

	RenderTarget( const int &w, const int &h )
	{
		width_ = w;
		height_ = h;
		dimensions_ = float2( w, h );
		colorBuffer = std::vector<std::vector<float3>>( h, std::vector<float3>( w ) );
		depthBuffer = std::vector<std::vector<float>>( h, std::vector<float>( w ) );
	}
	
	// Resets entire color buffer to background color
	void clearColor( const std::optional<float3> &backgroundColor = std::nullopt )
	{
		static const std::vector<std::vector<float3>> bufferDefault( height_, std::vector<float3>( width_ ) );

		if ( backgroundColor.has_value() ) {
			colorBuffer = std::vector<std::vector<float3>>( height_, std::vector<float3>( width_, backgroundColor.value() ) );
		}
		else {
			colorBuffer = bufferDefault;
		}
	}

	void clearDepth()
	{
		static const std::vector<std::vector<float>> bufferDefault( height_, std::vector<float>( width_, f_limits::infinity() ) );
		depthBuffer = bufferDefault;
	}

	const int &width() const { return width_; }
	const int &height() const { return height_; }
	const float2 &dimensions() const { return dimensions_; }
};

// Move to header with Transform class? Move inside Transform class?
// Converts world coordinates to screen coordinates
float3 vertexToScreen( const float3 &vert, Transform transform, const float2 &pixDimensions, const float &fov )
{
	float3 vertex_world = transform.toWorld( vert );

	// Calculate screen-to-world scale
	// Visualized as double right triangle: FOV = 2 * angle, screen height = 2 * opposite side, adjacent side = 1 unit (depth)
	float screenHeight_world = tanf( fov / 360 * num::pi ) * 2;
	float pixelsPerWorldUnit = pixDimensions.y / screenHeight_world / vertex_world.z;

	// Return scaled x, y coords and actual z depth
	float2 pixelOffset = float2( vertex_world.x, vertex_world.y ) * pixelsPerWorldUnit;
	float2 vertex_screen = (pixDimensions / 2) + pixelOffset;
	return float3{ vertex_screen.x, vertex_screen.y, vertex_world.z };
}

// Sets all pixels in render target to background color
void clearImage( RenderTarget &target )
{
	float3 backgroundColor{ 0.0f, 0.0f, 0.0f };

	target.clearColor();
	target.clearDepth();
}
 
// Draws triangles onto image
void render( RenderTarget &target, const Model &model )
{
	// Draw 1 triangle at a time (three vertices)
	for ( int i = 0; i < model.size(); i += 3 ) {
		float3 vertA = vertexToScreen( model.vertices()[i + 0], model.transform, target.dimensions(), 60.0f );
		float3 vertB = vertexToScreen( model.vertices()[i + 1], model.transform, target.dimensions(), 60.0f );
		float3 vertC = vertexToScreen( model.vertices()[i + 2], model.transform, target.dimensions(), 60.0f );
		float2 flatA{ vertA };
		float2 flatB{ vertB };
		float2 flatC{ vertC };

		// Bounding box of triangle
		float minX = std::min( { vertA.x, vertB.x, vertC.x } );
		float minY = std::min( { vertA.y, vertB.y, vertC.y } );
		float maxX = std::max( { vertA.x, vertB.x, vertC.x } );
		float maxY = std::max( { vertA.y, vertB.y, vertC.y } );

		// Clamped pixel block of bounding box
		int blockStartX = std::max( (int)minX, 0 );
		int blockStartY = std::max( (int)minY, 0 );
		int blockEndX = std::min( (int)std::ceil( maxX ), target.width() );
		int blockEndY = std::min( (int)std::ceil( maxY ), target.height() );

		// TODO: Cache triangle calculations


		// Scan bounding box one pixel at a time
		for ( int y = blockStartY; y < blockEndY; y++ ) {
			for ( int x = blockStartX; x < blockEndX; x++ ) {
				float2 pixel = float2( x, y );
				float3 weights;

				// Color pixel if inside a visible triangle
				if ( pointInsideTriangle( flatA, flatB, flatC, pixel, weights ) ) {
					// Find depth by combining each vertex's weighted depth
					float3 depths{ vertA.z, vertB.z, vertC.z };
					float depth = depths.dot( weights );

					// Do not draw over closer pixels
					if ( depth > target.depthBuffer[y][x] ) { continue; }

					// Update pixel if nothing nearer has been drawn in the way
					target.colorBuffer[y][x] = model.triColors()[i / 3];
					target.depthBuffer[y][x] = depth;
				}
			}
		}
	}
}

void renderMany( RenderTarget &target, const std::vector<Model> &modelList )
{
	for ( const Model &model : modelList ) {
		render( target, model );
	}
}