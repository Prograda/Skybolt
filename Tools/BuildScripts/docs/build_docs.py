import argparse
import os
import subprocess as sp
from pathlib import Path

def build(source_dir: Path, output_dir: Path, deploy_to_github=False):
    python_source_dir = source_dir / "Assets/Core/Scripts"

    env = os.environ.copy()
    env["PYTHONPATH"] = os.pathsep.join([str(python_source_dir)])

    # Create dummy skybolt python module so that mkdocs collects the skybolt.pyi file to generate interface documentation.
    # This is not required if the compile python .pyd file is on the PYTHONPATH, but we don't assume this is the case as
    # skybolt might not have been compiled.
    skybolt_module_filename = (python_source_dir / "skybolt.py")
    skybolt_module_filename.touch()
    skybolt_module_filename.write_text("#This is a temporary file used as a stub for generating documentation. It should not exist except for when building docs.")

    try:
        command = "gh-deploy" if deploy_to_github else "build"
        sp.run(f"mkdocs {command} --site-dir {output_dir} --dirty", cwd=str(source_dir), env=env, shell=True, check=True)
    except RuntimeError as e:
        raise e
    finally:
        skybolt_module_filename.unlink()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate mkdocs")
    parser.add_argument("skybolt_source_dir", type=str, help="Directory where skybolt source code is located")
    parser.add_argument("output_dir", type=str, help="Directory to output the docs to")
    parser.add_argument("--deploy_to_github", action="store_true", help="Deploy the docs to github")

    args = parser.parse_args()

    source_dir = Path(args.skybolt_source_dir)
    output_dir = Path(args.output_dir)
    print(args.deploy_to_github)
    build(source_dir, output_dir, deploy_to_github=args.deploy_to_github)