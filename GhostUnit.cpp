/***********************************************************************
GhostUnit - Class for ghost copies of structural units.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

#include "GhostUnit.h"

namespace NCK {

/**************************
Methods of class GhostUnit:
**************************/

void GhostUnit::updateState(void)
	{
	/* Copy source unit's state: */
	locked=sourceUnit->getLocked();
	position=sourceUnit->getPosition()+sourceOffset;
	orientation=sourceUnit->getOrientation();
	linearVelocity=sourceUnit->getLinearVelocity();
	angularVelocity=sourceUnit->getAngularVelocity();
	
	#if 0
	/* Create fake vertex link information for links our source unit has but we don't: */
	for(int i=0;i<numVertices;++i)
		{
		VertexLink& vl=sourceUnit->getVertexLink(i);
		if(vl.unit!=0&&vertexLinks[i].unit==0)
			{
			/* Link this vertex to itself: */
			vertexLinks[i].unit=this;
			vertexLinks[i].vertexIndex=i;
			}
		else if(vl.unit==0&&vertexLinks[i].unit!=0)
			{
			/* Reset this vertex link: */
			vertexLinks[i].unit=0;
			vertexLinks[i].vertexIndex=-1;
			}
		}
	#endif
	}

void GhostUnit::glRenderAction(GLContextData& contextData) const
	{
	glPushMatrix();
	glTranslate(sourceOffset);
	sourceUnit->glRenderAction(contextData);
	glPopMatrix();
	}

}
