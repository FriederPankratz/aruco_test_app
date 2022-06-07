# /usr/bin/python3
import os
from conans import ConanFile, CMake, tools


class Traact(ConanFile):
    name = "aruco_test_app"
    version = "0.0.1"
    
    description = ""
    url = ""
    license = ""
    author = ""

    short_paths = True

    generators = "cmake", "traact_virtualrunenv_generator"
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
            "cppfs/1.3.0-r1@camposs/stable",
            "onetbb/2021.3.0",
            )

    def requirements(self):        
        self.requires("traact_run_env/%s@camposs/stable" % self.version)
        self.requires("traact_core/%s@camposs/stable" % self.version)
        self.requires("traact_spatial/%s@camposs/stable" % self.version)
        self.requires("traact_vision/%s@camposs/stable" % self.version)
        self.requires("traact_component_basic/%s@camposs/stable" % self.version)
        self.requires("traact_component_kinect_azure/%s@camposs/stable" % self.version)
        self.requires("traact_serialization/%s@camposs/stable" % self.version)
        self.requires("traact_component_aruco/%s@camposs/stable" % self.version)
                


    def configure(self):
        self.options['traact_core'].shared = self.options.shared
        self.options['traact_facade'].shared = self.options.shared
        self.options['traact_spatial'].shared = self.options.shared
        #self.options['traact_vision'].shared = self.options.shared        





