/***********************************************************************
SpaceGrid - Class for 3D grids of cells to store and locate structural
units.
Copyright (c) 2003 Oliver Kreylos
***********************************************************************/

#include <queue>
#include <iostream>
#include <iomanip>
#include <Misc/HashTable.h>
#include <Misc/OneTimeQueue.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>

#include "StructuralUnit.h"
#include "GhostUnit.h"

#include "SpaceGrid.h"

namespace NCK {

/**************************
Methods of class SpaceGrid:
**************************/

void SpaceGrid::initializeGrid(void)
	{
	/* Pad grid size to include ghost cells: */
	Index fullGridSize=gridSize;
	for(int i=0;i<3;++i)
		fullGridSize[i]+=2;
	
	/* Resize cell array including ghost cells: */
	cells.resize(fullGridSize);
	
	/* Initialize all grid cells: */
	for(Index index(0);index[0]<fullGridSize[0];index.preInc(fullGridSize))
		{
		SpaceGridCell& cell=cells(index);
		
		/* Calculate cell's border case mask: */
		cell.borderCaseMask=0x0;
		for(int i=0;i<3;++i)
			{
			bool isGhostCell=false;
			if(periodicFlags[i])
				{
				if(index[i]<1&&index[i]>gridSize[i])
					isGhostCell=true;
				if(index[i]==1)
					cell.borderCaseMask|=0x1<<(2*i+0);
				if(index[i]==gridSize[i])
					cell.borderCaseMask|=0x1<<(2*i+1);
				}
			if(isGhostCell)
				cell.borderCaseMask|=0x40;
			}
		
		/* Initialize cell's unit list: */
		cell.firstUnit=cell.lastUnit=0;
		}
	}

SpaceGridCell* SpaceGrid::findCell(const Point& p)
	{
	/* Calculate index of cell containing point: */
	Index cellIndex;
	for(int i=0;i<3;++i)
		{
		/* Calculate index component: */
		cellIndex[i]=int(Math::floor((p[i]-gridBox.min[i])/cellSize[i]))+1;
		if(cellIndex[i]<1)
			cellIndex[i]=1;
		else if(cellIndex[i]>gridSize[i])
			cellIndex[i]=gridSize[i];
		}
	
	/* Return pointer to found cell: */
	return &cells(cellIndex);
	}

SpaceGrid::SpaceGrid(const Box& sGridBox,Scalar sMaxUnitRadius,int periodicMask)
	:gridBox(sGridBox),maxUnitRadius(sMaxUnitRadius),nextUnitId(1),
	 firstUnit(0),lastUnit(0),firstGhostUnit(0),lastGhostUnit(0),
	 attenuation(0.5),
	 showGridBoundary(true),
	 gridBoundaryColor(1.0f,1.0f,1.0f),
	 gridBoundaryLineWidth(1.0f),
	 showUnits(true),
	 defaultUnitColor(0.6f,0.6f,0.6f),
	 markedUnitColor(0.5f,1.0f,0.5f),
	 lockedUnitColor(1.0f,0.5f,0.5f),
	 unitMaterial(defaultUnitColor,Color(1.0f,1.0f,1.0f),50.0f),
	 showVelocities(false),
	 showVertexLinks(true),
	 vertexLinkColor(0.7f,0.5f,0.7f),
	 vertexLinkLineWidth(1.0f),
	 showUnlinkedVertices(false),
	 unlinkedVertexRadius(0.5f),
	 unlinkedVertexSubdivision(3),
	 unlinkedVertexMaterial(Color(1.0f,0.0f,0.0f),Color(1.0f,1.0f,1.0f),25.0f)
	{
	/* Calculate "optimal" grid cell size: */
	Scalar minCellSize=Scalar(2)*maxUnitRadius+StructuralUnit::vertexForceRadius;
	
	/* Calculate "optimal" grid size: */
	Size gridBoxSize;
	for(int i=0;i<3;++i)
		{
		gridBoxSize[i]=gridBox.getSize(i);
		gridSize[i]=int(Math::floor(gridBoxSize[i]/minCellSize));
		cellSize[i]=gridBoxSize[i]/Scalar(gridSize[i]);
		}
	
	/* Set periodicity flags: */
	for(int i=0;i<3;++i)
		periodicFlags[i]=periodicMask&(1<<i);
	
	/* Initialize grid cell array: */
	initializeGrid();
	
	/* Calculate cell neighbour offsets: */
	Index index;
	int neighbourIndex;
	for(neighbourIndex=0,index[0]=-1;index[0]<=1;++index[0])
		for(index[1]=-1;index[1]<=1;++index[1])
			for(index[2]=-1;index[2]<=1;++index[2],++neighbourIndex)
				{
				cellNeighbourOffsets[neighbourIndex]=0;
				for(int j=0;j<3;++j)
					cellNeighbourOffsets[neighbourIndex]+=index[j]*cells.getIncrement(j);
				}
	
	/* Create face border cases: */
	int caseIndex=0;
	for(int i=0;i<3;++i)
		for(int i1=0;i1<2;++i1)
			borderCases[caseIndex++].caseMask=0x1<<(2*i+i1);
	
	/* Create edge border cases: */
	for(int i=0;i<2;++i)
		for(int j=i+1;j<3;++j)
			for(int i1=0;i1<2;++i1)
				for(int j1=0;j1<2;++j1)
					borderCases[caseIndex++].caseMask=(0x1<<(2*i+i1))|(0x1<<(2*j+j1));
	
	/* Create corner border cases: */
	for(int i1=0;i1<2;++i1)
		for(int j1=0;j1<2;++j1)
			for(int k1=0;k1<2;++k1)
				borderCases[caseIndex++].caseMask=(0x1<<(2*0+i1))|(0x1<<(2*1+j1))|(0x1<<(2*2+k1));
	
	/* Initialize index and position offsets: */
	for(int i=0;i<26;++i)
		{
		borderCases[i].pointerOffset=0;
		for(int j=0;j<3;++j)
			{
			if(borderCases[i].caseMask&(0x1<<(2*j+0)))
				{
				borderCases[i].indexOffset[j]=gridSize[j];
				borderCases[i].pointerOffset+=gridSize[j]*cells.getIncrement(j);
				borderCases[i].positionOffset[j]=gridBoxSize[j];
				}
			else if(borderCases[i].caseMask&(0x1<<(2*j+1)))
				{
				borderCases[i].indexOffset[j]=-gridSize[j];
				borderCases[i].pointerOffset-=gridSize[j]*cells.getIncrement(j);
				borderCases[i].positionOffset[j]=-gridBoxSize[j];
				}
			else
				{
				borderCases[i].indexOffset[j]=0;
				borderCases[i].positionOffset[j]=Scalar(0);
				}
			}
		}
	}

SpaceGrid::~SpaceGrid(void)
	{
	/* Delete all structural units: */
	while(firstUnit!=0)
		{
		StructuralUnit* succ=firstUnit->succ;
		delete firstUnit;
		firstUnit=succ;
		}
	
	/* Delete all ghost units: */
	while(firstGhostUnit!=0)
		{
		StructuralUnit* succ=firstGhostUnit->succ;
		delete firstGhostUnit;
		firstGhostUnit=succ;
		}
	}

void SpaceGrid::initContext(GLContextData& contextData) const
	{
	/* Create a context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Render the unlinked vertex marker: */
	glNewList(dataItem->vertexMarkerDisplayListId,GL_COMPILE);
	glDrawSphereIcosahedron(unlinkedVertexRadius,unlinkedVertexSubdivision);
	glEndList();
	}

int SpaceGrid::getPeriodicMask(void) const
	{
	int periodicMask=0x0;
	for(int i=0;i<3;++i)
		if(periodicFlags[i])
			periodicMask|=1<<i;
	
	return periodicMask;
	}

void SpaceGrid::setAttenuation(Scalar newAttenuation)
	{
	attenuation=newAttenuation;
	}

void SpaceGrid::addUnit(StructuralUnit* newUnit)
	{
	/* Set unit's ID number: */
	newUnit->id=nextUnitId;
	++nextUnitId;
	
	/* Link new unit to main list: */
	newUnit->pred=lastUnit;
	if(lastUnit!=0)
		lastUnit->succ=newUnit;
	else
		firstUnit=newUnit;
	lastUnit=newUnit;
	
	/* Set unit's color to default: */
	newUnit->setColor(StructuralUnit::Color(defaultUnitColor.getRgba()));
	
	/* Find grid cell containing unit: */
	SpaceGridCell* cell=findCell(newUnit->position);
	
	/* Link unit to its cell: */
	cell->linkUnit(newUnit);
	
	if(cell->borderCaseMask!=0x0)
		{
		/* Create ghosts of new unit in each ghost cell associated with this cell: */
		for(int i=0;i<26;++i)
			if((cell->borderCaseMask&borderCases[i].caseMask)==borderCases[i].caseMask)
				{
				/* Create a new ghost unit for the new unit and link it to the ghost cell: */
				GhostUnit* newGhostUnit=new GhostUnit(newUnit,borderCases[i].positionOffset);
				newGhostUnit->id=0;
				newGhostUnit->pred=lastGhostUnit;
				if(lastGhostUnit!=0)
					lastGhostUnit->succ=newGhostUnit;
				else
					firstGhostUnit=newGhostUnit;
				lastGhostUnit=newGhostUnit;
				(cell+borderCases[i].pointerOffset)->linkUnit(newGhostUnit);
				}
		}
	}

SpaceGrid::StructuralUnitList SpaceGrid::getAllUnits(void)
	{
	/* Add all units from the main list to the result list: */
	StructuralUnitList result;
	for(StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
		result.push_back(uPtr);
	return result;
	}

StructuralUnit* SpaceGrid::findUnit(const Point& p)
	{
	/* Find pointer of the cell containing the query point: */
	SpaceGridCell* cell=findCell(p);
	
	/* Search cell's neighbourhood for units: */
	Scalar minDist2=Math::sqr(maxUnitRadius);
	StructuralUnit* result=0;
	for(int i=0;i<27;++i)
		{
		/* Get pointer to cell's neighbour: */
		SpaceGridCell* cell1=cell+cellNeighbourOffsets[i];
		
		/* Exclude ghost cells from search: */
		if(!(cell1->borderCaseMask&0x40))
			{
			/* Test all units in the cell: */
			for(StructuralUnit* uPtr=cell1->firstUnit;uPtr!=0;uPtr=uPtr->cellSucc)
				{
				Scalar dist2=Geometry::sqrDist(uPtr->position,p);
				if(dist2<minDist2&&dist2<=Math::sqr(uPtr->getRadius()))
					{
					minDist2=dist2;
					result=uPtr;
					}
				}
			}
		}
	
	return result;
	}

StructuralUnit* SpaceGrid::findUnit(const Ray& r)
	{
	/* For now, intersect ray with all objects: */
	StructuralUnit* closestUnit=0;
	Scalar closestLambda=Math::Constants<Scalar>::max;
	Scalar dirLen2=Geometry::sqr(r.getDirection());
	for(StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
		{
		/* Intersect unit's bounding sphere with ray: */
		Vector d=uPtr->getPosition()-r.getOrigin();
		Scalar ph=(d*r.getDirection())/dirLen2;
		Scalar det=Math::sqr(ph)+(Math::sqr(uPtr->getRadius())-Geometry::sqr(d))/dirLen2;
		if(det>=Scalar(0))
			{
			det=Math::sqrt(det);
			Scalar lambda1=ph-det;
			Scalar lambda2=ph+det;
			if(lambda1>=Scalar(0))
				{
				if(lambda1<closestLambda)
					{
					closestUnit=uPtr;
					closestLambda=lambda1;
					}
				}
			else if(lambda2>=Scalar(0))
				{
				if(Scalar(0)<closestLambda)
					{
					closestUnit=uPtr;
					closestLambda=Scalar(0);
					}
				}
			}
		}
	
	return closestUnit;
	}

SpaceGrid::StructuralUnitList SpaceGrid::findUnits(const Point& p,Scalar radius)
	{
	/* Calculate range of cell potentially overlapping the query sphere: */
	Index cellIndexMin,cellIndexMax;
	for(int i=0;i<3;++i)
		{
		/* Calculate index components: */
		cellIndexMin[i]=int(Math::floor((p[i]-radius-gridBox.min[i])/cellSize[i]))+0;
		if(cellIndexMin[i]<1)
			cellIndexMin[i]=1;
		else if(cellIndexMin[i]>gridSize[i]+1)
			cellIndexMin[i]=gridSize[i]+1;
		cellIndexMax[i]=int(Math::floor((p[i]+radius-gridBox.min[i])/cellSize[i]))+3;
		if(cellIndexMax[i]<1)
			cellIndexMax[i]=1;
		else if(cellIndexMax[i]>gridSize[i]+1)
			cellIndexMax[i]=gridSize[i]+1;
		}
	
	/* Search range of cells for units: */
	StructuralUnitList result;
	for(Index cellIndex=cellIndexMin;cellIndex[0]<cellIndexMax[0];cellIndex.preInc(cellIndexMin,cellIndexMax))
		{
		/* Test all units in the cell: */
		SpaceGridCell& cell=cells(cellIndex);
		for(StructuralUnit* uPtr=cell.firstUnit;uPtr!=0;uPtr=uPtr->cellSucc)
			{
			Scalar dist2=Geometry::sqrDist(uPtr->position,p);
			if(dist2<=Math::sqr(uPtr->getRadius()+radius))
				result.push_back(uPtr);
			}
		}
	
	return result;
	}

SpaceGrid::StructuralUnitList SpaceGrid::getLinkedUnits(StructuralUnit* unit)
	{
	/* Traverse linkage graph to find all units linked to the selected one: */
	StructuralUnitList result;
	Misc::OneTimeQueue<StructuralUnit*> unitQueue(101);
	if(unit!=0)
		unitQueue.push(unit);
	while(!unitQueue.empty())
		{
		/* Get next unit from traversal queue: */
		StructuralUnit* unit=unitQueue.front();
		unitQueue.pop();
		
		/* Add unit to result list: */
		result.push_back(unit);
		
		/* Insert all units linked to the current unit into the unit queue: */
		int numVertices=unit->getNumVertices();
		for(int i=0;i<numVertices;++i)
			{
			StructuralUnit* linkedUnit=unit->getVertexLink(i).unit;
			
			/* Check if the vertex link is valid: */
			if(linkedUnit!=0)
				{
				/* Resolve ghost unit indirection: */
				linkedUnit=GhostUnit::getSourceUnit(linkedUnit);
				unitQueue.push(linkedUnit);
				}
			}
		}
	
	/* Return result list: */
	return result;
	}

SpaceGrid::StructuralUnitList SpaceGrid::findLinkedUnits(const Point& p)
	{
	/* Find unit containing query point: */
	StructuralUnit* selectedUnit=findUnit(p);
	
	/* Return list of all units linked to selected unit: */
	return getLinkedUnits(selectedUnit);
	}

SpaceGrid::StructuralUnitList SpaceGrid::findLinkedUnits(const Ray& r)
	{
	/* Find closest unit intersecting query ray: */
	StructuralUnit* selectedUnit=findUnit(r);
	
	/* Return list of all units linked to selected unit: */
	return getLinkedUnits(selectedUnit);
	}

StructuralUnit* SpaceGrid::findClosestUnit(const Point& p,Scalar maxDist)
	{
	/* Find index of the cell containing the query point: */
	Index startCellIndex;
	for(int i=0;i<3;++i)
		{
		/* Calculate index component: */
		startCellIndex[i]=int(Math::floor((p[i]-gridBox.min[i])/cellSize[i]))+1;
		if(startCellIndex[i]<1)
			startCellIndex[i]=1;
		else if(startCellIndex[i]>gridSize[i])
			startCellIndex[i]=gridSize[i];
		}
	
	/* Search outwards from the start cell until a unit is found: */
	Scalar minDist2=Math::sqr(maxDist);
	StructuralUnit* result=0;
	std::queue<Index> cellQueue;
	Misc::HashTable<SpaceGridCell*,void> pushedCells(101);
	cellQueue.push(startCellIndex);
	pushedCells.setEntry(Misc::HashTable<SpaceGridCell*,void>::Entry(&cells(startCellIndex)));
	while(!cellQueue.empty())
		{
		/* Visit the next cell in the queue: */
		Index cellIndex=cellQueue.front();
		cellQueue.pop();
		
		/* Calculate minimal distance between query point and cell: */
		Scalar cellDist2=Scalar(0);
		for(int i=0;i<3;++i)
			{
			Scalar d;
			if((d=Scalar(cellIndex[i])*cellSize[i]-p[i])>Scalar(0))
				cellDist2+=d*d;
			else if((d=p[i]-Scalar(cellIndex[i]+1)*cellSize[i])>Scalar(0))
				cellDist2+=d*d;
			}
		
		/* Disregard the cell if it is farther away than the current closest unit: */
		if(cellDist2<minDist2)
			{
			/* Check all structural units in the cell: */
			SpaceGridCell* cell=&cells(cellIndex);
			for(StructuralUnit* uPtr=cell->firstUnit;uPtr!=0;uPtr=uPtr->cellSucc)
				{
				Scalar dist2=Geometry::sqrDist(uPtr->position,p);
				if(minDist2>dist2)
					{
					minDist2=dist2;
					result=uPtr;
					}
				}
			
			/* Put all neighbours of the cell into the queue: */
			for(int i=0;i<3;++i)
				{
				/* Go to left neighbour: */
				Index n1Index=cellIndex;
				if(n1Index[i]>1)
					--n1Index[i];
				else
					n1Index[i]=gridSize[i];
				SpaceGridCell* n1=&cells(n1Index);
				if(!pushedCells.isEntry(n1))
					{
					/* Add the cell to the queue: */
					cellQueue.push(n1Index);
					pushedCells.setEntry(Misc::HashTable<SpaceGridCell*,void>::Entry(n1));
					}
				
				/* Go to right neighbour: */
				Index n2Index=cellIndex;
				if(n2Index[i]<gridSize[i])
					++n2Index[i];
				else
					n2Index[i]=1;
				SpaceGridCell* n2=&cells(n2Index);
				if(!pushedCells.isEntry(n2))
					{
					/* Add the cell to the queue: */
					cellQueue.push(n2Index);
					pushedCells.setEntry(Misc::HashTable<SpaceGridCell*,void>::Entry(n2));
					}
				}
			}
		}
	
	return result;
	}

void SpaceGrid::markUnit(StructuralUnit* unit)
	{
	/* Mark the given unit: */
	unit->setMarked(true);
	}

void SpaceGrid::unmarkUnit(StructuralUnit* unit)
	{
	/* Unmark the given unit: */
	unit->setMarked(false);
	}

void SpaceGrid::toggleUnitMark(StructuralUnit* unit)
	{
	/* Toggle the unit's marked state: */
	unit->setMarked(!unit->getMarked());
	}

void SpaceGrid::toggleUnitsMark(const SpaceGrid::StructuralUnitList& units)
	{
	/* Determine marked state of majority of units: */
	int numMarkedUnits=0;
	int numUnmarkedUnits=0;
	for(StructuralUnitList::const_iterator uIt=units.begin();uIt!=units.end();++uIt)
		{
		if((*uIt)->getMarked())
			++numMarkedUnits;
		else
			++numUnmarkedUnits;
		}
	
	/* Apply new marked state: */
	bool newMarkedState=numUnmarkedUnits>numMarkedUnits;
	for(StructuralUnitList::const_iterator uIt=units.begin();uIt!=units.end();++uIt)
		(*uIt)->setMarked(newMarkedState);
	}

void SpaceGrid::lockUnit(StructuralUnit* unit)
	{
	/* Lock the given unit: */
	unit->setLocked(true);
	}

void SpaceGrid::unlockUnit(StructuralUnit* unit)
	{
	/* Unlock the given unit: */
	unit->setLocked(false);
	}

void SpaceGrid::toggleUnitLock(StructuralUnit* unit)
	{
	/* Toggle the unit's locked state: */
	unit->setLocked(!unit->getLocked());
	}

void SpaceGrid::toggleUnitsLock(const SpaceGrid::StructuralUnitList& units)
	{
	/* Determine locked state of majority of units: */
	int numLockedUnits=0;
	int numUnlockedUnits=0;
	for(StructuralUnitList::const_iterator uIt=units.begin();uIt!=units.end();++uIt)
		{
		if((*uIt)->getLocked())
			++numLockedUnits;
		else
			++numUnlockedUnits;
		}
	
	/* Apply new locked state: */
	bool newLockedState=numUnlockedUnits>numLockedUnits;
	for(StructuralUnitList::const_iterator uIt=units.begin();uIt!=units.end();++uIt)
		(*uIt)->setLocked(newLockedState);
	}

void SpaceGrid::moveUnit(StructuralUnit* unit)
	{
	SpaceGridCell* cell=unit->cell;
	
	/* Find new grid cell containing unit: */
	SpaceGridCell* newCell=findCell(unit->position);
	
	/* Check if unit has moved to another grid cell: */
	if(newCell!=cell)
		{
		/* Unlink unit from its old cell: */
		cell->unlinkUnit(unit);
		
		/* Update any ghost units associated with the unit: */
		if(newCell->borderCaseMask!=0x0||cell->borderCaseMask!=0x0)
			{
			for(int i=0;i<26;++i)
				{
				bool cellHasCase=(cell->borderCaseMask&borderCases[i].caseMask)==borderCases[i].caseMask;
				bool newCellHasCase=(newCell->borderCaseMask&borderCases[i].caseMask)==borderCases[i].caseMask;
				if(cellHasCase)
					{
					/* Find ghost unit associated with unit in old ghost cell: */
					SpaceGridCell* ghostCell=cell+borderCases[i].pointerOffset;
					GhostUnit* ghostUnit;
					for(ghostUnit=static_cast<GhostUnit*>(ghostCell->firstUnit);ghostUnit->getSourceUnit()!=unit;ghostUnit=static_cast<GhostUnit*>(ghostUnit->cellSucc))
						;
					
					/* Unlink ghost unit from ghost cell: */
					ghostCell->unlinkUnit(ghostUnit);
					
					if(newCellHasCase)
						{
						/* Move ghost unit to new ghost cell: */
						SpaceGridCell* newGhostCell=newCell+borderCases[i].pointerOffset;
						newGhostCell->linkUnit(ghostUnit);
						}
					else
						{
						/* Delete ghost unit: */
						ghostUnit->clearVertexLinks();
						if(ghostUnit->pred!=0)
							ghostUnit->pred->succ=ghostUnit->succ;
						else
							firstGhostUnit=ghostUnit->succ;
						if(ghostUnit->succ!=0)
							ghostUnit->succ->pred=ghostUnit->pred;
						else
							lastGhostUnit=ghostUnit->pred;
						delete ghostUnit;
						}
					}
				else if(newCellHasCase)
					{
					/* Create new ghost unit in new ghost cell: */
					GhostUnit* newGhostUnit=new GhostUnit(unit,borderCases[i].positionOffset);
					newGhostUnit->id=0;
					newGhostUnit->pred=lastGhostUnit;
					if(lastGhostUnit!=0)
						lastGhostUnit->succ=newGhostUnit;
					else
						firstGhostUnit=newGhostUnit;
					lastGhostUnit=newGhostUnit;
					(newCell+borderCases[i].pointerOffset)->linkUnit(newGhostUnit);
					}
				}
			}
		
		/* Link unit to its new cell: */
		newCell->linkUnit(unit);
		}
	}

void SpaceGrid::setUnitPositionOrientation(StructuralUnit* unit,const Point& newPosition,const Rotation& newOrientation)
	{
	/* Set unit's position and orientation: */
	unit->position=newPosition;
	unit->orientation=newOrientation;
	
	/* Update unit's grid linkage: */
	moveUnit(unit);
	}

void SpaceGrid::transformUnits(const SpaceGrid::StructuralUnitList& units,const OrthonormalTransformation& t)
	{
	/* Transform all units in the list: */
	for(StructuralUnitList::const_iterator uIt=units.begin();uIt!=units.end();++uIt)
		{
		/* Set unit's position and orientation: */
		(*uIt)->position=t.transform((*uIt)->position);
		(*uIt)->orientation.leftMultiply(t.getRotation());
		moveUnit(*uIt);
		}
	}

void SpaceGrid::removeUnit(StructuralUnit* unit)
	{
	if(unit->cell->borderCaseMask!=0x0)
		{
		/* Remove all ghost units associated with the unit: */
		for(int i=0;i<26;++i)
			if((unit->cell->borderCaseMask&borderCases[i].caseMask)==borderCases[i].caseMask)
				{
				/* Find ghost unit associated with unit in old ghost cell: */
				SpaceGridCell* ghostCell=unit->cell+borderCases[i].pointerOffset;
				GhostUnit* ghostUnit;
				for(ghostUnit=static_cast<GhostUnit*>(ghostCell->firstUnit);ghostUnit->getSourceUnit()!=unit;ghostUnit=static_cast<GhostUnit*>(ghostUnit->cellSucc))
					;
				
				/* Clear the ghost unit's vertex links: */
				ghostUnit->clearVertexLinks();
				
				/* Unlink ghost unit from ghost cell: */
				ghostCell->unlinkUnit(ghostUnit);
				
				/* Delete the ghost unit: */
				if(ghostUnit->pred!=0)
					ghostUnit->pred->succ=ghostUnit->succ;
				else
					firstGhostUnit=ghostUnit->succ;
				if(ghostUnit->succ!=0)
					ghostUnit->succ->pred=ghostUnit->pred;
				else
					lastGhostUnit=ghostUnit->pred;
				delete ghostUnit;
				}
		}
	
	/* Clear the given unit's vertex links: */
	unit->clearVertexLinks();
	
	/* Remove the unit from the cell containing it: */
	unit->cell->unlinkUnit(unit);
	
	/* Remove the unit from the main list: */
	if(unit->pred!=0)
		unit->pred->succ=unit->succ;
	else
		firstUnit=unit->succ;
	if(unit->succ!=0)
		unit->succ->pred=unit->pred;
	else
		lastUnit=unit->pred;
	}

void SpaceGrid::removeUnits(const SpaceGrid::StructuralUnitList& units)
	{
	/* Remove all units in the list: */
	for(StructuralUnitList::const_iterator uIt=units.begin();uIt!=units.end();++uIt)
		removeUnit(*uIt);
	}

void SpaceGrid::advanceTime(Scalar timeStep)
	{
	/* Check all existing vertex links and apply their vertex forces: */
	for(StructuralUnit* unit=firstUnit;unit!=0;unit=unit->succ)
		unit->checkVertexLinks(timeStep);
	
	/* Compute interactions between all pairs of units: */
	for(StructuralUnit* unit1=firstUnit;unit1!=0;unit1=unit1->succ)
		{
		/* Construct neighbourhood of cells around unit's cell: */
		for(int i=0;i<27;++i)
			{
			SpaceGridCell* cell=unit1->cell+cellNeighbourOffsets[i];
			for(StructuralUnit* unit2=cell->firstUnit;unit2!=0;unit2=unit2->cellSucc)
				if(unit1->id>unit2->id)
					StructuralUnit::interact(unit1,unit2,timeStep);
			}
		}
	
	/* Move all units to the end of the time step: */
	Scalar att=Math::pow(attenuation,timeStep);
	//Scalar totalLinearVelocity(0);
	//Scalar totalAngularVelocity(0);
	for(StructuralUnit* unit=firstUnit;unit!=0;unit=unit->succ)
		{
		if(!unit->locked)
			{
			/* Move unit according to velocities: */
			unit->orientation.leftMultiply(Rotation(unit->angularVelocity*timeStep));
			unit->position+=unit->linearVelocity*timeStep;
			
			//totalLinearVelocity+=Geometry::mag(unit->linearVelocity);
			//totalAngularVelocity+=Geometry::mag(unit->angularVelocity);
			
			/* Attenuate unit's velocities: */
			unit->linearVelocity*=att;
			unit->angularVelocity*=att;
			
			/* Limit unit's position to grid box: */
			for(int i=0;i<3;++i)
				{
				/* Repeat until unit's position is inside box: */
				while(true)
					{
					if(unit->position[i]<gridBox.min[i])
						{
						if(periodicFlags[i])
							unit->position[i]+=gridBox.getSize(i);
						else
							{
							unit->position[i]=Scalar(2)*gridBox.min[i]-unit->position[i];
							unit->linearVelocity[i]=-unit->linearVelocity[i];
							}
						}
					else if(unit->position[i]>gridBox.max[i])
						{
						if(periodicFlags[i])
							unit->position[i]-=gridBox.getSize(i);
						else
							{
							unit->position[i]=Scalar(2)*gridBox.max[i]-unit->position[i];
							unit->linearVelocity[i]=-unit->linearVelocity[i];
							}
						}
					else
						break;
					}
				}
			
			/* Update unit's grid linkage: */
			moveUnit(unit);
			}
		else
			{
			unit->linearVelocity=Vector::zero;
			unit->angularVelocity=Vector::zero;
			}
		}
	
	#if 1
	/* Update all ghost units to reflect the state of their source units: */
	for(StructuralUnit* uPtr=firstGhostUnit;uPtr!=0;uPtr=uPtr->succ)
		{
		GhostUnit* guPtr=static_cast<GhostUnit*>(uPtr);
		guPtr->updateState();
		}
	#endif
	
	//std::cout<<std::setw(20)<<totalLinearVelocity<<", "<<std::setw(20)<<totalAngularVelocity<<"         \r"<<std::flush;
	}

SpaceGrid::GridStatistics SpaceGrid::calcGridStatistics(void) const
	{
	/* Initialize statistics structure: */
	GridStatistics result;
	result.numUnits=0;
	result.numTriangles=0;
	result.numTetrahedra=0;
	result.numOctahedra=0;
	result.numSpheres=0;
	result.numUnsharedVertices=0;
	result.bondLengthMin=Scalar(1.2);
	result.bondLengthMax=Scalar(2.2);
	Scalar bondLengthBinSize=(result.bondLengthMax-result.bondLengthMin)/Scalar(GridStatistics::numBondLengthBins);
	for(int i=0;i<GridStatistics::numBondLengthBins;++i)
		result.bondLengthHistogram[i]=0;
	result.averageBondLength=Scalar(0);
	result.bondAngleMin=Scalar(0.5)*Math::Constants<Scalar>::pi;
	result.bondAngleMax=Math::Constants<Scalar>::pi;
	Scalar bondAngleBinSize=(result.bondAngleMax-result.bondAngleMin)/Scalar(GridStatistics::numBondAngleBins);
	for(int i=0;i<GridStatistics::numBondAngleBins;++i)
		result.bondAngleHistogram[i]=0;
	result.averageBondAngle=Scalar(0);
	result.centerDistMin=Scalar(2.0);
	result.centerDistMax=Scalar(3.5);
	Scalar centerDistBinSize=(result.centerDistMax-result.centerDistMin)/Scalar(GridStatistics::numCenterDistBins);
	for(int i=0;i<GridStatistics::numCenterDistBins;++i)
		result.centerDistHistogram[i]=0;
	result.averageCenterDist=Scalar(0);
	result.internalBondLengthMin=Scalar(2.2);
	result.internalBondLengthMax=Scalar(3.2);
	Scalar internalBondLengthBinSize=(result.internalBondLengthMax-result.internalBondLengthMin)/Scalar(GridStatistics::numInternalBondLengthBins);
	for(int i=0;i<GridStatistics::numInternalBondLengthBins;++i)
		result.internalBondLengthHistogram[i]=0;
	result.averageInternalBondLength=Scalar(0);
	result.internalBondAngleMin=Scalar(0.5)*Math::Constants<Scalar>::pi;
	result.internalBondAngleMax=Math::Constants<Scalar>::pi;
	Scalar internalBondAngleBinSize=(result.internalBondAngleMax-result.internalBondAngleMin)/Scalar(GridStatistics::numInternalBondAngleBins);
	for(int i=0;i<GridStatistics::numInternalBondAngleBins;++i)
		result.internalBondAngleHistogram[i]=0;
	result.averageInternalBondAngle=Scalar(0);
	
	/* Iterate through the list of structural units: */
	int numBonds=0;
	int numInternalBonds=0;
	for(const StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
		{
		/* Increment total unit count: */
		++result.numUnits;
		
		/* Increment unit type count: */
		/* ... */
		
		/* Check unit's links: */
		Point* vertices=new Point[uPtr->getNumVertices()];
		Vector* vertexDirs=new Vector[uPtr->getNumVertices()];
		Scalar* vertexDists=new Scalar[uPtr->getNumVertices()];
		for(int i=0;i<uPtr->getNumVertices();++i)
			{
			const StructuralUnit::VertexLink& vl=uPtr->getVertexLink(i);
			if(vl.unit!=0)
				{
				/* Calculate position of bond midpoint: */
				vertices[i]=Geometry::mid(uPtr->getVertex(i),vl.unit->getVertex(vl.vertexIndex));
				Vector v1=uPtr->getPosition()-vertices[i];
				vertexDirs[i]=v1;
				Vector v2=vl.unit->getPosition()-vertices[i];
				
				/* Calculate bond lengths for this bond: */
				Scalar l1=Geometry::mag(v1);
				vertexDists[i]=l1;
				Scalar l2=Geometry::mag(v2);
				int l1Index=int(Math::floor((l1-result.bondLengthMin)/bondLengthBinSize));
				if(l1Index<=0)
					++result.bondLengthHistogram[0];
				else if(l1Index>=GridStatistics::numBondLengthBins-1)
					++result.bondLengthHistogram[GridStatistics::numBondLengthBins-1];
				else
					++result.bondLengthHistogram[l1Index];
				int l2Index=int(Math::floor((l2-result.bondLengthMin)/bondLengthBinSize));
				if(l2Index<=0)
					++result.bondLengthHistogram[0];
				else if(l2Index>=GridStatistics::numBondLengthBins-1)
					++result.bondLengthHistogram[GridStatistics::numBondLengthBins-1];
				else
					++result.bondLengthHistogram[l2Index];
				result.averageBondLength+=l1+l2;
				
				/* Calculate bond angle for this bond: */
				Scalar cosAngle=(v1*v2)/(l1*l2);
				if(cosAngle<Scalar(-1))
					cosAngle=Scalar(-1);
				else if(cosAngle>Scalar(1))
					cosAngle=Scalar(1);
				Scalar angle=Math::acos(cosAngle);
				int angleIndex=int(Math::floor((angle-result.bondAngleMin)/bondAngleBinSize));
				if(angleIndex<=0)
					++result.bondAngleHistogram[0];
				else if(angleIndex>=GridStatistics::numBondAngleBins-1)
					++result.bondAngleHistogram[GridStatistics::numBondAngleBins-1];
				else
					++result.bondAngleHistogram[angleIndex];
				result.averageBondAngle+=angle;
				
				/* Calculate center distance for this bond: */
				Scalar cd=Geometry::dist(uPtr->getPosition(),vl.unit->getPosition());
				int cdIndex=int(Math::floor((cd-result.centerDistMin)/centerDistBinSize));
				if(cdIndex<=0)
					++result.centerDistHistogram[0];
				else if(cdIndex>=GridStatistics::numCenterDistBins-1)
					++result.centerDistHistogram[GridStatistics::numCenterDistBins-1];
				else
					++result.centerDistHistogram[cdIndex];
				result.averageCenterDist+=cd;
				
				++numBonds;
				}
			else
				{
				vertices[i]=uPtr->getVertex(i);
				vertexDirs[i]=uPtr->getPosition()-vertices[i];
				vertexDists[i]=Geometry::mag(vertexDirs[i]);
				
				/* Increment number of unshared vertices: */
				++result.numUnsharedVertices;
				}
			}
		
		/* Calculate internal bond lengths and bond angles: */
		for(int i1=0;i1<uPtr->getNumVertices()-1;++i1)
			for(int i2=i1+1;i2<uPtr->getNumVertices();++i2)
				{
				/* Calculate internal bond length: */
				Scalar l=Geometry::dist(vertices[i1],vertices[i2]);
				int lIndex=int(Math::floor((l-result.internalBondLengthMin)/internalBondLengthBinSize));
				if(lIndex<=0)
					++result.internalBondLengthHistogram[0];
				else if(lIndex>=GridStatistics::numInternalBondLengthBins-1)
					++result.internalBondLengthHistogram[GridStatistics::numInternalBondLengthBins-1];
				else
					++result.internalBondLengthHistogram[lIndex];
				result.averageInternalBondLength+=l;
				
				/* Calculate internal bond angle: */
				Scalar cosAngle=(vertexDirs[i1]*vertexDirs[i2])/(vertexDists[i1]*vertexDists[i2]);
				if(cosAngle<Scalar(-1))
					cosAngle=Scalar(-1);
				else if(cosAngle>Scalar(1))
					cosAngle=Scalar(1);
				Scalar angle=Math::acos(cosAngle);
				int angleIndex=int(Math::floor((angle-result.internalBondAngleMin)/internalBondAngleBinSize));
				if(angleIndex<=0)
					++result.internalBondAngleHistogram[0];
				else if(angleIndex>=GridStatistics::numInternalBondAngleBins-1)
					++result.internalBondAngleHistogram[GridStatistics::numInternalBondAngleBins-1];
				else
					++result.internalBondAngleHistogram[angleIndex];
				result.averageInternalBondAngle+=angle;
				
				++numInternalBonds;
				}
		
		delete[] vertices;
		delete[] vertexDirs;
		delete[] vertexDists;
		}
	
	/* Calculate average bond length: */
	result.averageBondLength/=Scalar(numBonds)*Scalar(2);
	
	/* Calculate average bond angle: */
	result.averageBondAngle/=Scalar(numBonds);
	
	/* Calculate average center distance: */
	result.averageCenterDist/=Scalar(numBonds);
	
	/* Calculate average internal bond length: */
	result.averageInternalBondLength/=Scalar(numInternalBonds);
	
	/* Calculate average internal bond angle: */
	result.averageInternalBondAngle/=Scalar(numInternalBonds);
	
	return result;
	}

void SpaceGrid::readRenderingFlags(const Misc::ConfigurationFileSection& configFileSection)
	{
	showGridBoundary=configFileSection.retrieveValue<bool>("./showGridBoundary",showGridBoundary);
	gridBoundaryColor=configFileSection.retrieveValue<Color>("./gridBoundaryColor",gridBoundaryColor);
	gridBoundaryLineWidth=configFileSection.retrieveValue<GLfloat>("./gridBoundaryLineWidth",gridBoundaryLineWidth);
	showUnits=configFileSection.retrieveValue<bool>("./showUnits",showUnits);
	defaultUnitColor=configFileSection.retrieveValue<Color>("./defaultUnitColor",defaultUnitColor);
	markedUnitColor=configFileSection.retrieveValue<Color>("./markedUnitColor",markedUnitColor);
	lockedUnitColor=configFileSection.retrieveValue<Color>("./lockedUnitColor",lockedUnitColor);
	unitMaterial=configFileSection.retrieveValue<GLMaterial>("./unitMaterial",unitMaterial);
	showVelocities=configFileSection.retrieveValue<bool>("./showVelocities",showVelocities);
	showVertexLinks=configFileSection.retrieveValue<bool>("./showVertexLinks",showVertexLinks);
	vertexLinkColor=configFileSection.retrieveValue<Color>("./vertexLinkColor",vertexLinkColor);
	vertexLinkLineWidth=configFileSection.retrieveValue<GLfloat>("./vertexLinkLineWidth",vertexLinkLineWidth);
	showUnlinkedVertices=configFileSection.retrieveValue<bool>("./showUnlinkedVertices",showUnlinkedVertices);
	unlinkedVertexRadius=configFileSection.retrieveValue<GLfloat>("./unlinkedVertexRadius",unlinkedVertexRadius);
	unlinkedVertexSubdivision=configFileSection.retrieveValue<int>("./unlinkedVertexSubdivision",unlinkedVertexSubdivision);
	unlinkedVertexMaterial=configFileSection.retrieveValue<GLMaterial>("./unlinkedVertexMaterial",unlinkedVertexMaterial);
	}

void SpaceGrid::setShowGridBoundary(bool newShowGridBoundary)
	{
	showGridBoundary=newShowGridBoundary;
	}

void SpaceGrid::setGridBoundaryColor(const SpaceGrid::Color& newGridBoundaryColor)
	{
	gridBoundaryColor=newGridBoundaryColor;
	}

void SpaceGrid::setGridBoundaryLineWidth(GLfloat newGridBoundaryLineWidth)
	{
	gridBoundaryLineWidth=newGridBoundaryLineWidth;
	}

void SpaceGrid::setShowUnits(bool newShowUnits)
	{
	showUnits=newShowUnits;
	}

void SpaceGrid::setUnitMaterial(const GLMaterial& newUnitMaterial)
	{
	unitMaterial=newUnitMaterial;
	}

void SpaceGrid::setShowVelocities(bool newShowVelocities)
	{
	showVelocities=newShowVelocities;
	}

void SpaceGrid::setShowVertexLinks(bool newShowVertexLinks)
	{
	showVertexLinks=newShowVertexLinks;
	}

void SpaceGrid::setVertexLinkColor(const SpaceGrid::Color& newVertexLinkColor)
	{
	vertexLinkColor=newVertexLinkColor;
	}

void SpaceGrid::setVertexLinkLineWidth(GLfloat newVertexLinkLineWidth)
	{
	vertexLinkLineWidth=newVertexLinkLineWidth;
	}

void SpaceGrid::setShowUnlinkedVertices(bool newShowUnlinkedVertices)
	{
	showUnlinkedVertices=newShowUnlinkedVertices;
	}

void SpaceGrid::glRenderAction(GLContextData& contextData) const
	{
	/* Retrieve the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	glPushAttrib(GL_TRANSFORM_BIT|GL_LIGHTING_BIT|GL_LINE_BIT);
	
	glDisable(GL_LIGHTING);
	
	if(showGridBoundary)
		{
		glColor(gridBoundaryColor);
		glLineWidth(gridBoundaryLineWidth);
		#if 0
		/* Render the space grid: */
		glBegin(GL_LINES);
		for(int x=0;x<=gridSize[0];++x)
			for(int y=0;y<=gridSize[1];++y)
				{
				glVertex(gridBox.getMin(0)+Scalar(x)*cellSize[0],gridBox.getMin(1)+Scalar(y)*cellSize[1],gridBox.getMin(2));
				glVertex(gridBox.getMin(0)+Scalar(x)*cellSize[0],gridBox.getMin(1)+Scalar(y)*cellSize[1],gridBox.getMax(2));
				}
		for(int x=0;x<=gridSize[0];++x)
			for(int z=0;z<=gridSize[2];++z)
				{
				glVertex(gridBox.getMin(0)+Scalar(x)*cellSize[0],gridBox.getMin(1),gridBox.getMin(2)+Scalar(z)*cellSize[2]);
				glVertex(gridBox.getMin(0)+Scalar(x)*cellSize[0],gridBox.getMax(1),gridBox.getMin(2)+Scalar(z)*cellSize[2]);
				}
		for(int y=0;y<=gridSize[1];++y)
			for(int z=0;z<=gridSize[2];++z)
				{
				glVertex(gridBox.getMin(0),gridBox.getMin(1)+Scalar(y)*cellSize[1],gridBox.getMin(2)+Scalar(z)*cellSize[2]);
				glVertex(gridBox.getMax(0),gridBox.getMin(1)+Scalar(y)*cellSize[1],gridBox.getMin(2)+Scalar(z)*cellSize[2]);
				}
		glEnd();
		#else
		/* Render the space grid's outline: */
		glBegin(GL_LINE_STRIP);
		glVertex(gridBox.getVertex(0));
		glVertex(gridBox.getVertex(1));
		glVertex(gridBox.getVertex(3));
		glVertex(gridBox.getVertex(2));
		glVertex(gridBox.getVertex(0));
		glVertex(gridBox.getVertex(4));
		glVertex(gridBox.getVertex(5));
		glVertex(gridBox.getVertex(7));
		glVertex(gridBox.getVertex(6));
		glVertex(gridBox.getVertex(4));
		glEnd();
		glBegin(GL_LINES);
		glVertex(gridBox.getVertex(1));
		glVertex(gridBox.getVertex(5));
		glVertex(gridBox.getVertex(3));
		glVertex(gridBox.getVertex(7));
		glVertex(gridBox.getVertex(2));
		glVertex(gridBox.getVertex(6));
		glEnd();
		#endif
		}
	
	#if 1
	/* Set up clipping planes to restrict rendering to space grid box: */
	GLdouble clipPlanes[6][4];
	for(int i=0;i<3;++i)
		{
		GLdouble clipPlane[4];
		for(int j=0;j<3;++j)
			clipPlane[j]=0.0;
		clipPlane[i]=1.0;
		clipPlane[3]=-gridBox.min[i];
		glEnable(GL_CLIP_PLANE0+2*i+0);
		glClipPlane(GL_CLIP_PLANE0+2*i+0,clipPlane);
		
		for(int j=0;j<3;++j)
			clipPlane[j]=0.0;
		clipPlane[i]=-1.0;
		clipPlane[3]=gridBox.max[i];
		glEnable(GL_CLIP_PLANE0+2*i+1);
		glClipPlane(GL_CLIP_PLANE0+2*i+1,clipPlane);
		}
	#endif
	
	glLineWidth(1.0f);
	
	if(showVelocities)
		{
		/* Render structural units' velocities: */
		glBegin(GL_LINES);
		for(const StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			glColor3f(1.0f,0.0f,0.0f);
			glVertex(uPtr->position);
			glVertex(uPtr->position+uPtr->linearVelocity);
			glColor3f(0.0f,0.0f,1.0f);
			glVertex(uPtr->position);
			glVertex(uPtr->position+uPtr->angularVelocity);
			}
		for(const StructuralUnit* uPtr=firstGhostUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			glColor3f(1.0f,0.0f,0.0f);
			glVertex(uPtr->position);
			glVertex(uPtr->position+uPtr->linearVelocity);
			glColor3f(0.0f,0.0f,1.0f);
			glVertex(uPtr->position);
			glVertex(uPtr->position+uPtr->angularVelocity);
			}
		glEnd();
		}
	
	if(showVertexLinks)
		{
		/* Render vertex links: */
		glLineWidth(vertexLinkLineWidth);
		glColor(vertexLinkColor);
		glBegin(GL_LINES);
		for(const StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			int numVertices=uPtr->getNumVertices();
			for(int i=0;i<numVertices;++i)
				{
				const StructuralUnit::VertexLink& vl=uPtr->getVertexLink(i);
				if(vl.unit!=0&&uPtr->id<vl.unit->id)
					{
					glVertex(uPtr->position);
					glVertex(vl.unit->position);
					}
				}
			}
		#if 0
		for(const StructuralUnit* uPtr=firstGhostUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			int numVertices=uPtr->getNumVertices();
			for(int i=0;i<numVertices;++i)
				{
				const StructuralUnit::VertexLink& vl=uPtr->getVertexLink(i);
				if(vl.unit!=0&&uPtr->id<vl.unit->id)
					{
					glVertex(uPtr->position);
					glVertex(vl.unit->position+static_cast<const GhostUnit*>(uPtr)->getSourceOffset());
					}
				}
			}
		#endif
		glEnd();
		}
	
	glEnable(GL_LIGHTING);
	
	if(showUnits)
		{
		/* Render all units (real ones and ghost units): */
		glMaterial(GLMaterialEnums::FRONT,unitMaterial);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
		for(const StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			if(uPtr->marked)
				glColor(markedUnitColor);
			else if(uPtr->locked)
				glColor(lockedUnitColor);
			else
				glColor(uPtr->getColor());
			uPtr->glRenderAction(contextData);
			}
		for(const StructuralUnit* uPtr=firstGhostUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			if(uPtr->marked)
				glColor(markedUnitColor);
			else if(uPtr->locked)
				glColor(lockedUnitColor);
			else
				glColor(uPtr->getColor());
			uPtr->glRenderAction(contextData);
			}
		glDisable(GL_COLOR_MATERIAL);
		}
	
	if(showUnlinkedVertices)
		{
		/* Render a marker for all unlinked vertices: */
		glMaterial(GLMaterialEnums::FRONT,unlinkedVertexMaterial);
		glPushMatrix();
		Point lastPos=Point::origin;
		for(const StructuralUnit* uPtr=firstUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			int numVertices=uPtr->getNumVertices();
			for(int i=0;i<numVertices;++i)
				if(uPtr->getVertexLink(i).unit==0)
					{
					/* Render vertex marker at vertex' position: */
					Vector offset=uPtr->getVertex(i)-lastPos;
					glTranslate(offset);
					glCallList(dataItem->vertexMarkerDisplayListId);
					lastPos+=offset;
					}
			}
		for(const StructuralUnit* uPtr=firstGhostUnit;uPtr!=0;uPtr=uPtr->succ)
			{
			int numVertices=uPtr->getNumVertices();
			for(int i=0;i<numVertices;++i)
				if(uPtr->getVertexLink(i).unit==0)
					{
					/* Render vertex marker at vertex' position: */
					Vector offset=uPtr->getVertex(i)-lastPos;
					glTranslate(offset);
					glCallList(dataItem->vertexMarkerDisplayListId);
					lastPos+=offset;
					}
			}
		glPopMatrix();
		}
	
	glPopAttrib();
	}

}
