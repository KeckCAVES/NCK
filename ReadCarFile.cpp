/***********************************************************************
ReadCarFile - Function to generate a space grid with structural units
from an atom coordinate file in CAR format.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>

#include "AffineSpace.h"
#include "SpaceGrid.h"
#include "GhostUnit.h"
#include "Tetrahedron.h"

#include "ReadCarFile.h"

namespace NCK {

typedef Geometry::ComponentArray<Scalar,3> Size; // Data type for sizes of grids and cells

struct Atom // Structure for atoms found in CAR files
	{
	/* Embedded classes: */
	public:
	enum Element // Enumerated type for chemical elements
		{
		SI,O
		};
	
	/* Elements: */
	public:
	Element element; // Atom's element
	Point position; // Atom's position
	Atom* succ; // Pointer to next atom in main list
	Atom* cellSucc; // Pointer to next atom in cell's list
	
	/* Constructors and destructors: */
	Atom(Element sElement,const Point& sPosition)
		:element(sElement),position(sPosition),succ(0),cellSucc(0)
		{
		};
	Atom(const char* elementName,const Point& sPosition)
		:position(sPosition),succ(0),cellSucc(0)
		{
		/* Parse element name: */
		if(strncasecmp(elementName,"Si",2)==0)
			element=SI;
		else if(strncasecmp(elementName,"O",1)==0)
			element=O;
		else
			throw std::runtime_error(std::string("Atom: Unknown element name ")+elementName);
		};
	
	/* Methods: */
	Atom* offset(const Vector& offset) // Returns an offset copy of the atom
		{
		return new Atom(element,position+offset);
		};
	};

struct CarFileCell // Structure for grid array cells containing atoms
	{
	/* Elements: */
	public:
	Atom* firstAtom; // Pointer to first atom in cell
	Atom* lastAtom; // Pointer to last atom in cell
	
	/* Constructors and destructors: */
	CarFileCell(void) // Creates empty cell
		:firstAtom(0),lastAtom(0)
		{
		};
	~CarFileCell(void) // Deletes all atoms in cell
		{
		while(firstAtom!=0)
			{
			Atom* succ=firstAtom->cellSucc;
			delete firstAtom;
			firstAtom=succ;
			}
		};
	
	/* Methods: */
	void addAtom(Atom* newAtom) // Adds an atom to the grid cell
		{
		if(lastAtom!=0)
			lastAtom->cellSucc=newAtom;
		else
			firstAtom=newAtom;
		lastAtom=newAtom;
		};
	};

typedef Misc::Array<CarFileCell,3> CellGrid;
typedef CellGrid::Index Index;

Scalar fitTetrahedron(Tetrahedron* tet,Atom* oxygens[4])
	{
	/* Calculate offset vectors of given atom configuration: */
	Vector atomOffsets[4];
	for(int i=0;i<4;++i)
		atomOffsets[i]=oxygens[i]->position-tet->getPosition();
	
	/* Calculate atom configuration's orientation: */
	Vector d[3];
	for(int i=0;i<3;++i)
		d[i]=atomOffsets[i+1]-atomOffsets[0];
	
	/* Create mapping between atoms and tetrahedron vertices: */
	int atomIndices[4]={0,1,-1,-1};
	if(Geometry::cross(d[0],d[1])*d[2]>0)
		{
		/* Atom configuration has positive orientation: */
		atomIndices[2]=2;
		atomIndices[3]=3;
		}
	else
		{
		/* Atom configuration has negative orientation: */
		atomIndices[2]=3;
		atomIndices[3]=2;
		}
	
	/* Calculate optimal orientation for tetrahedron: */
	Rotation orientation=Rotation::identity;
	tet->setPositionOrientation(tet->getPosition(),orientation);
	Scalar totalTorqueMag2=Scalar(0);
	for(int i=0;i<100;++i)
		{
		/* Calculate total torque on result tetrahedron: */
		Vector totalTorque=Vector::zero;
		for(int i=0;i<4;++i)
			{
			Vector vo=tet->getVertexOffset(i);
			Vector force=atomOffsets[atomIndices[i]]-vo;
			Vector torque=Geometry::cross(vo,force);
			totalTorque+=torque;
			}
		totalTorqueMag2=Geometry::sqr(totalTorque);
		
		/* Apply rotation to tetrahedron: */
		orientation.leftMultiply(Rotation(totalTorque*Scalar(0.25)));
		tet->setPositionOrientation(tet->getPosition(),orientation);
		}
	
	return totalTorqueMag2;
	}

