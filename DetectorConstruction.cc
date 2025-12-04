
#include "DetectorConstruction.hh"

#include "DetectorMessenger.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4ProductionCuts.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
  : G4VUserDetectorConstruction()
  , fDetectorMessenger(nullptr)
{
  const G4int NUM = 4;
  G4double eph[4] = { 1.5 * eV, 2.0 * eV, 2.5 * eV, 3.0 * eV };  // CsI spectrum
  G4double nCsI[4] = { 1.79, 1.79, 1.79, 1.79 };

  fTankMPT    = new G4MaterialPropertiesTable();
  fWorldMPT   = new G4MaterialPropertiesTable();
  fSurfaceMPT = new G4MaterialPropertiesTable();

  fSurface = new G4OpticalSurface("Surface");
  fSurface->SetType(dielectric_dielectric);

  fSurface->SetModel(unified);
  fSurface->SetFinish(polished);
  
  auto nist = G4NistManager::Instance();
  fWorldMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

  G4double E[4] = { 1.5 * eV, 2.0 * eV, 2.5 * eV, 3.0 * eV };
  G4double nAir[4] = { 1.0,    1.0,    1.0,    1.0 };

  fWorldMPT->AddProperty("RINDEX", E, nAir, 4);
  fWorldMaterial->SetMaterialPropertiesTable(fWorldMPT);
   
  // Elements
  G4Element* elCs = nist->FindOrBuildElement("Cs");
  G4Element* elI = nist->FindOrBuildElement("I");

  // Base CsI
  G4Material* matCsI = new G4Material("CsI", 4.51 * g / cm3, 2);
  matCsI->AddElement(elCs, 1);
  matCsI->AddElement(elI, 1);

  // Thallium dopant
  G4Material* dopTl = nist->FindOrBuildMaterial("G4_Tl");

  // CsI:Tl mixture (0.001% Tl typical)
  G4double TlConc = 0.001 * perCent;

  G4Material* matCsITl = new G4Material("CsI_Tl", 4.51 * g / cm3, 2);
  matCsITl->AddMaterial(dopTl, TlConc);
  matCsITl->AddMaterial(matCsI, 100. * perCent - TlConc);

  // Assign as tank material
  fTankMaterial = matCsITl;
   

  // --- Photodiode material (Si) and its MPT ---
  fPDMaterial = nist->FindOrBuildMaterial("G4_Si");
  auto siMPT = new G4MaterialPropertiesTable();

  G4double nSi[4] = { 3.5, 3.5, 3.5, 3.5 };              // rough, ok for now
  //G4double absSi[4] = { 100 * mm, 100 * mm, 100 * mm, 100 * mm };
  G4double absSi[] = {0.1*mm,0.1*mm,0.1*mm,0.1*mm};

  siMPT->AddProperty("RINDEX", eph, nSi, NUM);
  siMPT->AddProperty("ABSLENGTH", eph, absSi, NUM);
  fPDMaterial->SetMaterialPropertiesTable(siMPT);

  fDetectorMessenger = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction() {
  delete fTankMPT;
  delete fWorldMPT;
  delete fSurfaceMPT;
  delete fSurface;
  delete fDetectorMessenger;
 }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
 
// Constructor and Destructor remain the same as your original...

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  fTankMaterial->GetIonisation()->SetBirksConstant(0.003 * mm / MeV);

  // World
  auto world_box = new G4Box("World", fExpHall_x, fExpHall_y, fExpHall_z);
  fWorld_LV = new G4LogicalVolume(world_box, fWorldMaterial, "World");
  G4VPhysicalVolume* world_PV = new G4PVPlacement(
      nullptr, G4ThreeVector(), fWorld_LV, "World", nullptr, false, 0);

  // CsI Tank
  auto tank_box = new G4Box("Tank", fTank_x, fTank_y, fTank_z);
  auto csiMPT = new G4MaterialPropertiesTable();

  const G4int n = 4;
  G4double photonE[n] = { 2.0 * eV, 2.25 * eV, 2.5 * eV, 3.0 * eV };
  G4double rindex[n] = { 1.79, 1.79, 1.79, 1.79 };
  G4double abslen[n] = { 50 * cm, 80 * cm, 60 * cm, 40 * cm };
  G4double rayleigh[n] = { 100 * cm, 150 * cm, 120 * cm, 100 * cm };
  G4double scint[n] = { 0.5, 1.0, 0.8, 0.3 };

  csiMPT->AddProperty("RINDEX", photonE, rindex, n);
  csiMPT->AddProperty("ABSLENGTH", photonE, abslen, n);
  csiMPT->AddProperty("RAYLEIGH", photonE, rayleigh, n);
  csiMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonE, scint, n);
  csiMPT->AddConstProperty("SCINTILLATIONYIELD", 54 / keV);
  csiMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
  csiMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 1000 * ns);
  csiMPT->AddConstProperty("SCINTILLATIONYIELD1", 1.0);

  G4cout << "=== CsI MPT ===" << G4endl;
  csiMPT->DumpTable();
  fTankMaterial->SetMaterialPropertiesTable(csiMPT);

  fTank_LV = new G4LogicalVolume(tank_box, fTankMaterial, "Tank");
  fTank = new G4PVPlacement(nullptr, G4ThreeVector(), fTank_LV, "Tank",
                            fWorld_LV, false, 0);

  // Photodiode - positioned with NO gap
  auto pd_box = new G4Box("Photodiode", fTank_x, fTank_y, fPD_z);
  fPD_LV = new G4LogicalVolume(pd_box, fPDMaterial, "Photodiode");

  // CRITICAL: Position so PD front face touches CsI back face
  G4double zpos = fTank_z + fPD_z;
  fPD_PV = new G4PVPlacement(nullptr, G4ThreeVector(0, 0, zpos),
      fPD_LV, "Photodiode", fWorld_LV, false, 0, true);

  // CRITICAL FIX: Use dielectric_metal for detection surface
  auto pdSurf = new G4OpticalSurface("CsI_to_PD");
  pdSurf->SetType(dielectric_metal);  // CHANGED from dielectric_dielectric
  pdSurf->SetModel(unified);
  pdSurf->SetFinish(polished);

  auto pdSurfMPT = new G4MaterialPropertiesTable();
  
  // For dielectric_metal: REFLECTIVITY + EFFICIENCY must be handled carefully
  // Photons that aren't reflected are "detected" (absorbed)
  G4double reflectivity[n] = { 0.05, 0.03, 0.05, 0.10 };  // small reflection
  G4double efficiency[n] = { 0.90, 0.95, 0.93, 0.85 };     // detection QE

  pdSurfMPT->AddProperty("REFLECTIVITY", photonE, reflectivity, n);
  pdSurfMPT->AddProperty("EFFICIENCY", photonE, efficiency, n);
  pdSurf->SetMaterialPropertiesTable(pdSurfMPT);

  // Define border surface from Tank to PD
  new G4LogicalBorderSurface("TankToPD", fTank, fPD_PV, pdSurf);
  
  // ========================================
  // ADD REFLECTIVE WRAPPING ON 5 SIDES
  // ========================================
  
  // Create reflective optical surface (white diffuse reflector)
  auto reflectiveSurface = new G4OpticalSurface("ReflectiveWrap");
  reflectiveSurface->SetType(dielectric_metal);
  reflectiveSurface->SetFinish(polished);
  reflectiveSurface->SetModel(unified);
  
  auto reflectiveMPT = new G4MaterialPropertiesTable();
  G4double refl_values[n] = { 0.98, 0.98, 0.98, 0.98 };  // 98% reflective
  G4double refl_eff[n] = { 0.0, 0.0, 0.0, 0.0 };         // Not a detector
  
  reflectiveMPT->AddProperty("REFLECTIVITY", photonE, refl_values, n);
  reflectiveMPT->AddProperty("EFFICIENCY", photonE, refl_eff, n);
  reflectiveSurface->SetMaterialPropertiesTable(reflectiveMPT);
  
  // Apply reflective surface to Tank-World boundary (all 5 sides except +Z)
  new G4LogicalSkinSurface("ReflectiveWrap", fTank_LV, reflectiveSurface);
  
  G4cout << "\n*** REFLECTIVE WRAPPING ADDED ***" << G4endl;
  G4cout << "98% reflective on 5 sides (not +Z face)" << G4endl;
  G4cout << "This channels photons toward photodiode" << G4endl;
  G4cout << "**********************************\n" << G4endl;

  G4cout << "\n======================================" << G4endl;
  G4cout << "==== CRITICAL GEOMETRY CHECK ====" << G4endl;
  G4cout << "======================================" << G4endl;
  G4cout << "Tank half-Z     = " << fTank_z / mm << " mm" << G4endl;
  G4cout << "PD half-Z       = " << fPD_z / mm << " mm" << G4endl;
  G4cout << "PD center Z     = " << zpos / mm << " mm" << G4endl;
  G4cout << "CsI +Z face     = " << (+fTank_z) / mm << " mm" << G4endl;
  G4cout << "PD front face Z = " << (zpos - fPD_z) / mm << " mm" << G4endl;
  G4double gap = (zpos - fPD_z) - fTank_z;
  G4cout << "Gap             = " << gap / mm << " mm";
  if (std::abs(gap) < 1e-9 * mm) {
      G4cout << " (GOOD - no gap!)" << G4endl;
  } else {
      G4cout << " *** WARNING: GAP EXISTS! ***" << G4endl;
      G4cout << "*** PHOTONS WILL ESCAPE TO WORLD! ***" << G4endl;
  }
  G4cout << "\nPD X size = " << fTank_x / mm << " mm" << G4endl;
  G4cout << "PD Y size = " << fTank_y / mm << " mm" << G4endl;
  G4cout << "Tank X = " << fTank_x / mm << ", Tank Y = " << fTank_y / mm << G4endl;
  
  G4cout << "\nBOUNDARY SURFACE INFO:" << G4endl;
  G4cout << "LogicalBorderSurface 'TankToPD' created" << G4endl;
  G4cout << "From: Tank PV" << G4endl;
  G4cout << "To: Photodiode PV" << G4endl;
  G4cout << "Surface type: " << pdSurf->GetType() << G4endl;
  G4cout << "======================================\n" << G4endl;

  return world_PV;
} 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::SetSurfaceSigmaAlpha(G4double v)
{
  fSurface->SetSigmaAlpha(v);
  G4RunManager::GetRunManager()->GeometryHasBeenModified();

  G4cout << "Surface sigma alpha set to: " << fSurface->GetSigmaAlpha()
         << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::SetSurfacePolish(G4double v)
{
  fSurface->SetPolish(v);
  G4RunManager::GetRunManager()->GeometryHasBeenModified();

  G4cout << "Surface polish set to: " << fSurface->GetPolish() << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::AddTankMPV(const G4String& prop,
                                      G4MaterialPropertyVector* mpv)
{
  fTankMPT->AddProperty(prop, mpv);
  G4cout << "The MPT for the box is now: " << G4endl;
  fTankMPT->DumpTable();
  G4cout << "............." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::AddWorldMPV(const G4String& prop,
                                       G4MaterialPropertyVector* mpv)
{
  fWorldMPT->AddProperty(prop, mpv);
  G4cout << "The MPT for the world is now: " << G4endl;
  fWorldMPT->DumpTable();
  G4cout << "............." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::AddSurfaceMPV(const G4String& prop,
                                         G4MaterialPropertyVector* mpv)
{
  fSurfaceMPT->AddProperty(prop, mpv);
  G4cout << "The MPT for the surface is now: " << G4endl;
  fSurfaceMPT->DumpTable();
  G4cout << "............." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::AddTankMPC(const G4String& prop, G4double v)
{
  fTankMPT->AddConstProperty(prop, v);
  G4cout << "The MPT for the box is now: " << G4endl;
  fTankMPT->DumpTable();
  G4cout << "............." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::AddWorldMPC(const G4String& prop, G4double v)
{
  fWorldMPT->AddConstProperty(prop, v);
  G4cout << "The MPT for the world is now: " << G4endl;
  fWorldMPT->DumpTable();
  G4cout << "............." << G4endl;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::AddSurfaceMPC(const G4String& prop, G4double v)
{
  fSurfaceMPT->AddConstProperty(prop, v);
  G4cout << "The MPT for the surface is now: " << G4endl;
  fSurfaceMPT->DumpTable();
  G4cout << "............." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::SetWorldMaterial(const G4String& mat)
{
  G4Material* pmat = G4NistManager::Instance()->FindOrBuildMaterial(mat);
  if(pmat && fWorldMaterial != pmat)
  {
    fWorldMaterial = pmat;
    if(fWorld_LV)
    {
      fWorld_LV->SetMaterial(fWorldMaterial);
      fWorldMaterial->SetMaterialPropertiesTable(fWorldMPT);
    }
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    G4cout << "World material set to " << fWorldMaterial->GetName() << G4endl;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void DetectorConstruction::SetTankMaterial(const G4String& mat)
{
  G4Material* pmat = G4NistManager::Instance()->FindOrBuildMaterial(mat);
  if(pmat && fTankMaterial != pmat)
  {
    fTankMaterial = pmat;
    if(fTank_LV)
    {
      fTank_LV->SetMaterial(fTankMaterial);
      fTankMaterial->SetMaterialPropertiesTable(fTankMPT);
      fTankMaterial->GetIonisation()->SetBirksConstant(0.126 * mm / MeV);
    }
    G4RunManager::GetRunManager()->PhysicsHasBeenModified();
    G4cout << "Tank material set to " << fTankMaterial->GetName() << G4endl;
  }
}
