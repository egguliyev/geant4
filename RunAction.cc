#include "RunAction.hh"
#include "HistoManager.hh"
#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4AnalysisManager.hh"

extern long gTotalScint;
extern long gEscape;

// ADD THIS GLOBAL COUNTER
long gDetectedPhotons = 0;

RunAction::RunAction(PrimaryGeneratorAction* prim)
    : G4UserRunAction(),
    fRun(nullptr),
    fHistoManager(new HistoManager()),
    fPrimary(prim),
    fExitPhotonCount(0)
{
    G4AccumulableManager::Instance()->RegisterAccumulable(fExitPhotonCount);
}

RunAction::~RunAction()
{
    delete fHistoManager;
}

G4Run* RunAction::GenerateRun()
{
    fRun = new Run();
    return fRun;
}

void RunAction::BeginOfRunAction(const G4Run*)
{
    G4AccumulableManager::Instance()->Reset();
    gTotalScint = 0;
    gEscape = 0;
    gDetectedPhotons = 0;  // RESET GLOBAL COUNTER
    
    // reset counters
    if (IsMaster()) {  
    }
    // copy primary generator info
    if (fPrimary) {
        auto gun = fPrimary->GetParticleGun();
        fRun->SetPrimary(
            gun->GetParticleDefinition(),
            gun->GetParticleEnergy(),
            fPrimary->GetPolarized(),
            fPrimary->GetPolarization());
    }
    // open histograms
    auto analysis = G4AnalysisManager::Instance();
    if (analysis->IsActive())
        analysis->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run* aRun)
{
    auto analysis = G4AnalysisManager::Instance();
    if (IsMaster()) {
        G4AccumulableManager::Instance()->Merge();
        auto run = static_cast<const Run*>(aRun);
        
        G4cout << "\n=== CsI SCINTILLATION SUMMARY ===\n";
        G4cout << "Total scintillation photons created: " << gTotalScint << "\n";
        G4cout << "Total photons that escaped CsI:     " << gEscape << "\n";
        G4cout << "Photons exiting CsI +Z face:        " << run->GetExitPlusZ() << "\n";
        
        // USE BOTH COUNTERS
        G4cout << "Photons detected at PD (global):      " << gDetectedPhotons << "\n";
        
        auto Ndet = gDetectedPhotons;  // USE GLOBAL COUNTER
        G4double QE = 0.9;
        G4cout << "Estimated electrons: " << (QE * Ndet) << G4endl;
        G4cout << "====================================\n\n";
        
        run->EndOfRun();
    }
    if (analysis->IsActive()) { analysis->Write(); analysis->CloseFile(); }
}