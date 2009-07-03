/***********************************************************************
Polyhedron - Class for interstitial void polyhedra.
Copyright (c) 2004 Oliver Kreylos
***********************************************************************/

#include <assert.h>
#include <Misc/HashTable.h>
#include <Misc/OneTimeQueue.h>
#include <Math/Constants.h>
#include <Geometry/AffineCombiner.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLGeometryWrappers.h>

#include "StructuralUnit.h"
#include "Tetrahedron.h"
#include "SpaceGrid.h"

#include "Polyhedron.h"

namespace NCK {

/***************************
Methods of class Polyhedron:
***************************/

Polyhedron::Vertex* Polyhedron::createVertex(const Point& newPos)
	{
	return &*vertices.insert(vertices.end(),Vertex(newPos));
	}

Polyhedron::Edge* Polyhedron::createEdge(void)
	{
	return &*edges.insert(edges.end(),Edge());
	}

Polyhedron::Face* Polyhedron::createFace(void)
	{
	return &*faces.insert(faces.end(),Face());
	}

Polyhedron::Polyhedron(void)
	{
	}

Polyhedron::Polyhedron(SpaceGrid& grid,const Point& queryPosition)
	{
	typedef Misc::HashTable<Tetrahedron*,TetVertices> TetVertexHash; // Hash table to associate polyhedron vertices with tetrahedra
	
	/* Create hash table to map tetrahedra to polyhedron vertices: */
	TetVertexHash vertices(101);
	
	typedef Misc::OneTimeQueue<Tetrahedron*> TetQueue;
	
	/* Create traversal queue for linked tetrahedra: */
	TetQueue tetQueue(101);
	
	/* Start traversal by queueing closest tetrahedron to query position: */
	Tetrahedron* firstTet=dynamic_cast<Tetrahedron*>(grid.findClosestUnit(queryPosition,Math::Constants<Scalar>::max));
	if(firstTet!=0)
		tetQueue.push(firstTet);
	
	/* Traverse all convexly connected tetrahedra: */
	while(!tetQueue.empty())
		{
		/* Get the next tetrahedron from the queue: */
		Tetrahedron* tet=tetQueue.front();
		tetQueue.pop();
		
		/* Calculate the positions of the tet's vertices: */
		Point tetPos[4];
		Point::AffineCombiner tetCentroidCombiner;
		bool mustAddVertex[4];
		TetVertices tv;
		for(int i=0;i<4;++i)
			{
			tv.vertices[i]=0;
			tv.incomingEdges[i]=0;
			tv.outgoingEdges[i]=0;
			}
		for(int i=0;i<4;++i)
			{
			StructuralUnit::VertexLink& vl=tet->getVertexLink(i);
			Tetrahedron* otherTet=dynamic_cast<Tetrahedron*>(vl.unit);
			if(otherTet!=0)
				{
				tetPos[i]=Geometry::mid(tet->getVertex(i),otherTet->getVertex(vl.vertexIndex));
				TetVertexHash::Iterator tvIt=vertices.findEntry(otherTet);
				mustAddVertex[i]=tvIt.isFinished()||tvIt->getDest().vertices[vl.vertexIndex]==0;
				if(!mustAddVertex[i])
					{
					/* Store a pointer to the existing vertex for later: */
					tv.vertices[i]=tvIt->getDest().vertices[vl.vertexIndex];
					tv.incomingEdges[i]=tvIt->getDest().incomingEdges[vl.vertexIndex];
					tv.outgoingEdges[i]=tvIt->getDest().outgoingEdges[vl.vertexIndex];
					}
				}
			else
				{
				tetPos[i]=tet->getVertex(i);
				mustAddVertex[i]=true;
				}
			tetCentroidCombiner.addPoint(tetPos[i]);
			}
		Point tetCentroid=tetCentroidCombiner.getPoint();
		
		/* Determine which vertices are facing the query point: */
		int numFacingVertices=0;
		Vertex* facingVertices[4];
		for(int i=0;i<4;++i)
			{
			#if 1
			/* Calculate the tet face's plane equation: */
			const Point& fp=tetPos[(i+1)%4];
			Vector f1=tetPos[(i+2)%4]-fp;
			Vector f2=tetPos[(i+3)%4]-fp;
			Vector fn=(i&1)?Geometry::cross(f1,f2):Geometry::cross(f2,f1);
			Scalar fd=fn*fp;
			
			/* Consider the vertex if the query position is on the same side of the face plane as the vertex: */
			if(fn*queryPosition>fd)
			#else
			if((queryPosition-tetCentroid)*(tetPos[i]-tetCentroid)>Scalar(0))
			#endif
				{
				/* Add the vertex to the polyhedron: */
				if(mustAddVertex[i])
					tv.vertices[i]=createVertex(tetPos[i]);
				
				/* Store the vertex for creating edges and faces later: */
				facingVertices[numFacingVertices]=tv.vertices[i];
				++numFacingVertices;
				
				/* Add the neighbouring tetrahedron to the queue if it has not been added yet: */
				Tetrahedron* neighbour=dynamic_cast<Tetrahedron*>(tet->getVertexLink(i).unit);
				if(neighbour!=0)
					tetQueue.push(neighbour);
				}
			}
		
		Edge* externalEdges[3];
		if(numFacingVertices==2)
			{
			/* Create external edges: */
			for(int i=0;i<2;++i)
				externalEdges[i]=createEdge();
			for(int i=0;i<2;++i)
				{
				externalEdges[i]->start=facingVertices[i];
				externalEdges[i]->face=0;
				externalEdges[i]->external=true;
				externalEdges[i]->faceSucc=0;
				externalEdges[i]->opposite=externalEdges[(i+1)%2];
				}
			}
		else if(numFacingVertices==3)
			{
			/* Create an interior triangular face: */
			Face* f=createFace();
			Edge* es[3];
			for(int i=0;i<3;++i)
				es[i]=createEdge();
			f->normal=Geometry::cross(facingVertices[1]->pos-facingVertices[0]->pos,facingVertices[2]->pos-facingVertices[0]->pos);
			Scalar fd=f->normal*facingVertices[0]->pos;
			f->edge=es[0];
			if(f->normal*queryPosition<fd)
				{
				for(int i=0;i<3;++i)
					es[i]->start=facingVertices[i];
				}
			else
				{
				f->normal=-f->normal;
				for(int i=0;i<3;++i)
					es[i]->start=facingVertices[2-i];
				}
			for(int i=0;i<3;++i)
				{
				es[i]->face=f;
				es[i]->external=true;
				es[i]->faceSucc=es[(i+1)%3];
				}
			
			/* Create external edges: */
			for(int i=0;i<3;++i)
				externalEdges[i]=createEdge();
			for(int i=0;i<3;++i)
				{
				externalEdges[i]->start=es[i]->faceSucc->start;
				externalEdges[i]->face=0;
				externalEdges[i]->external=true;
				externalEdges[i]->faceSucc=0;
				externalEdges[i]->opposite=es[i];
				es[i]->opposite=externalEdges[i];
				}
			}
		
		/* Associate the external edges with their vertices: */
		Edge* out[4];
		Edge* in[4];
		for(int i=0;i<4;++i)
			{
			out[4]=0;
			in[4]=0;
			}
		for(int i=0;i<numFacingVertices;++i)
			{
			/* Find the tet's vertex indices of the edge's start and end points: */
			int si=-1,ei=-1;
			for(int j=0;j<4;++j)
				{
				if(externalEdges[i]->start==tv.vertices[j])
					si=j;
				if(externalEdges[i]->opposite->start==tv.vertices[j])
					ei=j;
				}
			assert(si>=0&&ei>=0);
			
			/* Connect the external edge with existing edges: */
			if(tv.incomingEdges[si]!=0)
				{
				/* Link the two edges: */
				tv.incomingEdges[si]->faceSucc=externalEdges[i];
				}
			if(tv.outgoingEdges[ei]!=0)
				{
				/* Link the two edges: */
				externalEdges[i]->faceSucc=tv.outgoingEdges[ei];
				}
			
			#if 1
			/* Check if the edge closes a face loop: */
			Edge* ePtr=externalEdges[i];
			do
				{
				ePtr=ePtr->faceSucc;
				}
			while(ePtr!=0&&ePtr!=externalEdges[i]);
			if(ePtr==externalEdges[i])
				{
				#if 0
				/* Create a new face and close the face loop: */
				Face* face=createFace();
				face->edge=ePtr;
				do
					{
					ePtr->face=face;
					ePtr=ePtr->faceSucc;
					}
				while(ePtr!=externalEdges[i]);
				#else
				/* Calculate the face's centroid: */
				Point::AffineCombiner ac;
				do
					{
					ac.addPoint(ePtr->start->pos);
					ePtr=ePtr->faceSucc;
					}
				while(ePtr!=externalEdges[i]);
				
				/* Triangulate the face: */
				Vertex* centroid=createVertex(ac.getPoint());
				Edge* firstRadialEdge=0;
				Edge* lastRadialEdge=0;
				do
					{
					Edge* nextEptr=ePtr->faceSucc;
					
					/* Triangulate the face: */
					Face* tri=createFace();
					tri->edge=ePtr;
					Edge* es[3];
					es[0]=ePtr;
					for(int i=1;i<3;++i)
						es[i]=createEdge();
					es[1]->start=nextEptr->start;
					es[1]->external=false;
					es[2]->start=centroid;
					es[2]->external=false;
					for(int i=0;i<3;++i)
						{
						es[i]->face=tri;
						es[i]->faceSucc=es[(i+1)%3];
						}
					tri->normal=Geometry::cross(es[1]->start->pos-es[0]->start->pos,es[2]->start->pos-es[0]->start->pos);
					
					/* Connect the face to the other faces: */
					es[1]->opposite=0;
					es[2]->opposite=lastRadialEdge;
					if(lastRadialEdge!=0)
						lastRadialEdge->opposite=es[2];
					else
						firstRadialEdge=es[2];
					lastRadialEdge=es[1];
					
					ePtr=nextEptr;
					}
				while(ePtr!=externalEdges[i]);
				
				/* Close the triangle fan: */
				lastRadialEdge->opposite=firstRadialEdge;
				firstRadialEdge->opposite=lastRadialEdge;
				#endif
				}
			#endif
			
			/* Store the new edge: */
			out[si]=externalEdges[i];
			in[ei]=externalEdges[i];
			}
		
		/* Store the polyhedron vertex association for this tet: */
		for(int i=0;i<4;++i)
			{
			tv.incomingEdges[i]=in[i];
			tv.outgoingEdges[i]=out[i];
			}
		vertices.setEntry(TetVertexHash::Entry(tet,tv));
		}
	}

Polyhedron::~Polyhedron(void)
	{
	}

void Polyhedron::glRenderAction(GLContextData& contextData) const
	{
	glPushAttrib(GL_LIGHTING_BIT|GL_LINE_BIT|GL_POINT_BIT|GL_POLYGON_BIT);
	
	/* Render all vertices: */
	glDisable(GL_LIGHTING);
	glPointSize(5.0f);
	glColor3f(1.0f,0.0f,0.0f);
	glBegin(GL_POINTS);
	for(VertexList::const_iterator vIt=vertices.begin();vIt!=vertices.end();++vIt)
		glVertex(vIt->pos);
	glEnd();
	
	/* Render all edges: */
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	for(EdgeList::const_iterator eIt=edges.begin();eIt!=edges.end();++eIt)
		{
		if(eIt->external&&eIt->opposite!=0)
			{
			glVertex(eIt->start->pos);
			glVertex(eIt->opposite->start->pos);
			}
		}
	glEnd();
	
	/* Render all faces: */
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	glMaterial(GLMaterialEnums::FRONT,GLMaterial(GLMaterial::Color(0.9f,0.7f,0.7f),GLMaterial::Color(0.7f,1.7f,0.7f),25.0f));
	glMaterial(GLMaterialEnums::BACK,GLMaterial(GLMaterial::Color(0.7f,0.7f,0.9f),GLMaterial::Color(0.7f,1.7f,0.7f),25.0f));
	glDisable(GL_CULL_FACE);
	for(FaceList::const_iterator fIt=faces.begin();fIt!=faces.end();++fIt)
		{
		glBegin(GL_POLYGON);
		glNormal(fIt->normal);
		const Edge* ePtr=fIt->edge;
		do
			{
			glVertex(ePtr->start->pos);
			ePtr=ePtr->faceSucc;
			}
		while(ePtr!=fIt->edge);
		glEnd();
		}
	
	glPopAttrib();
	}

}
