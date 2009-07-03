/***********************************************************************
ReadUnitFile - Function to insert units from a unit file into an
existing space grid.
Copyright (c) 2004 Oliver Kreylos
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
