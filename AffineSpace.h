/***********************************************************************
AffineSpace - Definition of the affine space used by the Nanotech
Construction Kit.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#ifndef AFFINESPACE_INCLUDED
#define AFFINESPACE_INCLUDED

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/Ray.h>
#include <Geometry/Box.h>

namespace NCK {

typedef double Scalar;
typedef Geometry::Vector<Scalar,3> Vector;
typedef Geometry::Point<Scalar,3> Point;
typedef Geometry::Rotation<Scalar,3> Rotation;
typedef Geometry::OrthonormalTransformation<Scalar,3> OrthonormalTransformation;
typedef Geometry::Ray<Scalar,3> Ray;
typedef Geometry::Box<Scalar,3> Box;

}

#endif
