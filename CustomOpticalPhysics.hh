#ifndef CustomOpticalPhysics_h
#define CustomOpticalPhysics_h 1

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

class CustomOpticalPhysics : public G4VPhysicsConstructor
{
public:
    CustomOpticalPhysics(G4int verbose = 0, const G4String& name = "Optical");
    virtual ~CustomOpticalPhysics();

    virtual void ConstructParticle();
    virtual void ConstructProcess();

private:
    G4int fVerboseLevel;
};

#endif