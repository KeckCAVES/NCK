/***********************************************************************
Octahedron - Class for aluminum oxide structural units (used to build
heterogeneous silicate crystals).
Copyright (c) 2003 Oliver Kreylos
***********************************************************************/

#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include "Octahedron.h"

namespace NCK {

/***********************************
Methods of class OctahedronRenderer:
***********************************/

void OctahedronRenderer::initContext(GLContextData& contextData) const
	{
	/* Create context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create model display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	
	/* Render octahedron: */
	static const int faceVertexIndices[8][3]={{2,0,4},{1,2,4},{3,1,4},{0,3,4},
	                                          {0,2,5},{2,1,5},{1,3,5},{3,0,5}};
	glBegin(GL_TRIANGLES);
	for(int face=0;face<8;++face)
		{
		glNormal(Octahedron::renderNormals[face]);
		for(int i=0;i<3;++i)
			glVertex(Octahedron::renderVertices[faceVertexIndices[face][i]]);
		}
	glEnd();
	
	/* Finish model display list: */
	glEndList();
	}

/***********************************
Static elements of class Octahedron:
***********************************/

Scalar Octahedron::radius;
Scalar Octahedron::radius2;
Scalar Octahedron::mass;
Vector Octahedron::vertexOffsets[6];
Point Octahedron::renderVertices[6];
Vector Octahedron::renderNormals[8];
OctahedronRenderer* Octahedron::unitRenderer=0;

/***************************
Methods of class Octahedron:
***************************/

void Octahedron::calculateShape(void)
	{
	/* Calculate vertex offset radius based on force parameters: */
	Scalar vertexRadius=calcVertexRadius(radius);
	
	/* Recalculate vertex offsets and render vertices: */
	for(int i=0;i<6;++i)
		{
		vertexOffsets[i]=Vector::zero;
		renderVertices[i]=Point::origin;
		}
	for(int i=0;i<3;++i)
		{
		vertexOffsets[2*i+0][i]=-vertexRadius;
		vertexOffsets[2*i+1][i]=vertexRadius;
		renderVertices[2*i+0][i]=-vertexRadius;
		renderVertices[2*i+1][i]=vertexRadius;
		}
	
	/* Recalculate render normals: */
	Scalar n=Scalar(1.0)/Math::sqrt(3.0);
	renderNormals[0]=Vector(-n,-n,-n);
	renderNormals[1]=Vector(n,-n,-n);
	renderNormals[2]=Vector(n,n,-n);
	renderNormals[3]=Vector(-n,n,-n);
	renderNormals[4]=Vector(-n,-n,n);
	renderNormals[5]=Vector(n,-n,n);
	renderNormals[6]=Vector(n,n,n);
	renderNormals[7]=Vector(-n,n,n);
	}

void Octahedron::initClass(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read class settings: */
	radius=configFileSection.retrieveValue<Scalar>("./radius",1.9);
	radius2=Math::sqr(radius);
	mass=configFileSection.retrieveValue<Scalar>("./mass",4.0/3.0);
	
	/* Calculate octahedron shape: */
	calculateShape();
	
	/* Create octahedron renderer: */
	unitRenderer=new OctahedronRenderer;
	}

void Octahedron::deinitClass(void)
	{
	/* Destroy octahedron renderer: */
	delete unitRenderer;
	unitRenderer=0;
	}

void Octahedron::setRadius(Scalar newRadius)
	{
	/* Set octahedron size: */
	radius=newRadius;
	radius2=Math::sqr(radius);
	
	/* Re-calculate octahedron shape: */
	calculateShape();
	}

void Octahedron::setMass(Scalar newMass)
	{
	/* Set octahedron mass: */
	mass=newMass;
	}

void Octahedron::applyVertexForce(int index,const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	
	/* Calculate torque: */
	Vector torque=Geometry::cross(orientation.transform(vertexOffsets[index]),force);
	
	/* Apply angular acceleration: */
	angularVelocity+=torque*(timeStep/mass); // Should be moment of inertia instead of mass here
	}

void Octahedron::applyCentralForce(const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	}

void Octahedron::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve data item from GL context: */
	OctahedronRenderer::DataItem* dataItem=contextData.retrieveDataItem<OctahedronRenderer::DataItem>(unitRenderer);
	
	/* Move model coordinate system to octahedron's position and orientation: */
	glPushMatrix();
	glTranslate(position.getComponents());
	glRotate(orientation);
	
	/* Render octahedron: */
	glCallList(dataItem->displayListId);
	
	/* Reset model coordinates: */
	glPopMatrix();
	}

}
