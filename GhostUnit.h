/***********************************************************************
GhostUnit - Class for ghost copies of structural units.
Copyright (c) 2004 Oliver Kreylos
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
