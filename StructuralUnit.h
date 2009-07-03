/***********************************************************************
StructuralUnit - Base class for structural units in molecular
arrangements.
***********************************************************************/

#ifndef STRUCTURALUNIT_INCLUDED
#define STRUCTURALUNIT_INCLUDED

#include <Misc/File.h>
#include <Misc/HashTable.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLObject.h>

#include "AffineSpace.h"

/* Forward declarations: */
namespace Misc {
class File;
class ConfigurationFileSection;
}
namespace NCK {
class SpaceGridCell;
class SpaceGrid;
}

namespace NCK {

class UnitRenderer:public GLObject // Base class to help rendering structural units of derived types
	{
	/* Embedded classes: */
	public:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint displayListId; // ID of a display list caching a unit's model
		
		/* Constructors and destructors: */
		DataItem(void)
			{
			/* Generate a display list: */
			displayListId=glGenLists(1);
			};
		virtual ~DataItem(void)
			{
			/* Destroy the display list: */
			glDeleteLists(displayListId,1);
			};
		};
	};

class StructuralUnit
	{
	friend class SpaceGridCell;
	friend class SpaceGrid;
	
	/* Embedded classes: */
	public:
	typedef Misc::HashTable<StructuralUnit*,unsigned int> UnitIndexMap; // Data type for maps from unit pointers to unit indices
	typedef Misc::HashTable<unsigned int,StructuralUnit*> IndexUnitMap; // Data type for maps from unit indices to unit pointers
	typedef GLColor<GLfloat,3> Color; // Data type for unit colors
	
	struct VertexLink // Structure to represent vertex-vertex links between two structural units
		{
		/* Elements: */
		public:
		StructuralUnit* unit; // Pointer to other structural unit
		int vertexIndex; // Pointer to index of linked vertex in other structural unit
		
		/* Constructors and destructors: */
		VertexLink(void)
			:unit(0),vertexIndex(-1)
			{
			};
		};
	
	private:
	struct FileVertexLink // Structure to represent vertex links in a binary file
		{
		/* Elements: */
		public:
		int thisVertexIndex; // Index of vertex link in this structural unit
		unsigned int otherUnitIndex; // File index of other structural unit
		int otherVertexIndex; // Index of vertex link in other structural unit
		
		/* Constructors and destructors: */
		FileVertexLink(int sThisVertexIndex,unsigned int sOtherUnitIndex,int sOtherVertexIndex)
			:thisVertexIndex(sThisVertexIndex),otherUnitIndex(sOtherUnitIndex),otherVertexIndex(sOtherVertexIndex)
			{
			};
		FileVertexLink(Misc::File& file)
			{
			file.read(thisVertexIndex);
			file.read(otherUnitIndex);
			file.read(otherVertexIndex);
			};
		
		/* Methods: */
		void write(Misc::File& file) const
			{
			file.write(thisVertexIndex);
			file.write(otherUnitIndex);
			file.write(otherVertexIndex);
			};
		};
	
	/* Elements: */
	protected:
	static Scalar vertexForceRadius,vertexForceRadius2; // Radius and squared radius of vertex force field
	static Scalar vertexForceStrength; // Strength of vertex attraction force
	static Scalar centralForceOvershoot; // Factor of how much centroid repelling force overshoots units' radii
	static Scalar centralForceStrength; // Strength of centroid repelling force
	
	private:
	unsigned int id; // ID number unique inside space grid containing unit
	StructuralUnit* pred; // Pointer to previous unit in space grid's list
	StructuralUnit* succ; // Pointer to next unit in space grid's list
	SpaceGridCell* cell; // Pointer to grid cell containing this unit
	StructuralUnit* cellPred; // Pointer to previous unit in same grid cell
	StructuralUnit* cellSucc; // Pointer to next unit in same grid cell
	
	protected:
	bool marked; // Flag if the unit has been marked for subsequent operations
	bool locked; // Flag if the unit's position and orientation are locked
	Point position; // Unit's current position
	Rotation orientation; // Unit's current orientation
	Vector linearVelocity,angularVelocity; // Unit's current linear and angular velocities
	int numVertices; // Number of vertices of this unit
	VertexLink* vertexLinks; // Array of vertex links of this unit
	Color color; // Rendering color for the unit
	
	/* Constructors and destructors: */
	public:
	static void initClass(const Misc::ConfigurationFileSection& configFileSection); // Initializes class settings based on the given configuration file section
	StructuralUnit(const Point& sPosition,const Rotation& sOrientation,int sNumVertices) // Places structural unit at given position and orientation
		:pred(0),succ(0),cell(0),cellPred(0),cellSucc(0),
		 marked(false),locked(false),
		 position(sPosition),orientation(sOrientation),
		 linearVelocity(Vector::zero),angularVelocity(Vector::zero),
		 numVertices(sNumVertices),vertexLinks(new VertexLink[numVertices])
		{
		};
	StructuralUnit(int sNumVertices,Misc::File& file); // Creates structural unit by reading a binary file
	virtual ~StructuralUnit(void)
		{
		delete[] vertexLinks;
		};
	static void deinitClass(void); // Destroys class data
	
	/* Methods: */
	static void setVertexForceRadius(Scalar newVertexForceRadius); // Sets radius of vertex force field
	static void setVertexForceStrength(Scalar newVertexForceStrength); // Sets strength of vertex attraction force
	static void setCentralForceOvershoot(Scalar newCentralForceOvershoot); // Sets amount of overshoot for central repelling force
	static void setCentralForceStrength(Scalar newCentralForceStrength); // Sets strength of centroid repelling force
	static Scalar calcVertexRadius(Scalar unitRadius); // Calculates a unit's vertex offset radius based on unit's intended radius
	int getId(void) const // Returns a unit's ID
		{
		return id;
		};
	bool getMarked(void) const // Returns unit's marked flag
		{
		return marked;
		};
	bool getLocked(void) const // Returns unit's locked flag
		{
		return locked;
		};
	const Point& getPosition(void) const // Returns unit's current position
		{
		return position;
		};
	const Rotation& getOrientation(void) const // Returns unit's current orientation
		{
		return orientation;
		};
	const Vector& getLinearVelocity(void) const // Returns unit's current linear velocity
		{
		return linearVelocity;
		};
	const Vector& getAngularVelocity(void) const // Returns unit's current angular velocity
		{
		return angularVelocity;
		};
	const Color& getColor(void) const // Returns unit's color
		{
		return color;
		};
	void setMarked(bool newMarked) // Sets unit's marked flag
		{
		marked=newMarked;
		};
	void setLocked(bool newLocked) // Sets unit's locked flag
		{
		locked=newLocked;
		};
	void setPositionOrientation(const Point& newPosition,const Rotation& newOrientation) // Overrides position and orientation for kinematic control
		{
		position=newPosition;
		orientation=newOrientation;
		};
	void setVelocities(const Vector& newLinearVelocity,const Vector& newAngularVelocity) // Overrides velocities for kinematic control
		{
		linearVelocity=newLinearVelocity;
		angularVelocity=newAngularVelocity;
		};
	int getNumVertices(void) const // Returns unit's number of linkable vertices
		{
		return numVertices;
		};
	const VertexLink& getVertexLink(int index) const // Returns pointer to one vertex link structure
		{
		return vertexLinks[index];
		};
	VertexLink& getVertexLink(int index) // Ditto
		{
		return vertexLinks[index];
		};
	void setColor(const Color& newColor); // Sets the unit's rendering color
	virtual Scalar getRadius(void) const =0; // Returns radius of unit's circumsphere
	virtual Scalar getMass(void) const =0; // Returns unit's mass
	virtual Vector getVertexOffset(int index) const =0; // Returns offset vector of one vertex in global coordinates
	virtual Point getVertex(int index) const =0; // Returns position of one vertex in global coordinates
	virtual void applyVertexForce(int index,const Vector& force,Scalar timeStep) =0; // Applies a force to one of the unit's vertices
	virtual void applyCentralForce(const Vector& force,Scalar timeStep) =0; // Applies a force to the unit's centroid
	void clearVertexLinks(void); // Removes all vertex links from a unit
	void checkVertexLinks(Scalar timeStep); // Checks all existing vertex links and applies vertex forces for valid ones
	static void establishVertexLinks(StructuralUnit* unit1,StructuralUnit* unit2); // Tries to establish a vertex link between the given units
	static void interact(StructuralUnit* unit1,StructuralUnit* unit2,Scalar timeStep); // Calculates centroid interaction and interaction of newly established vertex links between the given units
	virtual void glRenderAction(GLContextData& contextData) const =0; // Renders the unit
	void readVertexLinks(Misc::File& file,const IndexUnitMap& indexUnitMap); // Reads a unit's vertex links from a binary file
	virtual void writeState(Misc::File& file) const; // Writes unit's current state to a binary file
	void writeVertexLinks(Misc::File& file,const UnitIndexMap& unitIndexMap) const; // Writes a unit's vertex links to a binary file
	};

}

#endif
