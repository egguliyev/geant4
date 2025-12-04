//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file optical/OpNovice2/OpNovice2.cc
/// \brief Main program of the optical/OpNovice2 example
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "SteppingVerbose.hh"

// Include the implementation directly to avoid linker issues
#include "src/CustomOpticalPhysics.cc"  // Full path from project root

#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4RunManagerFactory.hh"
#include "G4String.hh"
#include "G4Types.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4OpticalPhoton.hh"
#include "G4ProcessManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main(int argc, char** argv)
{
  // detect interactive mode (if no arguments) and define UI session
  G4UIExecutive* ui = nullptr;
  if(argc == 1)
    ui = new G4UIExecutive(argc, argv);

  // application-specific SteppingVerbose
  auto steppingVerbose = new SteppingVerbose;

  auto runManager = G4RunManagerFactory::CreateRunManager();
  //auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);

  auto detector = new DetectorConstruction();
  runManager->SetUserInitialization(detector);

  G4VModularPhysicsList* physicsList = new FTFP_BERT;
  physicsList->ReplacePhysics(new G4EmStandardPhysics_option4());
  
  // CRITICAL FIX: Use custom optical physics that explicitly adds OpBoundary
  G4cout << "\n========================================" << G4endl;
  G4cout << "Registering CustomOpticalPhysics..." << G4endl;
  G4cout << "========================================\n" << G4endl;
  
  physicsList->RegisterPhysics(new CustomOpticalPhysics(1, "Optical"));  // verbose=1
  
  runManager->SetUserInitialization(physicsList);
  runManager->SetUserInitialization(new ActionInitialization());
  
  // Initialize the run manager (this constructs the physics processes)
  runManager->Initialize();

  // VERIFICATION: Check that optical boundary process is registered
  G4cout << "\n========================================" << G4endl;
  G4cout << "VERIFYING OPTICAL PHYSICS" << G4endl;
  G4cout << "========================================" << G4endl;
  
  G4OpticalPhoton* optPhoton = G4OpticalPhoton::OpticalPhotonDefinition();
  G4ProcessManager* pm = optPhoton->GetProcessManager();
  G4ProcessVector* pv = pm->GetProcessList();
  
  G4cout << "Optical photon has " << pv->size() << " processes:" << G4endl;
  bool boundaryFound = false;
  for (size_t i = 0; i < pv->size(); i++) {
    G4String processName = (*pv)[i]->GetProcessName();
    G4cout << "  [" << i << "] " << processName << G4endl;
    if (processName == "OpBoundary") boundaryFound = true;
  }
  
  G4cout << "========================================" << G4endl;
  if (boundaryFound) {
    G4cout << "*** SUCCESS: OpBoundary is ACTIVE! ***" << G4endl;
    G4cout << "*** Detection should work now! ***" << G4endl;
  } else {
    G4cout << "*** ERROR: OpBoundary NOT FOUND! ***" << G4endl;
    G4cout << "*** Detection will NOT work! ***" << G4endl;
  }
  G4cout << "========================================\n" << G4endl;

  // initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if(ui)
  {
    // interactive mode
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
  }
  else
  {
    // batch mode
    G4String command  = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }

  // job termination
  delete visManager;
  delete runManager;
  delete steppingVerbose;
  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......