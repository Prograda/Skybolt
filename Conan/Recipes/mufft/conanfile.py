import os
from conan import ConanFile
from conan.tools import files
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.scm import Git

class MufftConan(ConanFile):
    name = "mufft"
    version = "1.0.0"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "*"

    def source(self):
        git = Git(self, folder=self.name)
        git.clone('https://github.com/Themaister/muFFT', target=".")
        git.checkout("47bb08652eab399c2c7d460abe5184857110f130")

        # Patch CMakeLists.txt:
        # CMake version should be at least 3.15 to work with Conan.
        files.replace_in_file(self, "mufft/CMakeLists.txt", "cmake_minimum_required(VERSION 3.5)", "cmake_minimum_required(VERSION 3.15)")
        # Add install command at end of file
        f = open(self.source_folder + "/mufft/CMakeLists.txt", 'a')
        f.write("install(TARGETS muFFT muFFT-sse muFFT-sse3 muFFT-avx)")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder=self.name)
        cmake.build()
		
    def package(self):
        files.copy(self, pattern="*.h", src=os.path.join(self.source_folder, self.name), dst=os.path.join(self.package_folder, "include/muFFT"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "muFFT"
        self.cpp_info.libs = ["muFFT", "muFFT-avx", "muFFT-sse", "muFFT-sse3"]