#define IN(x) const in x
#define OUT(x) out x
#define TEMPLATE(x)
#define TEMPLATE_ARGUMENT(x)
#define assert(x)
const int TRANSMITTANCE_TEXTURE_WIDTH = 256;
const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;
const int SCATTERING_TEXTURE_R_SIZE = 32;
const int SCATTERING_TEXTURE_MU_SIZE = 128;
const int SCATTERING_TEXTURE_MU_S_SIZE = 32;
const int SCATTERING_TEXTURE_NU_SIZE = 8;
const int IRRADIANCE_TEXTURE_WIDTH = 64;
const int IRRADIANCE_TEXTURE_HEIGHT = 16;
#define COMBINED_SCATTERING_TEXTURES
/**
 * Copyright (c) 2017 Eric Bruneton
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*<h2>atmosphere/definitions.glsl</h2>

<p>This GLSL file defines the physical types and constants which are used in the
main <a href="functions.glsl.html">functions</a> of our atmosphere model, in
such a way that they can be compiled by a GLSL compiler (a
<a href="reference/definitions.h.html">C++ equivalent</a> of this file
provides the same types and constants in C++, to allow the same functions to be
compiled by a C++ compiler - see the <a href="../index.html">Introduction</a>).

<h3>Physical quantities</h3>

<p>The physical quantities we need for our atmosphere model are
<a href="https://en.wikipedia.org/wiki/Radiometry">radiometric</a> and
<a href="https://en.wikipedia.org/wiki/Photometry_(optics)">photometric</a>
quantities. In GLSL we can't define custom numeric types to enforce the
homogeneity of expressions at compile time, so we define all the physical
quantities as <code>float</code>, with preprocessor macros (there is no
<code>typedef</code> in GLSL).

<p>We start with six base quantities: length, wavelength, angle, solid angle,
power and luminous power (wavelength is also a length, but we distinguish the
two for increased clarity).
*/

#define Length float
#define Wavelength float
#define Angle float
#define SolidAngle float
#define Power float
#define LuminousPower float

/*
<p>From this we "derive" the irradiance, radiance, spectral irradiance,
spectral radiance, luminance, etc, as well pure numbers, area, volume, etc (the
actual derivation is done in the <a href="reference/definitions.h.html">C++
equivalent</a> of this file).
*/

#define Number float
#define InverseLength float
#define Area float
#define Volume float
#define NumberDensity float
#define Irradiance float
#define Radiance float
#define SpectralPower float
#define SpectralIrradiance float
#define SpectralRadiance float
#define SpectralRadianceDensity float
#define ScatteringCoefficient float
#define InverseSolidAngle float
#define LuminousIntensity float
#define Luminance float
#define Illuminance float

/*
<p>We  also need vectors of physical quantities, mostly to represent functions
depending on the wavelength. In this case the vector elements correspond to
values of a function at some predefined wavelengths. Again, in GLSL we can't
define custom vector types to enforce the homogeneity of expressions at compile
time, so we define these vector types as <code>vec3</code>, with preprocessor
macros. The full definitions are given in the
<a href="reference/definitions.h.html">C++ equivalent</a> of this file).
*/

// A generic function from Wavelength to some other type.
#define AbstractSpectrum vec3
// A function from Wavelength to Number.
#define DimensionlessSpectrum vec3
// A function from Wavelength to SpectralPower.
#define PowerSpectrum vec3
// A function from Wavelength to SpectralIrradiance.
#define IrradianceSpectrum vec3
// A function from Wavelength to SpectralRadiance.
#define RadianceSpectrum vec3
// A function from Wavelength to SpectralRadianceDensity.
#define RadianceDensitySpectrum vec3
// A function from Wavelength to ScaterringCoefficient.
#define ScatteringSpectrum vec3

// A position in 3D (3 length values).
#define Position vec3
// A unit direction vector in 3D (3 unitless values).
#define Direction vec3
// A vector of 3 luminance values.
#define Luminance3 vec3
// A vector of 3 illuminance values.
#define Illuminance3 vec3

/*
<p>Finally, we also need precomputed textures containing physical quantities in
each texel. Since we can't define custom sampler types to enforce the
homogeneity of expressions at compile time in GLSL, we define these texture
types as <code>sampler2D</code> and <code>sampler3D</code>, with preprocessor
macros. The full definitions are given in the
<a href="reference/definitions.h.html">C++ equivalent</a> of this file).
*/

#define TransmittanceTexture sampler2D
#define AbstractScatteringTexture sampler3D
#define ReducedScatteringTexture sampler3D
#define ScatteringTexture sampler3D
#define ScatteringDensityTexture sampler3D
#define IrradianceTexture sampler2D

/*
<h3>Physical units</h3>

<p>We can then define the units for our six base physical quantities:
meter (m), nanometer (nm), radian (rad), steradian (sr), watt (watt) and lumen
(lm):
*/

const Length m = 1.0;
const Wavelength nm = 1.0;
const Angle rad = 1.0;
const SolidAngle sr = 1.0;
const Power watt = 1.0;
const LuminousPower lm = 1.0;

/*
<p>From which we can derive the units for some derived physical quantities,
as well as some derived units (kilometer km, kilocandela kcd, degree deg):
*/

const float PI = 3.14159265358979323846;

const Length km = 1000.0 * m;
const Area m2 = m * m;
const Volume m3 = m * m * m;
const Angle pi = PI * rad;
const Angle deg = pi / 180.0;
const Irradiance watt_per_square_meter = watt / m2;
const Radiance watt_per_square_meter_per_sr = watt / (m2 * sr);
const SpectralIrradiance watt_per_square_meter_per_nm = watt / (m2 * nm);
const SpectralRadiance watt_per_square_meter_per_sr_per_nm =
    watt / (m2 * sr * nm);
