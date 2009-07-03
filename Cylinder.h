/***********************************************************************
Cylinder - Class for cylindrical structural units.
Copyright (c) 2008 Oliver Kreylos
***********************************************************************/

#ifndef CYLINDER_INCLUDED
#define CYLINDER_INCLUDED

#include "StructuralUnit.h"

namespace NCK {

class CylinderRenderer:public UnitRenderer
	{
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	};

class Cylinder:public StructuralUnit
	{
	friend class CylinderRenderer;
	
	/* Elements: */
	protected:
	static Scalar radius,radius2; // Radius and squared radius of cylinder's circumspheres
	static Scalar mass; // Mass of a cylinder
	static Vector vertexOffsets[4]; // Cylinder vertices in local coordinates
	static CylinderRenderer* unitRenderer; // Renderer for tetrahedron units
	
	/* Protected methods: */
	static void calculateShape(void); // Re-calculates cylinder shape after size change
	
	/* Constructors and destructors: */
	public:
	static void initClass(const Misc::ConfigurationFileSection& configFileSection); // Initializes class settings based on the given configuration file section
	Cylinder(const Point& sPosition,const Rotation& sOrientation)
		:StructuralUnit(sPosition,sOrientation,4)
		{
		};
	Cylinder(Misc::File& file)
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
	static Vector getClassVertexOffset(int vertexIndex) // Returns offset from cylinder center to one cylinder vertex
		{
		return vertexOffsets[vertexIndex];
		};
	static void setRadius(Scalar newRadius); // Sets cylinder's radius
	static void setMass(Scalar newMass); // Sets cylinder's mass
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
