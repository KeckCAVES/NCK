/***********************************************************************
NanotechConstructionKit - Construction kit for nanostructures based on
simplified geometric simulation and VR interaction with structural unit
building blocks.
Copyright (c) 2004-2005 Oliver Kreylos
***********************************************************************/

#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <time.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLValueCoders.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Vrui.h>

#include "AffineSpace.h"
#include "StructuralUnit.h"
#include "UnitManager.h"
#include "SpaceGrid.h"
#include "ReadUnitFile.h"
#include "ReadCarFile.h"
#include "Polyhedron.h"

#include "UnitDragger.h"

#include "NanotechConstructionKit.h"

/****************************************
Methods of class NanotechConstructionKit:
****************************************/

GLMotif::Popup* NanotechConstructionKit::createTypeSelectionMenu(void)
	{
	GLMotif::Popup* typeSelectionMenuPopup=new GLMotif::Popup("TypeSelectionMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* typeSelectionMenu=new GLMotif::RadioBox("TypeSelectionMenu",typeSelectionMenuPopup,false);
	typeSelectionMenu->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	GLMotif::ToggleButton* noneToggle=new GLMotif::ToggleButton("NoneToggle",typeSelectionMenu,"Don't Create Units");
	GLMotif::ToggleButton* markToggle=new GLMotif::ToggleButton("MarkToggle",typeSelectionMenu,"Mark Selected Unit(s)");
	GLMotif::ToggleButton* unmarkToggle=new GLMotif::ToggleButton("UnmarkToggle",typeSelectionMenu,"Unmark Selected Unit(s)");
	GLMotif::ToggleButton* lockToggle=new GLMotif::ToggleButton("LockToggle",typeSelectionMenu,"Lock Selected Unit(s)");
	GLMotif::ToggleButton* unlockToggle=new GLMotif::ToggleButton("UnlockToggle",typeSelectionMenu,"Unlock Selected Unit(s)");
	GLMotif::ToggleButton* deleteToggle=new GLMotif::ToggleButton("DeleteToggle",typeSelectionMenu,"Delete Selected Unit(s)");
	for(int i=0;i<NCK::UnitManager::getNumUnitTypes();++i)
		{
		char toggleNameBuffer[20];
		snprintf(toggleNameBuffer,sizeof(toggleNameBuffer),"unitTypeToggle%d",i);
		new GLMotif::ToggleButton(toggleNameBuffer,typeSelectionMenu,NCK::UnitManager::getUnitTypeName(i));
		}
	
	typeSelectionMenu->manageChild();
	switch(createType)
		{
		case UnitDragger::NONE:
			typeSelectionMenu->setSelectedToggle(noneToggle);
			break;
		
		case UnitDragger::MARK:
			typeSelectionMenu->setSelectedToggle(markToggle);
			break;
		
		case UnitDragger::UNMARK:
			typeSelectionMenu->setSelectedToggle(unmarkToggle);
			break;
		
		case UnitDragger::LOCK:
			typeSelectionMenu->setSelectedToggle(lockToggle);
			break;
		
		case UnitDragger::UNLOCK:
			typeSelectionMenu->setSelectedToggle(unlockToggle);
			break;
		
		case UnitDragger::DELETE:
			typeSelectionMenu->setSelectedToggle(deleteToggle);
			break;
		
		default:
			typeSelectionMenu->setSelectedToggle(createType-UnitDragger::FIRST_UNITTYPE+6);
		}
	typeSelectionMenu->getValueChangedCallbacks().add(this,&NanotechConstructionKit::typeSelectionMenuEntrySelectCallback);
	
	return typeSelectionMenuPopup;
	}

GLMotif::Popup* NanotechConstructionKit::createDraggingModeMenu(void)
	{
	GLMotif::Popup* draggingModeMenuPopup=new GLMotif::Popup("DraggingModeMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* draggingModeMenu=new GLMotif::RadioBox("DraggingModeMenu",draggingModeMenuPopup,false);
	draggingModeMenu->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	GLMotif::ToggleButton* singleUnitToggle=new GLMotif::ToggleButton("SingleUnitToggle",draggingModeMenu,"Single Structural Unit");
	GLMotif::ToggleButton* linkedAssemblyToggle=new GLMotif::ToggleButton("LinkedAssemblyToggle",draggingModeMenu,"Linked Unit Assembly");
	GLMotif::ToggleButton* influenceSphereToggle=new GLMotif::ToggleButton("InfluenceSphereToggle",draggingModeMenu,"Influence Sphere");
	GLMotif::ToggleButton* interstitialVoidToggle=new GLMotif::ToggleButton("InterstitialVoidToggle",draggingModeMenu,"Interstitial Void");
	
	draggingModeMenu->manageChild();
	switch(draggingMode)
		{
		case UnitDragger::SINGLE_UNIT:
			draggingModeMenu->setSelectedToggle(singleUnitToggle);
			break;
		
		case UnitDragger::LINKED_ASSEMBLY:
			draggingModeMenu->setSelectedToggle(linkedAssemblyToggle);
			break;
		
		case UnitDragger::INFLUENCE_SPHERE:
			draggingModeMenu->setSelectedToggle(influenceSphereToggle);
			break;
		
		case UnitDragger::INTERSTITIAL_VOID:
			draggingModeMenu->setSelectedToggle(interstitialVoidToggle);
			break;
		}
	draggingModeMenu->getValueChangedCallbacks().add(this,&NanotechConstructionKit::draggingModeMenuEntrySelectCallback);
	
	return draggingModeMenuPopup;
	}

GLMotif::Popup* NanotechConstructionKit::createUnitOperationsMenu(void)
	{
	GLMotif::Popup* unitOperationsMenuPopup=new GLMotif::Popup("UnitOperationsMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RowColumn* unitOperationsMenu=new GLMotif::RowColumn("UnitOperationsMenu",unitOperationsMenuPopup,false);
	
	GLMotif::Button* unmarkAllUnitsButton=new GLMotif::Button("UnmarkAllUnitsButton",unitOperationsMenu,"Unmark All Units");
	unmarkAllUnitsButton->getSelectCallbacks().add(this,&NanotechConstructionKit::unmarkAllUnitsCallback);
	
	GLMotif::Button* unlockAllUnitsButton=new GLMotif::Button("UnlockAllUnitsButton",unitOperationsMenu,"Unlock All Units");
	unlockAllUnitsButton->getSelectCallbacks().add(this,&NanotechConstructionKit::unlockAllUnitsCallback);
	
	unitOperationsMenu->manageChild();
	
	return unitOperationsMenuPopup;
	}

GLMotif::Popup* NanotechConstructionKit::createRenderTogglesMenu(void)
	{
	GLMotif::Popup* renderTogglesMenuPopup=new GLMotif::Popup("RenderTogglesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RowColumn* renderTogglesMenu=new GLMotif::RowColumn("RenderTogglesMenu",renderTogglesMenuPopup,false);
	
	GLMotif::ToggleButton* showUnitsToggle=new GLMotif::ToggleButton("ShowUnitsToggle",renderTogglesMenu,"Show Units");
	showUnitsToggle->setToggle(grid->getShowUnits());
	showUnitsToggle->getValueChangedCallbacks().add(this,&NanotechConstructionKit::menuToggleSelectCallback);
	GLMotif::ToggleButton* showVelocitiesToggle=new GLMotif::ToggleButton("ShowVelocitiesToggle",renderTogglesMenu,"Show Velocities");
	showVelocitiesToggle->setToggle(grid->getShowVelocities());
	showVelocitiesToggle->getValueChangedCallbacks().add(this,&NanotechConstructionKit::menuToggleSelectCallback);
	GLMotif::ToggleButton* showVertexLinksToggle=new GLMotif::ToggleButton("ShowVertexLinksToggle",renderTogglesMenu,"Show Vertex Links");
	showVertexLinksToggle->setToggle(grid->getShowVertexLinks());
	showVertexLinksToggle->getValueChangedCallbacks().add(this,&NanotechConstructionKit::menuToggleSelectCallback);
	GLMotif::ToggleButton* showUnlinkedVerticesToggle=new GLMotif::ToggleButton("ShowUnlinkedVerticesToggle",renderTogglesMenu,"Show Unlinked Vertices");
	showUnlinkedVerticesToggle->setToggle(grid->getShowUnlinkedVertices());
	showUnlinkedVerticesToggle->getValueChangedCallbacks().add(this,&NanotechConstructionKit::menuToggleSelectCallback);
	
	renderTogglesMenu->manageChild();
	
	return renderTogglesMenuPopup;
	}

GLMotif::Popup* NanotechConstructionKit::createIoMenu(void)
	{
	GLMotif::Popup* ioMenuPopup=new GLMotif::Popup("IoMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RowColumn* ioMenu=new GLMotif::RowColumn("IoMenu",ioMenuPopup,false);
	
	GLMotif::Button* loadUnitsButton=new GLMotif::Button("LoadUnitsButton",ioMenu,"Load Units...");
	loadUnitsButton->getSelectCallbacks().add(this,&NanotechConstructionKit::loadUnitsCallback);
	
	GLMotif::Button* saveUnitsButton=new GLMotif::Button("SaveUnitsButton",ioMenu,"Save Units...");
	saveUnitsButton->getSelectCallbacks().add(this,&NanotechConstructionKit::saveUnitsCallback);
	
	GLMotif::Button* saveGridStatisticsButton=new GLMotif::Button("SaveGridStatisticsButton",ioMenu,"Save Grid Statistics...");
	saveGridStatisticsButton->getSelectCallbacks().add(this,&NanotechConstructionKit::saveGridStatisticsCallback);
	
	ioMenu->manageChild();
	
	return ioMenuPopup;
	}

GLMotif::PopupMenu* NanotechConstructionKit::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Nanotech Construction Kit");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&NanotechConstructionKit::centerDisplayCallback);
	
	GLMotif::CascadeButton* typeSelectionCascade=new GLMotif::CascadeButton("TypeSelectionCascade",mainMenu,"Structural Unit Types");
	typeSelectionCascade->setPopup(createTypeSelectionMenu());
	
	GLMotif::CascadeButton* draggingModeCascade=new GLMotif::CascadeButton("DraggingModeCascade",mainMenu,"Dragging Modes");
	draggingModeCascade->setPopup(createDraggingModeMenu());
	
	GLMotif::CascadeButton* unitOperationsCascade=new GLMotif::CascadeButton("UnitOperationsCascade",mainMenu,"Unit Operations");
	unitOperationsCascade->setPopup(createUnitOperationsMenu());
	
	GLMotif::ToggleButton* overrideToolsToggle=new GLMotif::ToggleButton("OverrideToolsToggle",mainMenu,"Override Tools");
	overrideToolsToggle->setToggle(overrideTools);
	overrideToolsToggle->getValueChangedCallbacks().add(this,&NanotechConstructionKit::overrideToolsToggleValueChangedCallback);
	
	GLMotif::CascadeButton* renderTogglesCascade=new GLMotif::CascadeButton("RenderTogglesCascade",mainMenu,"Rendering Modes");
	renderTogglesCascade->setPopup(createRenderTogglesMenu());
	
	GLMotif::CascadeButton* ioMenuCascade=new GLMotif::CascadeButton("IoMenuCascade",mainMenu,"I/O Menus");
	ioMenuCascade->setPopup(createIoMenu());
	
	GLMotif::ToggleButton* showStatisticsToggle=new GLMotif::ToggleButton("ShowStatisticsToggle",mainMenu,"Show Statistics Dialog");
	showStatisticsToggle->getValueChangedCallbacks().add(this,&NanotechConstructionKit::showStatisticsToggleValueChangedCallback);
	showStatisticsToggle->setToggle(false);
	
	GLMotif::Button* createInputDeviceButton=new GLMotif::Button("CreateInputDeviceButton",mainMenu,"Create Input Device");
	createInputDeviceButton->getSelectCallbacks().add(this,&NanotechConstructionKit::createInputDeviceCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

GLMotif::PopupWindow* NanotechConstructionKit::createStatisticsDialog(void)
	{
	GLMotif::PopupWindow* statisticsDialogPopup=new GLMotif::PopupWindow("StatisticsDialogPopup",Vrui::getWidgetManager(),"Simulation Statistics");
	
	GLMotif::RowColumn* statistics=new GLMotif::RowColumn("Statistics",statisticsDialogPopup,false);
	statistics->setNumMinorWidgets(2);
	
	/* Create a label / text field combo to display number of triangles: */
	GLMotif::Label* numTrianglesLabel=new GLMotif::Label("NumTrianglesLabel",statistics,"Number of Triangles:");
	numTrianglesLabel->setHAlignment(GLFont::Left);
	
	numTrianglesValue=new GLMotif::TextField("NumTrianglesValue",statistics,6);
	numTrianglesValue->setFieldWidth(6);
	
	/* Create a label / text field combo to display number of tetrahedra: */
	GLMotif::Label* numTetrahedraLabel=new GLMotif::Label("NumTetrahedraLabel",statistics,"Number of Tetrahedra:");
	numTetrahedraLabel->setHAlignment(GLFont::Left);
	
	numTetrahedraValue=new GLMotif::TextField("NumTetrahedraValue",statistics,6);
	numTetrahedraValue->setFieldWidth(6);
	
	/* Create a label / text field combo to display number of octahedra: */
	GLMotif::Label* numOctahedraLabel=new GLMotif::Label("NumOctahedraLabel",statistics,"Number of Octahedra:");
	numOctahedraLabel->setHAlignment(GLFont::Left);
	
	numOctahedraValue=new GLMotif::TextField("NumOctahedraValue",statistics,6);
	numOctahedraValue->setFieldWidth(6);
	
	/* Create a label / text field combo to display number of unshared vertices: */
	GLMotif::Label* numUnsharedVerticesLabel=new GLMotif::Label("NumUnsharedVerticesLabel",statistics,"Number of Unshared Vertices:");
	numUnsharedVerticesLabel->setHAlignment(GLFont::Left);
	
	numUnsharedVerticesValue=new GLMotif::TextField("NumUnsharedVerticesValue",statistics,6);
	numUnsharedVerticesValue->setFieldWidth(6);
	
	/* Create a label / text field combo to display average bond length: */
	GLMotif::Label* averageBondLengthLabel=new GLMotif::Label("AverageBondLengthLabel",statistics,"Average bond length:");
	averageBondLengthLabel->setHAlignment(GLFont::Left);
	
	averageBondLengthValue=new GLMotif::TextField("AverageBondLengthValue",statistics,6);
	averageBondLengthValue->setFieldWidth(6);
	averageBondLengthValue->setPrecision(2);
	
	/* Create a label / text field combo to display average bond angle: */
	GLMotif::Label* averageBondAngleLabel=new GLMotif::Label("AverageBondAngleLabel",statistics,"Average bond angle:");
	averageBondAngleLabel->setHAlignment(GLFont::Left);
	
	averageBondAngleValue=new GLMotif::TextField("AverageBondAngleValue",statistics,6);
	averageBondAngleValue->setFieldWidth(6);
	averageBondAngleValue->setPrecision(2);
	
	statistics->manageChild();
	
	return statisticsDialogPopup;
	}

void NanotechConstructionKit::updateStatisticsDialog(void)
	{
	/* Get statistics from space grid: */
	NCK::SpaceGrid::GridStatistics stats=grid->calcGridStatistics();
	
	/* Update number of tetrahedra: */
	numTetrahedraValue->setValue(stats.numUnits);
	
	/* Update number of unshared vertices: */
	numUnsharedVerticesValue->setValue(stats.numUnsharedVertices);
	
	/* Update average bond angle: */
	averageBondAngleValue->setValue(Math::deg(stats.averageBondAngle));
	
	/* Update average bond length: */
	averageBondLengthValue->setValue(stats.averageBondLength);
	}

NanotechConstructionKit::NanotechConstructionKit(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 grid(0),
	 createType(UnitDragger::NONE),draggingMode(UnitDragger::SINGLE_UNIT),
	 overrideTools(true),
	 influenceSphereRadius(10.0*Vrui::getUiSize()),
	 currentVoid(0),
	 mainMenu(0),statisticsDialog(0)
	{
	/* Parse the command line: */
	int loadFileMode=0;
	const char* loadFileName=0;
	const char* configFileName=NANOTECHCONSTRUCTIONKIT_CFGFILENAME;
	NCK::Scalar domainSize(24);
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"CAR")==0)
				loadFileMode=1;
			else if(strcasecmp(argv[i]+1,"UNITS")==0)
				loadFileMode=0;
			else if(strcasecmp(argv[i]+1,"CONFIGFILE")==0)
				{
				++i;
				configFileName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"DOMAINSIZE")==0)
				{
				++i;
				domainSize=NCK::Scalar(atof(argv[i]));
				}
			}
		else
			loadFileName=argv[i];
		}
	
	/* Initialize structural unit classes: */
	Misc::ConfigurationFile nckConfigFile(configFileName);
	nckConfigFile.setCurrentSection("/NanotechConstructionKit");
	NCK::Scalar maxUnitRadius=NCK::UnitManager::initializeUnitTypes(nckConfigFile.getCurrentSection());
	
	/* Load the input file: */
	if(loadFileName!=0)
		{
		if(loadFileMode==0)
			{
			/* Read a unit file: */
			grid=NCK::readUnitFile(maxUnitRadius,loadFileName);
			}
		else
			{
			/* Read a CAR file: */
			grid=NCK::readCarFile(loadFileName);
			}
		}
	else
		{
		/* Create empty space grid: */
		NCK::Box gridBox(NCK::Point(-domainSize,-domainSize,-domainSize),NCK::Point(domainSize,domainSize,domainSize));
		grid=new NCK::SpaceGrid(gridBox,maxUnitRadius,0x7);
		}
	
	/* Set space grid parameters: */
	grid->setAttenuation(Math::pow(nckConfigFile.retrieveValue<double>("./attenuation",0.95),20.0));
	
	/* Set space grid rendering flags: */
	grid->readRenderingFlags(nckConfigFile.getSection("./RenderingFlags"));
	
	/* Calculate grid boundary color based on background color: */
	Vrui::Color gridColor;
	for(int i=0;i<3;++i)
		gridColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	gridColor[3]=1.0f;
	grid->setGridBoundaryColor(gridColor);
	
	/* Create the UI widgets: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	statisticsDialog=createStatisticsDialog();
	
	/* Initialize navigation transformation: */
	centerDisplayCallback(0);
	
	/* Enable continuous update: */
	Vrui::updateContinuously();
	}

NanotechConstructionKit::~NanotechConstructionKit(void)
	{
	delete grid;
	delete currentVoid;
	delete mainMenu;
	delete statisticsDialog;
	
	/* Deinitialize structural unit types: */
	NCK::UnitManager::deinitializeUnitTypes();
	}

void NanotechConstructionKit::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Create a unit dragger object and associate it with the new tool: */
		UnitDragger* newDragger=new UnitDragger(tool,this);
		
		/* Add new dragger to list: */
		unitDraggers.push_back(newDragger);
		}
	}

