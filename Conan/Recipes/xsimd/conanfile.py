import os
from conan import ConanFile
from conan.tools import files
from conan.tools.scm import Git

class XsimdConan(ConanFile):
    name = "xsimd"
    version = "7.4.10"
    exports_sources = "*"
    no_copy_source = True

    def source(self):
        git = Git(self, folder=self.name)
        git.clone('https://github.com/xtensor-stack/xsimd/', target=".")
        git.checkout(self.version)

    def package(self):
        files.copy(self, pattern="*", src=os.path.join(self.source_folder, "xsimd/include"), dst=os.path.join(self.package_folder, "include"))
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]