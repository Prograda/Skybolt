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

/*<h2>atmosphere/model.cc</h2>

<p>This file implements the <a href="model.h.html">API of our atmosphere
model</a>. Its main role is to precompute the transmittance, scattering and
irradiance textures. The GLSL functions to precompute them are provided in
<a href="functions.glsl.html">functions.glsl</a>, but they are not sufficient.
They must be used in fully functional shaders and programs, and these programs
must be called in the correct order, with the correct input and output textures
(via framebuffer objects), to precompute each scattering order in sequence, as
described in Algorithm 4.1 of
<a href="https://hal.inria.fr/inria-00288758/en">our paper</a>. This is the role
of the following C++ code.
*/

#include "model.h"
#include "constants.h"

#include <SkyboltVis/Shader/OsgShaderHelpers.h>

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <memory>

/*
<p>The rest of this file is organized in 3 parts:
<ul>
<li>the <a href="#shaders">first part</a> defines the shaders used to precompute
the atmospheric textures,</li>
<li>the <a href="#utilities">second part</a> provides utility classes and
functions used to compile shaders, create textures, draw quads, etc,</li>
<li>the <a href="#implementation">third part</a> provides the actual
implementation of the <code>Model</code> class, using the above tools.</li>
</ul>

<h3 id="shaders">Shader definitions</h3>

<p>In order to precompute a texture we attach it to a framebuffer object (FBO)
and we render a full quad in this FBO. For this we need a basic vertex shader:
*/

namespace atmosphere {

double Interpolate(
	const std::vector<double>& wavelengths,
	const std::vector<double>& wavelength_function,
	double wavelength) {
	assert(wavelength_function.size() == wavelengths.size());
	if (wavelength < wavelengths[0]) {
		return wavelength_function[0];
	}
	for (unsigned int i = 0; i < wavelengths.size() - 1; ++i) {
		if (wavelength < wavelengths[i + 1]) {
			double u =
				(wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
			return
				wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
		}
	}
	return wavelength_function[wavelength_function.size() - 1];
}

namespace {

const char kVertexShader[] = R"(
    #version 330
    in vec4 osg_Vertex;
    void main() {
      gl_Position = vec4(osg_Vertex.xy * vec2(2) - vec2(1), 0, 1);
    })";

/*
<p>a basic geometry shader (only for 3D textures, to specify in which layer we
want to write):
*/

const char kGeometryShader[] = R"(
    #version 330
    layout(triangles) in;
    layout(triangle_strip, max_vertices = 3) out;
    uniform int layer;
    void main() {
      gl_Position = gl_in[0].gl_Position;
      gl_Layer = layer;
      EmitVertex();
      gl_Position = gl_in[1].gl_Position;
      gl_Layer = layer;
      EmitVertex();
      gl_Position = gl_in[2].gl_Position;
      gl_Layer = layer;
      EmitVertex();
      EndPrimitive();
    })";

/*
<p>and a fragment shader, which depends on the texture we want to compute. This
is the role of the following shaders, which simply wrap the precomputation
functions from <a href="functions.glsl.html">functions.glsl</a> in complete
shaders (with a <code>main</code> function and a proper declaration of the
shader inputs and outputs). Note that these strings must be concatenated with
<code>definitions.glsl</code> and <code>functions.glsl</code> (provided as C++
string literals by the generated <code>.glsl.inc</code> files), as well as with
a definition of the <code>ATMOSPHERE</code> constant - containing the atmosphere
parameters, to really get a complete shader. Note also the
<code>luminance_from_radiance</code> uniforms: these are used in precomputed
illuminance mode to convert the radiance values computed by the
<code>functions.glsl</code> functions to luminance values (see the
<code>Init</code> method for more details).
*/

const char kComputeTransmittanceShader[] = R"(
    out vec3 transmittance;
    void main() {
      transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(
          ATMOSPHERE, gl_FragCoord.xy);
    })";

const char kComputeDirectIrradianceShader[] = R"(
    layout(location = 0) out vec3 delta_irradiance;
    layout(location = 1) out vec3 irradiance;
    uniform sampler2D transmittance_texture;
    void main() {
      delta_irradiance = ComputeDirectIrradianceTexture(
          ATMOSPHERE, transmittance_texture, gl_FragCoord.xy);
      irradiance = vec3(0.0);
    })";

