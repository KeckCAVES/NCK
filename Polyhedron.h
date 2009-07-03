/***********************************************************************
Polyhedron - Class for interstitial void polyhedra.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#ifndef POLYHEDRON_INCLUDED
#define POLYHEDRON_INCLUDED

#include <list>

#include "AffineSpace.h"

/* Forward declarations: */
class GLContextData;
namespace NCK {
class SpaceGrid;
}

namespace NCK {

class Polyhedron
	{
	/* Embedded classes: */
	private:
	struct Edge;
	struct Face;
	
	struct Vertex // Class for polyhedron vertices
		{
		/* Elements: */
		public:
		Point pos; // Vertex' position
		Edge* edge; // Pointer to one half-edge starting at the vertex
		
		/* Constructors and destructors: */
		Vertex(const Point& sPos) // Creates a vertex at the given position
			:pos(sPos),
			 edge(0)
			{
			};
		};
	
	struct Edge // Class for half-edges
		{
		/* Elements: */
		public:
		Vertex* start; // Pointer to start vertex of half-edge
		Face* face; // Pointer to face this half-edge belongs to
		bool external; // Flag if edge is boundary between two polyhedron faces
		Edge* faceSucc; // Pointer to next half-edge around same face
		Edge* opposite; // Pointer to other half of same edge
		};
	
	struct Face // Class for polyhedron faces
		{
		/* Elements: */
		public:
		Vector normal; // Normal vector for the face
		Edge* edge; // Pointer to one half-edge bounding the face
		};
	
	typedef std::list<Vertex> VertexList;
	typedef std::list<Edge> EdgeList;
	typedef std::list<Face> FaceList;
	
	struct TetVertices // Helper structure to associate polyhedron vertices with tetrahedra
		{
		/* Elements: */
		public:
		Vertex* vertices[4]; // Array of four pointers to vertices
		Edge* incomingEdges[4]; // Array of pointers to an incoming edge for each vertex
		Edge* outgoingEdges[4]; // Array of pointers to an outgoing edge for each vertex
		};
	
	#if 0
	struct TetPair // Helper structure to use pairs of tetrahedra as hash sources
		{
		/* Elements: */
		public:
		Tetrahedron* tets[2]; // Pointer to two tetrahedra, sorted by numerical value
		
		/* Constructors and destructors: */
		TetPair(const Tetrahedron* sTet1,const Tetrahedron* sTet2)
			{
			unsigned int sTetVal1=reinterpret_cast<unsigned int>(sTet1);
			unsigned int sTetVal2=reinterpret_cast<unsigned int>(sTet2);
			if(sTetVal1<sTetVal2)
				{
				tets[0]=sTet1;
				tets[1]=sTet2;
				}
			else
				{
				tets[0]=sTet2;
				tets[1]=sTet1;
				}
			};
		
		/* Methods: */
		friend bool operator==(const TetPair& tp1,const TetPair& tp2)
			{
			return tp1.tets[0]==tp2.tets[0]&&tp1.tets[1]==tp2.tets[1];
			};
		friend bool operator!=(const TetPair& tp1,const TetPair& tp2)
			{
			return tp1.tets[0]!=tp2.tets[0]||tp1.tets[1]!=tp2.tets[1];
			};
		friend unsigned int hash(const TetPair& tp,unsigned int tableSize)
			{
			unsigned int result=0;
			result+=reinterpret_cast<unsigned int>(tp.tets[0])*17;
			result+=reinterpret_cast<unsigned int>(tp.tets[1])*31;
			return result%tableSize;
			};
		};
	#endif
	
	/* Elements: */
	VertexList vertices; // Vector of vertices
	EdgeList edges; // Vector of edges
	FaceList faces; // Vector of faces
	
	/* Private methods: */
	Vertex* createVertex(const Point& newPos); // Creates a new vertex
	Edge* createEdge(void); // Creates a new edge
	Face* createFace(void); // Creates a new face
	
	/* Constructors and destructors: */
	public:
	Polyhedron(void); // Creates empty polyhedron
	Polyhedron(SpaceGrid& grid,const Point& queryPosition); // Creates an interstitial void polyhedron
	private:
	Polyhedron(const Polyhedron& source); // Prohibit copy constructor
	Polyhedron& operator=(const Polyhedron& source); // Prohibit assignment operator
	public:
	~Polyhedron(void);
	
	/* Methods: */
	void glRenderAction(GLContextData& contextData) const; // Renders the polyhedron
	};

}

#endif