void NanotechConstructionKit::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool is a dragging tool: */
	Vrui::DraggingTool* tool=dynamic_cast<Vrui::DraggingTool*>(cbData->tool);
	if(tool!=0)
		{
		/* Find the unit dragger associated with the tool in the list: */
		UnitDraggerList::iterator udIt;
		for(udIt=unitDraggers.begin();udIt!=unitDraggers.end()&&(*udIt)->getTool()!=tool;++udIt)
			;
		if(udIt!=unitDraggers.end())
			{
			/* Remove the unit dragger: */
			delete *udIt;
			unitDraggers.erase(udIt);
			}
		}
	}

void NanotechConstructionKit::frame(void)
	{
	/* Advance simulation state: */
	grid->advanceTime(NCK::Scalar(0.05));
	
	/* Update statistics dialog: */
	updateStatisticsDialog();
	}

void NanotechConstructionKit::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_LIGHTING_BIT|GL_POLYGON_BIT|GL_TRANSFORM_BIT);
	
	/* Render the grid (and all units): */
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	grid->glRenderAction(contextData);
	
	/* Render the current interstitial void: */
	if(currentVoid!=0)
		currentVoid->glRenderAction(contextData);
	
	/* Render all unit draggers: */
	for(UnitDraggerList::const_iterator udIt=unitDraggers.begin();udIt!=unitDraggers.end();++udIt)
		(*udIt)->glRenderAction(contextData);
	
	glPopAttrib();
	}

