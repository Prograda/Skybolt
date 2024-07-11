/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

float orenNayar(vec3 L, vec3 V, vec3 N, float roughness)
{  
  float LdotV = dot(L, V);
  float NdotL = dot(N, L);
  float NdotV = dot(N, V);

  float s = LdotV - NdotL * NdotV;
  float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

  float sigma2 = roughness * roughness;
  float A = 1.0 - 0.5 * sigma2 / (sigma2 + 0.33);
  float B = 0.45 * sigma2 / (sigma2 + 0.09);

  return max(0.0, NdotL) * (A + B * s / t);
}
