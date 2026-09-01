import argparse
import logging
import os
import subprocess as sp
import shutil
from pathlib import Path

from docs import build_docs

import stubs

logging.basicConfig(level=logging.INFO)


def build(skybolt_source_dir: Path, skybolt_build_dir: Path, profile_filepath: Path, build_config_name: str):
    """
    Build Skybolt using conan. This will also build dependencies if required.
    """
    logging.info(f"Building...")
    
    sp.run(f"conan remote add skybolt-deps {skybolt_source_dir / 'Conan'} --force", shell=True, check=True)
    
    build_config_args = "-c skybolt/*:build_type=RelWithDebInfo -c skybolt-widgets/*:build_type=RelWithDebInfo" if build_config_name == "RelWithDebInfo" else ""
    
    sp.run(f"conan build {skybolt_source_dir} --output-folder={skybolt_build_dir} -pr:a=default -pr:a={profile_filepath} --build=missing", shell=True, check=True)


def copy_tree(source_dir: Path, destination_dir: Path):
    logging.info(f"Copying files from {source_dir} to {destination_dir}")
    shutil.copytree(source_dir, destination_dir, dirs_exist_ok=True, copy_function=shutil.copyfile)

def copy_file(source_filename: Path, destination_dir: Path):
    logging.info(f"Copying file {source_filename} to {destination_dir}")
    shutil.copy(source_filename, destination_dir)


def package(skybolt_source_dir: Path, skybolt_build_dir: Path, package_dir: Path, build_config_name: str):
    """
    Copy the build artifacts, dependency binaries, and required runtime resources to a stand-alone package folder.
    """
    logging.info(f"Packaging build...")

    if os.sys.platform.startswith('linux'):
        # FIXME: build dir is under config folder on Linux. We should update Windows to do this as well.
        skybolt_build_dir = skybolt_build_dir / build_config_name

    sp.run(f"cmake --install {skybolt_build_dir} --prefix {skybolt_build_dir}/install --config={build_config_name}", shell=True, check=True)
    sp.run(f"cmake --install {skybolt_build_dir} --prefix {skybolt_build_dir}/install --config={build_config_name} --component QtPlugins", shell=True, check=True)
    sp.run(f"cmake --install {skybolt_build_dir} --prefix {skybolt_build_dir}/install --config={build_config_name} --component OsgPlugins", shell=True, check=True)
    sp.run(f"cmake --install {skybolt_build_dir} --prefix {skybolt_build_dir}/install --config={build_config_name} --component SkyboltDependencies", shell=True, check=True)

    if os.sys.platform.startswith('linux'):
        skybolt_runtime_library_dir = skybolt_build_dir / "install/lib"
    else:
        skybolt_runtime_library_dir = skybolt_build_dir / "install/bin"
        
    if not os.sys.platform.startswith('linux'):
        # TODO: get this working on linux. It currently fails because Linux can't find the dependency .so files. Maybe we should set the rpaths at compile time.
        stubs.generate_skybolt_stubs("skybolt", [skybolt_runtime_library_dir], skybolt_runtime_library_dir / "skybolt.pyi")

    copy_tree(skybolt_build_dir / "install/bin", package_dir / "bin")

    if os.sys.platform.startswith('linux'):
        # Shared libraries on linux go in lib folder
        copy_tree(skybolt_build_dir / "install/lib", package_dir / "lib")

    copy_tree(skybolt_source_dir / "Assets", package_dir / "Assets")
    copy_tree(skybolt_source_dir / "Scenarios", package_dir / "Scenarios")
    copy_file(skybolt_source_dir / "License.txt", package_dir)
    copy_file(skybolt_source_dir / "README.md", package_dir)


def package_docs(skybolt_source_dir: Path, package_dir: Path):
    logging.info(f"Packaging docs...")
    skybolt_binary_dir = package_dir / "bin"
    output_dir = package_dir / "Docs" / "Skybolt"
    build_docs.build(skybolt_source_dir, skybolt_binary_dir, output_dir)

    # Delete unneeded output files
    (output_dir / "objects.inv").unlink(missing_ok=True)
    (output_dir / "sitemap.xml").unlink(missing_ok=True)
    (output_dir / "sitemap.xml.gz").unlink(missing_ok=True)
        

BUILD_STAGES = ["build", "package", "package_docs"]


if __name__ == "__main__":
    # Read command line arguments
    parser = argparse.ArgumentParser(description="Build and package Skybolt")
    parser.add_argument("--source-dir", type=str, help="Directory where skybolt source code is located")
    parser.add_argument("--build-dir", type=str, help="Directory to build to")
    parser.add_argument("--package-dir", type=str, help="Directory to output package to")
    parser.add_argument("--build-config-name", type=str, default="Release", help="Name of the build config to package")
    parser.add_argument("--profile-filepath", type=str, default=None, help="Optional filepath to a conan profile to use")
    parser.add_argument("--stage", type=str, default="package_docs", help="Name of the stage to run. Preceeding stages will also run unless --only flag is set.")
    parser.add_argument("--only", action="store_true", help="Only run the stage given in --stage, skipping preceeding stages"),

    args = parser.parse_args()

    profile_filepath = args.profile_filepath or Path(f"{args.source_dir}/Conan/Profiles/BuildEverythingWithPlugins.ini")

    if args.stage not in BUILD_STAGES:
        raise RuntimeError(f"No build stage named {args.stage}. Valid options are: {BUILD_STAGES}")

    def should_run_stage(name: str) -> bool:
        if args.only:
            return name == args.stage
        else:
            return BUILD_STAGES.index(name) <= BUILD_STAGES.index(args.stage)

    source_dir = Path(args.source_dir).resolve()
    build_dir = Path(args.build_dir).resolve()
    package_dir = Path(args.package_dir).resolve()
    conan_build_dir = build_dir / "build"
    skybolt_binary_dir = conan_build_dir / "bin" / args.build_config_name

    # Run build stages
    if should_run_stage("build"):
        build(source_dir, build_dir, profile_filepath, args.build_config_name)
    if should_run_stage("package"):
        package(source_dir, conan_build_dir, package_dir, args.build_config_name)
    if should_run_stage("package_docs"):
        package_docs(source_dir, package_dir)