void NanotechConstructionKit::typeSelectionMenuEntrySelectCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	int toggleIndex=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	switch(toggleIndex)
		{
		case 0:
			createType=UnitDragger::NONE;
			break;
		
		case 1:
			createType=UnitDragger::MARK;
			break;
		
		case 2:
			createType=UnitDragger::UNMARK;
			break;
		
		case 3:
			createType=UnitDragger::LOCK;
			break;
		
		case 4:
			createType=UnitDragger::UNLOCK;
			break;
		
		case 5:
			createType=UnitDragger::DELETE;
			break;
		
		default:
			createType=toggleIndex-6+UnitDragger::FIRST_UNITTYPE;
		}
	
	if(overrideTools)
		{
		/* Set the modes of all tools to the new defaults: */
		for(UnitDraggerList::iterator udIt=unitDraggers.begin();udIt!=unitDraggers.end();++udIt)
			(*udIt)->setModes(createType,draggingMode);
		}
	}

void NanotechConstructionKit::draggingModeMenuEntrySelectCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	int toggleIndex=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	switch(toggleIndex)
		{
		case 0:
			draggingMode=UnitDragger::SINGLE_UNIT;
			break;
		
		case 1:
			draggingMode=UnitDragger::LINKED_ASSEMBLY;
			break;
		
		case 2:
			draggingMode=UnitDragger::INFLUENCE_SPHERE;
			break;
		
		case 3:
			draggingMode=UnitDragger::INTERSTITIAL_VOID;
			break;
		
		default:
			;
		}
	
	if(overrideTools)
		{
		/* Set the modes of all tools to the new defaults: */
		for(UnitDraggerList::iterator udIt=unitDraggers.begin();udIt!=unitDraggers.end();++udIt)
			(*udIt)->setModes(createType,draggingMode);
		}
	}

