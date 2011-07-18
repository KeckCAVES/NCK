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
