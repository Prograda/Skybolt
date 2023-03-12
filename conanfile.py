from conans import ConanFile, CMake
import os

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.5.0"
    settings = "os", "compiler", "arch", "build_type"
    options = {
        "enable_bullet": [True, False],
        "enable_cigi": [True, False],
        "enable_fft_ocean": [True, False],
        "enable_python": [True, False],
        "enable_sprocket": [True, False],
        "shared": [True, False],
        "shared_plugins": [True, False], # Build plugins as shared libraries
        "fPIC": [True, False]
    }
    default_options = {
        "enable_bullet": False,
        "enable_cigi": False,
        "enable_fft_ocean": True,
        "enable_python": False,
        "enable_sprocket": False,
        "qt:shared": True, # Use shared Qt to avoid Qt's LGPL viral static linking
        "qt:qtsvg":True, # Build the Qt SVG plugin to display Sprocket icons
        "qwt:shared": True,
        "shared": False,
        "shared_plugins": True,
        "fPIC": True
    }
    generators = ["cmake_paths", "cmake_find_package", "virtualrunenv"]
    exports = "Conan/*"
    exports_sources = "*"
    no_copy_source = True

    requires = [
        "boost/1.75.0@_/_",
        "catch2/2.13.8@_/_",
        "cpp-httplib/0.10.1@_/_",
        "earcut/2.2.3@_/_",
        "glm/0.9.9.8@_/_",
        "nlohmann_json/3.10.5@_/_",
        "zlib/1.2.13@_/_", # Indirect dependency. Specified to resolve version clash between qt and openscenegraph.
        "libjpeg/9e@_/_", # Indirect dependency. Specified to resolve version clash between qt and openscenegraph.
        "zstd/1.5.4" # Indirect dependency. Specified to resolve version clash between libtiff and libmysqlclient.
	]

    def include_package(self, name, version, subfolder=None):
        currentDir = os.path.dirname(os.path.abspath(__file__))
        recipes_path = os.path.join(currentDir, "Conan/Recipes", name)
        if (subfolder):
            recipes_path = os.path.join(recipes_path, subfolder)
            
        self.run(("conan export . %s/%s@user/stable" % (name, version)), cwd=recipes_path)
        self.requires(("%s/%s@user/stable" % (name, version)))

    def configure(self):
        self.options["openscenegraph-mr"].with_curl = True # Required for loading terrain tiles from http sources
        self.options["bullet3"].double_precision = True
        if self.settings.compiler == 'Visual Studio':
            del self.options.fPIC

    def requirements(self):
        self.include_package("cxxtimer", "1.0.0")
        self.include_package("px_sched", "1.0.0")
        self.include_package("openscenegraph-mr", "3.7.0", "all")

        if self.options.enable_bullet:
            self.requires("bullet3/3.22a@_/_")

        if self.options.enable_cigi:
            self.include_package("cigicl", "4.0.6a")

        if self.options.enable_fft_ocean:
            self.include_package("mufft", "1.0.0")
            self.include_package("xsimd", "7.4.10")

        if self.options.enable_python:
            self.requires("pybind11/2.9.1@_/_")
            
        if self.options.enable_sprocket:
            self.requires("expat/2.4.8@_/_") # Indirect dependency. Specified to resolve version clash between wayland (used by Qt) and fontconfig (used by OSG)
            self.requires("openssl/1.1.1s@_/_") # Indirect dependency. Specified to resolve version clash between qt/5.15.3 and libcurl/7.83.1
            self.requires("ois/1.5@_/_")
            self.requires("qt/5.15.3@_/_")
            self.requires("qwt/6.1.6@_/_")
            self.include_package("toolwindowmanager", "1.0.0")

    def build(self):
        cmake = CMake(self)

        cmake.definitions["Boost_STATIC_LIBS"] = str(not self.options["boost"].shared)
        cmake.definitions["OSG_STATIC_LIBS"] = str(not self.options["openscenegraph-mr"].shared)
        cmake.definitions["SKYBOLT_PLUGINS_STATIC_BUILD"] = str(not self.options.shared_plugins)
        if "fPIC" in self.options:
            cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC

        if self.options.enable_bullet:
            cmake.definitions["BUILD_BULLET_PLUGIN"] = "true"

        if self.options.enable_cigi:
            cmake.definitions["BUILD_CIGI_COMPONENT_PLUGIN"] = "true"

        if self.options.enable_fft_ocean:
            cmake.definitions["BUILD_FFT_OCEAN_PLUGIN"] = "true"

        if self.options.enable_python:
            cmake.definitions["BUILD_PYTHON_BINDINGS"] = "true"
            cmake.definitions["BUILD_PYTHON_COMPONENT_PLUGIN"] = "true"
            
        if self.options.enable_sprocket:
            cmake.definitions["BUILD_SEQUENCE_EDITOR_PLUGIN"] = "true"
            cmake.definitions["BUILD_SPROCKET"] = "true"

        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = "conan_paths.cmake"
        cmake.configure()
        cmake.build()
		
    def package(self):
        cmake = CMake(self)
        cmake.install()
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "Skybolt"
        self.cpp_info.libs = ["AircraftHud", "SkyboltEngine", "SkyboltVis", "SkyboltSim", "SkyboltCommon"]
		
        if self.options.enable_fft_ocean and not self.options.shared_plugins:
            self.cpp_info.libs.append("FftOcean")
        if self.options.enable_sprocket == True:
            self.cpp_info.libs.append("Sprocket")
