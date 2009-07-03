/***********************************************************************
ReadCarFile - Function to generate a space grid with structural units
from an atom coordinate file in CAR format.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#ifndef READCARFILE_INCLUDED
#define READCARFILE_INCLUDED

namespace NCK {

/* Forward declarations: */
class SpaceGrid;

SpaceGrid* readCarFile(const char* carFileName);
void writeCarFile(const char* carFileName,SpaceGrid* grid);

}

#endif
