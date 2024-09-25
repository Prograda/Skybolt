import argparse
import os
import subprocess as sp
import time
import webbrowser
from pathlib import Path

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Start mkdocs server for interactive editing")
    parser.add_argument("skybolt_source_dir", type=str, help="Directory where skybolt source code is located")
    parser.add_argument("skybolt_binary_dir", type=str, help="Directory where the compiled skybolt python libraries are located")

    args = parser.parse_args()

    source_dir = Path(args.skybolt_source_dir)
    binary_dir = Path(args.skybolt_binary_dir)
    python_source_dir = source_dir / "Assets/Core/Scripts"

    env = os.environ.copy()
    env["PYTHONPATH"] = os.pathsep.join([str(python_source_dir), str(binary_dir)])

    process = sp.Popen(["mkdocs", "serve"], cwd=str(source_dir), stdout=sp.PIPE, stderr=sp.PIPE, text=True, env=env)

    while True:
        output = process.stderr.readline()
        print(output, end="")
        if output == "" and process.poll() is not None:
            break
        if "Serving on" in output:
            # Open the web browser once the server is running
            url = output.split("Serving on ", 1)[1].strip()
            webbrowser.open(url)
            print("Server is up! Opening web browser. Press Ctrl-C to stop server.")
            break
        time.sleep(0.1)

    process.wait()
