#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "globals.hh"
#include "G4OpticalSurface.hh"
#include "G4RunManager.hh"
#include "G4VUserDetectorConstruction.hh"

#include <CLHEP/Units/SystemOfUnits.h>

class DetectorMessenger;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction
{
 public:
  DetectorConstruction();
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;

  G4VPhysicalVolume* GetTank() { return fTank; }
  G4double GetTankXSize() { return fTank_x; }

  G4OpticalSurface* GetSurface(void) { return fSurface; }

  void SetSurfaceFinish(const G4OpticalSurfaceFinish finish)
  {
    fSurface->SetFinish(finish);
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
  }
  G4OpticalSurfaceFinish GetSurfaceFinish()
  {
    return fSurface->GetFinish();
  }

  void SetSurfaceType(const G4SurfaceType type)
  {
    fSurface->SetType(type);
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
  }

  void SetSurfaceModel(const G4OpticalSurfaceModel model)
  {
    fSurface->SetModel(model);
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
  }
  G4OpticalSurfaceModel GetSurfaceModel() { return fSurface->GetModel(); }

  void SetSurfaceSigmaAlpha(G4double v);
  void SetSurfacePolish(G4double v);

  void AddTankMPV(const G4String& prop, G4MaterialPropertyVector* mpv);
  void AddTankMPC(const G4String& prop, G4double v);
  G4MaterialPropertiesTable* GetTankMaterialPropertiesTable()
  {
    return fTankMPT;
  }

  void AddWorldMPV(const G4String& prop, G4MaterialPropertyVector* mpv);
  void AddWorldMPC(const G4String& prop, G4double v);
  G4MaterialPropertiesTable* GetWorldMaterialPropertiesTable()
  {
    return fWorldMPT;
  }

  void AddSurfaceMPV(const G4String& prop, G4MaterialPropertyVector* mpv);
  void AddSurfaceMPC(const G4String& prop, G4double v);
  G4MaterialPropertiesTable* GetSurfaceMaterialPropertiesTable()
  {
    return fSurfaceMPT;
  }

  void SetWorldMaterial(const G4String&);
  G4Material* GetWorldMaterial() const { return fWorldMaterial; }
  void SetTankMaterial(const G4String&);
  G4Material* GetTankMaterial() const { return fTankMaterial; }

  G4double GetTankX() const { return fTank_x; }
  G4double GetTankY() const { return fTank_y; }
  G4double GetTankZ() const { return fTank_z; }   // half-lengths

  G4double GetTankXSize() const { return fTank_x; }
  G4double GetTankYSize() const { return fTank_y; }



 private:
  G4double fExpHall_x = 200*CLHEP::mm;
  G4double fExpHall_y = 200*CLHEP::mm;
  G4double fExpHall_z = 200*CLHEP::mm;

  G4VPhysicalVolume* fTank = nullptr;

  G4double fTank_x = 0.024 *CLHEP::mm;
  G4double fTank_y = 0.024 *CLHEP::mm;
  G4double fTank_z = 0.2 *CLHEP::mm;

  G4LogicalVolume* fWorld_LV = nullptr;
  G4LogicalVolume* fTank_LV = nullptr;

  G4LogicalVolume* fPD_LV = nullptr;
  G4VPhysicalVolume* fPD_PV = nullptr;
  G4double fPD_z = 0.05 *CLHEP::mm;    

  G4Material* fWorldMaterial = nullptr;
  G4Material* fTankMaterial = nullptr;

  G4OpticalSurface* fSurface = nullptr;

  DetectorMessenger* fDetectorMessenger = nullptr;

  G4MaterialPropertiesTable* fTankMPT = nullptr;
  G4MaterialPropertiesTable* fWorldMPT = nullptr;
  G4MaterialPropertiesTable* fSurfaceMPT = nullptr;
  G4Material* fPDMaterial = nullptr;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif /*DetectorConstruction_h*/
