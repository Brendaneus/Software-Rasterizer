#pragma once
#include <vector>

// Gets size of all elements (x * y) in 2D vector
int sizeof2DVector(std::vector<std::vector<float3>> image)
{
	int size = 0;
	for (std::vector<float3> row : image) {
		size += row.size();
	}
	return size;
}