// Controls models of a scene as they exist in vector coordinates
// TODO LIST: Scene
#pragma once
#include <vector>
#include <map>
#include <random>
#include "vector_math.h"
#include "object_loader.h"

// Utility class used by other elements for spatial coordinate manipulation
class Transform
{
private:
	float pitch = 0.0f; // model x axis rotation
	float yaw = 0.0f; // model y axis rotation
	float roll = 0.0f; // model z axis rotation
	float3 position{ 0.0f, 0.0f, 0.0f };
	float scale = 1.0f; // model size scale
public:
	// Predicting some overflow problems here...
	void rotateYaw( const float &deg )
	{
		yaw = yaw + (deg);
		//if (yaw < 0.0f) { yaw += 360.0f; }
		//else if (yaw >= 360.0f) { yaw - 360.0f; }
	}

	void rotatePitch( const float &deg )
	{
		pitch = pitch + (deg);
		//if (pitch < 0.0f) { pitch += 360.0f; }
		//else if (pitch >= 360.0f) { pitch - 360.0f; }
	}

	void rotateRoll( const float &deg )
	{
		roll = roll + (deg);
		//if (roll < 0.0f) { roll += 360.0f; }
		//else if (roll >= 360.0f) { roll - 360.0f; }
	}

	// Moves position by vector
	void translate( const float3 &offset )
	{
		position += offset;
	}

	void setOrientation( float3 newOrientation)
	{
		pitch = newOrientation.x;
		yaw = newOrientation.y;
		roll = newOrientation.z;
	}

	void setPosition( float3 newPosition )
	{
		position = newPosition;
	}

	void setScale( float newScale )
	{
		scale = newScale;
	}

	// Rotates vector to match current orientaion
	float3 transformVector( const float3 &ihat, const float3 &jhat, const float3 &khat, const float3 &v )
	{
		// Multiply each unit vector by its component scalar and add it all together
		return (ihat * v.x) + (jhat * v.y) + (khat * v.z);
	}

	// Get unit vectors representing current orientation
	std::vector<float3> getlocalUnitVecs()
	{
		const float yawRads = (yaw / 180.0f) * num::pi;
		const float pitchRads = (pitch / 180.0f) * num::pi;
		const float rollRads = (roll / 180.0f) * num::pi;

		// -- Pitch -- spins counter-clockwise in the Z-Y plane
		float3 ihat_pitch( 1, 0, 0 );
		float3 jhat_pitch( 0, cos( pitchRads ), -sin( pitchRads ) );
		float3 khat_pitch( 0, sin( pitchRads ), cos( pitchRads ) );
		// -- Yaw -- spins clockwise in X-Z plane
		float3 ihat_yaw( cosf( yawRads ), 0, sinf( yawRads ) );
		float3 jhat_yaw( 0, 1, 0 );
		float3 khat_yaw( -sinf( yawRads ), 0, cosf( yawRads ) );
		// -- Roll -- spins clockwise in the X-Y plane
		float3 ihat_roll( cosf( rollRads ), sinf( rollRads ), 0 );
		float3 jhat_roll( -sinf( rollRads ), cosf( rollRads ), 0 );
		float3 khat_roll( 0, 0, 1 );

		// Combine all unit vectors
		float3 ihat_py = transformVector( ihat_yaw, jhat_yaw, khat_yaw, ihat_pitch );
		// --- Pitch --- | ---  Yaw  --- || ------  P * Y ------
		// [ i | j | k ]   [ i | j | k ]    [   i     j     k  ]
		// [ x         ]   [ x   x   x ]    [ ix*x  iy*x  iz*x ]
		// [ y         ]   [ y   y   y ] -> [ ix*y  iy*y  iz*y ]
		// [ z         ]   [ z   z   z ]    [ ix*z  iy*z  iz*z ]
		float3 jhat_py = transformVector( ihat_yaw, jhat_yaw, khat_yaw, jhat_pitch );
		// --- Pitch --- | ---  Yaw  --- || ------  P * Y ------
		// [ i | j | k ]   [ i | j | k ]    [   i     j     k  ]
		// [     x     ]   [ x   x   x ]    [ jx*x  jy*x  jz*x ]
		// [     y     ]   [ y   y   y ] -> [ jx*y  jy*y  jz*y ]
		// [     z     ]   [ z   z   z ]    [ jx*z  jy*z  jz*z ]
		float3 khat_py = transformVector( ihat_yaw, jhat_yaw, khat_yaw, khat_pitch );
		// --- Pitch --- | ---  Yaw  --- || ------  P * Y ------
		// [ i | j | k ]   [ i | j | k ]    [   i     j     k  ]
		// [         x ]   [ x   x   x ]    [ kx*x  ky*x  kz*x ]
		// [         y ]   [ y   y   y ] -> [ kx*y  ky*y  kz*y ]
		// [         z ]   [ z   z   z ]    [ kx*z  ky*z  kz*z ]
		float3 ihat = transformVector( ihat_roll, jhat_roll, khat_roll, ihat_py );
		// ------  P * Y ------ | --- Roll  --- || ------------ P * Y * R ------------
		// [   i     j     k  ]   [ i | j | k ]    [     i          j          k     ]
		// [ ix*x             ]   [ x   x   x ]    [ i(ix*x)*x  i(iy*x)*x  i(iz*x)*x ]
		// [ ix*y             ]   [ y   y   y ] -> [ i(ix*y)*y  i(iy*y)*y  i(iz*y)*y ]
		// [ ix*z             ]   [ z   z   z ]    [ i(ix*z)*z  i(iy*z)*z  i(iz*z)*z ]
		float3 jhat = transformVector( ihat_roll, jhat_roll, khat_roll, jhat_py );
		// ------  P * Y ------ | --- Roll  --- || ------------P * Y * R ------------
		// [   i     j     k  ]   [ i | j | k ]    [     i          j          k     ]
		// [       jy*x       ]   [ x   x   x ]    [ j(ix*x)*x  j(iy*x)*x  j(iz*x)*x ]
		// [       jy*y       ]   [ y   y   y ] -> [ j(ix*y)*y  j(iy*y)*y  j(iz*y)*y ]
		// [       jy*z       ]   [ z   z   z ]    [ j(ix*z)*z  j(iy*z)*z  j(iz*z)*z ]
		float3 khat = transformVector( ihat_roll, jhat_roll, khat_roll, khat_py );
		// ------  P * Y ------ | --- Roll  --- || ------------ P * Y * R ------------
		// [   i     j     k  ]   [ i | j | k ]    [     i          j          k     ]
		// [             kz*x ]   [ x   x   x ]    [ k(ix*x)*x  k(iy*x)*x  k(iz*x)*x ]
		// [             kz*y ]   [ y   y   y ] -> [ k(ix*y)*y  k(iy*y)*y  k(iz*y)*y ]
		// [             kz*z ]   [ z   z   z ]    [ k(ix*z)*z  k(iy*z)*z  k(iz*z)*z ]

		return std::vector<float3> {ihat, jhat, khat};
	}

