from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
import os
from pathlib import Path

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.8.0"
    settings = "os", "compiler", "arch", "build_type"
    options = {
        "enable_boost_log": [True, False],
        "enable_bullet": [True, False],
        "enable_fft_ocean": [True, False],
        "enable_jsbsim": [True, False],
        "enable_map_features_converter": [True, False],
        "enable_osg_curl_plugin": [True, False], # Whether to link the OSG curl plugin into the static build. Only relavent if shared = False.
        "enable_python": [True, False],
        "enable_qt": [True, False],
        "shared": [True, False],
        "shared_plugins": [True, False], # Build plugins as shared libraries.
        "fPIC": [True, False]
    }
    default_options = {
        "enable_boost_log": True,
        "enable_bullet": False,
        "enable_osg_curl_plugin": True,
        "enable_fft_ocean": True,
		"enable_jsbsim": False,
        "enable_map_features_converter": True,
        "enable_python": True,
        "enable_qt": True,
        "shared": False,
        "shared_plugins": True,
        "fPIC": True
    }
    exports_sources = (
        "CMake/*",
        "src/*",
        ".python-version",
        "CMakeLists.txt",
        "conanfile.py",
        "pyproject.toml",
        "uv.lock"
    )
    no_copy_source = True

    implements = ["auto_shared_fpic"]

    def configure(self):
        if self.options.get_safe("shared"):
            self.options.rm_safe("fPIC")

        self.options["openscenegraph-mr"].with_curl = True # Required for loading terrain tiles from http sources
        self.options["bullet3"].double_precision = True

    def requirements(self):
        self.requires("boost/1.84.0", transitive_headers=True)
        self.requires("catch2/2.13.8")
        self.requires("cpp-httplib/0.10.1")
        self.requires("earcut/2.2.3")
        self.requires("glm/0.9.9.8", transitive_headers=True)
        self.requires("nlohmann_json/3.10.5", transitive_headers=True)
        self.requires("fontconfig/2.17.1", override=True) # Transitive dependency to resolve conflict between qt and openscenegraph
		
        self.requires("cxxtimer/1.0.0")
        self.requires("px_sched/1.0.0", transitive_headers=True)
        self.requires("openscenegraph-mr/3.7.0", transitive_headers=True)
        self.requires("skybolt-reflect/1.0.0", transitive_headers=True)

        if self.options.enable_bullet:
            self.requires("bullet3/3.22a")

        if self.options.enable_fft_ocean:
            self.requires("mufft/1.0.0")
            self.requires("xsimd/7.4.10", transitive_headers=True)

        if self.options.enable_jsbsim:
            self.requires("jsbsim/1.1.13")

        if self.options.enable_map_features_converter:
            self.requires("readosm/1.1.0a")

        if self.options.enable_python:
            self.requires("pybind11/2.13.6")
            
        if self.options.enable_qt:
            self.requires("qt/6.10.1", transitive_headers=True)
            self.requires("skybolt-widgets/1.0.0", transitive_headers=True)

    def _is_cmake_multi_generator(self) -> bool:
        gen = self.conf.get("tools.cmake.cmaketoolchain:generator")
        if gen:
            return "Visual" in gen or "Xcode" in gen or "Multi-Config" in gen
        else:
            compiler = self.settings.get_safe("compiler")
            if compiler == "msvc":
                return True
            else:
                return False

    def _build_type_path_if_multi_generator(self) -> bool:
        return "{self.build_type}/" if self._is_cmake_multi_generator() else ""

    def layout(self):
        cmake_layout(self)
        self.cpp.source.includedirs = ["src"]
        self.cpp.source.builddirs = ["CMake"]

        self.cpp.build.libdirs = ["lib", f"lib{self._build_type_path_if_multi_generator()}/plugins"]
        self.cpp.build.set_property("cmake_build_modules", [f"conan-skybolt-init-buildtree.cmake"])

    def generate(self):
        tc = CMakeToolchain(self)
        # FIXME: We shouldn't be configuring flags for dependencies here; that should be done by the dependency itself
        tc.variables["Boost_STATIC_LIBS"] = bool(not self.dependencies["boost"].options.shared)
        tc.variables["OSG_STATIC_LIBS"] = bool(not self.dependencies["openscenegraph-mr"].options.shared)

        tc.variables["SKYBOLT_PLUGINS_STATIC_BUILD"] = bool(not self.options.shared_plugins)
        tc.variables["Skybolt_VERSION"] = self.version

        # FIXME: These variables should be prefixed with "SKYBOLT_"
        tc.variables["BUILD_JSBSIM_PLUGIN"] = bool(self.options.enable_jsbsim)
        tc.variables["BUILD_BULLET_PLUGIN"] = bool(self.options.enable_bullet)
        tc.variables["BUILD_FFT_OCEAN_PLUGIN"] = bool(self.options.enable_fft_ocean)
        tc.variables["BUILD_MAP_FEATURES_CONVERTER"] = bool(self.options.enable_map_features_converter)
        tc.variables["BUILD_PYTHON_BINDINGS"] = bool(self.options.enable_python)
        tc.variables["BUILD_PYTHON_PLUGIN"] = bool(self.options.enable_python)
        tc.variables["BUILD_WITH_BOOST_LOG"] = bool(self.options.enable_boost_log)
        tc.variables["BUILD_WITH_QT"] = bool(self.options.enable_qt)
        tc.variables["BUILD_WITH_OSG_CURL_PLUGIN"] = bool(self.options.enable_osg_curl_plugin)

        tc.variables["BUILD_TESTING"] = False

        tc.variables["SKYBOLT_BUILD_EXAMPLES"] = False
        tc.variables["SKYBOLT_INSTALL_DEPENDENCIES"] = False

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
		
    def package(self):
        cmake = CMake(self)
        # Install Skybolt binaries (excluding dependencies)
        cmake.install()
        # Install development files (headers, etc)
        cmake.install(component="Development")

    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.libs = ["AircraftHud", "SkyboltEngine"]
        if self.options.enable_qt:
            self.cpp_info.libs.append("SkyboltEngineQt")

        # When building static libraries, the order of the libs matters; SkyboltEngineQt depends on SkyboltVis
        # The linker walks left to right, and collates items that are required and expects them to be satisfied later on
        self.cpp_info.libs.extend(["SkyboltVis", "SkyboltSim", "SkyboltCommon"])
        self.cpp_info.builddirs = ["CMake"]
        self.cpp_info.set_property("cmake_build_modules", ["CMake/conan-skybolt-init-installtree.cmake"])

        if self.options.enable_fft_ocean:
            self.cpp_info.libs.append("FftOcean")