void NanotechConstructionKit::menuToggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	if(strcmp(cbData->toggle->getName(),"ShowUnitsToggle")==0)
		grid->setShowUnits(cbData->set);
	if(strcmp(cbData->toggle->getName(),"ShowVelocitiesToggle")==0)
		grid->setShowVelocities(cbData->set);
	else if(strcmp(cbData->toggle->getName(),"ShowVertexLinksToggle")==0)
		grid->setShowVertexLinks(cbData->set);
	else if(strcmp(cbData->toggle->getName(),"ShowUnlinkedVerticesToggle")==0)
		grid->setShowUnlinkedVertices(cbData->set);
	}

void NanotechConstructionKit::centerDisplayCallback(Misc::CallbackData* cbData)
	{
	/* Center space grid in the display: */
	NCK::Point bbCenter=Geometry::mid(grid->getBoundingBox().min,grid->getBoundingBox().max);
	NCK::Scalar bbSize=Geometry::dist(grid->getBoundingBox().min,grid->getBoundingBox().max);
	Vrui::setNavigationTransformation(Vrui::Point(bbCenter),Vrui::Scalar(bbSize));
	}

void NanotechConstructionKit::overrideToolsToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	overrideTools=cbData->set;
	}

void NanotechConstructionKit::showStatisticsToggleValueChangedCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show statistics dialog: */
	if(cbData->set)
		Vrui::getWidgetManager()->popupPrimaryWidget(statisticsDialog,Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
	else
		Vrui::popdownPrimaryWidget(statisticsDialog);
	}

