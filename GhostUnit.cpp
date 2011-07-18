/***********************************************************************
GhostUnit - Class for ghost copies of structural units.
Copyright (c) 2004-2011 Oliver Kreylos

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
