from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
import os

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.7.0"
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
        "enable_python": True,
        "enable_qt": True,
        "shared": False,
        "shared_plugins": True,
        "fPIC": True,
        "qt/*:shared": True, # Use shared Qt to avoid Qt's LGPL viral static linking
    }
    generators = ["VirtualRunEnv"]
    exports = "Conan/*"
    exports_sources = "*"
    no_copy_source = True

    requires = [
        "boost/1.75.0",
        "catch2/2.13.8",
        "cpp-httplib/0.10.1",
        "earcut/2.2.3",
        "glm/0.9.9.8",
        "nlohmann_json/3.10.5",
	]

    def include_package(self, name, version, subfolder=None):
        currentDir = os.path.dirname(os.path.abspath(__file__))
        recipes_path = os.path.join(currentDir, "Conan/Recipes", name)
        if (subfolder):
            recipes_path = os.path.join(recipes_path, subfolder)
            
        self.run(f"conan export --version {version} .", cwd=recipes_path)
        self.requires((f"{name}/{version}"))

    def configure(self):
        self.options["openscenegraph-mr"].with_curl = True # Required for loading terrain tiles from http sources
        self.options["bullet3"].double_precision = True
        if self.settings.compiler == 'msvc':
            del self.options.fPIC

    def requirements(self):
        self.include_package("cxxtimer", "1.0.0")
        self.include_package("px_sched", "1.0.0")
        self.include_package("openscenegraph-mr", "3.7.0", "all")

        if self.options.enable_bullet:
            self.requires("bullet3/3.22a")

        if self.options.enable_cigi:
            self.include_package("cigicl", "4.0.6a")

        if self.options.enable_fft_ocean:
            self.include_package("mufft", "1.0.0")
            self.include_package("xsimd", "7.4.10")

        if self.options.enable_map_features_converter:
            self.requires("readosm/1.1.0a")

        if self.options.enable_python:
            self.requires("pybind11/2.13.6")
            
        if self.options.enable_qt:
            self.requires("qt/5.15.14")
            self.include_package("toolwindowmanager", "1.0.0")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["Boost_STATIC_LIBS"] = str(not self.dependencies["boost"].options.shared)
        tc.variables["OSG_STATIC_LIBS"] = str(not self.dependencies["openscenegraph-mr"].options.shared)
        tc.variables["SKYBOLT_PLUGINS_STATIC_BUILD"] = str(not self.options.shared_plugins)
        tc.variables["Skybolt_VERSION"] = self.version
        if "fPIC" in self.options:
            tc.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC

        if self.options.enable_bullet:
            tc.variables["BUILD_BULLET_PLUGIN"] = "true"

        if self.options.enable_cigi:
            tc.variables["BUILD_CIGI_COMPONENT_PLUGIN"] = "true"

        if self.options.enable_fft_ocean:
            tc.variables["BUILD_FFT_OCEAN_PLUGIN"] = "true"

        if self.options.enable_map_features_converter:
            tc.variables["BUILD_MAP_FEATURES_CONVERTER"] = "true"

        if self.options.enable_python:
            tc.variables["BUILD_PYTHON_BINDINGS"] = "true"
            tc.variables["BUILD_PYTHON_PLUGIN"] = "true"

        if self.options.enable_qt:
            tc.variables["BUILD_SKYBOLT_QT"] = "true"

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
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "Skybolt"
        self.cpp_info.libs = ["AircraftHud", "SkyboltEngine", "SkyboltVis", "SkyboltSim", "SkyboltReflection", "SkyboltCommon"]
        self.cpp_info.builddirs = ["CMake"]
		
        if self.options.enable_fft_ocean and not self.options.shared_plugins:
            self.cpp_info.libs.append("FftOcean")
        if self.options.enable_qt == True:
            self.cpp_info.libs.append("SkyboltQt")
