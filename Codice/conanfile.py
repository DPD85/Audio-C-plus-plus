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