Tetrahedron* alignTetrahedron(Atom* silicon,int numOxygens,Atom* oxygens[])
	{
	/* Create result tetrahedron: */
	Tetrahedron* result=new Tetrahedron(silicon->position,Rotation::identity);
	
	/* Test all possible subsets of four atoms from the given oxygens for best fit: */
	Scalar bestFitTorque=Math::Constants<Scalar>::max;
	Rotation bestFitOrientation;
	
	int is[4];
	for(is[0]=0;is[0]<numOxygens-3;++is[0])
		for(is[1]=is[0]+1;is[1]<numOxygens-2;++is[1])
			for(is[2]=is[1]+1;is[2]<numOxygens-1;++is[2])
				for(is[3]=is[2]+1;is[3]<numOxygens-0;++is[3])
					{
					/* Pick the four candidate oxygens: */
					Atom* fit[4];
					for(int i=0;i<4;++i)
						fit[i]=oxygens[is[i]];
					
					/* Fit the tetrahedron to the selected oxygen atoms: */
					Scalar fitTorque=fitTetrahedron(result,fit);
					if(fitTorque<bestFitTorque)
						{
						bestFitTorque=fitTorque;
						bestFitOrientation=result->getOrientation();
						}
					}
	
	/* Return result tetrahedron with best fit: */
	result->setPositionOrientation(result->getPosition(),bestFitOrientation);
	return result;
	}

