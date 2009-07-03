/***********************************************************************
SpaceGrid - Class for 3D grids of cells to store and locate structural
units.
Copyright (c) 2003 Oliver Kreylos
***********************************************************************/

#ifndef SPACEGRID_INCLUDED
#define SPACEGRID_INCLUDED

#include <vector>
#include <Misc/Array.h>
#include <GL/gl.h>
#include <GL/GLObject.h>
#include <GL/GLColor.h>
#include <GL/GLMaterial.h>

#include "AffineSpace.h"
#include "SpaceGridCell.h"

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace NCK {
class StructuralUnit;
}

namespace NCK {

class SpaceGrid:public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Geometry::ComponentArray<Scalar,3> Size; // Type for grid and grid cell sizes
	typedef std::vector<StructuralUnit*> StructuralUnitList; // Type for lists of structural units
	private:
	typedef Misc::Array<SpaceGridCell,3> CellArray; // Type for grid cell array
	typedef CellArray::Index Index; // Type for indices in grid cell arrays
	
	struct BorderCase // Structure determining how to find one ghost cell of a border cell
		{
		/* Elements: */
		public:
		unsigned int caseMask; // Bit mask of border case
		Index indexOffset; // Index offset from cell to ghost cell
		int pointerOffset; // Pointer offset from cell to ghost cell
		Vector positionOffset; // Position offset from cell to ghost cell
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexMarkerDisplayListId; // Display list ID for unlinked vertex marker
		
		/* Constructors and destructors: */
		DataItem(void)
			:vertexMarkerDisplayListId(glGenLists(1))
			{
			};
		~DataItem(void)
			{
			glDeleteLists(vertexMarkerDisplayListId,1);
			};
		};
	
	public:
	struct GridStatistics // Data structure to report statistics about the current simulation state
		{
		/* Elements: */
		public:
		int numUnits;
		int numTriangles;
		int numTetrahedra;
		int numOctahedra;
		int numSpheres;
		int numUnsharedVertices;
		Scalar bondLengthMin,bondLengthMax;
		static const int numBondLengthBins=20;
		int bondLengthHistogram[numBondLengthBins];
		Scalar averageBondLength;
		Scalar bondAngleMin,bondAngleMax;
		static const int numBondAngleBins=18;
		int bondAngleHistogram[numBondAngleBins];
		Scalar averageBondAngle;
		Scalar centerDistMin,centerDistMax;
		static const int numCenterDistBins=30;
		int centerDistHistogram[numCenterDistBins];
		Scalar averageCenterDist;
		Scalar internalBondLengthMin,internalBondLengthMax;
		static const int numInternalBondLengthBins=20;
		int internalBondLengthHistogram[numInternalBondLengthBins];
		Scalar averageInternalBondLength;
		Scalar internalBondAngleMin,internalBondAngleMax;
		static const int numInternalBondAngleBins=18;
		int internalBondAngleHistogram[numInternalBondAngleBins];
		Scalar averageInternalBondAngle;
		};
	
	typedef GLMaterial::Color Color; // Type for color values
	
	/* Elements: */
	private:
	Box gridBox; // Bounding box of entire space grid
	Scalar maxUnitRadius; // Maximum radius of any structural unit contained in space grid
	Index gridSize; // Number of cells in grid in each direction (not including ghost cells)
	Size cellSize; // Size of a single grid cell
	bool periodicFlags[3]; // Flags if the space grid is periodic along any of its axes
	BorderCase borderCases[26]; // Array of associations from a border cell to its ghost cell(s)
	CellArray cells; // 3D array of grid cells
	int cellNeighbourOffsets[27]; // Pointer offsets from a cell to its 27 neighbours (including itself)
	unsigned int nextUnitId; // Next ID number to be assigned to a new unit
	StructuralUnit* firstUnit; // Pointer to first structural unit in this grid
	StructuralUnit* lastUnit; // Pointer to last structural unit in this grid
	StructuralUnit* firstGhostUnit; // Pointer to first ghost unit in this grid
	StructuralUnit* lastGhostUnit; // Pointer to last ghost unit in this grid
	Scalar attenuation; // Attenuation factor for linear and angular velocities
	
	/* Rendering flags: */
	bool showGridBoundary; // Flag for rendering of the grid boundaries
	Color gridBoundaryColor; // Color for grid boundary rendering
	GLfloat gridBoundaryLineWidth; // Line width for grid boundary rendering
	bool showUnits; // Flag for visualization of structural units
	Color defaultUnitColor; // Default color for new structural units
	Color markedUnitColor; // Color to render marked units
	Color lockedUnitColor; // Color to render locked units
	GLMaterial unitMaterial; // Material for rendering structural units
	bool showVelocities; // Flag for visualization of structural unit velocities
	bool showVertexLinks; // Flag for visualization of vertex links
	Color vertexLinkColor; // Color to render vertex links
	GLfloat vertexLinkLineWidth; // Line width to render vertex links
	bool showUnlinkedVertices; // Flag for visualization of unlinked structural unit vertices
	GLfloat unlinkedVertexRadius; // Radius to render unlinked structural unit vertices
	int unlinkedVertexSubdivision; // Subdivision level to render unlinked structural unit vertices
	GLMaterial unlinkedVertexMaterial; // Material to render unlinked structural unit vertices
	
