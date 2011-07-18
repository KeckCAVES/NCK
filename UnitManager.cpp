/***********************************************************************
UnitManager - Class to manage the initialization and creation of
structural unit classes
Copyright (c) 2005-2011 Oliver Kreylos

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

#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>

#include "Triangle.h"
#include "Tetrahedron.h"
#include "Octahedron.h"
#include "Cylinder.h"
#include "Sphere.h"

#include "UnitManager.h"

namespace NCK {

/****************************
Methods of class UnitManager:
****************************/

int UnitManager::getNumUnitTypes(void)
	{
	return NUM_UNITTYPES;
	}

const char* UnitManager::getUnitTypeName(int unitTypeIndex)
	{
	static const char* unitTypeNames[]={"Triangle","Tetrahedron","Octahedron","Cylinder","Sphere"};
	return unitTypeNames[unitTypeIndex];
	}

Scalar UnitManager::initializeUnitTypes(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Initialize base class: */
	StructuralUnit::initClass(configFileSection);
	
	/* Initialize triangle class: */
	Triangle::initClass(configFileSection.getSection("./Triangle"));
	
	/* Initialize tetrahedron class: */
	Tetrahedron::initClass(configFileSection.getSection("./Tetrahedron"));
	
	/* Initialize octahedron class: */
	Octahedron::initClass(configFileSection.getSection("./Octahedron"));
	
	/* Initialize cylinder class: */
	Cylinder::initClass(configFileSection.getSection("./Cylinder"));
	
	/* Initialize sphere class: */
	Sphere::initClass(configFileSection.getSection("./Sphere"));
	
	/* Determine radius of largest structural unit: */
	Scalar maxUnitRadius=Triangle::getClassRadius();
	if(maxUnitRadius<Tetrahedron::getClassRadius())
		maxUnitRadius=Tetrahedron::getClassRadius();
	if(maxUnitRadius<Octahedron::getClassRadius())
		maxUnitRadius=Octahedron::getClassRadius();
	if(maxUnitRadius<Cylinder::getClassRadius())
		maxUnitRadius=Cylinder::getClassRadius();
	if(maxUnitRadius<Sphere::getClassRadius())
		maxUnitRadius=Sphere::getClassRadius();
	
	return maxUnitRadius;
	}

void UnitManager::deinitializeUnitTypes(void)
	{
	/* Deinitialize sphere class: */
	Sphere::deinitClass();
	
	/* Deinitialize cylinder class: */
	Cylinder::deinitClass();
	
	/* Deinitialize octahedron class: */
	Octahedron::deinitClass();
	
	/* Deinitialize tetrahedron class: */
	Tetrahedron::deinitClass();
	
	/* Deinitialize triangle class: */
	Triangle::deinitClass();
	
	/* Deinitialize base class: */
	StructuralUnit::deinitClass();
	}

StructuralUnit* UnitManager::createUnit(int unitType,const Point& position,const Rotation& orientation)
	{
	switch(unitType)
		{
		case TRIANGLE:
			return new Triangle(position,orientation);
			break;
		
		case TETRAHEDRON:
			return new Tetrahedron(position,orientation);
			break;
		
		case OCTAHEDRON:
			return new Octahedron(position,orientation);
			break;
		
		case CYLINDER:
			return new Cylinder(position,orientation);
			break;
		
		case SPHERE:
			return new Sphere(position,orientation);
			break;
		
		default:
			Misc::throwStdErr("UnitManager::createUnit: Unknown unit type %d",unitType);
			return 0;
		}
	}

StructuralUnit* UnitManager::readUnit(Misc::File& file)
	{
	/* Determine unit's type: */
	int unitType=file.read<int>();
	
	/* Create and return new unit: */
	switch(unitType)
		{
		case TRIANGLE:
			return new Triangle(file);
			break;
		
		case TETRAHEDRON:
			return new Tetrahedron(file);
			break;
		
		case OCTAHEDRON:
			return new Octahedron(file);
			break;
		
		case CYLINDER:
			return new Cylinder(file);
			break;
		
		case SPHERE:
			return new Sphere(file);
			break;
		
		default:
			Misc::throwStdErr("UnitManager::readUnit: Unknown unit type %d encountered in file",unitType);
			return 0;
		}
	}

void UnitManager::writeUnit(StructuralUnit* unit,Misc::File& file)
	{
	/* Determine unit's type: */
	int unitType=-1;
	if(dynamic_cast<Triangle*>(unit)!=0)
		unitType=TRIANGLE;
	else if(dynamic_cast<Tetrahedron*>(unit)!=0)
		unitType=TETRAHEDRON;
	else if(dynamic_cast<Octahedron*>(unit)!=0)
		unitType=OCTAHEDRON;
	else if(dynamic_cast<Cylinder*>(unit)!=0)
		unitType=CYLINDER;
	else if(dynamic_cast<Sphere*>(unit)!=0)
		unitType=SPHERE;
	if(unitType<0)
		Misc::throwStdErr("UnitManager::writeUnit: Unit of unkown type encountered");
	
	/* Write unit's state to file: */
	file.write<int>(unitType);
	unit->writeState(file);
	}

}
