/***********************************************************************
Triangle - Class for carbon structural units (used to build nanotubes
and Buckyballs).
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

#ifndef TRIANGLE_INCLUDED
#define TRIANGLE_INCLUDED

#include "StructuralUnit.h"

namespace NCK {

class TriangleRenderer:public UnitRenderer
	{
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	};

class Triangle:public StructuralUnit
	{
	friend class TriangleRenderer;
	
	/* Elements: */
	protected:
	static Scalar radius,radius2; // Radius and squared radius of triangle's circumspheres
	static Scalar width; // Width of a triangle
	static Scalar mass; // Mass of a triangle
	static Vector vertexOffsets[3]; // Triangle vertices in local coordinates
	static Point renderVertices[6]; // Triangle vertices for rendering
	static Vector renderNormals[5]; // Triangle face normals in local coordinates
	static TriangleRenderer* unitRenderer; // Renderer for triangle units
	
	/* Protected methods: */
	static void calculateShape(void); // Re-calculates triangle shape after size change
	
	/* Constructors and destructors: */
	public:
	static void initClass(const Misc::ConfigurationFileSection& configFileSection); // Initializes class settings based on the given configuration file section
	Triangle(const Point& sPosition,const Rotation& sOrientation)
		:StructuralUnit(sPosition,sOrientation,3)
		{
		};
	Triangle(Misc::File& file)
		:StructuralUnit(3,file)
		{
		};
	static void deinitClass(void); // Destroys class data
	
	/* Methods: */
	static Scalar getClassRadius(void) // Returns radius of all triangles
		{
		return radius;
		};
	static int getClassNumVertices(void) // Returns number of vertices of triangle class
		{
		return 3;
		};
	static Vector getClassVertexOffset(int vertexIndex) // Returns offset from triangle center to one triangle vertex
		{
		return vertexOffsets[vertexIndex];
		};
	static void setRadius(Scalar newRadius); // Sets triangles' radius
	static void setWidth(Scalar newWidth); // Sets triangles' width
	static void setMass(Scalar newMass); // Sets triangles' mass
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
		return orientation.transform(vertexOffsets[index]);
		};
	virtual Point getVertex(int index) const
		{
		return position+orientation.transform(vertexOffsets[index]);
		};
	virtual void applyVertexForce(int index,const Vector& force,Scalar timeStep);
	virtual void applyCentralForce(const Vector& force,Scalar timeStep);
	virtual void glRenderAction(GLContextData& contextData) const;
	};

}

#endif
