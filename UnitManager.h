/***********************************************************************
UnitManager - Class to manage the initialization and creation of
structural unit classes
Copyright (c) 2005-2011 Oliver Kreylos

This file is part of the Nanotech Construction Kit (NCK).

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

#ifndef UNITMANAGER_INCLUDED
#define UNITMANAGER_INCLUDED

#include "AffineSpace.h"

/* Forward declarations: */
namespace Misc {
class File;
class ConfigurationFileSection;
}
class GLContextData;
namespace NCK {
class StructuralUnit;
}

namespace NCK {

class UnitManager
	{
	/* Embedded classes: */
	public:
	enum UnitType // Enumerated type for structural unit types
		{
		TRIANGLE=0,TETRAHEDRON,OCTAHEDRON,CYLINDER,SPHERE,
		NUM_UNITTYPES
		};
	
	
	/* Methods: */
	static int getNumUnitTypes(void); // Returns the number of defined unit types
	static const char* getUnitTypeName(int unitTypeIndex); // Returns a name for the given unit type
	static Scalar initializeUnitTypes(const Misc::ConfigurationFileSection& configFileSection); // Initializes unit type from given configuration file section and returns radius of largest unit type
	static void deinitializeUnitTypes(void); // De-initializes unit types
	static StructuralUnit* createUnit(int unitType,const Point& position,const Rotation& orientation); // Creates a structural unit of the given type
	static StructuralUnit* readUnit(Misc::File& file); // Reads a unit from a binary file
	static void writeUnit(StructuralUnit* unit,Misc::File& file); // Writes a unit to a binary file
	};

}

#endif
