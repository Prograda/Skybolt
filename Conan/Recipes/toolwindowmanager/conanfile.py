import os
from conan import ConanFile
from conan.tools import files
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.scm import Git

class ToolwindowmanagerConan(ConanFile):
    name = "toolwindowmanager"
    version = "1.0.0"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "*"

    requires = [
	    "qt/5.15.14"
    ]

    def source(self):
        git = Git(self, folder=self.name)
        git.clone('https://github.com/Piraxus/toolwindowmanager', target=".")
        git.checkout("4e948817e14355d3c6df2fa90a3bf5cd68257eab")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["TWM_BUILD_EXAMPLE"] = "OFF"
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder=self.name)
        cmake.build()
		
    def package(self):
        files.copy(self, pattern="*.h", src=os.path.join(self.source_folder, "toolwindowmanager/src"), dst=os.path.join(self.package_folder, "include/ToolWindowManager"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "ToolWindowManager"
        self.cpp_info.libs = ["toolwindowmanager"]