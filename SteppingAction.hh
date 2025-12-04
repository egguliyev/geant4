#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "globals.hh"
#include "G4UserSteppingAction.hh"

class SteppingMessenger;
class RunAction;
class DetectorConstruction;

extern long gTotalScint;
extern long gEscape;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(RunAction* runAction, const DetectorConstruction* det);
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

    inline void SetKillOnSecondSurface(G4bool val) { fKillOnSecondSurface = val; }
    inline G4bool GetKillOnSecondSurface() { return fKillOnSecondSurface; }

private:
    SteppingMessenger* fSteppingMessenger = nullptr;

    G4int fVerbose = 0;
    size_t fIdxVelocity = 0;

    G4bool fKillOnSecondSurface = false;

    RunAction* fRunAction = nullptr;
    const DetectorConstruction* fDetConstruction = nullptr;
};

#endif
