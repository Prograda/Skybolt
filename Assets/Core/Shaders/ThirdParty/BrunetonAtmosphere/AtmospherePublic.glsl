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
 
#include "ThirdParty/BrunetonAtmosphere/AtmospherePrivate.h"
 
uniform sampler2D transmittance_texture;
uniform sampler3D scattering_texture;
uniform sampler3D single_mie_scattering_texture;
uniform sampler2D irradiance_texture;
#ifdef RADIANCE_API_ENABLED
RadianceSpectrum GetSolarRadiance() {
  return solar_irradiance /
	  (PI * sun_angular_radius * sun_angular_radius);
}
RadianceSpectrum GetSkyRadiance(
	Position camera, Direction view_ray, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance) {
  return GetSkyRadiance(transmittance_texture,
	  scattering_texture, single_mie_scattering_texture,
	  camera, view_ray, shadow_length, sun_direction, transmittance);
}
RadianceSpectrum GetSkyRadianceToPoint(
	Position camera, Position point, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance) {
  return GetSkyRadianceToPoint(transmittance_texture,
	  scattering_texture, single_mie_scattering_texture,
	  camera, point, shadow_length, sun_direction, transmittance);
}
IrradianceSpectrum GetSunAndSkyIrradiance(
   Position p, Direction normal, Direction sun_direction,
   out IrradianceSpectrum sky_irradiance) {
  return GetSunAndSkyIrradiance(transmittance_texture,
	  irradiance_texture, p, normal, sun_direction, sky_irradiance);
}
#endif
Luminance3 GetSolarLuminance() {
  return solar_irradiance /
	  (PI * sun_angular_radius * sun_angular_radius) *
	  SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminance(
	Position camera, Direction view_ray, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance) {
  return GetSkyRadiance( transmittance_texture,
	  scattering_texture, single_mie_scattering_texture,
	  camera, view_ray, shadow_length, sun_direction, transmittance) *
	  SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Luminance3 GetSkyLuminanceToPoint(
	Position camera, Position point, Length shadow_length,
	Direction sun_direction, out DimensionlessSpectrum transmittance) {
  return GetSkyRadianceToPoint(transmittance_texture,
	  scattering_texture, single_mie_scattering_texture,
	  camera, point, shadow_length, sun_direction, transmittance) *
	  SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
Illuminance3 GetSunAndSkyIlluminance(
   Position p, Direction normal, Direction sun_direction,
   out IrradianceSpectrum sky_irradiance) {
  IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(
	  transmittance_texture, irradiance_texture, p, normal,
	  sun_direction, sky_irradiance);
  sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
  return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}