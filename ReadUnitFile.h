/***********************************************************************
ReadUnitFile - Function to insert units from a unit file into an
existing space grid.
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

#ifndef READUNITFILE_INCLUDED
#define READUNITFILE_INCLUDED

namespace NCK {

/* Forward declarations: */
class SpaceGrid;

SpaceGrid* readUnitFile(Scalar maxUnitRadius,const char* unitFileName); // Creates new space grid and adds units from given unit file
void readUnitFile(SpaceGrid* grid,const char* unitFileName); // Adds units from given unit file to existing space grid
void writeUnitFile(const char* unitFileName,SpaceGrid* grid); // Writes all units from given space grid into file

}

#endif
