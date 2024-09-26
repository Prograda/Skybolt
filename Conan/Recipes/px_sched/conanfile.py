import os
from conan import ConanFile
from conan.tools import files
from conan.tools.scm import Git

class PxschedConan(ConanFile):
    name = "px_sched"
    version = "1.0.0"
    exports_sources = "*"
    no_copy_source = True

    def source(self):
        git = Git(self, folder=self.name)
        git.clone('https://github.com/pplux/px', target=".")
        git.checkout("22a12e2039e62605692c06f0ede5a09277579bbf")

    def package(self):
        files.copy(self, pattern="*.h", src=os.path.join(self.source_folder, self.name), dst=os.path.join(self.package_folder, "include/px_sched"))
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]