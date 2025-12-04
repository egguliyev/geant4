import subprocess
import re
import matplotlib.pyplot as plt
import numpy as np

# Thickness values to sweep (in mm, converted from microns)
thicknesses_um = np.arange(50, 1050, 50)  # 50 to 1000 um, step 50
thicknesses_mm = thicknesses_um / 1000.0   # Convert to mm

# Results storage
results = {
    'thickness_um': [],
    'photons_created': [],
    'photons_detected': [],
    'electrons': [],
    'detection_efficiency': []
}

# Path to your executable and macro
exe_path = r".\build\Release\OpNovice2.exe"
macro_path = "run.mac"

print("Starting CsI thickness sweep...")
print("=" * 60)

for thickness_um, thickness_mm in zip(thicknesses_um, thicknesses_mm):
    print(f"\nRunning simulation for thickness = {thickness_um} μm ({thickness_mm} mm)")
    
    # Modify DetectorConstruction.hh programmatically
    # Read current header
    with open('include/DetectorConstruction.hh', 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    # Find and update fTank_z line
    for i, line in enumerate(lines):
        if 'fTank_z =' in line and 'CLHEP::mm' in line:
            # Update to new thickness (half-length)
            half_thickness = thickness_mm / 2.0
            lines[i] = f'  G4double fTank_z = {half_thickness:.6f} *CLHEP::mm;  // {thickness_um} um thick CsI\n'
            break
    
    # Write back
    with open('include/DetectorConstruction.hh', 'w', encoding='utf-8') as f:
        f.writelines(lines)
    
    # Recompile
    print("  Recompiling...")
    compile_result = subprocess.run(
        ["cmake", "--build", "build", "--config", "Release"],
        capture_output=True,
        text=True
    )
    
    if compile_result.returncode != 0:
        print(f"  ERROR: Compilation failed for {thickness_um} μm")
        continue
    
    # Run simulation
    print("  Running simulation...")
    run_result = subprocess.run(
        [exe_path, macro_path],
        capture_output=True,
        text=True
    )
    
    # Parse output
    output = run_result.stdout + run_result.stderr
    
    # Extract values using regex
    photons_created = re.search(r'Total scintillation photons created:\s*(\d+)', output)
    photons_detected = re.search(r'Photons detected at PD \(global\):\s*(\d+)', output)
    electrons = re.search(r'Estimated electrons:\s*([\d.]+)', output)
    
    if photons_created and photons_detected and electrons:
        created = int(photons_created.group(1))
        detected = int(photons_detected.group(1))
        elec = float(electrons.group(1))
        efficiency = (detected / created * 100) if created > 0 else 0
        
        results['thickness_um'].append(thickness_um)
        results['photons_created'].append(created)
        results['photons_detected'].append(detected)
        results['electrons'].append(elec)
        results['detection_efficiency'].append(efficiency)
        
        print(f"  ✓ Created: {created}, Detected: {detected}, Electrons: {elec:.1f}, Eff: {efficiency:.1f}%")
    else:
        print(f"  ERROR: Could not parse output for {thickness_um} μm")

print("\n" + "=" * 60)
print("Sweep complete! Generating plots...")

# Create figure with multiple subplots
fig, axes = plt.subplots(2, 2, figsize=(14, 10))
fig.suptitle('CsI Thickness Optimization for TDI Camera', fontsize=16, fontweight='bold')

# Plot 1: Scintillation photons created
ax1 = axes[0, 0]
ax1.plot(results['thickness_um'], results['photons_created'], 'b-o', linewidth=2, markersize=6)
ax1.set_xlabel('CsI Thickness (μm)', fontsize=12)
ax1.set_ylabel('Scintillation Photons Created', fontsize=12)
ax1.set_title('Total Light Output', fontsize=13)
ax1.grid(True, alpha=0.3)

# Plot 2: Photoelectrons (most important!)
ax2 = axes[0, 1]
ax2.plot(results['thickness_um'], results['electrons'], 'r-o', linewidth=2, markersize=6)
ax2.set_xlabel('CsI Thickness (μm)', fontsize=12)
ax2.set_ylabel('Photoelectrons Generated', fontsize=12)
ax2.set_title('Detected Signal (Photoelectrons)', fontsize=13)
ax2.grid(True, alpha=0.3)

# Find and mark optimal thickness
if results['electrons']:
    optimal_idx = np.argmax(results['electrons'])
    optimal_thickness = results['thickness_um'][optimal_idx]
    optimal_electrons = results['electrons'][optimal_idx]
    ax2.axvline(optimal_thickness, color='g', linestyle='--', alpha=0.7, linewidth=2)
    ax2.text(optimal_thickness, optimal_electrons * 1.05, 
             f'Optimal:\n{optimal_thickness}μm', 
             ha='center', fontsize=10, fontweight='bold',
             bbox=dict(boxstyle='round', facecolor='yellow', alpha=0.7))

# Plot 3: Detection efficiency
ax3 = axes[1, 0]
ax3.plot(results['thickness_um'], results['detection_efficiency'], 'g-o', linewidth=2, markersize=6)
ax3.set_xlabel('CsI Thickness (μm)', fontsize=12)
ax3.set_ylabel('Detection Efficiency (%)', fontsize=12)
ax3.set_title('Light Collection Efficiency', fontsize=13)
ax3.grid(True, alpha=0.3)

# Plot 4: Photons detected
ax4 = axes[1, 1]
ax4.plot(results['thickness_um'], results['photons_detected'], 'm-o', linewidth=2, markersize=6)
ax4.set_xlabel('CsI Thickness (μm)', fontsize=12)
ax4.set_ylabel('Photons Detected at PD', fontsize=12)
ax4.set_title('Collected Scintillation Photons', fontsize=13)
ax4.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('csi_thickness_optimization.png', dpi=300, bbox_inches='tight')
print("Plot saved as 'csi_thickness_optimization.png'")
plt.show()

# Print summary
print("\n" + "=" * 60)
print("SUMMARY:")
print("=" * 60)
if results['electrons']:
    optimal_idx = np.argmax(results['electrons'])
    print(f"Optimal Thickness: {results['thickness_um'][optimal_idx]} μm")
    print(f"Maximum Photoelectrons: {results['electrons'][optimal_idx]:.1f}")
    print(f"Detection Efficiency at Optimal: {results['detection_efficiency'][optimal_idx]:.1f}%")
    
    # Show trend
    if len(results['electrons']) > 5:
        mid_idx = len(results['electrons']) // 2
        if results['electrons'][-1] < results['electrons'][mid_idx]:
            print("\n✓ Peak detected - light collection degrades with excessive thickness")
        else:
            print("\n⚠ No clear peak yet - may need thicker CsI for this geometry")