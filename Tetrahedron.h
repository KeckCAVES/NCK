/***********************************************************************
Tetrahedron - Class for silicate structural units (used to build
glasses).
Copyright (c) 2003 Oliver Kreylos
***********************************************************************/

#ifndef TETRAHEDRON_INCLUDED
#define TETRAHEDRON_INCLUDED

#include "StructuralUnit.h"

namespace NCK {

class TetrahedronRenderer:public UnitRenderer
	{
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	};

class Tetrahedron:public StructuralUnit
	{
	friend class TetrahedronRenderer;
	
	/* Elements: */
	protected:
	static Scalar radius,radius2; // Radius and squared radius of tetrahedron's circumspheres
	static Scalar mass; // Mass of a tetrahedron
	static Vector vertexOffsets[4]; // Tetrahedron vertices in local coordinates
	static Point renderVertices[4]; // Tetrahedron vertices for rendering
	static Vector renderNormals[4]; // Tetrahedron face normals in local coordinates
	static TetrahedronRenderer* unitRenderer; // Renderer for tetrahedron units
	
	/* Protected methods: */
	static void calculateShape(void); // Re-calculates tetrahedron shape after size change
	
	/* Constructors and destructors: */
	public:
	static void initClass(const Misc::ConfigurationFileSection& configFileSection); // Initializes class settings based on the given configuration file section
	Tetrahedron(const Point& sPosition,const Rotation& sOrientation)
		:StructuralUnit(sPosition,sOrientation,4)
		{
		};
	Tetrahedron(Misc::File& file)
		:StructuralUnit(4,file)
		{
		};
	static void deinitClass(void); // Destroys class data
	
	/* Methods: */
	static Scalar getClassRadius(void) // Returns radius of all tetrahedra
		{
		return radius;
		};
	static int getClassNumVertices(void) // Returns number of vertices of tetrahedron class
		{
		return 4;
		};
	static Vector getClassVertexOffset(int vertexIndex) // Returns offset from tetrahedron center to one tetrahedron vertex
		{
		return vertexOffsets[vertexIndex];
		};
	static void setRadius(Scalar newRadius); // Sets tetrahedrons' radius
	static void setMass(Scalar newMass); // Sets tetrahedrons' mass
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
