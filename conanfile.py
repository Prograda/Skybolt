from conans import ConanFile, CMake
import os

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.6.0"
    settings = "os", "compiler", "arch", "build_type"
    options = {
        "enable_bullet": [True, False],
        "enable_cigi": [True, False],
        "enable_fft_ocean": [True, False],
        "enable_map_features_converter": [True, False],
        "enable_python": [True, False],
        "enable_qt": [True, False],
        "shared": [True, False],
        "shared_plugins": [True, False], # Build plugins as shared libraries
        "fPIC": [True, False]
    }
    default_options = {
        "enable_bullet": False,
        "enable_cigi": False,
        "enable_fft_ocean": True,
        "enable_map_features_converter": True,
        "enable_python": False,
        "enable_qt": False,
        "qt:shared": True, # Use shared Qt to avoid Qt's LGPL viral static linking
        "shared": False,
        "shared_plugins": True,
        "fPIC": True
    }
    generators = ["cmake_paths", "cmake_find_package", "virtualrunenv"]
    exports = "Conan/*"
    exports_sources = "*"
    no_copy_source = True

    requires = [
        "boost/1.75.0@_/_#989077de56cb85b727be210b5827d52f",
        "catch2/2.13.8@_/_#ac821c6881627aece6c7063bd5aa73ea",
        "cpp-httplib/0.10.1@_/_#5078af8ecb0001ebdc8c799d38ac9b16",
        "earcut/2.2.3@_/_#7c612e8a3119c4cbf446b7096ecf2831",
		"expat/2.5.0@_/_#91e43e4544923e4c934bfad1fa4306f9", # Indirect dependency to resolve version conflict between readosm and fontconfig
        "glm/0.9.9.8@_/_#550ca1927ce2617e53b4514cff326923",
        "nlohmann_json/3.10.5@_/_#1ebcf334c3f52d96e057d5aba398c491",
        "zlib/1.2.13@_/_#97d5730b529b4224045fe7090592d4c1" # Indirect dependency to resolve version conflict between libpng and boost
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
            self.requires("bullet3/3.22a@_/_#29b44d5d03e941af7f47285c379fef16")

        if self.options.enable_cigi:
            self.include_package("cigicl", "4.0.6a")

        if self.options.enable_fft_ocean:
            self.include_package("mufft", "1.0.0")
            self.include_package("xsimd", "7.4.10")

        if self.options.enable_map_features_converter:
            self.requires("readosm/1.1.0a@_/_#23e27a65a8846ce66e7f151d95c9229f")

        if self.options.enable_python:
            self.requires("pybind11/2.9.1@_/_#017b6606f856caa02c085b034720791e")
            
        if self.options.enable_qt:
            self.requires("qt/5.15.11@_/_#64fc18b0c5ab189f347993a9853144ad")
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

        if self.options.enable_map_features_converter:
            cmake.definitions["BUILD_MAP_FEATURES_CONVERTER"] = "true"

        if self.options.enable_python:
            cmake.definitions["BUILD_PYTHON_BINDINGS"] = "true"

        if self.options.enable_qt:
            cmake.definitions["BUILD_SKYBOLT_QT"] = "true"

        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = "conan_paths.cmake"
        cmake.configure()
        cmake.build()
		
    def package(self):
        cmake = CMake(self)
        cmake.install()
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "Skybolt"
        self.cpp_info.libs = ["AircraftHud", "SkyboltEngine", "SkyboltVis", "SkyboltSim", "SkyboltReflection", "SkyboltCommon"]
		
        if self.options.enable_fft_ocean and not self.options.shared_plugins:
            self.cpp_info.libs.append("FftOcean")
        if self.options.enable_qt == True:
            self.cpp_info.libs.append("SkyboltQt")
