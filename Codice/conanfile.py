from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps

class ConanApplication(ConanFile):
    package_type = "application"
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)
        self.folders.generators = "conan"

    def generate(self):
        cmake = CMakeDeps(self)
        cmake.generate();

    def requirements(self):
        self.requires("boost/1.88.0")
        # 'force' serve a far si che PlutoSVG usi questa versione invece della 2.13.2
        self.requires("freetype/2.14.1", force=True)
        self.requires("libalsa/1.2.13") # La stessa versione usata dalla libreria SDL v3.2.20
        self.requires("nlohmann_json/3.12.0")
        self.requires("platformfolders/4.3.0")
        self.requires("plutosvg/0.0.7")
        self.requires("sdl/3.2.20")
