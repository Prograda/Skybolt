#pragma once

#include <glm/glm.hpp>

// From https://github.com/sebh/TileableVolumeNoise
/*
The MIT License (MIT)

Copyright(c) 2017 Sébastien Hillaire

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

class Tileable3dNoise
{
public:

	/// @return Tileable Worley noise value in [0, 1].
	/// @param p 3d coordinate in [0, 1], being the range of the repeatable pattern.
	/// @param cellCount the number of cell for the repetitive pattern.
	static float WorleyNoise(const glm::vec3& p, float cellCount);

	/// @return Tileable Perlin noise value in [0, 1].
	/// @param p 3d coordinate in [0, 1], being the range of the repeatable pattern.
	static float PerlinNoise(const glm::vec3& p, float frequency, int octaveCount);

private:

	///
	/// Worley noise function based on https://www.shadertoy.com/view/Xl2XRR by Marc-Andre Loyer
	///

	static float hash(float n);
	static float noise(const glm::vec3& x);
	static float Cells(const glm::vec3& p, float numCells);

};

#pragma once
