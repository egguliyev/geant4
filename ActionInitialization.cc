#include "ActionInitialization.hh"

#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"
#include "G4RunManager.hh"
#include "DetectorConstruction.hh"
#include "G4Exception.hh"

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    // 1. Primary generator
    auto primary = new PrimaryGeneratorAction();
    SetUserAction(primary);

    // 2. RunAction
    RunAction* runAction = new RunAction(primary);
    SetUserAction(runAction);

    // 3. DetectorConstruction pointer (const ok)
    const DetectorConstruction* detConst =
        dynamic_cast<const DetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    if (!detConst) {
        G4Exception("ActionInitialization::Build", "DetConstructionNull",
            FatalException, "DetectorConstruction cast failed!");
    }

    // 4. SteppingAction (MUST use correct constructor)
    SteppingAction* stepping = new SteppingAction(runAction, detConst);
    SetUserAction(stepping);

    // 5. TrackingAction
    SetUserAction(new TrackingAction());
}
