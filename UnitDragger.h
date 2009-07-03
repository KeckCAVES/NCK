/***********************************************************************
UnitDragger - Class to drag structural units with a dragging tool.
Copyright (c) 2005 Oliver Kreylos
***********************************************************************/

#ifndef UNITDRAGGER_INCLUDED
#define UNITDRAGGER_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <Vrui/Tools/DraggingTool.h>
#include <Vrui/DraggingToolAdapter.h>

#include "AffineSpace.h"

/* Forward declarations: */
namespace NCK {
class StructuralUnit;
}
class NanotechConstructionKit;

class UnitDragger:public Vrui::DraggingToolAdapter,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef NCK::OrthonormalTransformation DragTransform; // Type for dragging transformations
	
	enum UnitType // Enumerated type for structural unit types
		{
		NONE,MARK,UNMARK,LOCK,UNLOCK,DELETE,FIRST_UNITTYPE
		};
	
	enum DraggingMode // Enumerated type for dragging modes
		{
		SINGLE_UNIT,LINKED_ASSEMBLY,INFLUENCE_SPHERE,INTERSTITIAL_VOID
		};
	
	private:
	struct UnitState // Dragging state for one dragged structural unit
		{
		/* Elements: */
		public:
		NCK::StructuralUnit* unit; // Pointer to dragged structural unit
		DragTransform dragTransformation; // Dragging transformation for dragged structural unit
		
		/* Constructors and destructors: */
		UnitState(NCK::StructuralUnit* sUnit,DragTransform inverseInitialTransformation)
			:unit(sUnit),
			 dragTransformation(inverseInitialTransformation*DragTransform(unit->getPosition()-NCK::Point::origin,unit->getOrientation()))
			{
			};
		};
	
	typedef std::vector<UnitState> UnitStateList;
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListId;
		
		/* Constructors and destructors: */
		public:
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	NanotechConstructionKit* application; // Pointer to the application object
	int createType; // Type of unit to be created when pinching fingers
	DraggingMode draggingMode; // Currently active dragging mode
	DragTransform influenceSphereTransform; // Current position and orientation of influence sphere
	NCK::Scalar influenceSphereRadius; // Current radius of dragger's influence sphere in physical coordinates
	NCK::Vector linearVelocity,angularVelocity; // Current linear and angular velocities of dragger scaled by current frame rate
	UnitStateList unitStates; // List of dragging states for dragged units
	
	/* Private methods: */
	void updateDragger(const Vrui::NavTrackerState& currentTransformation);
	
	/* Constructors and destructors: */
	public:
	UnitDragger(Vrui::DraggingTool* sTool,NanotechConstructionKit* sApplication); // Creates a unit dragger for the given dragging tool and application
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual void idleMotionCallback(Vrui::DraggingTool::IdleMotionCallbackData* cbData);
	virtual void dragStartCallback(Vrui::DraggingTool::DragStartCallbackData* cbData);
	virtual void dragCallback(Vrui::DraggingTool::DragCallbackData* cbData);
	virtual void dragEndCallback(Vrui::DraggingTool::DragEndCallbackData* cbData);
	
	/* New methods: */
	void setModes(int newCreateType,DraggingMode newDraggingMode);
	void glRenderAction(GLContextData& contextData) const;
	};

#endif
