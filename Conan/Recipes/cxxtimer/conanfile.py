import os
from conan import ConanFile
from conan.tools import files
from conan.tools.scm import Git

class CxxtimerConan(ConanFile):
    name = "cxxtimer"
    version = "1.0.0"
    exports_sources = "*"
    no_copy_source = True

    def source(self):
        git = Git(self, folder=self.name)
        git.clone('https://github.com/andremaravilha/cxxtimer', target=".")
        git.checkout("v" + self.version)

    def package(self):
        files.copy(self, pattern="*.hpp", src=self.source_folder, dst=os.path.join(self.package_folder, "include"))
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]