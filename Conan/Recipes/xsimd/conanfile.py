from conans import ConanFile, CMake, tools

class XsimdConan(ConanFile):
    name = "xsimd"
    version = "7.4.10"
    exports_sources = "*"
    no_copy_source = True

    scm = {
        "type": "git",
        "url": "https://github.com/xtensor-stack/xsimd/",
        "revision": version
    }

    def package(self):
        self.copy("include/*")
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]