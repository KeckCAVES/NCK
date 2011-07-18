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

#ifndef GHOSTUNIT_INCLUDED
#define GHOSTUNIT_INCLUDED

#include "StructuralUnit.h"

namespace NCK {

class GhostUnit:public StructuralUnit
	{
	/* Elements: */
	protected:
	StructuralUnit* sourceUnit; // Pointer to real unit ghosted by this one
	Vector sourceOffset; // Offset from source unit to ghost unit
	
	/* Constructors and destructors: */
	public:
	GhostUnit(StructuralUnit* sSourceUnit,const Vector& sSourceOffset)
		:StructuralUnit(sSourceUnit->getPosition()+sSourceOffset,sSourceUnit->getOrientation(),sSourceUnit->getNumVertices()),
		 sourceUnit(sSourceUnit),sourceOffset(sSourceOffset)
		{
		};
	
	/* Methods: */
	virtual Scalar getRadius(void) const
		{
		return sourceUnit->getRadius();
		};
	virtual Scalar getMass(void) const
		{
		return sourceUnit->getMass();
		};
	virtual Vector getVertexOffset(int index) const
		{
		return sourceUnit->getVertexOffset(index);
		};
	virtual Point getVertex(int index) const
		{
		return position+sourceUnit->getVertexOffset(index);
		};
	virtual void applyVertexForce(int index,const Vector& force,Scalar timeStep)
		{
		//sourceUnit->applyVertexForce(index,force,timeStep);
		};
	virtual void applyCentralForce(const Vector& force,Scalar timeStep)
		{
		//sourceUnit->applyCentralForce(force,timeStep);
		};
	virtual void glRenderAction(GLContextData& contextData) const;
	void updateState(void); // Aligns state of ghost unit with that of source unit
	const StructuralUnit* getSourceUnit(void) const // Returns source unit of the ghost unit
		{
		return sourceUnit;
		};
	StructuralUnit* getSourceUnit(void) // Ditto
		{
		return sourceUnit;
		};
	const Vector& getSourceOffset(void) const // Returns source offset of the ghost unit
		{
		return sourceOffset;
		};
	static StructuralUnit* getSourceUnit(StructuralUnit* unit) // Returns the source unit of a unit that might be a ghost unit
		{
		GhostUnit* ghostUnit=dynamic_cast<GhostUnit*>(unit);
		if(ghostUnit!=0)
			return ghostUnit->sourceUnit;
		else
			return unit;
		};
	};

}

#endif
