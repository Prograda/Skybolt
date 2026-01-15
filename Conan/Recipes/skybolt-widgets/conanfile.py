import os
from conan import ConanFile
from conan.tools import files
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
from conan.tools.scm import Git

class SkybolyWidgetsConan(ConanFile):
	implements = ["auto_shared_fpic"]
	name = "skybolt-widgets"
	version = "1.0.0"
	settings = "os", "compiler", "arch", "build_type"
	options = {
		"shared": [True, False],
		"fPIC": [True, False],
		"with_skybolt_reflect": [True, False]
	}
	default_options = {
		"shared": False,
		"fPIC": True,
		"with_skybolt_reflect": True
	}
	exports_sources = "*"
	no_copy_source = True

	def requirements(self):
		self.requires("catch2/2.13.8")
		self.requires("qt/6.10.1", transitive_headers=True)

		if self.options.with_skybolt_reflect:
			self.requires("skybolt-reflect/1.0.0", transitive_headers=True)

	def source(self):
		git = Git(self, folder=self.name)
		git.clone('https://github.com/prograda/skybolt-widgets', target=".")
		git.checkout("bd459070a3f556ce0a4f8fab3d4c511111747d77")

	def generate(self):
		tc = CMakeToolchain(self)
		tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
		
		if "fPIC" in self.options:
			tc.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC
		
		tc.variables["BUILD_WITH_SKYBOLT_REFLECT"] = self.options.with_skybolt_reflect
		
		tc.generate()

		deps = CMakeDeps(self)
		deps.generate()

	def build(self):
		cmake = CMake(self)
		cmake.configure(build_script_folder=self.name)
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()
		
	def package_info(self):
		self.cpp_info.includedirs = ["include"]
		self.cpp_info.libs = ["SkyboltWidgets"]
		self.cpp_info.builddirs = ["cmake"]
		if self.options.with_skybolt_reflect:
			self.cpp_info.defines.append("BUILD_WITH_SKYBOLT_REFLECT=1")
