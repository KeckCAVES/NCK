/***********************************************************************
UnitDragger - Class to drag structural units with a dragging tool.
Copyright (c) 2005 Oliver Kreylos
***********************************************************************/

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLModels.h>
#include <GL/GLTransformationWrappers.h>

#include "StructuralUnit.h"
#include "UnitManager.h"
#include "SpaceGrid.h"
#include "Polyhedron.h"
#include "NanotechConstructionKit.h"

#include "UnitDragger.h"

/**************************************
Methods of class UnitDragger::DataItem:
**************************************/

UnitDragger::DataItem::DataItem(void)
	:displayListId(glGenLists(1))
	{
	}

UnitDragger::DataItem::~DataItem(void)
	{
	glDeleteLists(displayListId,1);
	}

/****************************
Methods of class UnitDragger:
****************************/

void UnitDragger::updateDragger(const Vrui::NavTrackerState& currentTransformation)
	{
	/* Calculate new transformation and increment transformation: */
	DragTransform newTransform=DragTransform(currentTransformation.getTranslation(),currentTransformation.getRotation());
	NCK::Rotation dr=newTransform.getRotation()*Geometry::invert(influenceSphereTransform.getRotation());
	NCK::Vector dt=newTransform.getTranslation()-influenceSphereTransform.getTranslation();
	
	/* Update dragger's current position, orientation and radius: */
	influenceSphereTransform=newTransform;
	influenceSphereRadius=application->influenceSphereRadius*currentTransformation.getScaling();
	
	/* Calculate dragger's linear and angular velocities: */
	linearVelocity=dt;
	angularVelocity=dr.getScaledAxis();
	}

UnitDragger::UnitDragger(Vrui::DraggingTool* sTool,NanotechConstructionKit* sApplication)
	:DraggingToolAdapter(sTool),
	 application(sApplication),
	 createType(application->createType),
	 draggingMode(application->draggingMode)
	{
	}

void UnitDragger::initContext(GLContextData& contextData) const
	{
	/* Create a context data item and add it to the context data: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Render the influence model: */
	glNewList(dataItem->displayListId,GL_COMPILE);
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_ENABLE_BIT|GL_LINE_BIT|GL_POLYGON_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glColor4f(0.0f,1.0f,0.0f,0.33f);
	glDrawSphereIcosahedron(1.0f,5);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glColor4f(0.1f,0.5f,0.1f,0.33f);
	glDrawSphereIcosahedron(1.0f,5);
	glDepthMask(GL_TRUE);
	glPopAttrib();
	glEndList();
	}

void UnitDragger::idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData)
	{
	/* Update dragger's current position, orientation and radius: */
	updateDragger(cbData->currentTransformation);
	}

void UnitDragger::dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData)
	{
	/* Update dragger's current position, orientation and radius: */
	updateDragger(cbData->startTransformation);
	
	NCK::Point p=influenceSphereTransform.getOrigin();
	const NCK::Rotation& o=influenceSphereTransform.getRotation();
	if(draggingMode==INTERSTITIAL_VOID)
		{
		/* Extract the interstitial void polyhedron surrounding the query position: */
		delete application->currentVoid;
		// application->currentVoid=new NCK::Polyhedron(*application->grid,application->screenCenter);
		application->currentVoid=new NCK::Polyhedron(*application->grid,p);
		}
	else if(draggingMode!=INFLUENCE_SPHERE)
		{
		NCK::SpaceGrid::StructuralUnitList selectedUnits;
		switch(draggingMode)
			{
			case SINGLE_UNIT:
				{
				NCK::StructuralUnit* unit=0;
				if(cbData->rayBased)
					unit=application->grid->findUnit(cbData->ray);
				else
					unit=application->grid->findUnit(p);
				if(unit!=0)
					selectedUnits.push_back(unit);
				break;
				}
			
			case LINKED_ASSEMBLY:
				if(cbData->rayBased)
					selectedUnits=application->grid->findLinkedUnits(cbData->ray);
				else
					selectedUnits=application->grid->findLinkedUnits(p);
				break;
			}
		
		if(createType==MARK||createType==UNMARK)
			{
			/* Toggle marked state for all selected units: */
			application->grid->toggleUnitsMark(selectedUnits);
			
			/* Clear the selection list: */
			selectedUnits.clear();
			}
		else if(createType==LOCK||createType==UNLOCK)
			{
			/* Toggle locking state for all selected units: */
			application->grid->toggleUnitsLock(selectedUnits);
			
			/* Clear the selection list: */
			selectedUnits.clear();
			}
		else if(createType==DELETE)
			{
			/* Delete all selected units: */
			for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
				{
				/* Remove the object from the space grid: */
				application->grid->removeUnit(*uIt);
				
				/* Delete the object: */
				delete *uIt;
				}
			
			/* Clear the selection list: */
			selectedUnits.clear();
			}
		else if(createType>=FIRST_UNITTYPE&&selectedUnits.empty())
			{
			/* Add new structural unit at glove position and orientation: */
			NCK::StructuralUnit* newUnit=NCK::UnitManager::createUnit(createType-FIRST_UNITTYPE,p,o);
			if(newUnit!=0)
				{
				application->grid->addUnit(newUnit);
				selectedUnits.push_back(newUnit);
				}
			}
		
		/* Start dragging found structural unit(s): */
		DragTransform start=influenceSphereTransform;
		start.doInvert();
		for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
			unitStates.push_back(UnitState(*uIt,start));
		}
	}