SpaceGrid* readCarFile(const char* carFileName)
	{
	/* Open CAR file: */
	Misc::File carFile(carFileName,"rt");
	
	/* Skip CAR file header: */
	char line[256];
	for(int i=0;i<4;++i)
		carFile.gets(line,sizeof(line));
	
	/* Read grid size: */
	Geometry::ComponentArray<double,3> gridBoxSize;
	carFile.gets(line,sizeof(line));
	if(sscanf(line,"PBC %lf %lf %lf",&gridBoxSize[0],&gridBoxSize[1],&gridBoxSize[2])!=3)
		Misc::throwStdErr("readCarFile: Unable to parse grid size from input file");
	
	/* Calculate minimum cell size: */
	Scalar minCellSize=Scalar(2); // Maximal Si-O bond distance is 2 Angstrom
	
	/* Determine optimum number of cells: */
	Index gridSize;
	Size cellSize;
	Index fullGridSize;
	for(int i=0;i<3;++i)
		{
		/* Calculate number of cells and cell size: */
		gridSize[i]=int(Math::floor(gridBoxSize[i]/minCellSize));
		cellSize[i]=gridBoxSize[i]/Scalar(gridSize[i]);
		
		/* Adjust number of cells for layers of "ghost cells": */
		fullGridSize[i]=gridSize[i]+2;
		}
	
	/* Create cell grid: */
	Atom* firstAtom=0; // Pointer to first atom
	Atom* lastAtom=0; // Pointer to last atom
	CellGrid grid(fullGridSize); // Array of grid cells including layer of ghost cells
	
	/* Create bit masks of border cell cases: */
	unsigned int borderCaseMasks[26];
	
	/* Create face border cases: */
	int caseIndex=0;
	for(int i=0;i<3;++i)
		for(int i1=0;i1<2;++i1)
			borderCaseMasks[caseIndex++]=0x1<<(2*i+i1);
	
	/* Create edge border cases: */
	for(int i=0;i<2;++i)
		for(int j=i+1;j<3;++j)
			for(int i1=0;i1<2;++i1)
				for(int j1=0;j1<2;++j1)
					borderCaseMasks[caseIndex++]=(0x1<<(2*i+i1))|(0x1<<(2*j+j1));
	
	/* Create corner border cases: */
	for(int i1=0;i1<2;++i1)
		for(int j1=0;j1<2;++j1)
			for(int k1=0;k1<2;++k1)
				borderCaseMasks[caseIndex++]=(0x1<<(2*0+i1))|(0x1<<(2*1+j1))|(0x1<<(2*2+k1));
	
	/* Create position and index offsets for the border cell cases: */
	Vector pOffsets[26];
	Index iOffsets[26];
	for(int i=0;i<26;++i)
		for(int j=0;j<3;++j)
			{
			if(borderCaseMasks[i]&(0x1<<(2*j+0)))
				{
				pOffsets[i][j]=gridBoxSize[j];
				iOffsets[i][j]=gridSize[j];
				}
			else if(borderCaseMasks[i]&(0x1<<(2*j+1)))
				{
				pOffsets[i][j]=-gridBoxSize[j];
				iOffsets[i][j]=-gridSize[j];
				}
			else
				{
				pOffsets[i][j]=Scalar(0);
				iOffsets[i][j]=0;
				}
			}

	/* Insert all atoms from CAR file into cell grid: */
	while(true)
		{
		carFile.gets(line,sizeof(line));
		if(strncmp(line,"end",3)==0)
			break;
		
		try
			{
			/* Extract atom name and position from line: */
			char elementName[10];
			Point position;
			sscanf(line,"%s %lf %lf %lf",elementName,&position[0],&position[1],&position[2]);
			
			/* Create atom object and add it to main list: */
			Atom* newAtom=new Atom(elementName,position);
			if(lastAtom!=0)
				lastAtom->succ=newAtom;
			else
				firstAtom=newAtom;
			lastAtom=newAtom;
			
			/* Find index of cell containing new atom: */
			Index cellIndex;
			for(int i=0;i<3;++i)
				{
				cellIndex[i]=int(Math::floor(position[i]/cellSize[i]))+1;
				if(cellIndex[i]<1)
					cellIndex[i]=1;
				else if(cellIndex[i]>gridSize[i])
					cellIndex[i]=gridSize[i];
				}
			
			/* Add new atom to its cell: */
			grid(cellIndex).addAtom(newAtom);
			
			/* Check if new atom was added to a border cell: */
			unsigned int cellBorderCaseMask=0x0;
			for(int i=0;i<3;++i)
				{
				if(cellIndex[i]==1)
					cellBorderCaseMask|=0x1<<(2*i+0);
				if(cellIndex[i]==gridSize[i])
					cellBorderCaseMask|=0x1<<(2*i+1);
				}
			if(cellBorderCaseMask!=0x0)
				{
				/* Add offset copies of new atom to all applicable ghost cells: */
				for(int i=0;i<26;++i)
					if((cellBorderCaseMask&borderCaseMasks[i])==borderCaseMasks[i])
						grid(cellIndex+iOffsets[i]).addAtom(newAtom->offset(pOffsets[i]));
				}
			}
		catch(std::runtime_error err)
			{
			/* Print the error message, but keep going: */
			std::cerr<<"Caught error "<<err.what()<<std::endl;
			}
		}
	
	/* Construct a space grid containing all SiO_4 tetrahedra in the CAR file: */
	int numTetrahedraAdded=0;
	SpaceGrid* spaceGrid=new SpaceGrid(Box(Point::origin,gridBoxSize),Tetrahedron::getClassRadius(),0x7);
	
	/* Find all SiO_4 tetrahedra by considering all silicon atoms: */
	for(Atom* siPtr=firstAtom;siPtr!=0;siPtr=siPtr->succ)
		if(siPtr->element==Atom::SI)
			{
			/* Find the index range of the neighbourhood of grid cells containing the silicon atom: */
			Index index1,index2;
			for(int i=0;i<3;++i)
				{
				int cellIndex=int(Math::floor(siPtr->position[i]/cellSize[i]))+1;
				if(cellIndex<1)
					cellIndex=1;
				else if(cellIndex>gridSize[i])
					cellIndex=gridSize[i];
				index1[i]=cellIndex-1;
				index2[i]=cellIndex+1;
				}
			
			/* Find the four closest oxygen atoms surrounding the atom: */
			Scalar maxDist2=Math::sqr(Scalar(2.0));
			int numOxygens=0;
			const int maxNumOxygens=8;
			Atom* oxygens[maxNumOxygens];
			Scalar oxygenDist2s[maxNumOxygens];
			int numOxygensTested=0;
			
			Index index;
			for(index[0]=index1[0];index[0]<=index2[0];++index[0])
				for(index[1]=index1[1];index[1]<=index2[1];++index[1])
					for(index[2]=index1[2];index[2]<=index2[2];++index[2])
						for(Atom* oPtr=grid(index).firstAtom;oPtr!=0;oPtr=oPtr->cellSucc)
							if(oPtr->element==Atom::O)
								{
								/* Calculate distance from oxygen atom to silicon atom: */
								Scalar dist2=Geometry::sqrDist(oPtr->position,siPtr->position);
								if(dist2<Math::sqr(Scalar(2)))
									++numOxygensTested;
								
								if(dist2<maxDist2)
									{
									/* Sort oxygen atom into arrays: */
									if(numOxygens<maxNumOxygens)
										{
										int i;
										for(i=numOxygens;i>0&&oxygenDist2s[i-1]>dist2;--i)
											{
											oxygens[i]=oxygens[i-1];
											oxygenDist2s[i]=oxygenDist2s[i-1];
											}
										oxygens[i]=oPtr;
										oxygenDist2s[i]=dist2;
										++numOxygens;
										if(numOxygens==maxNumOxygens)
											maxDist2=oxygenDist2s[maxNumOxygens-1];
										}
									else
										{
										int i;
										for(i=numOxygens-1;i>0&&oxygenDist2s[i-1]>dist2;--i)
											{
											oxygens[i]=oxygens[i-1];
											oxygenDist2s[i]=oxygenDist2s[i-1];
											}
										oxygens[i]=oPtr;
										oxygenDist2s[i]=dist2;
										maxDist2=oxygenDist2s[maxNumOxygens-1];
										}
									}
								}
			
			/* Check if four oxygen atoms were found: */
			if(numOxygens>=4)
				{
				/* Print a warning if the correct four oxygen atoms cannot be detected: */
				if(numOxygensTested>maxNumOxygens)
					std::cerr<<numOxygensTested<<" candidate oxygens found for silica unit"<<std::endl;
				
				/* Construct a tetrahedron from the four oxygen positions: */
				StructuralUnit* newUnit=alignTetrahedron(siPtr,numOxygens,oxygens);
				spaceGrid->addUnit(newUnit);
				spaceGrid->lockUnit(newUnit);
				++numTetrahedraAdded;
				}
			}
	
	std::cout<<numTetrahedraAdded<<" silica units processed"<<std::endl;
	return spaceGrid;
	}

