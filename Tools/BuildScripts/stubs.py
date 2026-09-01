import os
import sys
from pathlib import Path
from mypy.stubgen import generate_stub_for_c_module

def generate_skybolt_stubs(module_name: str, build_dir: Path, output_file: Path):
    sys.path.append(str(build_dir.resolve()))
    import setup_environment
    setup_environment.setup_skybolt_environment() # Setup environment so that skybolt can find DLLs
    
    try:
        import skybolt
    except Exception as e:
        pattern = str(build_dir / (module_name + "*.pyd"))
        if glob.glob(pattern):
            raise RuntimeError(f"The module binary exists at '{pattern}' but python failed to import it. Is there a Python version mismatch?")
        else:
            raise RuntimeError(f"Module import failed because no module found at '{pattern}'.")

    output_file.parent.mkdir(parents=True, exist_ok=True)
    
    # Remove existing output file if one exists
    if output_file.exists():
        output_file.unlink()
    
    # Generate new output
    generate_stub_for_c_module(module_name, str(output_file), known_modules=None)

    # Verify the output is present
    if not output_file.exists():
        raise RuntimeError(f"Failed to create stubs for module '{module_name}'")