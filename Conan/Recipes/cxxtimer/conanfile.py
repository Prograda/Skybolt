from conans import ConanFile, CMake, tools

class CxxtimerConan(ConanFile):
    name = "cxxtimer"
    version = "1.0.0"
    exports_sources = "*"
    no_copy_source = True

    def source(self):
        git = tools.Git(folder=self.name)
        git.clone('https://github.com/andremaravilha/cxxtimer', branch="v" + self.version)

    def package(self):
        self.copy("*.hpp", "include")
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]