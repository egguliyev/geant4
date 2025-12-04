#ifndef RunAction_h
#define RunAction_h 1

#include "globals.hh"
#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4AccumulableManager.hh"

class Run;
class HistoManager;
class PrimaryGeneratorAction;

class RunAction : public G4UserRunAction
{
public:
    RunAction(PrimaryGeneratorAction* primary = nullptr);
    ~RunAction() override;

    G4Run* GenerateRun() override;
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void AddPhotonToExitCount() { fExitPhotonCount += 1; }
    G4int GetExitPhotonCount() const { return fExitPhotonCount.GetValue(); }

private:
    Run* fRun = nullptr;
    HistoManager* fHistoManager = nullptr;
    PrimaryGeneratorAction* fPrimary = nullptr;

    G4Accumulable<G4int> fExitPhotonCount{ 0 };

};

#endif
