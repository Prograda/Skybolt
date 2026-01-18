import os
import sys
from pathlib import Path

def extend_if_absent(sys_path: list, new_paths: list):
	for p in new_paths:
		abs_path = str(Path(p).resolve())
		if abs_path not in sys_path:
			sys_path.append(abs_path)

def setup_skybolt_environment():
	extend_if_absent(sys.path, os.getenv("SKYBOLT_PYTHON_PATH").split(os.pathsep))
	
	# PATH not automatically added since Python 3.8 and must be added manually
	if sys.platform == "win32":
		for path in os.environ["PATH"].split(os.pathsep):
			try:
				if path:
					path = path.strip('\'"')
					os.add_dll_directory(path)
			except FileNotFoundError:
				# Skip invalid directories
				pass
