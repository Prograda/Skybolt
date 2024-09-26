import argparse
import fnmatch
import logging
import os
import subprocess as sp
import shutil
from pathlib import Path

logging.basicConfig(level=logging.INFO)


def download_resources(resources_dir: Path):
    """
    Download resources (asset packages etc) which Skybolt requires at runtime.
    """
    if not (resources_dir / "SkyboltAssets").exists():
        sp.run(f"git clone https://github.com/Prograda/SkyboltAssets", cwd=f"{resources_dir}", shell=True, check=True)

    sp.run(f"dvc pull", cwd=f"{resources_dir}/SkyboltAssets", shell=True, check=True)


def build(skybolt_source_dir: Path, skybolt_build_dir: Path):
    """
    Build Skybolt using conan. This will also build dependencies if required.
    """
    sp.run(f"conan build {skybolt_source_dir} --output-folder={skybolt_build_dir}  -o openscenegraph-mr/*:shared=True -o enable_python=True -o enable_qt=True -o enable_bullet=True -o enable_cigi=True --build=missing -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True", shell=True, check=True)


LIBRARY_FILTER_PATTERNS = ["*.dll", "*.so", "*.dylib"]


def copy_tree(source_dir: Path, destination_dir: Path):
    logging.debug(f"Copying binaries from {source_dir} to {destination_dir}")
    shutil.copytree(source_dir, destination_dir, dirs_exist_ok=True)


def package(skybolt_source_dir: Path, skybolt_build_dir: Path, resources_dir: Path, package_dir: Path):
    """
    Copy the build artifacts, dependency binaries, and required runtime resources to a stand-alone package folder.
    """
    sp.run(f"cmake --install {skybolt_build_dir} --prefix {skybolt_build_dir}/install", shell=True, check=True)

    copy_tree(skybolt_build_dir / "install/bin", package_dir / "bin")
    copy_tree(skybolt_source_dir / "Assets", package_dir / "Assets")
    copy_tree(resources_dir / "SkyboltAssets/Assets", package_dir / "Assets")


BUILD_STAGES = ["download_resources", "build", "package"]


if __name__ == "__main__":
    # Read command line arguments
    parser = argparse.ArgumentParser(description="Build and package Skybolt")
    parser.add_argument("skybolt_source_dir", type=str, help="Directory where skybolt source code is located")
    parser.add_argument("output_dir", type=str, help="Directory to output package to")
    parser.add_argument("--stage", type=str, default="package", help="Name of the stage to run. Preceeding stages will also run unless --only flag is set.")
    parser.add_argument("--only", action="store_true", help="Only run the stage given in --stage, skipping preceeding stages"),

    args = parser.parse_args()

    skybolt_source_dir = Path(args.skybolt_source_dir)
    output_dir = Path(args.output_dir)
    skybolt_build_dir = output_dir / "Build"
    package_dir = output_dir / "Package"

    if args.stage not in BUILD_STAGES:
        raise RuntimeError(f"No build stage named {args.stage}. Valid options are: {BUILD_STAGES}")

    def should_run_stage(name: str) -> bool:
        if args.only:
            return name == args.stage
        else:
            return BUILD_STAGES.index(name) <= BUILD_STAGES.index(args.stage)

    # Create output directory
    output_dir.mkdir(exist_ok=True)

    # Run build stages
    if should_run_stage("download_resources"):
        download_resources(output_dir)
    if should_run_stage("build"):
        build(skybolt_source_dir, skybolt_build_dir)
    if should_run_stage("package"):
        package(skybolt_source_dir, skybolt_build_dir, output_dir, package_dir)
