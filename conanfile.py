from conans import ConanFile, CMake
import os

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.4.0"
    settings = "os", "compiler", "arch", "build_type"
    options = {
		"shared": [True, False],
		"enableFftOcean": [True, False],
		"shared_plugins": [True, False] # Build plugins as shared libraries
	}
    default_options = {
        "shared": False,
        "enableFftOcean": True,
		"shared_plugins": True
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
		"zlib/1.2.12@_/_" # Indirect dependency. Specified to resolve version clash between boost and freetype.
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

    def requirements(self):
        self.include_package("cxxtimer", "1.0.0")
        self.include_package("px_sched", "1.0.0")
        self.include_package("openscenegraph-mr", "3.7.0", "all")
		
        if self.options.enableFftOcean:
            self.include_package("mufft", "1.0.0")
            self.include_package("xsimd", "7.4.10")

    def build(self):
        cmake = CMake(self)

        cmake.definitions["Boost_STATIC_LIBS"] = str(not self.options["boost"].shared)
        cmake.definitions["OSG_STATIC_LIBS"] = str(not self.options["openscenegraph-mr"].shared)
        cmake.definitions["SKYBOLT_PLUGINS_STATIC_BUILD"] = str(not self.options.shared_plugins)

        if self.options.enableFftOcean == True:
            cmake.definitions["BUILD_FFT_OCEAN_PLUGIN"] = "true"

        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = "conan_paths.cmake"
        cmake.configure()
        cmake.build()
		
    def package(self):
        cmake = CMake(self)
        cmake.install()
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "Skybolt"
        self.cpp_info.libs = ["AircraftHud", "MapAttributesConverter", "SkyboltCommon", "SkyboltEngine", "SkyboltSim", "SkyboltVis"]
		
        if self.options.enableFftOcean and not self.options.shared_plugins:
            self.cpp_info.libs.append("FftOcean")