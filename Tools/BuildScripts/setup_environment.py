import os
import sys
from pathlib import Path


def setup_skybolt_environment():

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
