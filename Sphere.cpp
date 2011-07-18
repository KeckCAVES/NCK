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

#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>

#include "Sphere.h"

namespace NCK {

/***********************************
Methods of class OctahedronRenderer:
***********************************/

void SphereRenderer::initContext(GLContextData& contextData) const
	{
	/* Create context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create model display list: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	
	/* Render sphere: */
	glDrawSphereIcosahedron(GLfloat(Sphere::radius),3);
	
	/* Finish model display list: */
	glEndList();
	}

/*******************************
Static elements of class Sphere:
*******************************/

Scalar Sphere::radius;
Scalar Sphere::radius2;
Scalar Sphere::mass;
SphereRenderer* Sphere::unitRenderer=0;

/***********************
Methods of class Sphere:
***********************/

void Sphere::initClass(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read class settings: */
	radius=configFileSection.retrieveValue<Scalar>("./radius",1.0);
	radius2=Math::sqr(radius);
	mass=configFileSection.retrieveValue<Scalar>("./mass",1.0);
	
	/* Create sphere renderer: */
	unitRenderer=new SphereRenderer;
	}

void Sphere::deinitClass(void)
	{
	/* Destroy sphere renderer: */
	delete unitRenderer;
	unitRenderer=0;
	}

void Sphere::setRadius(Scalar newRadius)
	{
	/* Set sphere size: */
	radius=newRadius;
	radius2=Math::sqr(radius);
	}

void Sphere::setMass(Scalar newMass)
	{
	/* Set sphere mass: */
	mass=newMass;
	}

void Sphere::applyVertexForce(int index,const Vector& force,Scalar timeStep)
	{
	/* This method is never called */
	}

void Sphere::applyCentralForce(const Vector& force,Scalar timeStep)
	{
	/* Apply linear acceleration: */
	linearVelocity+=force*(timeStep/mass);
	}

void Sphere::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve data item from GL context: */
	SphereRenderer::DataItem* dataItem=contextData.retrieveDataItem<SphereRenderer::DataItem>(unitRenderer);
	
	/* Move model coordinate system to sphere's position and orientation: */
	glPushMatrix();
	glTranslate(position.getComponents());
	glRotate(orientation);
	
	/* Render sphere: */
	glCallList(dataItem->displayListId);
	
	/* Reset model coordinates: */
	glPopMatrix();
	}

}