	/* Simulation statistics: */
	int numUnits; // Number of real units
	int numGhostUnits; // Number of ghost units
	int numUnlinkedVertices; // Number of unlinked vertices
	Scalar totalEnergy; // Total internal energy of all units
	
	/* Private methods: */
	void initializeGrid(void); // Initializes the grid cell array
	SpaceGridCell* findCell(const Point& p); // Returns pointer to the grid cell containing the given point
	
	/* Constructors and destructors: */
	public:
	SpaceGrid(const Box& sGridBox,Scalar sMaxUnitRadius,int periodicMask); // Creates a grid of the given size with periodicity along the axes indicated in the bit mask	
	virtual ~SpaceGrid(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	const Box& getBoundingBox(void) const // Returns grid's bounding box
		{
		return gridBox;
		};
	int getPeriodicMask(void) const; // Returns bit mask describing the space grid's periodic boundary conditions
	void setAttenuation(Scalar newAttenuation); // Sets grid's attenuation factor
	
	/* Unit management methods: */
	void addUnit(StructuralUnit* newUnit); // Adds a new structural unit to this grid
	StructuralUnitList getAllUnits(void); // Returns list of all structural units currently in the grid
	StructuralUnit* findUnit(const Point& p); // Returns pointer to closest structural unit intersecting given ray
	StructuralUnit* findUnit(const Ray& r); // Returns pointer to structural unit containing given point
	StructuralUnitList findUnits(const Point& p,Scalar radius); // Returns list of pointers to structural units inside sphere
	StructuralUnitList getLinkedUnits(StructuralUnit* unit); // Returns list of all structural units linked to given unit
	StructuralUnitList findLinkedUnits(const Point& p); // Returns list of pointers to structural units linked to the one containing the given point
	StructuralUnitList findLinkedUnits(const Ray& r); // Returns list of pointers to structural units linked to closest strctural unit intersecting given ray
	StructuralUnit* findClosestUnit(const Point& p,Scalar maxDist); // Finds unit closest to given point
	void markUnit(StructuralUnit* unit); // Marks the given unit
	void unmarkUnit(StructuralUnit* unit); // Unmarks the given unit
	void toggleUnitMark(StructuralUnit* unit); // Toggles the marked state of the given unit
	void toggleUnitsMark(const StructuralUnitList& units); // Toggles marked state of the given list of units
	void lockUnit(StructuralUnit* unit); // Locks the given unit from simulation updates
	void unlockUnit(StructuralUnit* unit); // Unlocks the given unit
	void toggleUnitLock(StructuralUnit* unit); // Toggles the lock state of the given unit
	void toggleUnitsLock(const StructuralUnitList& units); // Toggles lock state of the given list of units
	void moveUnit(StructuralUnit* unit); // Updates a unit's grid linkage to its new position and orientation
	void setUnitPositionOrientation(StructuralUnit* unit,const Point& newPosition,const Rotation& newOrientation); // Sets position and orientation of given structural unit for kinematic control
	void transformUnits(const StructuralUnitList& units,const OrthonormalTransformation& t); // Applies transformation to positions and orientations of all structural units contained in the given list
	void removeUnit(StructuralUnit* unit); // Removes a structural unit from the space grid
	void removeUnits(const StructuralUnitList& units); // Removes all structural units contained in the given list from the space grid
	
	/* Simulation methods: */
	void advanceTime(Scalar timeStep); // Advances simulation time by calculating structural unit interactions, moving units and handling collisions
	GridStatistics calcGridStatistics(void) const; // Returns statistics about the current simulation state
	
	/* Rendering methods: */
	void readRenderingFlags(const Misc::ConfigurationFileSection& configFileSection); // Reads rendering flags from given section of configuration file
	bool getShowGridBoundary(void) const
		{
		return showGridBoundary;
		};
	void setShowGridBoundary(bool newShowGridBoundary);
	const Color& getGridBoundaryColor(void) const
		{
		return gridBoundaryColor;
		};
	void setGridBoundaryColor(const Color& newGridBoundaryColor);
	GLfloat getGridBoundaryLineWidth(void) const
		{
		return gridBoundaryLineWidth;
		};
	void setGridBoundaryLineWidth(GLfloat newGridBoundaryLineWidth);
	bool getShowUnits(void) const
		{
		return showUnits;
		};
	void setShowUnits(bool newShowUnits);
	const GLMaterial& getUnitMaterial(void) const
		{
		return unitMaterial;
		};
	void setUnitMaterial(const GLMaterial& newUnitMaterial);
	bool getShowVelocities(void) const
		{
		return showVelocities;
		};
	void setShowVelocities(bool newShowVelocities);
	bool getShowVertexLinks(void) const
		{
		return showVertexLinks;
		};
	void setShowVertexLinks(bool newShowVertexLinks);
	const Color& getVertexLinkColor(void) const
		{
		return vertexLinkColor;
		};
	void setVertexLinkColor(const Color& newVertexLinkColor);
	GLfloat getVertexLinkLineWidth(void) const
		{
		return vertexLinkLineWidth;
		};
	void setVertexLinkLineWidth(GLfloat newVertexLinkLineWidth);
	bool getShowUnlinkedVertices(void) const
		{
		return showUnlinkedVertices;
		};
	void setShowUnlinkedVertices(bool newShowUnlinkedVertices);
	void glRenderAction(GLContextData& contextData) const; // Renders the space grid and all structural units
	};

}

#endif
