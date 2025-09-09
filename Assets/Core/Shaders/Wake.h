#ifndef WAKE_H
#define WAKE_H

uniform samplerBuffer wakeHashMapTexture;
uniform samplerBuffer wakeParamsTexture;

// Evaluates a 2D Gaussian at point `x`
// - `sigma`: the standard deviation (spread / width)
// Returns the unnormalized Gaussian value at `x`
float sampleGaussian(float x, float sigma)
{
    float s2 = sigma * sigma;

    return exp(- x*x / (2.0 * s2));
}

float sampleDualGaussian(float x, float center, float sigma)
{
	return sampleGaussian(-x - center, sigma) + sampleGaussian(x - center, sigma);
}

// Biases x asymmetrically before applying a symmetric Gaussian
// - x: input position (assumed centered at 0)
// - sigma: width of the Gaussian
// - bias: [-1, 1], where 0 = symmetric, >0 = right-skewed, <0 = left-skewed
float sampleSkewedGaussian(float x, float sigma, float bias) {
    float b = clamp(bias, -0.999, 0.999);
    
    // Use asymmetric sigma based on sign, but preserve average width
    float sigmaL = sigma * (1.0 - b);
    float sigmaR = sigma * (1.0 + b);

    float s = x < 0.0 ? sigmaL : sigmaR;
    float s2 = s * s;

    return exp(-x * x / (2.0 * s2));
}

float sampleDualSkewedGaussian(float x, float center, float sigma, float bias)
{
	return sampleSkewedGaussian(-x - center, sigma, bias) + sampleSkewedGaussian(x - center, sigma, bias);
}

//! Evaluates the lateral variation in wake foam amount.
//! @param s is normalized lateral distance from wake center [0, 1].
float bowWakeLateralProfile(float s, float t)
{
	float peakCenter = mix(0.35, 0.35, t);
	float peakWidth = mix(0.05, 0.025, t);

	float wake = (sampleDualSkewedGaussian(s, peakCenter, peakWidth, -0.5));
	wake *= 1-t;

	return wake;
}

//! Evaluates the lateral variation in wake foam amount.
//! @param s is normalized lateral distance from wake center [0, 1].
float bowWakeDisplacementLateralProfile(float s, float t)
{
	float peakCenter = mix(0.3, 0.35, t);
	float peakWidth = mix(0.075, 0.025, t);

	float wake = (sampleDualSkewedGaussian(s, peakCenter, peakWidth, -0.4));
	wake *= 1-t;

	return wake;
}

//! Evaluates the lateral variation in wake foam amount.
//! @param s is normalized lateral distance from wake center [0, 1].
float sternWakeLateralProfile(float s, float t)
{
	float peakCenter = mix(0.6, 0.35, t);
	float peakWidth = mix(0.25, 0.22, t);

	float wake = (sampleDualSkewedGaussian(s, peakCenter, peakWidth, -0.6));

	wake *= 0.7 * (1-t);
	wake += 0.2 * (sampleDualGaussian(s, peakCenter, peakWidth*1.4)) * (1-t);

	return wake;
}

float bowWakeStrength = 0.8;
float propellerWakeStrength = 0.8;

// Project a 2D point `p` into a frame defined by origin `origin`
// and basis vectors `u` and `v`. Returns the coordinates of `p`
// in the frame (i.e., (x, y) such that p = origin + x*u + y*v).
vec2 projectToFrame(vec2 p, vec2 origin, vec2 u, vec2 v)
{
    vec2 d = p - origin;

    // Build a 2x2 matrix with u and v as columns
    mat2 basis = mat2(u, v);

    // Invert the basis matrix
    mat2 invBasis = inverse(basis);

    // Transform d into the frame's coordinate system
    return invBasis * d;
}

vec2 calcBowWakeMask(vec2 texCoord, float wakeStartWidth, float wakeEndWidth, float wakeLength)
{
	if (texCoord.y < -10) { return vec2(0); }

	float waveFrequency = 0.04; // waves per meter 
	float waveAmplitude = 0.02;

	// Add some wavy-ness to the wake
	texCoord.x *= 1 + sin(texCoord.y * waveFrequency * -2 * PI) * waveAmplitude;

	// Calculate parameteric coordinates
	float t = max(0, texCoord.y); // longitudional distance from start of wake
	float tNorm = t / wakeLength;

	float curveSmoothingFactor = 2;
	vec2 closestPointOnCenterline = vec2(0, t + curveSmoothingFactor);
	float s = length(distance(texCoord, closestPointOnCenterline)); // distance from central wake axis

	// Calculate wake half width at t
	float halfWidth = mix(wakeStartWidth, wakeEndWidth, pow(tNorm, 0.7)) * 0.5;
	float sNorm = s / halfWidth;
	
	// Add bands of lesser amount of foam
	float wake = bowWakeLateralProfile(sNorm, tNorm);
	float wakeDisplacement = bowWakeDisplacementLateralProfile(sNorm, tNorm);
	float displacement = wakeDisplacement + wakeDisplacement * sampleSkewedGaussian(texCoord.y-0.5, 5, 0.5);
	return vec2(wake, displacement);
}