const char kComputeSingleScatteringShader[] = R"(
    layout(location = 0) out vec3 delta_rayleigh;
    layout(location = 1) out vec3 delta_mie;
    layout(location = 2) out vec4 scattering;
    layout(location = 3) out vec3 single_mie_scattering;
    uniform mat3 luminance_from_radiance;
    uniform sampler2D transmittance_texture;
    uniform int layer;
    void main() {
      ComputeSingleScatteringTexture(
          ATMOSPHERE, transmittance_texture, vec3(gl_FragCoord.xy, layer + 0.5),
          delta_rayleigh, delta_mie);
      scattering = vec4(luminance_from_radiance * delta_rayleigh.rgb,
          (luminance_from_radiance * delta_mie).r);
      single_mie_scattering = luminance_from_radiance * delta_mie;
    })";

const char kComputeScatteringDensityShader[] = R"(
    layout(location = 0) out vec3 scattering_density;
    uniform sampler2D transmittance_texture;
    uniform sampler3D single_rayleigh_scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler3D multiple_scattering_texture;
    uniform sampler2D irradiance_texture;
    uniform int scattering_order;
    uniform int layer;
    void main() {
      scattering_density = ComputeScatteringDensityTexture(
          ATMOSPHERE, transmittance_texture, single_rayleigh_scattering_texture,
          single_mie_scattering_texture, multiple_scattering_texture,
          irradiance_texture, vec3(gl_FragCoord.xy, layer + 0.5),
          scattering_order);
    })";

const char kComputeIndirectIrradianceShader[] = R"(
    layout(location = 0) out vec3 delta_irradiance;
    layout(location = 1) out vec3 irradiance;
    uniform mat3 luminance_from_radiance;
    uniform sampler3D single_rayleigh_scattering_texture;
    uniform sampler3D single_mie_scattering_texture;
    uniform sampler3D multiple_scattering_texture;
    uniform int scattering_order;
    void main() {
      delta_irradiance = ComputeIndirectIrradianceTexture(
          ATMOSPHERE, single_rayleigh_scattering_texture,
          single_mie_scattering_texture, multiple_scattering_texture,
          gl_FragCoord.xy, scattering_order);
      irradiance = luminance_from_radiance * delta_irradiance;
    })";

const char kComputeMultipleScatteringShader[] = R"(
    layout(location = 0) out vec3 delta_multiple_scattering;
    layout(location = 1) out vec4 scattering;
    uniform mat3 luminance_from_radiance;
    uniform sampler2D transmittance_texture;
    uniform sampler3D scattering_density_texture;
    uniform int layer;
    void main() {
      float nu;
      delta_multiple_scattering = ComputeMultipleScatteringTexture(
          ATMOSPHERE, transmittance_texture, scattering_density_texture,
          vec3(gl_FragCoord.xy, layer + 0.5), nu);
      scattering = vec4(
          luminance_from_radiance *
              delta_multiple_scattering.rgb / RayleighPhaseFunction(nu),
          0.0);
    })";

osg::ref_ptr<osg::Texture2D> NewTexture2d(int width, int height) {

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
	texture->setTextureWidth(width);
	texture->setTextureHeight(height);
	texture->setInternalFormat(GL_RGBA32F_ARB);
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	return texture;
}

