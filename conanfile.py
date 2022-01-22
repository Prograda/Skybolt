from conans import ConanFile, CMake
import os

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.1"
    settings = "os", "compiler", "arch", "build_type"
    options = {
		"shared": [True, False],
		"enableFftOcean": [True, False]
	}
    default_options = {
        "shared": False,
        "enableFftOcean": True
    }
    generators = ["cmake_paths", "cmake_find_package", "virtualrunenv"]
    exports = "Conan/*"
    exports_sources = "*"

    requires = [
		"boost/1.75.0@_/_",
		"catch2/2.13.8@_/_",
		"cpp-httplib/0.10.1@_/_",
		"earcut/2.2.3@_/_",
		"glm/0.9.9.8@_/_",
		"nlohmann_json/3.10.5@_/_",
		"openscenegraph/3.6.5@_/_"
	]

    def configure(self):
	    self.options["openscenegraph"].shared = True

    def include_package(self, name, version):
        currentDir = os.path.dirname(os.path.abspath(__file__))
        recipes_path = os.path.join(currentDir, "Conan/Recipes", name)
            
        self.run("conan export . user/stable", cwd=recipes_path)
        self.requires(("%s/%s@user/stable" % (name, version)))

    def requirements(self):
        self.include_package("cxxtimer", "1.0.0")
        self.include_package("px_sched", "1.0.0")
		
        if self.options.enableFftOcean:
            self.include_package("xsimd", "7.4.10")

    def build(self):
        cmake = CMake(self)

        if self.options.shared == False:
            cmake.definitions["Boost_STATIC_LIBS"] = "true"
			
        if self.options.enableFftOcean == True:
            cmake.definitions["BUILD_FFT_OCEAN_PLUGIN"] = "true"
			
        cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = "conan_paths.cmake"
        cmake.configure()
        cmake.build()