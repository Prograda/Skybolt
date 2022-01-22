from conans import ConanFile, CMake, tools

class PxschedConan(ConanFile):
    name = "px_sched"
    version = "1.0.0"
    exports_sources = "*"
    no_copy_source = True

    scm = {
        "type": "git",
        "url": "https://github.com/pplux/px",
        "revision": "22a12e2039e62605692c06f0ede5a09277579bbf"
    }

    def package(self):
        self.copy("px_sched.h", "include/px_sched")
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]