vec2 calcSternWakeMask(vec2 texCoord, float wakeStartWidth, float wakeEndWidth, float wakeLength)
{
	if (texCoord.y < 0) { return vec2(0); }

	float waveFrequency = 0.04; // waves per meter 
	float waveAmplitude = 0.03;

	// Add some wavy-ness to the wake
	texCoord.x *= 1 + sin(texCoord.y * waveFrequency * -2 * PI) * waveAmplitude;

	// Calculate parameteric coordinates
	float s = abs(texCoord.x); // lateral distance from central wake axis
	float t = texCoord.y; // longitudional distance from start of wake
	float tNorm = t / wakeLength;

	// Calculate wake half width at t
	float halfWidth = mix(wakeStartWidth, wakeEndWidth, tNorm) * 0.5;

	// Add bands of lesser amount of foam
	float wake = sternWakeLateralProfile(s / halfWidth, tNorm);
	return vec2(wake);
}

float bowStartAheadDistance = 56;
float sternStartAheadDistance = -74;
float bowWakeLengthMultiplier = 0.5;

vec2 calcWakeMask(vec2 point, vec3 wakeStart, vec3 wakeEnd, float bowWakeStrength, float sternWakeStrength)
{
	vec2 wakeBasisBackward = normalize(wakeEnd.xy - wakeStart.xy);
	vec2 wakeBasisRight = vec2(wakeBasisBackward.y, -wakeBasisBackward.x);
	
	vec2 pointWakeSpace = projectToFrame(point, wakeStart.xy, wakeBasisRight, wakeBasisBackward);
	vec2 bowPointWakeSpace = vec2(pointWakeSpace.x, pointWakeSpace.y + bowStartAheadDistance);
	vec2 sternPointWakeSpace = vec2(pointWakeSpace.x, pointWakeSpace.y + sternStartAheadDistance);
	float startWidth = wakeStart.z * 2;
	float endWidth = wakeEnd.z * 2;
	
	return calcBowWakeMask(bowPointWakeSpace, startWidth*1.2, endWidth*5, distance(wakeStart.xy, wakeEnd.xy) * bowWakeLengthMultiplier) * bowWakeStrength
		+ calcSternWakeMask(sternPointWakeSpace, startWidth, endWidth, distance(wakeStart.xy, wakeEnd.xy)) * sternWakeStrength;
}

vec2 calcRotorWashMask(vec2 point, vec3 wakeStart, vec3 wakeEnd)
{
	vec2 wakeDir = normalize(wakeEnd.xy - wakeStart.xy);
	wakeStart.xy += wakeDir * 10;
	float elongate = dot(wakeDir, normalize(point.xy - wakeStart.xy)) * 0.5 + 0.5;
	float radius = length(wakeStart.xy - point);
	float foamRadius = mix(15, 25, elongate);
	float doughnut = 0.12 * (1.0 - 0.5 * elongate) * max(0.0, 1.0 - 0.05 * abs(radius - foamRadius)) * mix(0.2, 1.0, 0.5 * (1.0 + sin(radius)));
	return vec2(doughnut);
}

const int hashCellsX = 128;
vec2 cellWorldSize = vec2(100);

int modPositive(int a, int b)
{
	return (a%b+b)%b;
}

int calcHash(vec2 position)
{
	vec2 cellF = position / cellWorldSize;
	ivec2 cell = ivec2(floor(cellF));
	return modPositive(cell.x, hashCellsX) + modPositive(cell.y, hashCellsX) * hashCellsX;
}

// @returns [foamAmount, displacement]
vec2 calcWakeMask(vec2 point, float bowWakeStrength, float sternWakeStrength)
{
	int hashIndex = calcHash(point) * 2;
	int paramsBeginIndex = int(texelFetch(wakeHashMapTexture, hashIndex).x);
	int paramsEndIndex = int(texelFetch(wakeHashMapTexture, hashIndex + 1).x); // last index + 1

	vec2 mask = vec2(0);
	for (int i = paramsBeginIndex; i < paramsEndIndex; i += 2)
	{
		vec3 wakeStart = texelFetch(wakeParamsTexture, i).xyz;
		vec3 wakeEnd = texelFetch(wakeParamsTexture, i+1).xyz;
		if (wakeStart.z < 0.0)
		{
			mask = max(mask, calcRotorWashMask(point, wakeStart, wakeEnd));
		}
		else
		{
			mask = max(mask, calcWakeMask(point, wakeStart, wakeEnd, bowWakeStrength, sternWakeStrength));
		}
	}

	return mask;
}

#endif // WAKE_H