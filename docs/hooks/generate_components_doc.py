import json
import os
import subprocess
import tempfile
from jinja2 import Template
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.resolve()

def deep_merge(dict1, dict2):
    """Recursively merges dict2 into dict1."""
    for key, value in dict2.items():
        # If both values are dictionaries, merge them recursively
        if key in dict1 and isinstance(dict1[key], dict) and isinstance(value, dict):
            deep_merge(dict1[key], value)
        else:
            # Otherwise, overwrite or add the value
            dict1[key] = value
    return dict1

def run_skybolt_and_dump_components(output_filepath: Path):
    try:
        # Run a command (e.g., listing directory contents)
        args = ["skybolt", "component", "dump", str(output_filepath)]
        result = subprocess.run(
            args, 
            check=True, 
            text=True,           # Decodes stdout/stderr as strings instead of bytes
            capture_output=True  # Captures stdout and stderr
        )
        
        print("Command output:")
        print(result.stdout)

    except subprocess.CalledProcessError as e:
        print(f"Error output: {e.stderr}")
        raise RuntimeError(f"Command '{args}' failed with exit code {e.returncode}", e)
    except FileNotFoundError as e:
        raise RuntimeError(f"Failed to find '{args[0]}' executable", e)
    except Exception as e:
        raise RuntimeError(f"Command '{args}' failed with exit code {e.returncode}", e)
        

def on_page_markdown(markdown, page, config, files):
    print(page)
    if page.title != "Entities":
        return markdown
    
    with tempfile.TemporaryDirectory() as temp_dir:
        filename = Path(temp_dir) / "components.json"

        run_skybolt_and_dump_components(filename)

        with open(filename, 'r') as f:
            components_string = f.read()
        module_components = json.loads(components_string)["module_components"]

        components_string_extra = (SCRIPT_DIR / "component_parser.json").open(mode="r", encoding="utf-8").read()
        module_components_extra = json.loads(components_string_extra)["module_components"]

        module_components = deep_merge(module_components, module_components_extra)

        # Sort modules so that items begining with "_" appear first.
        # We use this to denote core modules as opposed to plugins.
        module_components = dict(sorted(
            module_components.items(),
            key=lambda item: (0 if item[0].startswith('_') else 1, item[0].lower())
        ))

        # Remove modules with no components
        module_components = {k: v for k, v in module_components.items() if len(v) > 0}

        # Generate content
        template = Template(markdown)
        return template.render(module_components=module_components)