osg::ref_ptr<osg::Texture3D> NewTexture3d(int width, int height, int depth, GLenum format,
    bool half_precision) {

	osg::ref_ptr<osg::Texture3D> texture = new osg::Texture3D();
	texture->setTextureWidth(width);
	texture->setTextureHeight(height);
	texture->setTextureDepth(depth);
	texture->setFilter(osg::Texture::FilterParameter::MIN_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setFilter(osg::Texture::FilterParameter::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

	GLenum internal_format = format == GL_RGBA ?
		(half_precision ? GL_RGBA16F_ARB : GL_RGBA32F_ARB) :
		(half_precision ? GL_RGB16F_ARB : GL_RGB32F_ARB);

	texture->setInternalFormat(internal_format);
  return texture;
}

/*
<p>Finally, we need a utility function to compute the value of the conversion
constants *<code>_RADIANCE_TO_LUMINANCE</code>, used above to convert the
spectral results into luminance values. These are the constants k_r, k_g, k_b
described in Section 14.3 of <a href="https://arxiv.org/pdf/1612.04336.pdf">A
Qualitative and Quantitative Evaluation of 8 Clear Sky Models</a>.

<p>Computing their value requires an integral of a function times a CIE color
matching function. Thus, we first need functions to interpolate an arbitrary
function (specified by some samples), and a CIE color matching function
(specified by tabulated values), at an arbitrary wavelength. This is the purpose
of the following two functions:
*/

constexpr int kLambdaMin = 360;
constexpr int kLambdaMax = 830;

double CieColorMatchingFunctionTableValue(double wavelength, int column) {
  if (wavelength <= kLambdaMin || wavelength >= kLambdaMax) {
    return 0.0;
  }
  double u = (wavelength - kLambdaMin) / 5.0;
  int row = static_cast<int>(std::floor(u));
  assert(row >= 0 && row + 1 < 95);
  assert(CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength &&
         CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
  u -= row;
  return CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) +
      CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
}

/*
<p>We can then implement a utility function to compute the "spectral radiance to
luminance" conversion constants (see Section 14.3 in <a
href="https://arxiv.org/pdf/1612.04336.pdf">A Qualitative and Quantitative
Evaluation of 8 Clear Sky Models</a> for their definitions):
*/

// The returned constants are in lumen.nm / watt.
void ComputeSpectralRadianceToLuminanceFactors(
    const std::vector<double>& wavelengths,
    const std::vector<double>& solar_irradiance,
    double lambda_power, double* k_r, double* k_g, double* k_b) {
  *k_r = 0.0;
  *k_g = 0.0;
  *k_b = 0.0;
  double solar_r = Interpolate(wavelengths, solar_irradiance, Model::kLambdaR);
  double solar_g = Interpolate(wavelengths, solar_irradiance, Model::kLambdaG);
  double solar_b = Interpolate(wavelengths, solar_irradiance, Model::kLambdaB);
  int dlambda = 1;
  for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
    double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
    double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
    double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
    const double* xyz2srgb = XYZ_TO_SRGB;
    double r_bar =
        xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
    double g_bar =
        xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
    double b_bar =
        xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
    double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);
    *k_r += r_bar * irradiance / solar_r *
        pow(lambda / Model::kLambdaR, lambda_power);
    *k_g += g_bar * irradiance / solar_g *
        pow(lambda / Model::kLambdaG, lambda_power);
    *k_b += b_bar * irradiance / solar_b *
        pow(lambda / Model::kLambdaB, lambda_power);
  }
  *k_r *= MAX_LUMINOUS_EFFICACY * dlambda;
  *k_g *= MAX_LUMINOUS_EFFICACY * dlambda;
  *k_b *= MAX_LUMINOUS_EFFICACY * dlambda;
}

}  // anonymous namespace

/*<h3 id="implementation">Model implementation</h3>

<p>Using the above utility functions and classes, we can now implement the
constructor of the <code>Model</code> class. This constructor generates a piece
of GLSL code that defines an <code>ATMOSPHERE</code> constant containing the
atmosphere parameters (we use constants instead of uniforms to enable constant
folding and propagation optimizations in the GLSL compiler), concatenated with
<a href="functions.glsl.html">functions.glsl</a>, and with
<code>kAtmosphereShader</code>, to get the shader exposed by our API in
<code>GetShader</code>. It also allocates the precomputed textures (but does not
initialize them), as well as a vertex buffer object to render a full screen quad
(used to render into the precomputed textures).
*/