const SpectralRadianceDensity watt_per_cubic_meter_per_sr_per_nm =
    watt / (m3 * sr * nm);
const LuminousIntensity cd = lm / sr;
const LuminousIntensity kcd = 1000.0 * cd;
const Luminance cd_per_square_meter = cd / m2;
const Luminance kcd_per_square_meter = kcd / m2;

/*
The atmosphere parameters are then defined by the following struct:
*/

// The solar irradiance at the top of the atmosphere.
uniform IrradianceSpectrum solar_irradiance;
// The sun's angular radius. Warning: the implementation uses approximations
// that are valid only if this angle is smaller than 0.1 radians.
uniform Angle sun_angular_radius;
// The distance between the planet center and the bottom of the atmosphere.
uniform Length bottom_radius;
// The distance between the planet center and the top of the atmosphere.
uniform Length top_radius;
// The scattering coefficient of air molecules at the altitude where their
// density is maximum (usually the bottom of the atmosphere), as a function of
// wavelength. The scattering coefficient at altitude h is equal to
// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
uniform ScatteringSpectrum rayleigh_scattering;
// The scattering coefficient of aerosols at the altitude where their density
// is maximum (usually the bottom of the atmosphere), as a function of
// wavelength. The scattering coefficient at altitude h is equal to
// 'mie_scattering' times 'mie_density' at this altitude.
uniform ScatteringSpectrum mie_scattering;
// The asymetry parameter for the Cornette-Shanks phase function for the
// aerosols.
uniform Number mie_phase_function_g;
// The cosine of the maximum Sun zenith angle for which atmospheric scattering
// must be precomputed (for maximum precision, use the smallest Sun zenith
// angle yielding negligible sky light radiance values. For instance, for the
// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
uniform Number mu_s_min;

const vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(114974.916437,71305.954816,65310.548555);
const vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(98242.786222,69954.398112,66475.012354);
/*<h2>atmosphere/functions.glsl</h2>

<p>This GLSL file contains the core functions that implement our atmosphere
model. It provides functions to compute the transmittance, the single scattering
and the second and higher orders of scattering, the ground irradiance, as well
as functions to store these in textures and to read them back. It uses physical
types and constants which are provided in two versions: a
<a href="definitions.glsl.html">GLSL version</a> and a
<a href="reference/definitions.h.html">C++ version</a>. This allows this file to
be compiled either with a GLSL compiler or with a C++ compiler (see the
<a href="../index.html">Introduction</a>).

<p>The functions provided in this file are organized as follows:
<ul>
<li><a href="#transmittance">Transmittance</a>
<ul>
<li><a href="#transmittance_computation">Computation</a></li>
<li><a href="#transmittance_precomputation">Precomputation</a></li>
<li><a href="#transmittance_lookup">Lookup</a></li>
</ul>
</li>
<li><a href="#single_scattering">Single scattering</a>
<ul>
<li><a href="#single_scattering_computation">Computation</a></li>
<li><a href="#single_scattering_precomputation">Precomputation</a></li>
<li><a href="#single_scattering_lookup">Lookup</a></li>
</ul>
</li>
<li><a href="#multiple_scattering">Multiple scattering</a>
<ul>
<li><a href="#multiple_scattering_computation">Computation</a></li>
<li><a href="#multiple_scattering_precomputation">Precomputation</a></li>
<li><a href="#multiple_scattering_lookup">Lookup</a></li>
</ul>
</li>
<li><a href="#irradiance">Ground irradiance</a>
<ul>
<li><a href="#irradiance_computation">Computation</a></li>
<li><a href="#irradiance_precomputation">Precomputation</a></li>
<li><a href="#irradiance_lookup">Lookup</a></li>
</ul>
</li>
<li><a href="#rendering">Rendering</a>
<ul>
<li><a href="#rendering_sky">Sky</a></li>
<li><a href="#rendering_aerial_perspective">Aerial perspective</a></li>
<li><a href="#rendering_ground">Ground</a></li>
</ul>
</li>
</ul>

<p>They use the following utility functions to avoid NaNs due to floating point
values slightly outside their theoretical bounds:
*/

Number ClampCosine(Number mu) {
  return clamp(mu, Number(-1.0), Number(1.0));
}

Length ClampDistance(Length d) {
  return max(d, 0.0 * m);
}

Length ClampRadius(Length r) {
  return clamp(r, bottom_radius, top_radius);
}

Length SafeSqrt(Area a) {
  return sqrt(max(a, 0.0 * m2));
}

