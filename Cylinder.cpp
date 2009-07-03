/***********************************************************************
Cylinder - Class for cylindrical structural units.
Copyright (c) 2008 Oliver Kreylos
***********************************************************************/

#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include "Cylinder.h"

namespace NCK {

/*********************************
Methods of class CylinderRenderer:
*********************************/

void CylinderRenderer::initContext(GLContextData& contextData) const
	{
	/* Create context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create model display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	
	/* Render cylinder: */
	Scalar hradius=Cylinder::radius/Scalar(3);
	glNormal3f(0.0f,0.0f,-1.0);
	glBegin(GL_POLYGON);
	for(int i=7;i>=0;--i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(8);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		glVertex(Point(c*hradius,s*hradius,-Cylinder::radius));
		}
	glEnd();
	
	glBegin(GL_QUAD_STRIP);
	for(int i=0;i<=8;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i%8)/Scalar(8);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		glNormal(Vector(c,s,Scalar(0)));
		glVertex(Point(c*hradius,s*hradius,Cylinder::radius));
		glVertex(Point(c*hradius,s*hradius,-Cylinder::radius));
		}
	glEnd();
	
	glNormal3f(0.0f,0.0f,1.0);
	glBegin(GL_POLYGON);
	for(int i=0;i<8;++i)
		{
		Scalar angle=Scalar(2)*Math::Constants<Scalar>::pi*Scalar(i)/Scalar(8);
		Scalar c=Math::cos(angle);
		Scalar s=Math::sin(angle);
		glVertex(Point(c*hradius,s*hradius,Cylinder::radius));
		}
	glEnd();
	
	/* Finish model display list: */
	glEndList();
	}

/*********************************
Static elements of class Cylinder:
*********************************/

Scalar Cylinder::radius;
Scalar Cylinder::radius2;
Scalar Cylinder::mass;
Vector Cylinder::vertexOffsets[4];
CylinderRenderer* Cylinder::unitRenderer=0;

/*************************
Methods of class Cylinder:
*************************/

void Cylinder::calculateShape(void)
	{
	/* Calculate vertex offset radius based on force parameters: */
	Scalar vertexRadius=calcVertexRadius(radius);
	
	/* Recalculate vertex offsets: */
	vertexOffsets[0]=Vector(-radius/Scalar(3),Scalar(0),-vertexRadius);
	vertexOffsets[1]=Vector(radius/Scalar(3),Scalar(0),-vertexRadius);
	vertexOffsets[2]=Vector(-radius/Scalar(3),Scalar(0),vertexRadius);
	vertexOffsets[3]=Vector(radius/Scalar(3),Scalar(0),vertexRadius);
	}

void Cylinder::initClass(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read class settings: */
	radius=configFileSection.retrieveValue<Scalar>("./radius",2.0);
	radius2=Math::sqr(radius);
	mass=configFileSection.retrieveValue<Scalar>("./mass",4.0);
	
	/* Calculate cylinder shape: */
	calculateShape();
	
	/* Create cylinder renderer: */
	unitRenderer=new CylinderRenderer;
	}

void Cylinder::deinitClass(void)
	{
	/* Destroy cylinder renderer: */
	delete unitRenderer;
	unitRenderer=0;
	}

void Cylinder::setRadius(Scalar newRadius)
	{
	/* Set cylinder size: */
	radius=newRadius;
	radius2=Math::sqr(radius);
	
	/* Re-calculate cylinder shape: */
	calculateShape();
	}

void Cylinder::setMass(Scalar newMass)
	{
	/* Set cylinder mass: */
	mass=newMass;
	}

void Cylinder::applyVertexForce(int index,const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	
	/* Calculate torque: */
	Vector torque=Geometry::cross(orientation.transform(vertexOffsets[index]),force);
	
	/* Apply angular acceleration: */
	angularVelocity+=torque*(timeStep/mass); // Should be moment of inertia instead of mass here
	}

void Cylinder::applyCentralForce(const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	}

void Cylinder::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve data item from GL context: */
	CylinderRenderer::DataItem* dataItem=contextData.retrieveDataItem<CylinderRenderer::DataItem>(unitRenderer);
	
	/* Move model coordinate system to cylinder's position and orientation: */
	glPushMatrix();
	glTranslate(position.getComponents());
	glRotate(orientation);
	
	/* Render cylinder: */
	glCallList(dataItem->displayListId);
	
	/* Reset model coordinates: */
	glPopMatrix();
	}

}
