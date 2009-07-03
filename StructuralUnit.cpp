/***********************************************************************
StructuralUnit - Base class for structural units in molecular
arrangements.
***********************************************************************/

#include <assert.h>
#include <Misc/File.h>
#include <Misc/ConfigurationFile.h>
#include <Misc/StandardValueCoders.h>
#include <Math/Math.h>
#include <Geometry/Endianness.h>

#include "GhostUnit.h"

#include "StructuralUnit.h"

namespace NCK {

/***************************************
Static elements of class StructuralUnit:
***************************************/

Scalar StructuralUnit::vertexForceRadius;
Scalar StructuralUnit::vertexForceRadius2;
Scalar StructuralUnit::vertexForceStrength;
Scalar StructuralUnit::centralForceOvershoot;
Scalar StructuralUnit::centralForceStrength;

/*******************************
Methods of class StructuralUnit:
*******************************/

void StructuralUnit::initClass(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Read class settings: */
	vertexForceRadius=configFileSection.retrieveValue<Scalar>("./vertexForceRadius",1.0);
	vertexForceRadius2=Math::sqr(vertexForceRadius);
	vertexForceStrength=configFileSection.retrieveValue<Scalar>("./vertexForceStrength",90.0);
	centralForceOvershoot=configFileSection.retrieveValue<Scalar>("./centralForceOvershoot",1.0/3.0);
	centralForceStrength=configFileSection.retrieveValue<Scalar>("./centralForceStrength",72.0);
	}

StructuralUnit::StructuralUnit(int sNumVertices,Misc::File& file)
	:pred(0),succ(0),cell(0),cellPred(0),cellSucc(0),
	 marked(false),locked(false),
	 numVertices(sNumVertices),vertexLinks(new VertexLink[numVertices])
	{
	/* Read position and orientation: */
	file.read(position);
	Scalar quaternion[4];
	file.read<Scalar>(quaternion,4);
	orientation=Rotation::fromQuaternion(quaternion);
	
	/* Read velocities: */
	file.read(linearVelocity);
	file.read(angularVelocity);
	}

void StructuralUnit::deinitClass(void)
	{
	}

void StructuralUnit::setVertexForceRadius(Scalar newVertexForceRadius)
	{
	vertexForceRadius=newVertexForceRadius;
	vertexForceRadius2=Math::sqr(vertexForceRadius);
	}

void StructuralUnit::setVertexForceStrength(Scalar newVertexForceStrength)
	{
	vertexForceStrength=newVertexForceStrength;
	}

void StructuralUnit::setCentralForceOvershoot(Scalar newCentralForceOvershoot)
	{
	centralForceOvershoot=newCentralForceOvershoot;
	}

void StructuralUnit::setCentralForceStrength(Scalar newCentralForceStrength)
	{
	centralForceStrength=newCentralForceStrength;
	}

Scalar StructuralUnit::calcVertexRadius(Scalar unitRadius)
	{
	/* Calculate vertex offsets based on force parameters: */
	Scalar ph=-Scalar(0.25)*(Scalar(4)*unitRadius-vertexForceRadius);
	Scalar q=-Scalar(0.5)*unitRadius*(vertexForceRadius-Scalar(2)*unitRadius);
	q+=Scalar(0.5)*centralForceStrength*unitRadius*(Math::sqr(vertexForceRadius)*centralForceOvershoot)/(vertexForceStrength*Math::sqr(Scalar(2)*unitRadius+centralForceOvershoot));
	Scalar det2=Math::sqr(ph)-q;
	Scalar det=Math::sqrt(det2);
	Scalar vertexRadius1=-ph-det;
	Scalar vertexRadius2=-ph+det;
	return vertexRadius2;
	}

void StructuralUnit::setColor(const StructuralUnit::Color& newColor)
	{
	color=newColor;
	}

void StructuralUnit::clearVertexLinks(void)
	{
	int numVertices1=this->getNumVertices();
	for(int i1=0;i1<numVertices1;++i1)
		{
		VertexLink& vl1=this->getVertexLink(i1);
		StructuralUnit* other=vl1.unit;
		if(other!=0)
			{
			/* Remove link: */
			VertexLink& vl2=other->getVertexLink(vl1.vertexIndex);
			vl1.unit=0;
			vl1.vertexIndex=-1;
			vl2.unit=0;
			vl2.vertexIndex=-1;
			}
		}
	}

void StructuralUnit::checkVertexLinks(Scalar timeStep)
	{
	//assert(dynamic_cast<GhostUnit*>(this)==0);
	
	int numVertices1=this->getNumVertices();
	for(int i1=0;i1<numVertices1;++i1)
		{
		VertexLink& vl1=this->getVertexLink(i1);
		if(vl1.unit!=0&&this->id>vl1.unit->id)
			{
			//VertexLink& vl2=vl1.unit->getVertexLink(vl1.vertexIndex);
			//assert(vl2.unit==this&&vl2.vertexIndex==i1);
			
			/* Check if vertex link is still valid: */
			Point p1=this->getVertex(i1);
			Point p2=vl1.unit->getVertex(vl1.vertexIndex);
			Vector dist=p2-p1;
			Scalar distLen2=Geometry::sqr(dist);
			if(distLen2>vertexForceRadius2)
				{
				/* Remove link: */
				VertexLink& vl2=vl1.unit->getVertexLink(vl1.vertexIndex);
				vl1.unit=0;
				vl1.vertexIndex=-1;
				vl2.unit=0;
				vl2.vertexIndex=-1;
				}
			else
				{
				/* Calculate vertex attracting force: */
				Scalar distLen=Math::sqrt(distLen2);
				Vector force=dist*(vertexForceStrength*(vertexForceRadius-distLen)/vertexForceRadius2);

				/* Apply forces to units: */
				this->applyVertexForce(i1,force,timeStep);
				vl1.unit->applyVertexForce(vl1.vertexIndex,force,-timeStep);
				}
			}
		}
	}

void StructuralUnit::establishVertexLinks(StructuralUnit* unit1,StructuralUnit* unit2)
	{
	/* Check if the units are close enough to link vertices: */
	Scalar distLen2=Geometry::sqrDist(unit1->position,unit2->position);
	if(distLen2>=Math::sqr(unit1->getRadius()+unit2->getRadius()+vertexForceRadius))
		return;
	
	/* Check if there is an existing vertex link between the units: */
	int i1,i2;
	int numVertices1=unit1->getNumVertices();
	int numVertices2=unit2->getNumVertices();
	for(i1=0;i1<numVertices1;++i1)
		{
		VertexLink& vl1=unit1->getVertexLink(i1);
		if(vl1.unit==unit2)
			{
			i2=vl1.vertexIndex;
			break;
			}
		}
	
	if(i1>=numVertices1)
		{
		/* Try establishing a vertex link between the two units: */
		for(i1=0;i1<numVertices1;++i1)
			{
			VertexLink& vl1=unit1->getVertexLink(i1);
			if(vl1.unit==0)
				{
				Point p1=unit1->getVertex(i1);
				for(i2=0;i2<numVertices2;++i2)
					{
					VertexLink& vl2=unit2->getVertexLink(i2);
					if(vl2.unit==0)
						{
						Point p2=unit2->getVertex(i2);
						Vector dist=p2-p1;
						Scalar distLen2=Geometry::sqr(dist);
						if(distLen2<=vertexForceRadius2)
							{
							/* Link the two vertices: */
							vl1.unit=unit2;
							vl1.vertexIndex=i2;
							vl2.unit=unit1;
							vl2.vertexIndex=i1;
							
							/* There can be only one vertex link between two units; stop looking: */
							goto foundVertexPair;
							}
						}
					}
				}
			}
		foundVertexPair:
		; // Dummy statement to make compiler happy
		}
	}

void StructuralUnit::interact(StructuralUnit* unit1,StructuralUnit* unit2,Scalar timeStep)
	{
	//assert(dynamic_cast<GhostUnit*>(unit1)==0);
	
	/* Calculate interaction between unit's centroids: */
	Scalar radius1=unit1->getRadius();
	Scalar radius2=unit2->getRadius();
	Scalar centralForceRadius=radius1+radius2+centralForceOvershoot;
	Scalar centralForceRadius2=Math::sqr(centralForceRadius);
	Vector dist=unit2->position-unit1->position;
	Scalar distLen2=Geometry::sqr(dist);
	if(distLen2<centralForceRadius2)
		{
		/* Calculate centroid repelling force: */
		Scalar distLen=Math::sqrt(distLen2);
		Vector force=dist*(centralForceStrength*(distLen-centralForceRadius)/centralForceRadius2);

		/* Apply forces to units: */
		unit1->applyCentralForce(force,timeStep);
		unit2->applyCentralForce(force,-timeStep);
		}
	
	/* Check for potential new vertex links if units are close enough: */
	if(distLen2<Math::sqr(radius1+radius2+vertexForceRadius))
		{
		/* Check if there is an existing vertex link between the units: */
		int i1;
		int numVertices1=unit1->getNumVertices();
		for(i1=0;i1<numVertices1&&unit1->getVertexLink(i1).unit!=unit2;++i1)
			;
		
		if(i1>=numVertices1)
			{
			/* Try establishing a vertex link between the two units: */
			int i2;
			int numVertices2=unit2->getNumVertices();
			for(i1=0;i1<numVertices1;++i1)
				{
				VertexLink& vl1=unit1->getVertexLink(i1);
				if(vl1.unit==0)
					{
					Point p1=unit1->getVertex(i1);
					for(i2=0;i2<numVertices2;++i2)
						{
						VertexLink& vl2=unit2->getVertexLink(i2);
						if(vl2.unit==0)
							{
							Point p2=unit2->getVertex(i2);
							Vector dist=p2-p1;
							Scalar distLen2=Geometry::sqr(dist);
							if(distLen2<=vertexForceRadius2)
								{
								/* Link the two vertices: */
								vl1.unit=unit2;
								vl1.vertexIndex=i2;
								vl2.unit=unit1;
								vl2.vertexIndex=i1;

								/* Calculate vertex attracting force: */
								Scalar distLen=Math::sqrt(distLen2);
								Vector force=dist*(vertexForceStrength*(vertexForceRadius-distLen)/vertexForceRadius2);

								/* Apply forces to units: */
								unit1->applyVertexForce(i1,force,timeStep);
								unit2->applyVertexForce(i2,force,-timeStep);

								/* There can be only one vertex link between two units; stop looking: */
								goto foundVertexPair;
								}
							}
						}
					}
				}
			foundVertexPair:
			; // Dummy statement to make compiler happy
			}
		}
	}

void StructuralUnit::readVertexLinks(Misc::File& file,const StructuralUnit::IndexUnitMap& indexUnitMap)
	{
	/* Read all vertex links: */
	int numVertexLinks=file.read<int>();
	for(int i=0;i<numVertexLinks;++i)
		{
		/* Read vertex link from file: */
		FileVertexLink fvl(file);
		
		/* Get pointer to other unit: */
		StructuralUnit* other=indexUnitMap.getEntry(fvl.otherUnitIndex).getDest();
		
		/* Set vertex links: */
		this->getVertexLink(fvl.thisVertexIndex).unit=other;
		this->getVertexLink(fvl.thisVertexIndex).vertexIndex=fvl.otherVertexIndex;
		other->getVertexLink(fvl.otherVertexIndex).unit=this;
		other->getVertexLink(fvl.otherVertexIndex).vertexIndex=fvl.thisVertexIndex;
		}
	}

void StructuralUnit::writeState(Misc::File& file) const
	{
	/* Write position and orientation: */
	file.write(position);
	file.write<Scalar>(orientation.getQuaternion(),4);
	
	/* Write velocities: */
	file.write(linearVelocity);
	file.write(angularVelocity);
	}

void StructuralUnit::writeVertexLinks(Misc::File& file,const StructuralUnit::UnitIndexMap& unitIndexMap) const
	{
	/* Count number of valid vertex links: */
	int numVertices=this->getNumVertices();
	int numVertexLinks=0;
	for(int i=0;i<numVertices;++i)
		if(this->getVertexLink(i).unit!=0&&unitIndexMap.isEntry(this->getVertexLink(i).unit))
			++numVertexLinks;
	file.write<int>(numVertexLinks);
	
	/* Write valid vertex links to file: */
	for(int i=0;i<numVertices;++i)
		if(this->getVertexLink(i).unit!=0&&unitIndexMap.isEntry(this->getVertexLink(i).unit))
			{
			/* Write vertex link structure: */
			FileVertexLink fvl(i,unitIndexMap.getEntry(this->getVertexLink(i).unit).getDest(),this->getVertexLink(i).vertexIndex);
			fvl.write(file);
			}
	}

}