/*
<h3 id="transmittance">Transmittance</h3>

<p>As the light travels from a point $\bp$ to a point $\bq$ in the atmosphere,
it is partially absorbed and scattered out of its initial direction because of
the air molecules and the aerosol particles. Thus, the light arriving at $\bq$
is only a fraction of the light from $\bp$, and this fraction, which depends on
wavelength, is called the
<a href="https://en.wikipedia.org/wiki/Transmittance">transmittance</a>. The
following sections describe how we compute it, how we store it in a precomputed
texture, and how we read it back.

<h4 id="transmittance_computation">Computation</h4>

<p>For 3 aligned points $\bp$, $\bq$ and $\br$ inside the atmosphere, in this
order, the transmittance between $\bp$ and $\br$ is the product of the
transmittance between $\bp$ and $\bq$ and between $\bq$ and $\br$. In
particular, the transmittance between $\bp$ and $\bq$ is the transmittance
between $\bp$ and the nearest intersection $\bi$ of the half-line $[\bp,\bq)$
with the top or bottom atmosphere boundary, divided by the transmittance between
$\bq$ and $\bi$ (or 0 if the segment $[\bp,\bq]$ intersects the ground):

<svg width="340px" height="195px">
  <style type="text/css"><![CDATA[
    circle { fill: #000000; stroke: none; }
    path { fill: none; stroke: #000000; }
    text { font-size: 16px; font-style: normal; font-family: Sans; }
    .vector { font-weight: bold; }
  ]]></style>
  <path d="m 0,26 a 600,600 0 0 1 340,0"/>
  <path d="m 0,110 a 520,520 0 0 1 340,0"/>
  <path d="m 170,190 0,-30"/>
  <path d="m 170,140 0,-130"/>
  <path d="m 170,50 165,-33"/>
  <path d="m 155,150 10,-10 10,10 10,-10"/>
  <path d="m 155,160 10,-10 10,10 10,-10"/>
  <path d="m 95,50 30,0"/>
  <path d="m 95,190 30,0"/>
  <path d="m 105,50 0,140" style="stroke-dasharray:8,2;"/>
  <path d="m 100,55 5,-5 5,5"/>
  <path d="m 100,185 5,5 5,-5"/>
  <path d="m 170,25 a 25,25 0 0 1 25,20" style="stroke-dasharray:4,2;"/>
  <path d="m 170,190 70,0"/>
  <path d="m 235,185 5,5 -5,5"/>
  <path d="m 165,125 5,-5 5,5"/>
  <circle cx="170" cy="190" r="2.5"/>
  <circle cx="170" cy="50" r="2.5"/>
  <circle cx="320" cy="20" r="2.5"/>
  <circle cx="270" cy="30" r="2.5"/>
  <text x="155" y="45" class="vector">p</text>
  <text x="265" y="45" class="vector">q</text>
  <text x="320" y="15" class="vector">i</text>
  <text x="175" y="185" class="vector">o</text>
  <text x="90" y="125">r</text>
  <text x="185" y="25">Î¼=cos(Î¸)</text>
  <text x="240" y="185">x</text>
  <text x="155" y="120">z</text>
</svg>

<p>Also, the transmittance between $\bp$ and $\bq$ and between $\bq$ and $\bp$
are the same. Thus, to compute the transmittance between arbitrary points, it
is sufficient to know the transmittance between a point $\bp$ in the atmosphere,
and points $\bi$ on the top atmosphere boundary. This transmittance depends on
only two parameters, which can be taken as the radius $r=\Vert\bo\bp\Vert$ and
the cosine of the "view zenith angle",
$\mu=\bo\bp\cdot\bp\bi/\Vert\bo\bp\Vert\Vert\bp\bi\Vert$. To compute it, we
first need to compute the length $\Vert\bp\bi\Vert$, and we need to know when
the segment $[\bp,\bi]$ intersects the ground.

<h5>Distance to the top atmosphere boundary</h5>

<p>A point at distance $d$ from $\bp$ along $[\bp,\bi)$ has coordinates
$[d\sqrt{1-\mu^2}, r+d\mu]^\top$, whose squared norm is $d^2+2r\mu d+r^2$.
Thus, by definition of $\bi$, we have
$\Vert\bp\bi\Vert^2+2r\mu\Vert\bp\bi\Vert+r^2=r_{\mathrm{top}}^2$,
from which we deduce the length $\Vert\bp\bi\Vert$:
*/

