/***********************************************************************
SpaceGridCell - Class for 3D grid cells containing structural units.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#ifndef SPACEGRIDCELL_INCLUDED
#define SPACEGRIDCELL_INCLUDED

#include "AffineSpace.h"

namespace NCK {

/* Forward declarations: */
class StructuralUnit;

class SpaceGridCell
	{
	friend class SpaceGrid;
	
	/* Elements: */
	private:
	unsigned int borderCaseMask; // Bit mask of grid borders the cell lies on
	StructuralUnit* firstUnit; // Pointer to first structural unit associated with this cell
	StructuralUnit* lastUnit; // Pointer to last structural unit associated with this cell
	
	/* Constructors and destructors: */
	public:
	SpaceGridCell(void) // Creates uninitialized grid cell
		{
		};
	
	/* Methods: */
	private:
	void linkUnit(StructuralUnit* newUnit); // Links a unit to the grid cell
	void unlinkUnit(StructuralUnit* removeUnit); // Unlinks a unit from the grid cell
	};

}

#endif
