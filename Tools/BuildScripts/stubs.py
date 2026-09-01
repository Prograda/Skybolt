import importlib
import os
import sys
from pathlib import Path
from typing import Sequence
from mypy.stubgen import generate_stub_for_c_module

def generate_skybolt_stubs(module_name: str, module_search_dirs: Sequence[Path], output_file: Path):
    for dir in module_search_dirs:
        sys.path.append(str(dir.resolve()))
    import setup_environment
    setup_environment.setup_skybolt_environment() # Setup environment so that skybolt can find DLLs
    print(f"module_search_dirs {module_search_dirs}")
    try:
        importlib.import_module(module_name)
    except Exception as e:
        found = False
        for dir in module_search_dirs:
            pattern = module_name + "*.{pyd,so}"
            if dir.glob(pattern):
                found = True
                break

        if found:
            raise RuntimeError(f"The module binary exists at '{pattern}' but python failed to import it. Is there a Python version mismatch?")
        else:
            raise RuntimeError(f"Module import failed because no module found with '{pattern}'.")

    output_file.parent.mkdir(parents=True, exist_ok=True)
    
    # Remove existing output file if one exists
    if output_file.exists():
        output_file.unlink()
    
    # Generate new output
    generate_stub_for_c_module(module_name, str(output_file), known_modules=None)

    # Verify the output is present
    if not output_file.exists():
        raise RuntimeError(f"Failed to create stubs for module '{module_name}'")