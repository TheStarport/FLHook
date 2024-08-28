import os

from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps, CMake


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = 'ConanPresets.json'
        tc.cache_variables["CMAKE_MSVC_DEBUG_INFORMATION_FORMAT"] = "Embedded"
        tc.generate()

    def requirements(self):
        self.requires("amqp-cpp/4.3.26")
        self.requires("concurrentqueue/1.0.4")
        self.requires("croncpp/cci.20220503")
        self.requires("glm/cci.20230113")
        self.requires("magic_enum/0.9.6")
        self.requires("re2/20240702")
        self.requires("spdlog/1.14.1")
        self.requires("stduuid/1.2.3")
        self.requires("uvw/3.4.0")
        self.requires("xbyak/7.07")

    def build_requirements(self):
        self.tool_requires("cmake/3.22.6")

    def layout(self):
        multi = True if self.settings.get_safe("compiler") == "msvc" else False
        if multi:
            self.folders.generators = os.path.join("build", "generators")
            self.folders.build = "build"
        else:
            self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
            self.folders.build = os.path.join("build", str(self.settings.build_type))

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        copy(self, "*.dll", src=os.path.join(self.build_folder), dst=self.package_folder)
