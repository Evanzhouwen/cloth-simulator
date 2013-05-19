
#include "spring.h"
#include "particle.h"

Spring::Spring(Particle& p1, Particle& p2) :
    P1(p1),
    P2(p2),
    RestDistance((p1.GetPosition()-p2.GetPosition()).Length())
{
}

void Spring::SolveSpring()
{
    //current vector from p1 to p2
    FLOAT3 currentVector(P2.GetPosition() - P1.GetPosition());

    //current distance using vector
    float currentDistance = currentVector.Length();

    //'error' vector between p1 and p2 (need to minimise this)
    FLOAT3 errorVector(currentVector-((currentVector/currentDistance)*RestDistance)); 

    //half of the error vector
    FLOAT3 errorVectorHalf(errorVector*0.5);
    
    //move the particles back into place
    P1.MovePosition(errorVectorHalf);
    P2.MovePosition(-errorVectorHalf);
}