	// Transforms vector based on current orientation and world position
	float3 toWorld( const float3 &p )
	{
		std::vector<float3> unitVectors = getlocalUnitVecs();
		return transformVector( unitVectors[0], unitVectors[1], unitVectors[2], p ) * scale + position;
	}
};

// Wrapper for object data and transformation
class Model
{
private:
	int size_;
	std::vector<float3> vertices_;
	std::vector<float3> triColors_;

public:
	Model( const std::vector<float3> &verts, const std::vector<float3> &cols )
	{
		size_ = verts.size();
		vertices_ = verts;
		triColors_ = cols;
	}

	// location, orientation
	Transform transform;

	// Generates list of random tri colors
	static std::vector<float3> generateColors(const int &triCount)
	{
		// Random color generator
		static std::random_device rd;
		static std::mt19937 gen( rd() );
		static std::uniform_real_distribution<float> distC( 0.0f, 255.0f );

		// Generate random colors for each tri...
		std::vector<float3> triColors( triCount );
		for ( int i = 0; i < triColors.size(); i++ ) {
			triColors[i] = float3{ distC( gen ), distC( gen ), distC( gen ) };
		}

		return triColors;
	}

	int size() const { return size_; }
	const std::vector<float3> &vertices() const { return vertices_; }
	const std::vector<float3> &triColors() const { return triColors_; }
};

// TODO: allow multiple instances of one object
// "Responsible for loading in models and moving them around"
class Scene
{
private:
	std::map<const char *, Model> modelLibrary_;
public:
	Scene( std::map<const char *, Model> modelLib )
	{
		modelLibrary_ = modelLib;
	}

	// Generates library of models from filenames
	static std::map<const char *, Model> generateLibrary( const std::vector<const char *> &filenames )
	{
		std::map<const char *, Model> modelLib;

		// Create model for each file
		for ( const char *filename : filenames ) {
			// Load object data from file
			std::vector<std::string> fileData = readObjFile( filename );
			std::vector<float3> vertices = loadObjFile( fileData );

			// Generate random colors for each tri in object
			std::vector<float3> triColors = Model::generateColors( vertices.size() / 3 );

			// Create model and add to library under filename
			modelLib.insert( { filename, Model( vertices, triColors ) } );
		}

		return modelLib;
	}

	// Add model to library under name
	void addModel( char *name, Model &model )
	{
		// Make sure model is not already in library
		if ( modelLibrary_.find( name ) != modelLibrary_.end() ) {
			throw "Model name is already in use!";
		}

		// Add model to library under name
		modelLibrary_.insert( { name, model } );
	}
	 
	// Get model by name
	Model &getModel( const char *name )
	{
		// Make sure model with name exists 
		if ( modelLibrary_.find( name ) == modelLibrary_.end() ) {
			throw "Model with name does not exist!";
		}

		// Add model to library under name
		return modelLibrary_.at( name );
	}

	// Gets list of all models in library
	const std::vector<Model> data() const
	{
		std::vector<Model> modelList;

		// Iterate through library and add each model to list
		for ( auto [name, model] : modelLibrary_ ) {
			modelList.push_back( model );
		}

		return modelList;
	}
};