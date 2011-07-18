/***********************************************************************
Sphere - Class for clay structural units (attraction only, for clay
modelling).
Copyright (c) 2003-2011 Oliver Kreylos

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

#ifndef SPHERE_INCLUDED
#define SPHERE_INCLUDED

#include "StructuralUnit.h"

namespace NCK {

class SphereRenderer:public UnitRenderer
	{
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	};

class Sphere:public StructuralUnit
	{
	friend class SphereRenderer;
	
	/* Elements: */
	protected:
	static Scalar radius,radius2; // Radius and squared radius of sphere
	static Scalar mass; // Mass of a sphere
	static SphereRenderer* unitRenderer; // Renderer for sphere units
	
	/* Constructors and destructors: */
	public:
	static void initClass(const Misc::ConfigurationFileSection& configFileSection); // Initializes class settings based on the given configuration file section
	Sphere(const Point& sPosition,const Rotation& sOrientation)
		:StructuralUnit(sPosition,sOrientation,0)
		{
		};
	Sphere(Misc::File& file)
		:StructuralUnit(0,file)
		{
		};
	static void deinitClass(void); // Destroys class data
	
	/* Methods: */
	static Scalar getClassRadius(void) // Returns radius of all spheres
		{
		return radius;
		};
	static int getClassNumVertices(void) // Returns number of vertices of sphere class
		{
		return 0;
		};
	static Vector getClassVertexOffset(int vertexIndex) // Returns offset from sphere center to one sphere vertex (there are none)
		{
		return Vector::zero;
		};
	static void setRadius(Scalar newRadius); // Sets spheres' radius
	static void setMass(Scalar newMass); // Sets spheres' mass
	virtual Scalar getRadius(void) const
		{
		return radius;
		};
	virtual Scalar getMass(void) const
		{
		return mass;
		};
	virtual Vector getVertexOffset(int index) const
		{
		return Vector::zero;
		};
	virtual Point getVertex(int index) const
		{
		return position;
		};
	virtual void applyVertexForce(int index,const Vector& force,Scalar timeStep);
	virtual void applyCentralForce(const Vector& force,Scalar timeStep);
	virtual void glRenderAction(GLContextData& contextData) const;
	};

}

#endif
