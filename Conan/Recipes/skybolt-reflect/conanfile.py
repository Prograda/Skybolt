import os
from conan import ConanFile
from conan.tools import files
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.scm import Git

class SkybolyReflectConan(ConanFile):
    implements = ["auto_shared_fpic"]
    name = "skybolt-reflect"
    version = "1.0.0"
    settings = "os", "compiler", "arch", "build_type"
    options = {
        "shared": [True, False],
		"fPIC": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True
    }
    exports_sources = "*"
    no_copy_source = True

    def requirements(self):
        self.requires("catch2/2.13.8")

    def source(self):
        git = Git(self, folder=self.name)
        git.clone('https://github.com/prograda/skybolt-reflect', target=".")
        git.checkout("ec98ac21451c5ae774a5dd8cd55f57a499a74f69")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
		
        if "fPIC" in self.options:
            tc.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC
		
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder=self.name)
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libs = ["SkyboltReflect"]