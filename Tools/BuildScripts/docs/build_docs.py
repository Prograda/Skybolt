import argparse
import os
import subprocess as sp
from pathlib import Path


def add_to_path(env, directory: str, prepend: bool = True):
  """Adds a directory to os.environ['PATH'] safely without duplicates."""
  target = os.path.normcase(os.path.abspath(directory))
  paths = [os.path.abspath(p) for p in env.get("PATH", "").split(os.pathsep) if p]
  
  # Filter out the target if it already exists, then insert/append
  filtered = [p for p in paths if os.path.normcase(p) != target]
  updated = [directory] + filtered if prepend else filtered + [directory]
  
  env["PATH"] = os.pathsep.join(updated)

def build(source_dir: Path, skybolt_binary_dir: Path, output_dir: Path, deploy_to_github=False):
    python_source_dir = source_dir / "Assets/Core/Scripts"

    env = os.environ.copy()
    env["PYTHONPATH"] = os.pathsep.join([str(python_source_dir.resolve()), str(skybolt_binary_dir.resolve())])
    add_to_path(env, str(skybolt_binary_dir.resolve()))

    try:
        command = "gh-deploy" if deploy_to_github else "build"
        commandline = f"mkdocs {command} --site-dir {output_dir} --clean"
        sp.run(commandline, cwd=str(source_dir), env=env, shell=True, check=True)
    except RuntimeError as e:
        raise RuntimeError(f"Command '{commandline}' failed", e)


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