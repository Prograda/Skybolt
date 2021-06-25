/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <SkyboltCommon/NumericComparison.h>
#include <SkyboltVis/MatrixHelpers.h>

using namespace skybolt;
using namespace skybolt::vis;

TEST_CASE("4x4 Matrix transpose")
{
	osg::Matrix m;
	{
		float v = 0.0f;
		for (int r = 0; r < 4; ++r)
		{
			for (int c = 0; c < 4; ++c)
			{
				m(r, c) = v;
				v += 1.0f;
			}
		}
	}

	osg::Matrix transposed = transpose(m);

	osg::Matrix expected;
	{
		float v = 0.0f;
		for (int c = 0; c < 4; ++c)
		{
			for (int r = 0; r < 4; ++r)
			{
				expected(r, c) = v;
				v += 1.0f;
			}
		}
	}

	CHECK(transposed == expected);
}

bool almostEqualVec(const osg::Vec4f& a, const osg::Vec4f& b, float epsilon)
{
	return almostEqual(a.x(), b.x(), epsilon) && almostEqual(a.y(), b.y(), epsilon)
		&& almostEqual(a.z(), b.z(), epsilon) && almostEqual(a.w(), b.w(), epsilon);
}

TEST_CASE("Matrix multiplied with plane")
{
	// translation
	{
		osg::Matrix m = osg::Matrix::translate(osg::Vec3f(1, 2, 3));
		{
			osg::Plane p(osg::Vec3f(1,0,0), -5);
			CHECK(almostEqualVec(mul(p, m).asVec4(), osg::Vec4f(1,0,0,-6), 1e-7f));
		}
		{
			osg::Plane p(osg::Vec3f(0,1,0), -5);
			CHECK(almostEqualVec(mul(p, m).asVec4(), osg::Vec4f(0,1,0,-7), 1e-7f));
		}
		{
			osg::Plane p(osg::Vec3f(0,0,1), -5);
			CHECK(almostEqualVec(mul(p, m).asVec4(), osg::Vec4f(0,0,1,-8), 1e-7f));
		}
	}
	// rotation
	{
		osg::Matrix m = osg::Matrix::rotate(osg::Quat(osg::PI_2, osg::Vec3f(0, 0, 1)));
		{
			osg::Plane p(osg::Vec3f(1,0,0), -5);
			CHECK(almostEqualVec(mul(p, m).asVec4(), osg::Vec4f(0,1,0,-5), 1e-7f));
		}
	}
	// scale
	{
		osg::Matrix m = osg::Matrix::scale(osg::Vec3f(0.5, 0.5, 0.5));
		{
			osg::Plane p(osg::Vec3f(0,0,1), -4);
			CHECK(almostEqualVec(mul(p, m).asVec4(), osg::Vec4f(0,0,1,-2), 1e-7f));
		}
	}
}
