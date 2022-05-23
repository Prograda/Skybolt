#ifndef CHECKER_H
#define CHECKER_H

// From https://www.geeks3d.com/hacklab/20190225/demo-checkerboard-in-glsl/
float checker(vec2 uv, float repeats) 
{
	float cx = floor(repeats * uv.x);
	float cy = floor(repeats * uv.y); 
	float result = mod(cx + cy, 2.0);
	return sign(result);
}

#endif // CHECKER_H