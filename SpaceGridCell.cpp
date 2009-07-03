/***********************************************************************
SpaceGridCell - Class for 3D grid cells containing structural units.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#include "StructuralUnit.h"

#include "SpaceGridCell.h"

namespace NCK {

/******************************
Methods of class SpaceGridCell:
******************************/

void SpaceGridCell::linkUnit(StructuralUnit* newUnit)
	{
	/* Link the new unit to the cell's list: */
	newUnit->cell=this;
	newUnit->cellPred=lastUnit;
	newUnit->cellSucc=0;
	if(lastUnit!=0)
		lastUnit->cellSucc=newUnit;
	else
		firstUnit=newUnit;
	lastUnit=newUnit;
	}

void SpaceGridCell::unlinkUnit(StructuralUnit* removeUnit)
	{
	/* Unlink the unit from the cell's list: */
	if(removeUnit->cellPred!=0)
		removeUnit->cellPred->cellSucc=removeUnit->cellSucc;
	else
		firstUnit=removeUnit->cellSucc;
	if(removeUnit->cellSucc!=0)
		removeUnit->cellSucc->cellPred=removeUnit->cellPred;
	else
		lastUnit=removeUnit->cellPred;
	
	/* Invalidate the unit's link pointers: */
	removeUnit->cell=0;
	removeUnit->cellPred=removeUnit->cellSucc=0;
	}

}
