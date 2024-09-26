from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools import files

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
        files.get(self, url, sha256=sha, strip_root=True)
        
        # Patch CMakeLists.txt:
        # PROJECT definition should come after CMAKE_MINIMUM_REQUIRED.
        # CMake version should be at least 3.15 to work with Conan.
        files.replace_in_file(self, "CMakeLists.txt", "PROJECT(ccl)", "")
        files.replace_in_file(self, "CMakeLists.txt", "CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7)", "CMAKE_MINIMUM_REQUIRED(VERSION 3.15)\nPROJECT(ccl)")
        # Replace backslashes with forward slashes to prevent unintentional escaping
        files.replace_in_file(self, "CMakeLists.txt", "Files\\", "Files/")

    def configure(self):
        if self.settings.compiler == 'msvc':
            del self.options.fPIC

    def generate(self):
        tc = CMakeToolchain(self)

        if "fPIC" in self.options:
            tc.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)	
        cmake.configure()
        cmake.build()
		
    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        if self.options.shared:
            self.cpp_info.libs = ["ccl_dll"]
            self.cpp_info.defines = ["CCL_DLL"]
        else:
            self.cpp_info.libs = ["ccl_lib"]
