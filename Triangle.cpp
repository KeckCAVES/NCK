/***********************************************************************
Triangle - Class for carbon structural units (used to build nanotubes
and Buckyballs).
Copyright (c) 2003 Oliver Kreylos
***********************************************************************/

#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include "Triangle.h"

namespace NCK {

/*********************************
Methods of class TriangleRenderer:
*********************************/

void TriangleRenderer::initContext(GLContextData& contextData) const
	{
	/* Create context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create model display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	
	/* Render triangle sides: */
	glBegin(GL_QUADS);
	glNormal(Triangle::renderNormals[0]);
	glVertex(Triangle::renderVertices[0]);
	glVertex(Triangle::renderVertices[1]);
	glVertex(Triangle::renderVertices[4]);
	glVertex(Triangle::renderVertices[3]);
	glNormal(Triangle::renderNormals[1]);
	glVertex(Triangle::renderVertices[1]);
	glVertex(Triangle::renderVertices[2]);
	glVertex(Triangle::renderVertices[5]);
	glVertex(Triangle::renderVertices[4]);
	glNormal(Triangle::renderNormals[2]);
	glVertex(Triangle::renderVertices[2]);
	glVertex(Triangle::renderVertices[0]);
	glVertex(Triangle::renderVertices[3]);
	glVertex(Triangle::renderVertices[5]);
	glEnd();
	
	/* Render triangle caps: */
	glBegin(GL_TRIANGLES);
	glNormal(Triangle::renderNormals[3]);
	glVertex(Triangle::renderVertices[0]);
	glVertex(Triangle::renderVertices[2]);
	glVertex(Triangle::renderVertices[1]);
	glNormal(Triangle::renderNormals[4]);
	glVertex(Triangle::renderVertices[3]);
	glVertex(Triangle::renderVertices[4]);
	glVertex(Triangle::renderVertices[5]);
	glEnd();
	
	/* Finish model display list: */
	glEndList();
	}

/*********************************
Static elements of class Triangle:
*********************************/

Scalar Triangle::radius;
Scalar Triangle::radius2;
Scalar Triangle::width;
Scalar Triangle::mass;
Vector Triangle::vertexOffsets[3];
Point Triangle::renderVertices[6];
Vector Triangle::renderNormals[5];
TriangleRenderer* Triangle::unitRenderer=0;

/*************************
Methods of class Triangle:
*************************/

void Triangle::calculateShape(void)
	{
	/* Calculate vertex offset radius based on force parameters: */
	Scalar vertexRadius=calcVertexRadius(radius);
	
	/* Recalculate vertex offsets: */
	Scalar c=Math::cos(Math::rad(Scalar(30)));
	Scalar s=Math::sin(Math::rad(Scalar(30)));
	Vector v[3];
	v[0]=Vector(-c,-s,Scalar(0));
	v[1]=Vector(c,-s,Scalar(0));
	v[2]=Vector(Scalar(0),Scalar(1),Scalar(0));
	for(int i=0;i<3;++i)
		vertexOffsets[i]=v[i]*vertexRadius;
	
	/* Recalculate render vertices and normals: */
	Point bottom=Point(Scalar(0),Scalar(0),-width);
	Point top=Point(Scalar(0),Scalar(0),width);
	for(int i=0;i<3;++i)
		{
		renderVertices[i]=bottom+v[i]*radius;
		renderVertices[3+i]=top+v[i]*radius;
		}
	for(int i=0;i<3;++i)
		renderNormals[i]=-v[(i+2)%3];
	renderNormals[3]=Vector(Scalar(0),Scalar(0),Scalar(-1));
	renderNormals[4]=Vector(Scalar(0),Scalar(0),Scalar(1));
	}

void Triangle::initClass(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read class settings: */
	radius=configFileSection.retrieveValue<Scalar>("./radius",1.550185);
	radius2=Math::sqr(radius);
	width=configFileSection.retrieveValue<Scalar>("./width",0.25);
	mass=configFileSection.retrieveValue<Scalar>("./mass",1.0);
	
	/* Calculate triangle shape: */
	calculateShape();
	
	/* Create triangle renderer: */
	unitRenderer=new TriangleRenderer;
	}

void Triangle::deinitClass(void)
	{
	/* Destroy triangle renderer: */
	delete unitRenderer;
	unitRenderer=0;
	}

void Triangle::setRadius(Scalar newRadius)
	{
	/* Set radius: */
	radius=newRadius;
	radius2=Math::sqr(radius);
	
	/* Re-calculate triangle shape: */
	calculateShape();
	}

void Triangle::setWidth(Scalar newWidth)
	{
	width=newWidth;
	
	/* Re-calculate triangle shape: */
	calculateShape();
	}

void Triangle::setMass(Scalar newMass)
	{
	mass=newMass;
	}

void Triangle::applyVertexForce(int index,const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	
	/* Calculate torque: */
	Vector torque=Geometry::cross(orientation.transform(vertexOffsets[index]),force);
	
	/* Apply angular acceleration: */
	angularVelocity+=torque*(timeStep/mass); // Should be moment of inertia instead of mass here
	}

void Triangle::applyCentralForce(const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	}

void Triangle::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve data item from GL context: */
	TriangleRenderer::DataItem* dataItem=contextData.retrieveDataItem<TriangleRenderer::DataItem>(unitRenderer);
	
	/* Move model coordinate system to triangle's position and orientation: */
	glPushMatrix();
	glTranslate(position.getComponents());
	glRotate(orientation);
	
	/* Render triangle: */
	glCallList(dataItem->displayListId);
	
	/* Reset model coordinates: */
	glPopMatrix();
	}

}
