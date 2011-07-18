/***********************************************************************
SpaceGridCell - Class for 3D grid cells containing structural units.
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
