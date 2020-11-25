/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ATMOSPHERIC_SCATTERING_H
#define ATMOSPHERIC_SCATTERING_H

#include "GlobalUniforms.h"
#include "ThirdParty/BrunetonAtmosphere/AtmospherePublic.glsl"

// Copy of GetSkyRadiance in AtmospherePrivate.glsl, but with
// single_mie_scattering returned instead of performing phase function evaluations.
// Designed to be used in vertex shader so phase functions
// can be evaluated later in fragment shader.
RadianceSpectrum GetSkyRadiance(
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    Position camera, IN(Direction) view_ray, Length shadow_length,
    IN(Direction) sun_direction, OUT(DimensionlessSpectrum) transmittance,
	OUT(IrradianceSpectrum) single_mie_scattering) {
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
	single_mie_scattering = IrradianceSpectrum(0.0);
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
  return scattering;
}

RadianceSpectrum GetSkyRadianceScattering(
	Position camera, Direction view_ray, Length shadow_length,
	Direction sun_direction, OUT(DimensionlessSpectrum) transmittance,
	OUT(IrradianceSpectrum) single_mie_scattering) {
	  return GetSkyRadiance( transmittance_texture,
	  scattering_texture, single_mie_scattering_texture,
	  camera, view_ray, shadow_length, sun_direction, transmittance, single_mie_scattering);
}

Luminance3 GetSkyRadianceFromScattering(
    IN(RadianceSpectrum) scattering,
    IN(IrradianceSpectrum) single_mie_scattering,
    IN(Direction) view_ray,
    IN(Direction) sun_direction) {

	Number nu = dot(view_ray, sun_direction);
	return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *
      MiePhaseFunction(mie_phase_function_g, nu);
}

IrradianceSpectrum GetSunAndSkyIrradiance(
    IN(TransmittanceTexture) transmittance_texture,
    IN(IrradianceTexture) irradiance_texture,
    IN(Position) point, IN(Direction) sun_direction,
    OUT(IrradianceSpectrum) sky_irradiance) {
  Length r = length(point);
  Number mu_s = dot(point, sun_direction) / r;

  // Indirect irradiance (approximated if the surface is not horizontal).
  sky_irradiance = GetIrradiance(irradiance_texture, r, mu_s);

  // Direct irradiance.
  return solar_irradiance *
      GetTransmittanceToSun(
          transmittance_texture, r, mu_s);
}

IrradianceSpectrum GetSunAndSkyIrradiance(
   Position p, Direction sun_direction,
   out IrradianceSpectrum sky_irradiance) {
  return GetSunAndSkyIrradiance(transmittance_texture,
	  irradiance_texture, p, sun_direction, sky_irradiance);
}

IrradianceSpectrum GetSunIrradianceInSpace() {
  return solar_irradiance;
}

DimensionlessSpectrum GetTransmittance(
    IN(TransmittanceTexture) transmittance_texture,
    Position camera, IN(Position) point) {
  // Compute the distance to the top atmosphere boundary along the view ray,
  // assuming the viewer is in space (or NaN if the view ray does not intersect
  // the atmosphere).
  Direction view_ray = normalize(point - camera);
  Length r = length(camera);
  Length rmu = dot(camera, view_ray);

  // Compute the r, mu, mu_s and nu parameters for the first texture lookup.
  Number mu = rmu / r;
  Length d = length(point - camera);
  bool ray_r_mu_intersects_ground = RayIntersectsGround(r, mu);

  return GetTransmittance(transmittance_texture,
      r, mu, d, ray_r_mu_intersects_ground);
}

DimensionlessSpectrum GetTransmittance(
    Position camera, IN(Position) point) {
	return GetTransmittance(transmittance_texture, camera, point);
}

#endif // ATMOSPHERIC_SCATTERING_H