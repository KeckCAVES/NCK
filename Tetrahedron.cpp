/***********************************************************************
Tetrahedron - Class for silicate structural units (used to build
glasses).
Copyright (c) 2003 Oliver Kreylos
***********************************************************************/

#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include "Tetrahedron.h"

namespace NCK {

/************************************
Methods of class TetrahedronRenderer:
************************************/

void TetrahedronRenderer::initContext(GLContextData& contextData) const
	{
	/* Create context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create model display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	
	/* Render tetrahedron: */
	glBegin(GL_TRIANGLES);
	glNormal(Tetrahedron::renderNormals[0]);
	glVertex(Tetrahedron::renderVertices[1]);
	glVertex(Tetrahedron::renderVertices[2]);
	glVertex(Tetrahedron::renderVertices[3]);
	glNormal(Tetrahedron::renderNormals[1]);
	glVertex(Tetrahedron::renderVertices[2]);
	glVertex(Tetrahedron::renderVertices[0]);
	glVertex(Tetrahedron::renderVertices[3]);
	glNormal(Tetrahedron::renderNormals[2]);
	glVertex(Tetrahedron::renderVertices[0]);
	glVertex(Tetrahedron::renderVertices[1]);
	glVertex(Tetrahedron::renderVertices[3]);
	glNormal(Tetrahedron::renderNormals[3]);
	glVertex(Tetrahedron::renderVertices[0]);
	glVertex(Tetrahedron::renderVertices[2]);
	glVertex(Tetrahedron::renderVertices[1]);
	glEnd();
	
	/* Finish model display list: */
	glEndList();
	}

/************************************
Static elements of class Tetrahedron:
************************************/

Scalar Tetrahedron::radius;
Scalar Tetrahedron::radius2;
Scalar Tetrahedron::mass;
Vector Tetrahedron::vertexOffsets[4];
Point Tetrahedron::renderVertices[4];
Vector Tetrahedron::renderNormals[4];
TetrahedronRenderer* Tetrahedron::unitRenderer=0;

/****************************
Methods of class Tetrahedron:
****************************/

void Tetrahedron::calculateShape(void)
	{
	/* Calculate vertex offset radius based on force parameters: */
	Scalar vertexRadius=calcVertexRadius(radius);
	
	/* Recalculate vertex offsets: */
	Scalar c1=Math::cos(Math::rad(30.0));
	Scalar s1=Math::sin(Math::rad(30.0));
	Scalar tetAngle=Math::acos(Scalar(-1)/Scalar(3));
	Scalar c2=Math::sin(tetAngle);
	Scalar s2=-Math::cos(tetAngle);
	Vector v[4];
	v[0]=Vector(-c1*c2,-s1*c2,-s2);
	v[1]=Vector(c1*c2,-s1*c2,-s2);
	v[2]=Vector(Scalar(0),c2,-s2);
	v[3]=Vector(Scalar(0),Scalar(0),Scalar(1));
	
	for(int i=0;i<4;++i)
		vertexOffsets[i]=v[i]*vertexRadius;
	
	/* Recalculate render vertices and normals: */
	for(int i=0;i<4;++i)
		{
		renderVertices[i]=Point::origin+v[i]*radius;
		renderNormals[i]=-v[i];
		}
	}

void Tetrahedron::initClass(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read class settings: */
	radius=configFileSection.retrieveValue<Scalar>("./radius",1.550185);
	radius2=Math::sqr(radius);
	mass=configFileSection.retrieveValue<Scalar>("./mass",1.0);
	
	/* Calculate tetrahedron shape: */
	calculateShape();
	
	/* Create tetrahedron renderer: */
	unitRenderer=new TetrahedronRenderer;
	}

void Tetrahedron::deinitClass(void)
	{
	/* Destroy tetrahedron renderer: */
	delete unitRenderer;
	unitRenderer=0;
	}

void Tetrahedron::setRadius(Scalar newRadius)
	{
	/* Set tetrahedron size: */
	radius=newRadius;
	radius2=Math::sqr(radius);
	
	/* Re-calculate tetrahedron shape: */
	calculateShape();
	}

void Tetrahedron::setMass(Scalar newMass)
	{
	/* Set tetrahedron mass: */
	mass=newMass;
	}

void Tetrahedron::applyVertexForce(int index,const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	
	/* Calculate torque: */
	Vector torque=Geometry::cross(orientation.transform(vertexOffsets[index]),force);
	
	/* Apply angular acceleration: */
	angularVelocity+=torque*(timeStep/mass); // Should be moment of inertia instead of mass here
	}

void Tetrahedron::applyCentralForce(const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	}

void Tetrahedron::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve data item from GL context: */
	TetrahedronRenderer::DataItem* dataItem=contextData.retrieveDataItem<TetrahedronRenderer::DataItem>(unitRenderer);
	
	/* Move model coordinate system to tetrahedron's position and orientation: */
	glPushMatrix();
	glTranslate(position.getComponents());
	glRotate(orientation);
	
	/* Render tetrahedron: */
	glCallList(dataItem->displayListId);
	
	/* Reset model coordinates: */
	glPopMatrix();
	}

}
