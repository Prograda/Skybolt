from conans import ConanFile, CMake, tools

class CigiclConan(ConanFile):
    name = "cigicl"
    version = "4.0.6a"

    licence = "LGPL 2.1"
    author = "The Boeing Company"
    description = "SDK used to create and format CIGI compliant messages"
    url = "https://sourceforge.net/projects/cigi"
    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "*"

    options = {
        "shared": [True, False],
        "fPIC": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True
    }

    def source(self):
        url = "https://versaweb.dl.sourceforge.net/project/cigi/CIGI%20Class%20Library%20%28CCL%29/CCL%20Version%204.0.6a/ccl_4_0%20rev6a.zip"
        sha = "a55099c0418c663e572109bd62ed2f7b411c51af7b404212988e36360e8a13cc"
        tools.get(url, sha256=sha, strip_root=True)

    def configure(self):
        if self.settings.compiler == 'Visual Studio':
            del self.options.fPIC

    def build(self):
        cmake = CMake(self)
		
        if "fPIC" in self.options:
            cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC
		
        cmake.configure()
        if self.options.shared:
            cmake.build(target="cigicl-shared")
        else:
            cmake.build(target="cigicl-static")
		
    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        if self.options.shared:
            self.cpp_info.libs = ["ccl_dll"]
            self.cpp_info.defines = ["CCL_DLL"]
        else:
            self.cpp_info.libs = ["ccl_lib"]
