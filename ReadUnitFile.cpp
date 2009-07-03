/***********************************************************************
ReadUnitFile - Function to insert units from a unit file into an
existing space grid.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#include <string>
#include <Misc/File.h>

#include "StructuralUnit.h"
#include "UnitManager.h"
#include "SpaceGrid.h"

#include "ReadUnitFile.h"

namespace NCK {

/****************
Helper functions:
****************/

void addUnitsFromFile(SpaceGrid* grid,Misc::File& unitFile)
	{
	/* Read number of units from file: */
	unsigned int numUnits=unitFile.read<unsigned int>();
	
	/* Create map from indices to units: */
	StructuralUnit::IndexUnitMap indexUnitMap((numUnits*3)/2);
	unsigned int unitIndex=0;
	
	/* Read all units from file: */
	for(unsigned int unitIndex=0;unitIndex<numUnits;++unitIndex)
		{
		/* Read next unit: */
		StructuralUnit* newUnit=UnitManager::readUnit(unitFile);
		
		/* Read unit's vertex links: */
		newUnit->readVertexLinks(unitFile,indexUnitMap);
		
		/* Add unit to map: */
		indexUnitMap.setEntry(StructuralUnit::IndexUnitMap::Entry(unitIndex,newUnit));
		
		/* Add unit to grid: */
		grid->addUnit(newUnit);
		}
	}

SpaceGrid* readUnitFile(Scalar maxUnitRadius,const char* unitFileName)
	{
	/* Open unit file: */
	Misc::File unitFile(unitFileName,"rb",Misc::File::LittleEndian);
	
	/* Read space grid dimensions from file: */
	Scalar gridBoxMin[3],gridBoxMax[3];
	unitFile.read<Scalar>(gridBoxMin,3);
	unitFile.read<Scalar>(gridBoxMax,3);
	
	/* Read periodic mask from file: */
	int periodicMask=unitFile.read<int>();
	
	/* Create space grid: */
	SpaceGrid* grid=new SpaceGrid(Box(Point(gridBoxMin),Point(gridBoxMax)),maxUnitRadius,periodicMask);
	
	/* Read units from file: */
	addUnitsFromFile(grid,unitFile);
	
	/* Return new grid: */
	return grid;
	}

void readUnitFile(SpaceGrid* grid,const char* unitFileName)
	{
	/* Open unit file: */
	Misc::File unitFile(unitFileName,"rb",Misc::File::LittleEndian);
	
	/* Read space grid dimensions from file: */
	Scalar gridBoxMin[3],gridBoxMax[3];
	unitFile.read<Scalar>(gridBoxMin,3);
	unitFile.read<Scalar>(gridBoxMax,3);
	
	/* Read periodic mask from file: */
	int periodicMask=unitFile.read<int>();
	
	/* Read units from file: */
	addUnitsFromFile(grid,unitFile);
	}

void writeUnitFile(const char* unitFileName,SpaceGrid* grid)
	{
	/* Open unit file: */
	Misc::File unitFile(unitFileName,"wb",Misc::File::LittleEndian);
	
	/* Write space grid dimensions to file: */
	unitFile.write<Scalar>(grid->getBoundingBox().min.getComponents(),3);
	unitFile.write<Scalar>(grid->getBoundingBox().max.getComponents(),3);
	
	/* Write periodic mask to file: */
	unitFile.write<int>(grid->getPeriodicMask());
	
	/* Get a list of all units from grid: */
	SpaceGrid::StructuralUnitList units=grid->getAllUnits();
	
	/* Write number of units to file: */
	unitFile.write<unsigned int>(units.size());
	
	/* Create map from units to indices: */
	StructuralUnit::UnitIndexMap unitIndexMap((units.size()*3)/2);
	unsigned int unitIndex=0;
	
	/* Write all units to file: */
	for(SpaceGrid::StructuralUnitList::iterator uIt=units.begin();uIt!=units.end();++uIt)
		{
		/* Write unit's state to file: */
		UnitManager::writeUnit(*uIt,unitFile);
		
		/* Write unit's vertex links: */
		(*uIt)->writeVertexLinks(unitFile,unitIndexMap);
		
		/* Add unit to map: */
		unitIndexMap.setEntry(StructuralUnit::UnitIndexMap::Entry(*uIt,unitIndex));
		++unitIndex;
		}
	}

}
