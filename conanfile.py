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
        tc.generator = 'Ninja'
        tc.cache_variables["CMAKE_MSVC_DEBUG_INFORMATION_FORMAT"] = "Embedded"
        tc.generate()

        for dep in self.dependencies.values():
            for bin_dir in dep.cpp_info.bindirs:
                copy(self, "*.dll", src=bin_dir,
                     dst=os.path.join(self.source_folder, f"build/{self.settings.build_type}/bin"),
                     keep_path=False)

    def requirements(self):
        self.requires("concurrentqueue/1.0.4")
        self.requires("cpp-httplib/0.18.2", options={
            "with_openssl": True,
            "with_zlib": True
        })
        self.requires("croncpp/cci.20220503")
        self.requires("glm/cci.20230113")
        self.requires("magic_enum/0.9.6")
        self.requires("msgpack-c/6.1.0")
        self.requires("mongo-c-driver/1.28.0", options={
            "shared": True
        })
        self.requires("mongo-cxx-driver/3.11.0", options={
            "shared": True,
            "polyfill": "std"
        })
        self.requires("openssl/3.3.2", force=True, options={
            "shared": True
        })
        self.requires('sol2/3.5.0')
        self.requires("spdlog/1.14.1")
        self.requires("stduuid/1.2.3")
        self.requires("xbyak/7.07")
        self.requires("zstd/1.5.7")

    def build_requirements(self):
        self.tool_requires("cmake/3.26.6")

    def layout(self):
        self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
        self.folders.build = os.path.join("build", str(self.settings.build_type))

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
