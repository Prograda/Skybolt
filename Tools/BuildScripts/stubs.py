import os
import sys
from pathlib import Path
from mypy.stubgen import generate_stub_for_c_module

def generate_skybolt_stubs(module_name: str, build_dir: Path, output_file: Path):
    sys.path.insert(0, str(build_dir))
    import setup_environment
    setup_environment.setup_skybolt_environment() # Setup environment so that skybolt can find DLLs
    import skybolt

    output_file.parent.mkdir(parents=True, exist_ok=True)
    
    # Remove existing output file if one exists
    if output_file.exists():
        output_file.unlink()
    
    # Generate new output
    generate_stub_for_c_module(module_name, str(output_file), known_modules=None)

    # Verify the output is present
    if not output_file.exists():
        raise RuntimeError(f"Failed to create stubs for module '{module_name}'")