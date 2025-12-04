#include "SteppingAction.hh"
#include "Run.hh"
#include "RunAction.hh"
#include "DetectorConstruction.hh"
#include "SteppingMessenger.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <set>  // For tracking unique photons

long gTotalScint = 0;
long gEscape = 0;
extern long gDetectedPhotons;  // DECLARE EXTERNAL COUNTER

SteppingAction::SteppingAction(RunAction* run, const DetectorConstruction* det)
    : G4UserSteppingAction(), fRunAction(run), fDetConstruction(det)
{
    fSteppingMessenger = new SteppingMessenger(this);
}

SteppingAction::~SteppingAction()
{
    delete fSteppingMessenger;
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    G4StepPoint* pre = step->GetPreStepPoint();
    G4StepPoint* post = step->GetPostStepPoint();

    Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());

    //------------------------------------------------------
    // Optical photon handling
    //------------------------------------------------------
    if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition())
    {
        auto prePV = pre->GetPhysicalVolume();
        auto postPV = post->GetPhysicalVolume();

        // SIMPLE APPROACH: Just detect when photon enters photodiode from Tank
        if (post->GetStepStatus() == fGeomBoundary && prePV && postPV)
        {
            // When photon crosses from Tank to Photodiode, COUNT IT!
            if (prePV->GetName() == "Tank" && postPV->GetName() == "Photodiode")
            {
                // COUNT AS DETECTED - use BOTH counters
                fRunAction->AddPhotonToExitCount();
                run->AddDetectedPD();
                gDetectedPhotons++;  // INCREMENT GLOBAL COUNTER
                
                G4double energy = track->GetKineticEnergy();
                run->AddScintEnergy(energy);
                
                // Kill the photon - it's been detected
                track->SetTrackStatus(fStopAndKill);
                
                static int detCount = 0;
                detCount++;
                if (detCount <= 10) {
                    G4cout << "PHOTON #" << detCount << " DETECTED at E=" 
                           << energy/eV << " eV" << G4endl;
                }
                return;
            }
            
            // Track all boundary events for statistics
            const G4VProcess* proc = post->GetProcessDefinedStep();
            if (proc && proc->GetProcessName() == "OpBoundary")
            {
                const G4OpBoundaryProcess* bp = 
                    dynamic_cast<const G4OpBoundaryProcess*>(proc);
                if (bp)
                {
                    run->AddTotalSurface();
                    run->CountBoundaryStatus(bp->GetStatus());
                }
            }
            
            // Track photons exiting +Z face (count each photon only ONCE)
            if (prePV && prePV->GetName() == "Tank")
            {
                G4ThreeVector pos = post->GetPosition();
                G4double tankZmax = fDetConstruction->GetTankZ();
                
                if (std::abs(pos.z() - tankZmax) < 0.1*mm)
                {
                    // Use a set to track which photons we've already counted
                    static std::set<G4int> countedPhotons;
                    G4int trackID = track->GetTrackID();
                    
                    if (countedPhotons.find(trackID) == countedPhotons.end())
                    {
                        // First time this photon exits +Z face
                        countedPhotons.insert(trackID);
                        run->AddExitPlusZ();
                        gEscape++;
                    }
                }
            }
        }

        // Optical processes
        const G4VProcess* pds = post->GetProcessDefinedStep();
        if (pds)
        {
            G4String name = pds->GetProcessName();
            if (name == "OpAbsorption")
            {
                run->AddOpAbsorption();
                if (pre->GetPhysicalVolume() && 
                    pre->GetPhysicalVolume()->GetName() == "Tank")
                {
                    run->AddOpAbsorptionPrior();
                }
            }
            else if (name == "OpRayleigh")
            {
                run->AddRayleigh();
            }
        }
    }
    //------------------------------------------------------
    // Scintillation photon creation
    //------------------------------------------------------
    else
    {
        const std::vector<const G4Track*>* secondaries = 
            step->GetSecondaryInCurrentStep();
        
        if (secondaries && secondaries->size() > 0)
        {
            for (auto sec : *secondaries)
            {
                if (sec->GetDefinition() == 
                    G4OpticalPhoton::OpticalPhotonDefinition())
                {
                    if (sec->GetCreatorProcess() &&
                        sec->GetCreatorProcess()->GetProcessName() == "Scintillation")
                    {
                        G4double photonE = sec->GetKineticEnergy();
                        run->AddScintillation();
                        run->AddScintEnergy(photonE);
                        gTotalScint++;
                    }
                }
            }
        }
    }
}