void writeCarFile(const char* carFileName,SpaceGrid* grid)
	{
	/* Open CAR file: */
	Misc::File carFile(carFileName,"wt");
	
	/* Write CAR file header: */
	FILE* carFilePtr=carFile.getFilePtr();
	fprintf(carFilePtr,"!BIOSYM archive 3\n");
	fprintf(carFilePtr,"PBC=ON\n\n");
	fprintf(carFilePtr,"!DATE Thu Feb 14 17:31:37 2002\n");
	
	/* Write grid size: */
	Geometry::ComponentArray<double,3> gridBoxSize=grid->getBoundingBox().getSize();
	fprintf(carFilePtr,"PBC %9.4lf %9.4lf %9.4lf %9.4lf %9.4lf %9.4lf (P1)\n",gridBoxSize[0],gridBoxSize[1],gridBoxSize[2],90.0,90.0,90.0);
	
	/* Write all units to the CAR file: */
	int siIndex=1;
	int oIndex=1;
	std::vector<Geometry::Point<double,3> > unsharedVertices;
	SpaceGrid::StructuralUnitList units=grid->getAllUnits();
	for(SpaceGrid::StructuralUnitList::iterator uIt=units.begin();uIt!=units.end();++uIt)
		{
		/* Check if unit is really a tetrahedron: */
		Tetrahedron* tet1=dynamic_cast<Tetrahedron*>(*uIt);
		if(tet1!=0)
			{
			/* Write a silicon atom for unit's centroid: */
			Geometry::Point<double,3> sp=tet1->getPosition();
			fprintf(carFilePtr,"SI%-4d%14.9lf %14.9lf %14.9lf XXX  1      ?       Si  0.000\n",siIndex,sp[0],sp[1],sp[2]);
			++siIndex;
			
			/* Process the unit's vertices: */
			for(int i=0;i<4;++i)
				{
				StructuralUnit::VertexLink& vl=tet1->getVertexLink(i);
				Tetrahedron* tet2=dynamic_cast<Tetrahedron*>(GhostUnit::getSourceUnit(vl.unit));
				if(tet2!=0)
					{
					if(tet1->getId()>tet2->getId())
						{
						/* Write an oxygen atom for the shared vertex: */
						Geometry::Point<double,3> op=Geometry::mid(tet1->getVertex(i),vl.unit->getVertex(vl.vertexIndex));
						fprintf(carFilePtr,"O%-5d%14.9lf %14.9lf %14.9lf XXX  1      ?       O   0.000\n",oIndex,op[0],op[1],op[2]);
						++oIndex;
						}
					}
				else
					{
					/* Store the unshared vertex: */
					unsharedVertices.push_back(tet1->getVertex(i));
					}
				}
			}
		}
	
	/* Process all unshared vertices: */
	for(std::vector<Geometry::Point<double,3> >::iterator vIt=unsharedVertices.begin();vIt!=unsharedVertices.end();++vIt)
		{
		Geometry::Point<double,3> op=*vIt;
		fprintf(carFilePtr,"O%-5d%14.9lf %14.9lf %14.9lf XXX  1      ?       O   0.000\n",oIndex,op[0],op[1],op[2]);
		++oIndex;
		}
	
	/* Finalize the CAR file: */
	fprintf(carFilePtr,"end\nend\n");
	}

}
