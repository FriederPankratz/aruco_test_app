# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class Traact(ConanFile):
    name = "aruco_test_app"
    version = "0.1.0"

    description = ""
    url = ""
    license = ""
    author = ""

    short_paths = True

    generators = "cmake", "TraactVirtualRunEnvGenerator"
    settings = "os", "compiler", "build_type", "arch"
    compiler = "cppstd"
    options = {
        "shared": [True, False],
        "with_tests": [True, False]
    }

    default_options = {
        "shared": True,
        "with_tests": True
    }

    exports_sources = "CMakeLists.txt", "main.cpp"

    # overwrite these dependencies
    requires = (
        "eigen/3.4.0"
    )

    def requirements(self):
        self.requires("traact_run_env/[>=1.0.0]@camposs/stable")
        self.requires("traact_core/[>=0.1.0]@camposs/stable")
        self.requires("traact_spatial/[>=0.1.0]@camposs/stable")
        self.requires("traact_vision/[>=0.1.0]@camposs/stable")
        self.requires("traact_component_basic/[>=0.1.0]@camposs/stable")
        self.requires("traact_component_kinect_azure/[>=0.1.0]@camposs/stable")
        self.requires("traact_component_cereal/[>=0.1.0]@camposs/stable")
        self.requires("traact_component_aruco/[>=0.1.0]@camposs/stable")

    def configure(self):
        self.options['traact_core'].shared = self.options.shared
        self.options['traact_facade'].shared = self.options.shared
        self.options['traact_spatial'].shared = self.options.shared
        self.options['traact_vision'].shared = self.options.shared