void UnitDragger::dragCallback(Vrui::DraggingTool::DragCallbackData* cbData)
	{
	/* Update dragger's current position, orientation and radius: */
	updateDragger(cbData->currentTransformation);
	
	/* Drag selected structural unit(s): */
	if(draggingMode==INFLUENCE_SPHERE)
		{
		/* Find all structural units inside the influence sphere: */
		NCK::Point p=influenceSphereTransform.getOrigin();
		NCK::SpaceGrid::StructuralUnitList selectedUnits=application->grid->findUnits(p,influenceSphereRadius);
		
		switch(createType)
			{
			case MARK:
				/* Mark all units inside the sphere: */
				for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
					application->grid->markUnit(*uIt);
				break;
			
			case UNMARK:
				/* Unmark all units inside the sphere: */
				for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
					application->grid->unmarkUnit(*uIt);
				break;
			
			case LOCK:
				/* Lock all units inside the sphere: */
				for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
					application->grid->lockUnit(*uIt);
				break;
			
			case UNLOCK:
				/* Unlock all units inside the sphere: */
				for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
					application->grid->unlockUnit(*uIt);
				break;
			
			case DELETE:
				break;
			
			default:
				{
				/* Drag all units currently inside the influence sphere: */
				NCK::Rotation r=NCK::Rotation::rotateScaledAxis(angularVelocity);
				for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=selectedUnits.begin();uIt!=selectedUnits.end();++uIt)
					{
					/* Move and rotate this unit: */
					NCK::Point up=(*uIt)->getPosition();
					NCK::Rotation uo=(*uIt)->getOrientation();
					NCK::Vector d=up-p;
					NCK::Scalar w=NCK::Scalar(1)-Geometry::mag(d)/influenceSphereRadius;
					if(w>NCK::Scalar(0))
						{
						NCK::Vector disp=linearVelocity;
						disp+=Geometry::cross(angularVelocity,d);
						disp*=w;
						up+=disp;
						uo.leftMultiply(NCK::Rotation::rotateScaledAxis(angularVelocity*w));
						application->grid->setUnitPositionOrientation(*uIt,up,uo);
						//(*uIt)->setVelocities(NCK::Vector::zero,NCK::Vector::zero);
						}
					}
				}
			}
		}
	else
		{
		for(UnitStateList::iterator uIt=unitStates.begin();uIt!=unitStates.end();++uIt)
			{
			/* Calculate goal transformation for object and set object's position and orientation: */
			DragTransform goalTransformation=influenceSphereTransform*uIt->dragTransformation;
			application->grid->setUnitPositionOrientation(uIt->unit,goalTransformation.getOrigin(),goalTransformation.getRotation());
			uIt->unit->setVelocities(NCK::Vector::zero,NCK::Vector::zero);
			}
		}
	}

void UnitDragger::dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData)
	{
	/* Update dragger's current position, orientation and radius: */
	updateDragger(cbData->finalTransformation);
	
	/* Release dragged units: */
	unitStates.clear();
	}

void UnitDragger::setModes(int newCreateType,UnitDragger::DraggingMode newDraggingMode)
	{
	createType=newCreateType;
	draggingMode=newDraggingMode;
	}

void UnitDragger::glRenderAction(GLContextData& contextData) const
	{
	/* Get a pointer to the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	if(draggingMode==INFLUENCE_SPHERE)
		{
		/* Translate coordinate system to dragger's position and orientation: */
		glPushMatrix();
		glMultMatrix(influenceSphereTransform);
		glScaled(influenceSphereRadius,influenceSphereRadius,influenceSphereRadius);
		glCallList(dataItem->displayListId);
		glPopMatrix();
		}
	}
