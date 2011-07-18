/***********************************************************************
NanotechConstructionKit - Construction kit for nanostructures based on
simplified geometric simulation and VR interaction with structural unit
building blocks.
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

#ifndef NANOTECHCONSTRUCTIONKIT_INCLUDED
#define NANOTECHCONSTRUCTIONKIT_INCLUDED

#include <vector>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/FileSelectionDialog.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Application.h>

#include "AffineSpace.h"
#include "UnitDragger.h"

/* Forward declarations: */
class GLContextData;
namespace GLMotif {
class TextField;
class Popup;
class PopupMenu;
class PopupWindow;
}
namespace NCK {
class SpaceGrid;
class Polyhedron;
}

struct NanotechConstructionKit:public Vrui::Application
	{
	/* Embedded classes: */
	public:
	typedef GLMaterial::Color Color;
	
	typedef std::vector<UnitDragger*> UnitDraggerList;
	
	friend class UnitDragger;
	
	/* Elements: */
	
	/* Grid state: */
	NCK::SpaceGrid* grid; // The space grid containing all tetrahedra
	
	/* Interaction state: */
	int createType; // Type of unit to be created by unit draggers
	UnitDragger::DraggingMode draggingMode; // Currently active dragging mode
	bool overrideTools; // Flag whether to override all existing tools' modes with new default modes
	double influenceSphereRadius; // Radius of influence sphere
	UnitDraggerList unitDraggers; // List of currently instantiated unit draggers
	NCK::Polyhedron* currentVoid; // Polyhedron defining the currently visualized interstitial void
	
	/* Vrui state: */
	GLMotif::PopupMenu* mainMenu; // The main menu widget
	GLMotif::PopupWindow* statisticsDialog; // The statistics dialog widget
	GLMotif::TextField* numTrianglesValue; // Text field for number of triangles
	GLMotif::TextField* numTetrahedraValue; // Text field for number of tetrahedra
	GLMotif::TextField* numOctahedraValue; // Text field for number of octahedra
	GLMotif::TextField* numUnsharedVerticesValue; // Text field for number of unshared vertices
	GLMotif::TextField* averageBondLengthValue; // Text field for average bond length
	GLMotif::TextField* averageBondAngleValue; // Text field for average bond angle
	
	/* Private methods: */
	GLMotif::Popup* createTypeSelectionMenu(void); // Creates program's structural type selection menu
	GLMotif::Popup* createDraggingModeMenu(void); // Creates program's dragging mode selection menu
	GLMotif::Popup* createUnitOperationsMenu(void); // Creates program's unit operations menu
	GLMotif::Popup* createRenderTogglesMenu(void); // Creates program's rendering mode selection menu
	GLMotif::Popup* createIoMenu(void); // Creates program's input/output menu
	GLMotif::PopupMenu* createMainMenu(void); // Creates program's main menu
	GLMotif::PopupWindow* createStatisticsDialog(void); // Creates simulation statistics dialog
	void updateStatisticsDialog(void); // Updates state of the statistics dialog to the space grid
	
	/* Constructors and destructors: */
	NanotechConstructionKit(int& argc,char**& argv,char**& appDefaults);
	virtual ~NanotechConstructionKit(void);
	
	/* Methods: */
	virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
	virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	void typeSelectionMenuEntrySelectCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void draggingModeMenuEntrySelectCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData);
	void menuToggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void centerDisplayCallback(Misc::CallbackData* cbData);
	void overrideToolsToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void showStatisticsToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData);
	void unmarkAllUnitsCallback(Misc::CallbackData* cbData);
	void unlockAllUnitsCallback(Misc::CallbackData* cbData);
	void loadUnitsCallback(Misc::CallbackData* cbData);
	void loadUnitsOKCallback(GLMotif::FileSelectionDialog::OKCallbackData* cbData);
	void saveUnitsCallback(Misc::CallbackData* cbData);
	void saveGridStatisticsCallback(Misc::CallbackData* cbData);
	void createInputDeviceCallback(Misc::CallbackData* cbData);
	};

#endif