Length DistanceToTopAtmosphereBoundary(
    Length r, Number mu) {
  assert(r <= top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  Area discriminant = r * r * (mu * mu - 1.0) +
      top_radius * top_radius;
  return ClampDistance(-r * mu + SafeSqrt(discriminant));
}

/*
<h5>Intersections with the ground</h5>

<p>The segment $[\bp,\bi]$ intersects the ground when
$d^2+2r\mu d+r^2=r_{\mathrm{bottom}}^2$ has a solution with $d \ge 0$. This
requires the discriminant $r^2(\mu^2-1)+r_{\mathrm{bottom}}^2$ to be positive,
from which we deduce the following function:
*/

bool RayIntersectsGround(
    Length r, Number mu) {
  assert(r >= bottom_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  return mu < 0.0 && r * r * (mu * mu - 1.0) +
      bottom_radius * bottom_radius >= 0.0 * m2;
}

/*
<h4 id="transmittance_precomputation">Precomputation</h4>

<p>The above function is quite costly to evaluate, and a lot of evaluations are
needed to compute single and multiple scattering. Fortunately this function
depends on only two parameters and is quite smooth, so we can precompute it in a
small 2D texture to optimize its evaluation.

<p>For this we need a mapping between the function parameters $(r,\mu)$ and the
texture coordinates $(u,v)$, and vice-versa, because these parameters do not
have the same units and range of values. And even if it was the case, storing a
function $f$ from the $[0,1]$ interval in a texture of size $n$ would sample the
function at $0.5/n$, $1.5/n$, ... $(n-0.5)/n$, because texture samples are at
the center of texels. Therefore, this texture would only give us extrapolated
function values at the domain boundaries ($0$ and $1$). To avoid this we need
to store $f(0)$ at the center of texel 0 and $f(1)$ at the center of texel
$n-1$. This can be done with the following mapping from values $x$ in $[0,1]$ to
texture coordinates $u$ in $[0.5/n,1-0.5/n]$ - and its inverse:
*/

Number GetTextureCoordFromUnitRange(Number x, int texture_size) {
  return 0.5 / Number(texture_size) + x * (1.0 - 1.0 / Number(texture_size));
}

Number GetUnitRangeFromTextureCoord(Number u, int texture_size) {
  return (u - 0.5 / Number(texture_size)) / (1.0 - 1.0 / Number(texture_size));
}

/*
<p>Using these functions, we can now define a mapping between $(r,\mu)$ and the
texture coordinates $(u,v)$, and its inverse, which avoid any extrapolation
during texture lookups. In the <a href=
"http://evasion.inrialpes.fr/~Eric.Bruneton/PrecomputedAtmosphericScattering2.zip"
>original implementation</a> this mapping was using some ad-hoc constants chosen
for the Earth atmosphere case. Here we use a generic mapping, working for any
atmosphere, but still providing an increased sampling rate near the horizon.
Our improved mapping is based on the parameterization described in our
<a href="https://hal.inria.fr/inria-00288758/en">paper</a> for the 4D textures:
we use the same mapping for $r$, and a slightly improved mapping for $\mu$
(considering only the case where the view ray does not intersect the ground).
More precisely, we map $\mu$ to a value $x_{\mu}$ between 0 and 1 by considering
the distance $d$ to the top atmosphere boundary, compared to its minimum and
maximum values $d_{\mathrm{min}}=r_{\mathrm{top}}-r$ and
$d_{\mathrm{max}}=\rho+H$ (cf. the notations from the
<a href="https://hal.inria.fr/inria-00288758/en">paper</a> and the figure
below):

<svg width="505px" height="195px">
  <style type="text/css"><![CDATA[
    circle { fill: #000000; stroke: none; }
    path { fill: none; stroke: #000000; }
    text { font-size: 16px; font-style: normal; font-family: Sans; }
    .vector { font-weight: bold; }
  ]]></style>
  <path d="m 5,85 a 520,520 0 0 1 372,105"/>
  <path d="m 5,5 a 600,600 0 0 1 490,185"/>
  <path d="m 60,0 0,190"/>
  <path d="m 60,65 180,-35"/>
  <path d="m 55,5 5,-5 5,5"/>
  <path d="m 55,60 5,5 5,-5"/>
  <path d="m 55,70 5,-5 5,5"/>
  <path d="m 60,40 a 25,25 0 0 1 25,20" style="stroke-dasharray:4,2;"/>
  <path d="m 60,65 415,105"/>
  <circle cx="60" cy="65" r="2.5"/>
  <circle cx="240" cy="30" r="2.5"/>
  <circle cx="180" cy="95" r="2.5"/>
  <circle cx="475" cy="170" r="2.5"/>
  <text x="20" y="40">d<tspan style="font-size:10px" dy="2">min</tspan></text>
  <text x="35" y="70" class="vector">p</text>
  <text x="35" y="125">r</text>
  <text x="75" y="40">Î¼=cos(Î¸)</text>
  <text x="120" y="75">Ï</text>
  <text x="155" y="60">d</text>
  <text x="315" y="125">H</text>
</svg>

<p>With these definitions, the mapping from $(r,\mu)$ to the texture coordinates
$(u,v)$ can be implemented as follows:
*/

vec2 GetTransmittanceTextureUvFromRMu(
    Length r, Number mu) {
  assert(r >= bottom_radius && r <= top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(top_radius * top_radius -
      bottom_radius * bottom_radius);
  // Distance to the horizon.
  Length rho =
      SafeSqrt(r * r - bottom_radius * bottom_radius);
  // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
  // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
  Length d = DistanceToTopAtmosphereBoundary(r, mu);
  Length d_min = top_radius - r;
  Length d_max = rho + H;
  Number x_mu = (d - d_min) / (d_max - d_min);
  Number x_r = rho / H;
  return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
              GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}

/*
<h4 id="transmittance_lookup">Lookup</h4>

<p>With the help of the above precomputed texture, we can now get the
transmittance between a point and the top atmosphere boundary with a single
texture lookup (assuming there is no intersection with the ground):
*/

DimensionlessSpectrum GetTransmittanceToTopAtmosphereBoundary(
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu) {
  assert(r >= bottom_radius && r <= top_radius);
  vec2 uv = GetTransmittanceTextureUvFromRMu(r, mu);
  return DimensionlessSpectrum(texture(transmittance_texture, uv));
}

/*
<p>Also, with $r_d=\Vert\bo\bq\Vert=\sqrt{d^2+2r\mu d+r^2}$ and $\mu_d=
\bo\bq\cdot\bp\bi/\Vert\bo\bq\Vert\Vert\bp\bi\Vert=(r\mu+d)/r_d$ the values of
$r$ and $\mu$ at $\bq$, we can get the transmittance between two arbitrary
points $\bp$ and $\bq$ inside the atmosphere with only two texture lookups
(recall that the transmittance between $\bp$ and $\bq$ is the transmittance
between $\bp$ and the top atmosphere boundary, divided by the transmittance
between $\bq$ and the top atmosphere boundary, or the reverse - we continue to
assume that the segment between the two points does not intersect the ground):
*/

DimensionlessSpectrum GetTransmittance(
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu, Length d, bool ray_r_mu_intersects_ground) {
  assert(r >= bottom_radius && r <= top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(d >= 0.0 * m);

  Length r_d = ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_d = ClampCosine((r * mu + d) / r_d);

  if (ray_r_mu_intersects_ground) {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            transmittance_texture, r_d, -mu_d) /
        GetTransmittanceToTopAtmosphereBoundary(
            transmittance_texture, r, -mu),
        DimensionlessSpectrum(1.0));
  } else {
    return min(
        GetTransmittanceToTopAtmosphereBoundary(
            transmittance_texture, r, mu) /
        GetTransmittanceToTopAtmosphereBoundary(
            transmittance_texture, r_d, mu_d),
        DimensionlessSpectrum(1.0));
  }
}

/*
<p>where <code>ray_r_mu_intersects_ground</code> should be true iif the ray
defined by $r$ and $\mu$ intersects the ground. We don't compute it here with
<code>RayIntersectsGround</code> because the result could be wrong for rays
very close to the horizon, due to the finite precision and rounding errors of
floating point operations. And also because the caller generally has more robust
ways to know whether a ray intersects the ground or not (see below).

<p>Finally, we will also need the transmittance between a point in the
atmosphere and the Sun. The Sun is not a point light source, so this is an
integral of the transmittance over the Sun disc. Here we consider that the
transmittance is constant over this disc, except below the horizon, where the
transmittance is 0. As a consequence, the transmittance to the Sun can be
computed with <code>GetTransmittanceToTopAtmosphereBoundary</code>, times the
fraction of the Sun disc which is above the horizon.

<p>This fraction varies from 0 when the Sun zenith angle $\theta_s$ is larger
than the horizon zenith angle $\theta_h$ plus the Sun angular radius $\alpha_s$,
to 1 when $\theta_s$ is smaller than $\theta_h-\alpha_s$. Equivalently, it
varies from 0 when $\mu_s=\cos\theta_s$ is smaller than
$\cos(\theta_h+\alpha_s)\approx\cos\theta_h-\alpha_s\sin\theta_h$ to 1 when
$\mu_s$ is larger than
$\cos(\theta_h-\alpha_s)\approx\cos\theta_h+\alpha_s\sin\theta_h$. In between,
the visible Sun disc fraction varies approximately like a smoothstep (this can
be verified by plotting the area of <a
href="https://en.wikipedia.org/wiki/Circular_segment">circular segment</a> as a
function of its <a href="https://en.wikipedia.org/wiki/Sagitta_(geometry)"
>sagitta</a>). Therefore, since $\sin\theta_h=r_{\mathrm{bottom}}/r$, we can
approximate the transmittance to the Sun with the following function:
*/

DimensionlessSpectrum GetTransmittanceToSun(
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu_s) {
  Number sin_theta_h = bottom_radius / r;
  Number cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
  return GetTransmittanceToTopAtmosphereBoundary(
          transmittance_texture, r, mu_s) *
      smoothstep(-sin_theta_h * sun_angular_radius / rad,
                 sin_theta_h * sun_angular_radius / rad,
                 mu_s - cos_theta_h);
}

/*
<p>Note that we added the solar irradiance and the scattering coefficient terms
that we omitted in <code>ComputeSingleScatteringIntegrand</code>, but not the
phase function terms - they are added at <a href="#rendering">render time</a>
for better angular precision. We provide them here for completeness:
*/

InverseSolidAngle RayleighPhaseFunction(Number nu) {
  InverseSolidAngle k = 3.0 / (16.0 * PI * sr);
  return k * (1.0 + nu * nu);
}

InverseSolidAngle MiePhaseFunction(Number g, Number nu) {
  InverseSolidAngle k = 3.0 / (8.0 * PI * sr) * (1.0 - g * g) / (2.0 + g * g);
  return k * (1.0 + nu * nu) / pow(1.0 + g * g - 2.0 * g * nu, 1.5);
}

/*
<h4 id="single_scattering_precomputation">Precomputation</h4>

<p>The <code>ComputeSingleScattering</code> function is quite costly to
evaluate, and a lot of evaluations are needed to compute multiple scattering.
We therefore want to precompute it in a texture, which requires a mapping from
the 4 function parameters to texture coordinates. Assuming for now that we have
4D textures, we need to define a mapping from $(r,\mu,\mu_s,\nu)$ to texture
coordinates $(u,v,w,z)$. The function below implements the mapping defined in
our <a href="https://hal.inria.fr/inria-00288758/en">paper</a>, with some small
improvements (refer to the paper and to the above figures for the notations):
<ul>
<li>the mapping for $\mu$ takes the minimal distance to the nearest atmosphere
boundary into account, to map $\mu$ to the full $[0,1]$ interval (the original
mapping was not covering the full $[0,1]$ interval).</li>
<li>the mapping for $\mu_s$ is more generic than in the paper (the original
mapping was using ad-hoc constants chosen for the Earth atmosphere case). It is
based on the distance to the top atmosphere boundary (for the sun rays), as for
the $\mu$ mapping, and uses only one ad-hoc (but configurable) parameter. Yet,
as the original definition, it provides an increased sampling rate near the
horizon.</li>
</ul>
*/

vec4 GetScatteringTextureUvwzFromRMuMuSNu(
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  assert(r >= bottom_radius && r <= top_radius);
  assert(mu >= -1.0 && mu <= 1.0);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  assert(nu >= -1.0 && nu <= 1.0);

  // Distance to top atmosphere boundary for a horizontal ray at ground level.
  Length H = sqrt(top_radius * top_radius -
      bottom_radius * bottom_radius);
  // Distance to the horizon.
  Length rho =
      SafeSqrt(r * r - bottom_radius * bottom_radius);
  Number u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);

  // Discriminant of the quadratic equation for the intersections of the ray
  // (r,mu) with the ground (see RayIntersectsGround).
  Length r_mu = r * mu;
  Area discriminant =
      r_mu * r_mu - r * r + bottom_radius * bottom_radius;
  Number u_mu;
  if (ray_r_mu_intersects_ground) {
    // Distance to the ground for the ray (r,mu), and its minimum and maximum
    // values over all mu - obtained for (r,-1) and (r,mu_horizon).
    Length d = -r_mu - SafeSqrt(discriminant);
    Length d_min = r - bottom_radius;
    Length d_max = rho;
    u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 :
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
  } else {
    // Distance to the top atmosphere boundary for the ray (r,mu), and its
    // minimum and maximum values over all mu - obtained for (r,1) and
    // (r,mu_horizon).
    Length d = -r_mu + SafeSqrt(discriminant + H * H);
    Length d_min = top_radius - r;
    Length d_max = rho + H;
    u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange(
        (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
  }

  Length d = DistanceToTopAtmosphereBoundary(
      bottom_radius, mu_s);
  Length d_min = top_radius - bottom_radius;
  Length d_max = H;
  Number a = (d - d_min) / (d_max - d_min);
  Number A =
      -2.0 * mu_s_min * bottom_radius / (d_max - d_min);
  Number u_mu_s = GetTextureCoordFromUnitRange(
      max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);

  Number u_nu = (nu + 1.0) / 2.0;
  return vec4(u_nu, u_mu_s, u_mu, u_r);
}

/*
<h4 id="single_scattering_lookup">Lookup</h4>

<p>With the help of the above precomputed texture, we can now get the scattering
between a point and the nearest atmosphere boundary with two texture lookups (we
need two 3D texture lookups to emulate a single 4D texture lookup with
quadrilinear interpolation; the 3D texture coordinates are computed using the
inverse of the 3D-4D mapping defined in
<code>GetRMuMuSNuFromScatteringTextureFragCoord</code>):
*/

TEMPLATE(AbstractSpectrum)
AbstractSpectrum GetScattering(
    IN(AbstractScatteringTexture TEMPLATE_ARGUMENT(AbstractSpectrum))
        scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground) {
  vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  vec3 uvw0 = vec3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  return AbstractSpectrum(texture(scattering_texture, uvw0) * (1.0 - lerp) +
      texture(scattering_texture, uvw1) * lerp);
}

/*
<p>Finally, we provide here a convenience lookup function which will be useful
in the next section. This function returns either the single scattering, with
the phase functions included, or the $n$-th order of scattering, with $n>1$. It
assumes that, if <code>scattering_order</code> is strictly greater than 1, then
<code>multiple_scattering_texture</code> corresponds to this scattering order,
with both Rayleigh and Mie included, as well as all the phase function terms.
*/

RadianceSpectrum GetScattering(
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    int scattering_order) {
  if (scattering_order == 1) {
    IrradianceSpectrum rayleigh = GetScattering(
        single_rayleigh_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    IrradianceSpectrum mie = GetScattering(
        single_mie_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
    return rayleigh * RayleighPhaseFunction(nu) +
        mie * MiePhaseFunction(mie_phase_function_g, nu);
  } else {
    return GetScattering(
        multiple_scattering_texture, r, mu, mu_s, nu,
        ray_r_mu_intersects_ground);
  }
}

/*
<h4 id="irradiance_precomputation">Precomputation</h4>

<p>In order to precompute the ground irradiance in a texture we need a mapping
from the ground irradiance parameters to texture coordinates. Since we
precompute the ground irradiance only for horizontal surfaces, this irradiance
depends only on $r$ and $\mu_s$, so we need a mapping from $(r,\mu_s)$ to
$(u,v)$ texture coordinates. The simplest, affine mapping is sufficient here,
because the ground irradiance function is very smooth:
*/

vec2 GetIrradianceTextureUvFromRMuS(
    Length r, Number mu_s) {
  assert(r >= bottom_radius && r <= top_radius);
  assert(mu_s >= -1.0 && mu_s <= 1.0);
  Number x_r = (r - bottom_radius) /
      (top_radius - bottom_radius);
  Number x_mu_s = mu_s * 0.5 + 0.5;
  return vec2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH),
              GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}

/*
<h4 id="irradiance_lookup">Lookup</h4>

<p>Thanks to these precomputed textures, we can now get the ground irradiance
with a single texture lookup:
*/

IrradianceSpectrum GetIrradiance(
    IN(IrradianceTexture) irradiance_texture,
    Length r, Number mu_s) {
  vec2 uv = GetIrradianceTextureUvFromRMuS(r, mu_s);
  return IrradianceSpectrum(texture(irradiance_texture, uv));
}

/*
<h3 id="rendering">Rendering</h3>

<p>Here we assume that the transmittance, scattering and irradiance textures
have been precomputed, and we provide functions using them to compute the sky
color, the aerial perspective, and the ground radiance.

<p>More precisely, we assume that the single Rayleigh scattering, without its
phase function term, plus the multiple scattering terms (divided by the Rayleigh
phase function for dimensional homogeneity) are stored in a
<code>scattering_texture</code>. We also assume that the single Mie scattering
is stored, without its phase function term:
<ul>
<li>either separately, in a <code>single_mie_scattering_texture</code> (this
option was not provided our <a href=
"http://evasion.inrialpes.fr/~Eric.Bruneton/PrecomputedAtmosphericScattering2.zip"
>original implementation</a>),</li>
<li>or, if the <code>COMBINED_SCATTERING_TEXTURES</code> preprocessor
macro is defined, in the <code>scattering_texture</code>. In this case, which is
only available with a GLSL compiler, Rayleigh and multiple scattering are stored
in the RGB channels, and the red component of the single Mie scattering is
stored in the alpha channel).</li>
</ul>

<p>In the second case, the green and blue components of the single Mie
scattering are extrapolated as described in our
<a href="https://hal.inria.fr/inria-00288758/en">paper</a>, with the following
function:
*/

#ifdef COMBINED_SCATTERING_TEXTURES
vec3 GetExtrapolatedSingleMieScattering(
    IN(vec4) scattering) {
  // Algebraically this can never be negative, but rounding errors can produce
  // that effect for sufficiently short view rays.
  if (scattering.r <= 0.0) {
    return vec3(0.0);
  }
  return scattering.rgb * scattering.a / scattering.r *
	    (rayleigh_scattering.r / mie_scattering.r) *
	    (mie_scattering / rayleigh_scattering);
}
#endif

/*
<p>We can then retrieve all the scattering components (Rayleigh + multiple
scattering on one side, and single Mie scattering on the other side) with the
following function, based on
<a href="#single_scattering_lookup"><code>GetScattering</code></a> (we duplicate
some code here, instead of using two calls to <code>GetScattering</code>, to
make sure that the texture coordinates computation is shared between the lookups
in <code>scattering_texture</code> and
<code>single_mie_scattering_texture</code>):
*/

IrradianceSpectrum GetCombinedScattering(
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    OUT(IrradianceSpectrum) single_mie_scattering) {
  vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(
      r, mu, mu_s, nu, ray_r_mu_intersects_ground);
  Number tex_coord_x = uvwz.x * Number(SCATTERING_TEXTURE_NU_SIZE - 1);
  Number tex_x = floor(tex_coord_x);
  Number lerp = tex_coord_x - tex_x;
  vec3 uvw0 = vec3((tex_x + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
  vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / Number(SCATTERING_TEXTURE_NU_SIZE),
      uvwz.z, uvwz.w);
#ifdef COMBINED_SCATTERING_TEXTURES
  vec4 combined_scattering =
      texture(scattering_texture, uvw0) * (1.0 - lerp) +
      texture(scattering_texture, uvw1) * lerp;
  IrradianceSpectrum scattering = IrradianceSpectrum(combined_scattering);
  single_mie_scattering =
      GetExtrapolatedSingleMieScattering(combined_scattering);
#else
  IrradianceSpectrum scattering = IrradianceSpectrum(
      texture(scattering_texture, uvw0) * (1.0 - lerp) +
      texture(scattering_texture, uvw1) * lerp);
  single_mie_scattering = IrradianceSpectrum(
      texture(single_mie_scattering_texture, uvw0) * (1.0 - lerp) +
      texture(single_mie_scattering_texture, uvw1) * lerp);
#endif
  return scattering;
}

/*
<h4 id="rendering_sky">Sky</h4>

<p>To render the sky we simply need to display the sky radiance, which we can
get with a lookup in the precomputed scattering texture(s), multiplied by the
phase function terms that were omitted during precomputation. We can also return
the transmittance of the atmosphere (which we can get with a single lookup in
the precomputed transmittance texture), which is needed to correctly render the
objects in space (such as the Sun and the Moon). This leads to the following
function, where most of the computations are used to correctly handle the case
of viewers outside the atmosphere, and the case of light shafts:
*/

RadianceSpectrum GetSkyRadiance(
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Position camera, IN(Direction) view_ray, Length shadow_length,
    IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance) {
  // Compute the distance to the top atmosphere boundary along the view ray,
  // assuming the viewer is in space (or NaN if the view ray does not intersect
  // the atmosphere).
  Length r = length(camera);
  Length rmu = dot(camera, view_ray);
  Length distance_to_top_atmosphere_boundary = -rmu -
      sqrt(rmu * rmu - r * r + top_radius * top_radius);
  // If the viewer is in space and the view ray intersects the atmosphere, move
  // the viewer to the top atmosphere boundary (along the view ray):
  if (distance_to_top_atmosphere_boundary > 0.0 * m) {
    camera = camera + view_ray * distance_to_top_atmosphere_boundary;
    r = top_radius;
    rmu += distance_to_top_atmosphere_boundary;
  } else if (r > top_radius) {
    // If the view ray does not intersect the atmosphere, simply return 0.
    transmittance = DimensionlessSpectrum(1.0);
    return RadianceSpectrum(0.0 * watt_per_square_meter_per_sr_per_nm);
  }
  // Compute the r, mu, mu_s and nu parameters needed for the texture lookups.
  Number mu = rmu / r;
  Number mu_s = dot(camera, sun_direction) / r;
  Number nu = dot(view_ray, sun_direction);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(r, mu);

  transmittance = ray_r_mu_intersects_ground ? DimensionlessSpectrum(0.0) :
      GetTransmittanceToTopAtmosphereBoundary(
          transmittance_texture, r, mu);
  IrradianceSpectrum single_mie_scattering;
  IrradianceSpectrum scattering;
  if (shadow_length == 0.0 * m) {
    scattering = GetCombinedScattering(
        scattering_texture, single_mie_scattering_texture,
        r, mu, mu_s, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
  } else {
    // Case of light shafts (shadow_length is the total length noted l in our
    // paper): we omit the scattering between the camera and the point at
    // distance l, by implementing Eq. (18) of the paper (shadow_transmittance
    // is the T(x,x_s) term, scattering is the S|x_s=x+lv term).
    Length d = shadow_length;
    Length r_p =
        ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
    Number mu_p = (r * mu + d) / r_p;
    Number mu_s_p = (r * mu_s + d * nu) / r_p;

    scattering = GetCombinedScattering(
        scattering_texture, single_mie_scattering_texture,
        r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
        single_mie_scattering);
    DimensionlessSpectrum shadow_transmittance =
        GetTransmittance(transmittance_texture,
            r, mu, shadow_length, ray_r_mu_intersects_ground);
    scattering = scattering * shadow_transmittance;
    single_mie_scattering = single_mie_scattering * shadow_transmittance;
  }
  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(mie_phase_function_g, nu);
}

/*
<h4 id="rendering_aerial_perspective">Aerial perspective</h4>

<p>To render the aerial perspective we need the transmittance and the scattering
between two points (i.e. between the viewer and a point on the ground, which can
at an arbibrary altitude). We already have a function to compute the
transmittance between two points (using 2 lookups in a texture which only
contains the transmittance to the top of the atmosphere), but we don't have one
for the scattering between 2 points. Hopefully, the scattering between 2 points
can be computed from two lookups in a texture which contains the scattering to
the nearest atmosphere boundary, as for the transmittance (except that here the
two lookup results must be subtracted, instead of divided). This is what we
implement in the following function (the initial computations are used to
correctly handle the case of viewers outside the atmosphere):
*/

RadianceSpectrum GetSkyRadianceToPoint(

    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Position camera, IN(Position) point, Length shadow_length,
    IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance) {
  // Compute the distance to the top atmosphere boundary along the view ray,
  // assuming the viewer is in space (or NaN if the view ray does not intersect
  // the atmosphere).
  Direction view_ray = normalize(point - camera);
  Length r = length(camera);
  Length rmu = dot(camera, view_ray);
  Length distance_to_top_atmosphere_boundary = -rmu -
      sqrt(rmu * rmu - r * r + top_radius * top_radius);
  // If the viewer is in space and the view ray intersects the atmosphere, move
  // the viewer to the top atmosphere boundary (along the view ray):
  if (distance_to_top_atmosphere_boundary > 0.0 * m) {
    camera = camera + view_ray * distance_to_top_atmosphere_boundary;
    r = top_radius;
    rmu += distance_to_top_atmosphere_boundary;
  }

  // Compute the r, mu, mu_s and nu parameters for the first texture lookup.
  Number mu = rmu / r;
  Number mu_s = dot(camera, sun_direction) / r;
  Number nu = dot(view_ray, sun_direction);
  Length d = length(point - camera);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(r, mu);

  transmittance = GetTransmittance(transmittance_texture,
      r, mu, d, ray_r_mu_intersects_ground);

  IrradianceSpectrum single_mie_scattering;
  IrradianceSpectrum scattering = GetCombinedScattering(
      scattering_texture, single_mie_scattering_texture,
      r, mu, mu_s, nu, ray_r_mu_intersects_ground,
      single_mie_scattering);

  // Compute the r, mu, mu_s and nu parameters for the second texture lookup.
  // If shadow_length is not 0 (case of light shafts), we want to ignore the
  // scattering along the last shadow_length meters of the view ray, which we
  // do by subtracting shadow_length from d (this way scattering_p is equal to
  // the S|x_s=x_0-lv term in Eq. (17) of our paper).
  d = max(d - shadow_length, 0.0 * m);
  Length r_p = ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
  Number mu_p = (r * mu + d) / r_p;
  Number mu_s_p = (r * mu_s + d * nu) / r_p;

  IrradianceSpectrum single_mie_scattering_p;
  IrradianceSpectrum scattering_p = GetCombinedScattering(
      scattering_texture, single_mie_scattering_texture,
      r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
      single_mie_scattering_p);

  // Combine the lookup results to get the scattering between camera and point.
  DimensionlessSpectrum shadow_transmittance = transmittance;
  if (shadow_length > 0.0 * m) {
    // This is the T(x,x_s) term in Eq. (17) of our paper, for light shafts.
    shadow_transmittance = GetTransmittance(transmittance_texture,
        r, mu, d, ray_r_mu_intersects_ground);
  }
  scattering = scattering - shadow_transmittance * scattering_p;
  single_mie_scattering =
      single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
#ifdef COMBINED_SCATTERING_TEXTURES
  single_mie_scattering = GetExtrapolatedSingleMieScattering(
      vec4(scattering, single_mie_scattering.r));
#endif

  // Hack to avoid rendering artifacts when the sun is below the horizon.
  single_mie_scattering = single_mie_scattering *
      smoothstep(Number(0.0), Number(0.01), mu_s);

  return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(mie_phase_function_g, nu);
}

/*
<h4 id="rendering_ground">Ground</h4>

<p>To render the ground we need the irradiance received on the ground after 0 or
more bounce(s) in the atmosphere or on the ground. The direct irradiance can be
computed with a lookup in the transmittance texture,
via <code>GetTransmittanceToSun</code>, while the indirect irradiance is given
by a lookup in the precomputed irradiance texture (this texture only contains
the irradiance for horizontal surfaces; we use the approximation defined in our
<a href="https://hal.inria.fr/inria-00288758/en">paper</a> for the other cases).
The function below returns the direct and indirect irradiances separately:
*/

IrradianceSpectrum GetSunAndSkyIrradiance(
    IN(TransmittanceTexture) transmittance_texture,
    IN(IrradianceTexture) irradiance_texture,
    IN(Position) point, IN(Direction) normal, IN(Direction) sun_direction,
    OUT(IrradianceSpectrum) sky_irradiance) {
  Length r = length(point);
  Number mu_s = dot(point, sun_direction) / r;

  // Indirect irradiance (approximated if the surface is not horizontal).
  sky_irradiance = GetIrradiance(irradiance_texture, r, mu_s) *
      (1.0 + dot(normal, point) / r) * 0.5;

  // Direct irradiance.
  return solar_irradiance *
      GetTransmittanceToSun(
          transmittance_texture, r, mu_s) *
      max(dot(normal, sun_direction), 0.0);
}
#define RADIANCE_API_ENABLED