Model::Model(
    const std::vector<double>& wavelengths,
    const std::vector<double>& solar_irradiance,
    const double sun_angular_radius,
    double bottom_radius,
    double top_radius,
    const std::vector<DensityProfileLayer>& rayleigh_density,
    const std::vector<double>& rayleigh_scattering,
    const std::vector<DensityProfileLayer>& mie_density,
    const std::vector<double>& mie_scattering,
    const std::vector<double>& mie_extinction,
    double mie_phase_function_g,
    const std::vector<DensityProfileLayer>& absorption_density,
    const std::vector<double>& absorption_extinction,
    const std::vector<double>& ground_albedo,
    double max_sun_zenith_angle,
    double length_unit_in_meters,
    unsigned int num_precomputed_wavelengths,
    bool combine_scattering_textures,
    bool half_precision) :
        num_precomputed_wavelengths_(num_precomputed_wavelengths),
        half_precision_(half_precision),
        rgb_format_supported_(true) {
  auto to_string = [&wavelengths](const std::vector<double>& v,
      const vec3& lambdas, double scale) {
    double r = Interpolate(wavelengths, v, lambdas[0]) * scale;
    double g = Interpolate(wavelengths, v, lambdas[1]) * scale;
    double b = Interpolate(wavelengths, v, lambdas[2]) * scale;
    return "vec3(" + std::to_string(r) + "," + std::to_string(g) + "," +
        std::to_string(b) + ")";
  };
  auto density_layer =
      [length_unit_in_meters](const DensityProfileLayer& layer) {
        return "DensityProfileLayer(" +
            std::to_string(layer.width / length_unit_in_meters) + "," +
            std::to_string(layer.exp_term) + "," +
            std::to_string(layer.exp_scale * length_unit_in_meters) + "," +
            std::to_string(layer.linear_term * length_unit_in_meters) + "," +
            std::to_string(layer.constant_term) + ")";
      };
  auto density_profile =
      [density_layer](std::vector<DensityProfileLayer> layers) {
        constexpr int kLayerCount = 2;
        while (layers.size() < kLayerCount) {
          layers.insert(layers.begin(), DensityProfileLayer());
        }
        std::string result = "DensityProfile(DensityProfileLayer[" +
            std::to_string(kLayerCount) + "](";
        for (int i = 0; i < kLayerCount; ++i) {
          result += density_layer(layers[i]);
          result += i < kLayerCount - 1 ? "," : "))";
        }
        return result;
      };

  // Compute the values for the SKY_RADIANCE_TO_LUMINANCE constant. In theory
  // this should be 1 in precomputed illuminance mode (because the precomputed
  // textures already contain illuminance values). In practice, however, storing
  // true illuminance values in half precision textures yields artefacts
  // (because the values are too large), so we store illuminance values divided
  // by MAX_LUMINOUS_EFFICACY instead. This is why, in precomputed illuminance
  // mode, we set SKY_RADIANCE_TO_LUMINANCE to MAX_LUMINOUS_EFFICACY.
  bool precompute_illuminance = num_precomputed_wavelengths > 3;
  double sky_k_r, sky_k_g, sky_k_b;
  if (precompute_illuminance) {
    sky_k_r = sky_k_g = sky_k_b = MAX_LUMINOUS_EFFICACY;
  } else {
    ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance,
        -3 /* lambda_power */, &sky_k_r, &sky_k_g, &sky_k_b);
  }
  // Compute the values for the SUN_RADIANCE_TO_LUMINANCE constant.
  double sun_k_r, sun_k_g, sun_k_b;
  ComputeSpectralRadianceToLuminanceFactors(wavelengths, solar_irradiance,
      0 /* lambda_power */, &sun_k_r, &sun_k_g, &sun_k_b);

  std::string definitions_glsl = skybolt::vis::loadFileToString("Shaders/ThirdParty/BrunetonAtmosphere/definitions.glsl");
  std::string functions_glsl = skybolt::vis::loadFileToString("Shaders/ThirdParty/BrunetonAtmosphere/functions.glsl");

  // A lambda that creates a GLSL header containing our atmosphere computation
  // functions, specialized for the given atmosphere parameters and for the 3
  // wavelengths in 'lambdas'.
  glsl_header_factory_ = [=](const vec3& lambdas) {
    return
      "#version 330\n"
      "#define IN(x) const in x\n"
      "#define OUT(x) out x\n"
      "#define TEMPLATE(x)\n"
      "#define TEMPLATE_ARGUMENT(x)\n"
      "#define assert(x)\n"
      "const int TRANSMITTANCE_TEXTURE_WIDTH = " +
          std::to_string(TRANSMITTANCE_TEXTURE_WIDTH) + ";\n" +
      "const int TRANSMITTANCE_TEXTURE_HEIGHT = " +
          std::to_string(TRANSMITTANCE_TEXTURE_HEIGHT) + ";\n" +
      "const int SCATTERING_TEXTURE_R_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_R_SIZE) + ";\n" +
      "const int SCATTERING_TEXTURE_MU_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_MU_SIZE) + ";\n" +
      "const int SCATTERING_TEXTURE_MU_S_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_MU_S_SIZE) + ";\n" +
      "const int SCATTERING_TEXTURE_NU_SIZE = " +
          std::to_string(SCATTERING_TEXTURE_NU_SIZE) + ";\n" +
      "const int IRRADIANCE_TEXTURE_WIDTH = " +
          std::to_string(IRRADIANCE_TEXTURE_WIDTH) + ";\n" +
      "const int IRRADIANCE_TEXTURE_HEIGHT = " +
          std::to_string(IRRADIANCE_TEXTURE_HEIGHT) + ";\n" +
      (combine_scattering_textures ?
          "#define COMBINED_SCATTERING_TEXTURES\n" : "") +
	  definitions_glsl +
      "const AtmosphereParameters ATMOSPHERE = AtmosphereParameters(\n" +
          to_string(solar_irradiance, lambdas, 1.0) + ",\n" +
          std::to_string(sun_angular_radius) + ",\n" +
          std::to_string(bottom_radius / length_unit_in_meters) + ",\n" +
          std::to_string(top_radius / length_unit_in_meters) + ",\n" +
          density_profile(rayleigh_density) + ",\n" +
          to_string(
              rayleigh_scattering, lambdas, length_unit_in_meters) + ",\n" +
          density_profile(mie_density) + ",\n" +
          to_string(mie_scattering, lambdas, length_unit_in_meters) + ",\n" +
          to_string(mie_extinction, lambdas, length_unit_in_meters) + ",\n" +
          std::to_string(mie_phase_function_g) + ",\n" +
          density_profile(absorption_density) + ",\n" +
          to_string(
              absorption_extinction, lambdas, length_unit_in_meters) + ",\n" +
          to_string(ground_albedo, lambdas, 1.0) + ",\n" +
          std::to_string(cos(max_sun_zenith_angle)) + ");\n" +
      "const vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(" +
          std::to_string(sky_k_r) + "," +
          std::to_string(sky_k_g) + "," +
          std::to_string(sky_k_b) + ");\n" +
      "const vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = vec3(" +
          std::to_string(sun_k_r) + "," +
          std::to_string(sun_k_g) + "," +
          std::to_string(sun_k_b) + ");\n" +
     functions_glsl;
  };

  // Allocate the precomputed textures, but don't precompute them yet.
  transmittance_texture_ = NewTexture2d(
      TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
  scattering_texture_ = NewTexture3d(
      SCATTERING_TEXTURE_WIDTH,
      SCATTERING_TEXTURE_HEIGHT,
      SCATTERING_TEXTURE_DEPTH,
      combine_scattering_textures || !rgb_format_supported_ ? GL_RGBA : GL_RGB,
      half_precision);
  if (combine_scattering_textures) {
    optional_single_mie_scattering_texture_ = 0;
  } else {
    optional_single_mie_scattering_texture_ = NewTexture3d(
        SCATTERING_TEXTURE_WIDTH,
        SCATTERING_TEXTURE_HEIGHT,
        SCATTERING_TEXTURE_DEPTH,
        rgb_format_supported_ ? GL_RGB : GL_RGBA,
        half_precision);
  }
  irradiance_texture_ = NewTexture2d(
      IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
}

/*
<p>The destructor is trivial:
*/

Model::~Model() {
}

/*
<p>The Init method precomputes the atmosphere textures. It first allocates the
temporary resources it needs, then calls <code>Precompute</code> to do the
actual precomputations, and finally destroys the temporary resources.

<p>Note that there are two precomputation modes here, depending on whether we
want to store precomputed irradiance or illuminance values:
<ul>
  <li>In precomputed irradiance mode, we simply need to call
  <code>Precompute</code> with the 3 wavelengths for which we want to precompute
  irradiance, namely <code>kLambdaR</code>, <code>kLambdaG</code>,
  <code>kLambdaB</code> (with the identity matrix for
  <code>luminance_from_radiance</code>, since we don't want any conversion from
  radiance to luminance)</li>
  <li>In precomputed illuminance mode, we need to precompute irradiance for
  <code>num_precomputed_wavelengths_</code>, and then integrate the results,
  multiplied with the 3 CIE xyz color matching functions and the XYZ to sRGB
  matrix to get sRGB illuminance values.
  <p>A naive solution would be to allocate temporary textures for the
  intermediate irradiance results, then perform the integration from irradiance
  to illuminance and store the result in the final precomputed texture. In
  pseudo-code (and assuming one wavelength per texture instead of 3):
  <pre>
    create n temporary irradiance textures
    for each wavelength lambda in the n wavelengths:
       precompute irradiance at lambda into one of the temporary textures
    initializes the final illuminance texture with zeros
    for each wavelength lambda in the n wavelengths:
      accumulate in the final illuminance texture the product of the
      precomputed irradiance at lambda (read from the temporary textures)
      with the value of the 3 sRGB color matching functions at lambda (i.e.
      the product of the XYZ to sRGB matrix with the CIE xyz color matching
      functions).
  </pre>
  <p>However, this be would waste GPU memory. Instead, we can avoid allocating
  temporary irradiance textures, by merging the two above loops:
  <pre>
    for each wavelength lambda in the n wavelengths:
      accumulate in the final illuminance texture (or, for the first
      iteration, set this texture to) the product of the precomputed
      irradiance at lambda (computed on the fly) with the value of the 3
      sRGB color matching functions at lambda.
  </pre>
  <p>This is the method we use below, with 3 wavelengths per iteration instead
  of 1, using <code>Precompute</code> to compute 3 irradiances values per
  iteration, and <code>luminance_from_radiance</code> to multiply 3 irradiances
  with the values of the 3 sRGB color matching functions at 3 different
  wavelengths (yielding a 3x3 matrix).</li>
</ul>

<p>This yields the following implementation:
*/

Model::Pipeline Model::CreatePipeline(unsigned int num_scattering_orders) {
  // The precomputations require temporary textures, in particular to store the
  // contribution of one scattering order, which is needed to compute the next
  // order of scattering (the final precomputed textures store the sum of all
  // the scattering orders). We allocate them here, and destroy them at the end
  // of this method.
  auto delta_irradiance_texture = NewTexture2d(
      IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
  auto delta_rayleigh_scattering_texture = NewTexture3d(
      SCATTERING_TEXTURE_WIDTH,
      SCATTERING_TEXTURE_HEIGHT,
      SCATTERING_TEXTURE_DEPTH,
      rgb_format_supported_ ? GL_RGB : GL_RGBA,
      half_precision_);
  auto delta_mie_scattering_texture = NewTexture3d(
      SCATTERING_TEXTURE_WIDTH,
      SCATTERING_TEXTURE_HEIGHT,
      SCATTERING_TEXTURE_DEPTH,
      rgb_format_supported_ ? GL_RGB : GL_RGBA,
      half_precision_);
  auto delta_scattering_density_texture = NewTexture3d(
      SCATTERING_TEXTURE_WIDTH,
      SCATTERING_TEXTURE_HEIGHT,
      SCATTERING_TEXTURE_DEPTH,
      rgb_format_supported_ ? GL_RGB : GL_RGBA,
      half_precision_);
  // delta_multiple_scattering_texture is only needed to compute scattering
  // order 3 or more, while delta_rayleigh_scattering_texture and
  // delta_mie_scattering_texture are only needed to compute double scattering.
  // Therefore, to save memory, we can store delta_rayleigh_scattering_texture
  // and delta_multiple_scattering_texture in the same GPU texture.
  auto delta_multiple_scattering_texture = delta_rayleigh_scattering_texture;

  Pipeline pipeline;

  // The actual precomputations depend on whether we want to store precomputed
  // irradiance or illuminance values.
  if (num_precomputed_wavelengths_ <= 3) {
    vec3 lambdas{kLambdaR, kLambdaG, kLambdaB};
    mat3 luminance_from_radiance{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    Precompute(pipeline.precomputation, delta_irradiance_texture, delta_rayleigh_scattering_texture,
        delta_mie_scattering_texture, delta_scattering_density_texture,
        delta_multiple_scattering_texture, lambdas, luminance_from_radiance,
        false /* blend */, num_scattering_orders);
  } else {
    constexpr double kLambdaMin = 360.0;
    constexpr double kLambdaMax = 830.0;
    int num_iterations = (num_precomputed_wavelengths_ + 2) / 3;
    double dlambda = (kLambdaMax - kLambdaMin) / (3 * num_iterations);
    for (int i = 0; i < num_iterations; ++i) {
      vec3 lambdas{
        kLambdaMin + (3 * i + 0.5) * dlambda,
        kLambdaMin + (3 * i + 1.5) * dlambda,
        kLambdaMin + (3 * i + 2.5) * dlambda
      };
      auto coeff = [dlambda](double lambda, int component) {
        // Note that we don't include MAX_LUMINOUS_EFFICACY here, to avoid
        // artefacts due to too large values when using half precision on GPU.
        // We add this term back in kAtmosphereShader, via
        // SKY_SPECTRAL_RADIANCE_TO_LUMINANCE (see also the comments in the
        // Model constructor).
        double x = CieColorMatchingFunctionTableValue(lambda, 1);
        double y = CieColorMatchingFunctionTableValue(lambda, 2);
        double z = CieColorMatchingFunctionTableValue(lambda, 3);
        return static_cast<float>((
            XYZ_TO_SRGB[component * 3] * x +
            XYZ_TO_SRGB[component * 3 + 1] * y +
            XYZ_TO_SRGB[component * 3 + 2] * z) * dlambda);
      };
      mat3 luminance_from_radiance{
        coeff(lambdas[0], 0), coeff(lambdas[1], 0), coeff(lambdas[2], 0),
        coeff(lambdas[0], 1), coeff(lambdas[1], 1), coeff(lambdas[2], 1),
        coeff(lambdas[0], 2), coeff(lambdas[1], 2), coeff(lambdas[2], 2)
      };
      Precompute(pipeline.precomputation, delta_irradiance_texture,
          delta_rayleigh_scattering_texture, delta_mie_scattering_texture,
          delta_scattering_density_texture, delta_multiple_scattering_texture,
          lambdas, luminance_from_radiance, i > 0 /* blend */,
          num_scattering_orders);
    }

    // After the above iterations, the transmittance texture contains the
    // transmittance for the 3 wavelengths used at the last iteration. But we
    // want the transmittance at kLambdaR, kLambdaG, kLambdaB instead, so we
    // must recompute it here for these 3 wavelengths:
    std::string header = glsl_header_factory_({kLambdaR, kLambdaG, kLambdaB});

	osg::ref_ptr<osg::Program> compute_transmittance = new osg::Program();
	compute_transmittance->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
	compute_transmittance->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeTransmittanceShader));

	{
		Pipeline::Stage stage;
		stage.inputs = { Pipeline::Stage::Input("transmittance_texture", transmittance_texture_) };
		stage.outputs = { transmittance_texture_ };
		stage.program = compute_transmittance;
		pipeline.precomputation.push_back(stage);
	}
  }

  pipeline.transmittanceTexture = transmittance_texture_;
  pipeline.scatteringTexture = scattering_texture_;
  pipeline.irradianceTexture = irradiance_texture_;
  if (optional_single_mie_scattering_texture_)
  {
	  pipeline.optionalSingleMieScatteringTexture = optional_single_mie_scattering_texture_;
  };

  return pipeline;
}

/*
<p>The utility method <code>ConvertSpectrumToLinearSrgb</code> is implemented
with a simple numerical integration of the given function, times the CIE color
matching funtions (with an integration step of 1nm), followed by a matrix
multiplication:
*/

void Model::ConvertSpectrumToLinearSrgb(
    const std::vector<double>& wavelengths,
    const std::vector<double>& spectrum,
    double* r, double* g, double* b) {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  const int dlambda = 1;
  for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
    double value = Interpolate(wavelengths, spectrum, lambda);
    x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
    y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
    z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
  }
  *r = MAX_LUMINOUS_EFFICACY *
      (XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
  *g = MAX_LUMINOUS_EFFICACY *
      (XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
  *b = MAX_LUMINOUS_EFFICACY *
      (XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
}

osg::Matrix3 Model::toOsgMatrix3(const mat3& mat)
{
	// NOTE: OSG is row-major, openGL is column major
	return osg::Matrix3(
		mat[0], mat[3], mat[6],
		mat[1],	mat[4], mat[7],
		mat[2], mat[5], mat[8]);
}

/*
<p>Finally, we provide the actual implementation of the precomputation algorithm
described in Algorithm 4.1 of
<a href="https://hal.inria.fr/inria-00288758/en">our paper</a>. Each step is
explained by the inline comments below.
*/
void Model::Precompute(
	std::vector<Pipeline::Stage>& pipeline,
    osg::ref_ptr<osg::Texture2D> delta_irradiance_texture,
	osg::ref_ptr<osg::Texture3D> delta_rayleigh_scattering_texture,
	osg::ref_ptr<osg::Texture3D> delta_mie_scattering_texture,
	osg::ref_ptr<osg::Texture3D> delta_scattering_density_texture,
	osg::ref_ptr<osg::Texture3D> delta_multiple_scattering_texture,
    const vec3& lambdas,
    const mat3& luminance_from_radiance,
    bool blend,
    unsigned int num_scattering_orders) {
  // The precomputations require specific GLSL programs, for each precomputation
  // step. We create and compile them here (they are automatically destroyed
  // when this method returns, via the Program destructor).
  std::string header = glsl_header_factory_(lambdas);

  osg::ref_ptr<osg::Program> compute_transmittance = new osg::Program();
  compute_transmittance->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
  compute_transmittance->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeTransmittanceShader));

  osg::ref_ptr<osg::Program> compute_direct_irradiance = new osg::Program();
  compute_direct_irradiance->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
  compute_direct_irradiance->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeDirectIrradianceShader));

  osg::ref_ptr<osg::Program> compute_single_scattering = new osg::Program();
  compute_single_scattering->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
  compute_single_scattering->addShader(new osg::Shader(osg::Shader::GEOMETRY, kGeometryShader));
  compute_single_scattering->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeSingleScatteringShader));

  osg::ref_ptr<osg::Program> compute_scattering_density = new osg::Program();
  compute_scattering_density->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
  compute_scattering_density->addShader(new osg::Shader(osg::Shader::GEOMETRY, kGeometryShader));
  compute_scattering_density->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeScatteringDensityShader));

  osg::ref_ptr<osg::Program> compute_indirect_irradiance = new osg::Program();
  compute_indirect_irradiance->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
  compute_indirect_irradiance->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeIndirectIrradianceShader));

  osg::ref_ptr<osg::Program> compute_multiple_scattering = new osg::Program();
  compute_multiple_scattering->addShader(new osg::Shader(osg::Shader::VERTEX, kVertexShader));
  compute_multiple_scattering->addShader(new osg::Shader(osg::Shader::GEOMETRY, kGeometryShader));
  compute_multiple_scattering->addShader(new osg::Shader(osg::Shader::FRAGMENT, header + kComputeMultipleScatteringShader));

  // Compute the transmittance, and store it in transmittance_texture_.
  {
	  Pipeline::Stage stage;
	  stage.outputs = { transmittance_texture_ };
	  stage.program = compute_transmittance;
	  pipeline.push_back(stage);
  }

  // Compute the direct irradiance, store it in delta_irradiance_texture and,
  // depending on 'blend', either initialize irradiance_texture_ with zeros or
  // leave it unchanged (we don't want the direct irradiance in
  // irradiance_texture_, but only the irradiance from the sky).
  {
	  Pipeline::Stage stage;
	  stage.inputs = { Pipeline::Stage::Input("transmittance_texture", transmittance_texture_) };
	  stage.outputs = { delta_irradiance_texture, irradiance_texture_ };
	  stage.additive = { false, blend };
	  stage.program = compute_direct_irradiance;
	  pipeline.push_back(stage);
  }
 
  // Compute the rayleigh and mie single scattering, store them in
  // delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
  // either store them or accumulate them in scattering_texture_ and
  // optional_single_mie_scattering_texture_.
  {
	  Pipeline::Stage stage;
	  stage.inputs = { Pipeline::Stage::Input("transmittance_texture", transmittance_texture_) };
	  stage.outputs = { delta_rayleigh_scattering_texture, delta_mie_scattering_texture, scattering_texture_ };

	  if (optional_single_mie_scattering_texture_ != 0) {
		  stage.outputs.push_back(optional_single_mie_scattering_texture_);
	  }

	  stage.additive = { false, false, blend, blend };
	  stage.program = compute_single_scattering;

	  osg::ref_ptr<osg::Uniform> matrix = new osg::Uniform("luminance_from_radiance", toOsgMatrix3(luminance_from_radiance));

	  for (int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
			stage.uniforms = { matrix, new osg::Uniform("layer", layer) };
			pipeline.push_back(stage);
	  }
  }
  
  // Compute the 2nd, 3rd and 4th order of scattering, in sequence.
  for (int scattering_order = 2;
       scattering_order <= (int)num_scattering_orders;
       ++scattering_order) {
		// Compute the scattering density, and store it in
		// delta_scattering_density_texture.
		{
			Pipeline::Stage stage;
			stage.inputs = {
				Pipeline::Stage::Input("transmittance_texture", transmittance_texture_),
				Pipeline::Stage::Input("single_rayleigh_scattering_texture", delta_rayleigh_scattering_texture),
				Pipeline::Stage::Input("single_mie_scattering_texture", delta_mie_scattering_texture),
				Pipeline::Stage::Input("multiple_scattering_texture", delta_multiple_scattering_texture),
				Pipeline::Stage::Input("irradiance_texture", delta_irradiance_texture)
			};
			stage.outputs = { delta_scattering_density_texture };
			stage.program = compute_scattering_density;

			osg::ref_ptr<osg::Uniform> scatteringOrderUniform = new osg::Uniform("scattering_order", scattering_order);

			for (int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
				stage.uniforms = { scatteringOrderUniform, new osg::Uniform("layer", layer) };
				pipeline.push_back(stage);
			}
		}

		// Compute the indirect irradiance, store it in delta_irradiance_texture and
		// accumulate it in irradiance_texture_.
		{
			Pipeline::Stage stage;
			stage.inputs = {
				Pipeline::Stage::Input("single_rayleigh_scattering_texture", delta_rayleigh_scattering_texture),
				Pipeline::Stage::Input("single_mie_scattering_texture", delta_mie_scattering_texture),
				Pipeline::Stage::Input("multiple_scattering_texture", delta_multiple_scattering_texture)
			};
			stage.outputs = { delta_irradiance_texture, irradiance_texture_ };
			stage.additive = { false, true };
			stage.program = compute_indirect_irradiance;

			stage.uniforms = {
				new osg::Uniform("luminance_from_radiance", toOsgMatrix3(luminance_from_radiance)),
				new osg::Uniform("scattering_order",  scattering_order - 1)
			};
			pipeline.push_back(stage);
		}

		// Compute the multiple scattering, store it in
		// delta_multiple_scattering_texture, and accumulate it in
		// scattering_texture_.
		{
			Pipeline::Stage stage;
			stage.inputs = {
				Pipeline::Stage::Input("transmittance_texture", transmittance_texture_),
				Pipeline::Stage::Input("scattering_density_texture", delta_scattering_density_texture)
			};
			stage.outputs = { delta_multiple_scattering_texture, scattering_texture_ };
			stage.additive = { false, true };
			stage.program = compute_multiple_scattering;

			osg::ref_ptr<osg::Uniform> scatteringOrderUniform = new osg::Uniform("luminance_from_radiance", toOsgMatrix3(luminance_from_radiance));

			for (int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; ++layer) {
				stage.uniforms = { scatteringOrderUniform, new osg::Uniform("layer", layer) };
				pipeline.push_back(stage);
			}
		}
  }
}

}  // namespace atmosphere