void NanotechConstructionKit::unmarkAllUnitsCallback(Misc::CallbackData* cbData)
	{
	NCK::SpaceGrid::StructuralUnitList units=grid->getAllUnits();
	for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=units.begin();uIt!=units.end();++uIt)
		grid->unmarkUnit(*uIt);
	}

void NanotechConstructionKit::unlockAllUnitsCallback(Misc::CallbackData* cbData)
	{
	NCK::SpaceGrid::StructuralUnitList units=grid->getAllUnits();
	for(NCK::SpaceGrid::StructuralUnitList::iterator uIt=units.begin();uIt!=units.end();++uIt)
		grid->unlockUnit(*uIt);
	}

void NanotechConstructionKit::loadUnitsCallback(Misc::CallbackData* cbData)
	{
	NCK::readUnitFile(grid,"Scratch.units");
	}

void NanotechConstructionKit::saveUnitsCallback(Misc::CallbackData* cbData)
	{
	NCK::writeUnitFile("Scratch.units",grid);
	NCK::writeCarFile("Scratch.car",grid);
	}

void NanotechConstructionKit::saveGridStatisticsCallback(Misc::CallbackData* cbData)
	{
	/* Get statistics from space grid: */
	NCK::SpaceGrid::GridStatistics stats=grid->calcGridStatistics();
	
	/* Write statistics to file: */
	FILE* statFile=fopen("GridStatistics.txt","wt");
	fprintf(statFile,"Number of units            : %6d\n",stats.numUnits);
	fprintf(statFile,"Number of unshared vertices: %6d\n\n",stats.numUnsharedVertices);
	
	fprintf(statFile,"Bond length histogram:\n");
	int totalNumBondLengths=0;
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numBondLengthBins;++i)
		totalNumBondLengths+=stats.bondLengthHistogram[i];
	NCK::Scalar bondLengthBinSize=(stats.bondLengthMax-stats.bondLengthMin)/NCK::Scalar(NCK::SpaceGrid::GridStatistics::numBondLengthBins);
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numBondLengthBins;++i)
		{
		NCK::Scalar binMin=stats.bondLengthMin+bondLengthBinSize*NCK::Scalar(i);
		NCK::Scalar binMax=stats.bondLengthMin+bondLengthBinSize*NCK::Scalar(i+1);
		fprintf(statFile,"Bin %3d, [%8.4lf, %8.4lf]: %8.6lf\n",i,binMin,binMax,double(stats.bondLengthHistogram[i])/double(totalNumBondLengths));
		}
	fprintf(statFile,"Average bond length: %8.4lf\n\n",stats.averageBondLength);
	
	fprintf(statFile,"Bond angle histogram:\n");
	int totalNumBondAngles=0;
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numBondAngleBins;++i)
		totalNumBondAngles+=stats.bondAngleHistogram[i];
	NCK::Scalar bondAngleBinSize=(stats.bondAngleMax-stats.bondAngleMin)/NCK::Scalar(NCK::SpaceGrid::GridStatistics::numBondAngleBins);
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numBondAngleBins;++i)
		{
		NCK::Scalar binMin=stats.bondAngleMin+bondAngleBinSize*NCK::Scalar(i);
		NCK::Scalar binMax=stats.bondAngleMin+bondAngleBinSize*NCK::Scalar(i+1);
		fprintf(statFile,"Bin %3d, [%8.4lf, %8.4lf]: %8.6lf\n",i,Math::deg(binMin),Math::deg(binMax),double(stats.bondAngleHistogram[i])/double(totalNumBondAngles));
		}
	fprintf(statFile,"Average bond angle: %8.4lf\n\n",Math::deg(stats.averageBondAngle));
	
	fprintf(statFile,"Center distance histogram:\n");
	int totalNumCenterDists=0;
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numCenterDistBins;++i)
		totalNumCenterDists+=stats.centerDistHistogram[i];
	NCK::Scalar centerDistBinSize=(stats.centerDistMax-stats.centerDistMin)/NCK::Scalar(NCK::SpaceGrid::GridStatistics::numCenterDistBins);
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numCenterDistBins;++i)
		{
		NCK::Scalar binMin=stats.centerDistMin+centerDistBinSize*NCK::Scalar(i);
		NCK::Scalar binMax=stats.centerDistMin+centerDistBinSize*NCK::Scalar(i+1);
		fprintf(statFile,"Bin %3d, [%8.4lf, %8.4lf]: %8.6lf\n",i,binMin,binMax,double(stats.centerDistHistogram[i])/double(totalNumCenterDists));
		}
	fprintf(statFile,"Average center distance: %8.4lf\n\n",stats.averageCenterDist);
	
	fprintf(statFile,"Internal bond length histogram:\n");
	int totalNumInternalBondLengths=0;
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numInternalBondLengthBins;++i)
		totalNumInternalBondLengths+=stats.internalBondLengthHistogram[i];
	NCK::Scalar internalBondLengthBinSize=(stats.internalBondLengthMax-stats.internalBondLengthMin)/NCK::Scalar(NCK::SpaceGrid::GridStatistics::numInternalBondLengthBins);
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numInternalBondLengthBins;++i)
		{
		NCK::Scalar binMin=stats.internalBondLengthMin+internalBondLengthBinSize*NCK::Scalar(i);
		NCK::Scalar binMax=stats.internalBondLengthMin+internalBondLengthBinSize*NCK::Scalar(i+1);
		fprintf(statFile,"Bin %3d, [%8.4lf, %8.4lf]: %8.6lf\n",i,binMin,binMax,double(stats.internalBondLengthHistogram[i])/double(totalNumInternalBondLengths));
		}
	fprintf(statFile,"Average internal bond length: %8.4lf\n\n",stats.averageInternalBondLength);
	
	fprintf(statFile,"Internal bond angle histogram:\n");
	int totalNumInternalBondAngles=0;
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numInternalBondAngleBins;++i)
		totalNumInternalBondAngles+=stats.internalBondAngleHistogram[i];
	NCK::Scalar internalBondAngleBinSize=(stats.internalBondAngleMax-stats.internalBondAngleMin)/NCK::Scalar(NCK::SpaceGrid::GridStatistics::numInternalBondAngleBins);
	for(int i=0;i<NCK::SpaceGrid::GridStatistics::numInternalBondAngleBins;++i)
		{
		NCK::Scalar binMin=stats.internalBondAngleMin+internalBondAngleBinSize*NCK::Scalar(i);
		NCK::Scalar binMax=stats.internalBondAngleMin+internalBondAngleBinSize*NCK::Scalar(i+1);
		fprintf(statFile,"Bin %3d, [%8.4lf, %8.4lf]: %8.6lf\n",i,Math::deg(binMin),Math::deg(binMax),double(stats.internalBondAngleHistogram[i])/double(totalNumInternalBondAngles));
		}
	fprintf(statFile,"Average internal bond angle: %8.4lf\n",Math::deg(stats.averageInternalBondAngle));
	
	fclose(statFile);
	}

void NanotechConstructionKit::createInputDeviceCallback(Misc::CallbackData* cbData)
	{
	Vrui::addVirtualInputDevice("Virtual",2,0);
	}

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		NanotechConstructionKit nck(argc,argv,appDefaults);
		nck.run();
		}
	catch(std::runtime_error error)
		{
		fprintf(stderr,"Caught exception %s\n",error.what());
		return 1;
		}
	
	return 0;
	}
