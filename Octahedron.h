/***********************************************************************
Octahedron - Class for aluminum oxide structural units (used to build
heterogeneous silicate crystals).
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

#ifndef OCTAHEDRON_INCLUDED
#define OCTAHEDRON_INCLUDED

#include "StructuralUnit.h"

namespace NCK {

class OctahedronRenderer:public UnitRenderer
	{
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	};

class Octahedron:public StructuralUnit
	{
	friend class OctahedronRenderer;
	
	/* Elements: */
	protected:
	static Scalar radius,radius2; // Radius and squared radius of octahedron's circumspheres
	static Scalar mass; // Mass of an octahedron
	static Vector vertexOffsets[6]; // Octahedron vertices in local coordinates
	static Point renderVertices[6]; // Octahedron vertices for rendering
	static Vector renderNormals[8]; // Octahedron face normals in local coordinates
	static OctahedronRenderer* unitRenderer; // Renderer for octahedron units
	
	/* Protected methods: */
	static void calculateShape(void); // Re-calculates octahedron shape after size change
	
	/* Constructors and destructors: */
	public:
	static void initClass(const Misc::ConfigurationFileSection& configFileSection); // Initializes class settings based on the given configuration file section
	Octahedron(const Point& sPosition,const Rotation& sOrientation)
		:StructuralUnit(sPosition,sOrientation,6)
		{
		};
	Octahedron(Misc::File& file)
		:StructuralUnit(6,file)
		{
		};
	static void deinitClass(void); // Destroys class data
	
	/* Methods: */
	static Scalar getClassRadius(void) // Returns radius of all octahedra
		{
		return radius;
		};
	static int getClassNumVertices(void) // Returns number of vertices of octahedron class
		{
		return 6;
		};
	static Vector getClassVertexOffset(int vertexIndex) // Returns offset from octahedron center to one octahedron vertex
		{
		return vertexOffsets[vertexIndex];
		};
	static void setRadius(Scalar newRadius); // Sets octahedrons' radius
	static void setMass(Scalar newMass); // Sets octahedrons' mass
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
