from conans import ConanFile, CMake, tools

class ToolwindowmanagerConan(ConanFile):
    name = "toolwindowmanager"
    version = "1.0.0"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "*"

    generators = ["cmake_paths", "cmake_find_package"]

    requires = [
	    "qt/5.15.3@_/_"
    ]

    scm = {
        "type": "git",
        "url": "https://github.com/Piraxus/toolwindowmanager",
        "revision": "b0bf1c107a3f7ed3ca7821cbd3f53db13bd1cc0a"
    }

    def build(self):
        cmake = CMake(self)
        cmake.definitions["TWM_BUILD_EXAMPLE"] = "OFF"
        cmake.configure()
        cmake.build()
		
    def package(self):
        self.copy("*.h", dst="include/ToolWindowManager", src="src")
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "ToolWindowManager"
        self.cpp_info.libs = ["toolwindowmanager"]