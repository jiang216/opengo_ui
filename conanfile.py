from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get
import os


class OpenGLUIConan(ConanFile):
    name = "opengl-ui"
    version = "0.1.0"
    license = "MIT"
    description = "C++17 OpenGL UI framework with 50+ layui-styled widgets"
    url = "https://github.com/user/opengl_ui"
    homepage = "https://github.com/user/opengl_ui"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_ffmpeg": [True, False],
        "with_opencv": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_ffmpeg": False,
        "with_opencv": False,
    }
    exports_sources = "CMakeLists.txt", "cmake/*", "core/*", "gl/*", "ui/*", "media/*", "3rdparty/glad/*", "3rdparty/stb/*"
    no_copy_source = True

    def requirements(self):
        self.requires("glfw/3.4")
        if self.options.with_ffmpeg:
            self.requires("ffmpeg/5.1.9")
        if self.options.with_opencv:
            self.requires("opencv/4.5.2")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["OPENGUI_MEDIA_FFMPEG"] = self.options.with_ffmpeg
        tc.variables["OPENGUI_MEDIA_OPENCV"] = self.options.with_opencv
        tc.variables["OPENGUI_BUILD_APPS"] = False
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "opengl_ui")
        self.cpp_info.set_property("cmake_find_mode", "config")

        self.cpp_info.components["core"].libs = ["ui_core"]
        self.cpp_info.components["core"].set_property("cmake_target_name", "opengl_ui::ui_core")

        self.cpp_info.components["gl"].libs = ["ui_gl"]
        self.cpp_info.components["gl"].requires = ["core"]
        self.cpp_info.components["gl"].set_property("cmake_target_name", "opengl_ui::ui_gl")

        self.cpp_info.components["ui"].libs = ["ui_ui"]
        self.cpp_info.components["ui"].requires = ["gl"]
        self.cpp_info.components["ui"].set_property("cmake_target_name", "opengl_ui::ui_ui")

        if self.options.with_ffmpeg:
            self.cpp_info.components["ffmpeg"].libs = ["ui_media_ffmpeg"]
            self.cpp_info.components["ffmpeg"].requires = ["ui", "ffmpeg::ffmpeg"]
            self.cpp_info.components["ffmpeg"].set_property("cmake_target_name", "opengl_ui::ui_media_ffmpeg")

        if self.options.with_opencv:
            self.cpp_info.components["opencv"].libs = ["ui_media_opencv"]
            self.cpp_info.components["opencv"].requires = ["ui", "opencv::opencv"]
            self.cpp_info.components["opencv"].set_property("cmake_target_name", "opengl_ui::ui_media_opencv")

        # 默认组件（聚合）
        self.cpp_info.libs = ["ui_ui"]
        self.cpp_info.requires = ["ui"]
