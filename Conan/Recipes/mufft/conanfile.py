from conans import ConanFile, CMake, tools

class MufftConan(ConanFile):
    name = "mufft"
    version = "1.0.0"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "*"

    scm = {
        "type": "git",
        "url": "https://github.com/Themaister/muFFT",
        "revision": "47bb08652eab399c2c7d460abe5184857110f130"
    }

    def source(self):
        f = open(self.source_folder + "/CMakeLists.txt", 'a')
        f.write("install(TARGETS muFFT muFFT-sse muFFT-sse3 muFFT-avx)")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
		
    def package(self):
        self.copy("*.h", dst="include/muFFT")
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "muFFT"
        self.cpp_info.libs = ["muFFT", "muFFT-avx", "muFFT-sse", "muFFT-sse3"]