/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <osg/Matrix>
#include <osg/Plane>

namespace skybolt {
namespace vis {

inline osg::Matrix transpose(const osg::Matrix& m)
{
	return osg::Matrix( m(0,0), m(1,0), m(2,0), m(3,0),
						m(0,1), m(1,1), m(2,1), m(3,1),
						m(0,2), m(1,2), m(2,2), m(3,2),
						m(0,3), m(1,3), m(2,3), m(3,3));
}

inline osg::Plane mul(const osg::Plane& p, const osg::Matrix& matrix)
{
	osg::Matrix matrixInv = osg::Matrix::inverse(matrix);
	osg::Matrix matrixInvTrans = transpose(matrixInv);

	osg::Vec4f planeVec = p.asVec4() * matrixInvTrans;

	osg::Vec3f n(planeVec.x(), planeVec.y(), planeVec.z());
	planeVec.w() /= n.normalize();
	return osg::Plane(n, planeVec.w());
}

inline osg::Matrix makeMatrixFromTbn(const osg::Vec3f& T, const osg::Vec3f& B, const osg::Vec3f& N)
{
	return osg::Matrix(T.x(), T.y(), T.z(), 0,
		B.x(), B.y(), B.z(), 0,
		N.x(), N.y(), N.z(), 0,
		0, 0, 0, 1);
}

} // namespace vis
} // namespace skybolt
