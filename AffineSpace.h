/***********************************************************************
AffineSpace - Definition of the affine space used by the Nanotech
Construction Kit.
Copyright (c) 2004-2011 Oliver Kreylos

The Nanotech Construction Kit is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Nanotech Construction Kit is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Nanotech Construction Kit; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
