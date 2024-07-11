/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <mapbox/earcut.hpp>
#include <osg/Vec2>

namespace mapbox {
	namespace util {

		template <>
		struct nth<0, osg::Vec2f> {
			inline static float get(const osg::Vec2f &t) {
				return t.x();
			};
		};
		template <>
		struct nth<1, osg::Vec2f> {
			inline static float get(const osg::Vec2f &t) {
				return t.y();
			};
		};

		template <>
		struct nth<0, osg::Vec3f> {
			inline static float get(const osg::Vec3f &t) {
				return t.x();
			};
		};

		template <>
		struct nth<1, osg::Vec3f> {
			inline static float get(const osg::Vec3f &t) {
				return t.y();
			};
		};

		template <>
		struct nth<2, osg::Vec3f> {
			inline static float get(const osg::Vec3f &t) {
				return t.z();
			};
		};

	} // namespace util
} // namespace mapbox