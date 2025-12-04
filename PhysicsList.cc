#include "CustomOpticalPhysics.hh"

#include "G4OpticalPhoton.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"

// Optical processes
#include "G4Scintillation.hh"
#include "G4OpAbsorption.hh"
#include "G4OpRayleigh.hh"
#include "G4OpBoundaryProcess.hh"

#include "G4LossTableManager.hh"
#include "G4EmSaturation.hh"

CustomOpticalPhysics::CustomOpticalPhysics(G4int verbose, const G4String& name)
    : G4VPhysicsConstructor(name), fVerboseLevel(verbose)
{
    if (fVerboseLevel > 0) {
        G4cout << "CustomOpticalPhysics: Optical Physics Constructor" << G4endl;
    }
}

CustomOpticalPhysics::~CustomOpticalPhysics()
{
}

void CustomOpticalPhysics::ConstructParticle()
{
    // Optical photon
    G4OpticalPhoton::OpticalPhotonDefinition();
    
    if (fVerboseLevel > 0) {
        G4cout << "CustomOpticalPhysics: Optical photon defined" << G4endl;
    }
}

void CustomOpticalPhysics::ConstructProcess()
{
    if (fVerboseLevel > 0) {
        G4cout << "CustomOpticalPhysics: Constructing optical processes" << G4endl;
    }

    // Create optical processes
    G4Scintillation* theScintProcess = new G4Scintillation();
    theScintProcess->SetScintillationYieldFactor(1.0);
    theScintProcess->SetTrackSecondariesFirst(true);
    
    G4OpAbsorption* theAbsorptionProcess = new G4OpAbsorption();
    G4OpRayleigh* theRayleighScatteringProcess = new G4OpRayleigh();
    
    // CRITICAL: Boundary process
    G4OpBoundaryProcess* theBoundaryProcess = new G4OpBoundaryProcess();
    
    if (fVerboseLevel > 0) {
        G4cout << "CustomOpticalPhysics: Created all optical processes including OpBoundary" << G4endl;
    }

    // Add processes to optical photon
    auto particleIterator = GetParticleIterator();
    particleIterator->reset();
    
    while ((*particleIterator)()) {
        G4ParticleDefinition* particle = particleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();
        G4String particleName = particle->GetParticleName();
        
        if (particleName == "opticalphoton") {
            if (fVerboseLevel > 0) {
                G4cout << "CustomOpticalPhysics: Adding processes to optical photon" << G4endl;
            }
            
            pmanager->AddDiscreteProcess(theAbsorptionProcess);
            pmanager->AddDiscreteProcess(theRayleighScatteringProcess);
            
            // CRITICAL: Add boundary process
            pmanager->AddDiscreteProcess(theBoundaryProcess);
            
            if (fVerboseLevel > 0) {
                G4cout << "CustomOpticalPhysics: OpBoundary process ADDED to optical photon!" << G4endl;
            }
        }
        
        // Add scintillation to all particles
        if (theScintProcess->IsApplicable(*particle)) {
            pmanager->AddProcess(theScintProcess);
            pmanager->SetProcessOrderingToLast(theScintProcess, idxAtRest);
            pmanager->SetProcessOrderingToLast(theScintProcess, idxPostStep);
        }
    }
    
    // Enable Birks saturation
    G4EmSaturation* emSaturation = G4LossTableManager::Instance()->EmSaturation();
    theScintProcess->AddSaturation(emSaturation);
    
    if (fVerboseLevel > 0) {
        G4cout << "CustomOpticalPhysics: Construction complete!" << G4endl;
    }
}