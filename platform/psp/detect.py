import os
from pathlib import Path
from typing import TYPE_CHECKING

from platform_methods import validate_arch

if TYPE_CHECKING:
    from SCons.Script.SConscript import SConsEnvironment


def get_name():
    return "PSP"


def _pspdev():
    return Path(os.environ.get("PSPDEV", "/usr/local/pspdev"))


def _tool(name):
    suffix = ".exe" if os.name == "nt" else ""
    return str(_pspdev() / "bin" / (name + suffix))


def can_build():
    compiler = Path(_tool("psp-g++"))
    if not compiler.is_file():
        return False
    return True


def get_tools(env: "SConsEnvironment"):
    return ["cc", "c++", "ar", "link", "textfile"]


def get_opts():
    from SCons.Variables import BoolVariable

    return [
        BoolVariable("psp_build_eboot", "Build bin/EBOOT.PBP after linking the PSP ELF", True),
    ]


def get_flags():
    return {
        "arch": "mips32",
        "target": "template_release",
        "optimize": "size",
        "lto": "none",
        "threads": False,
        "disable_3d": False,
        "disable_physics_3d": False,
        "disable_navigation_3d": True,
        "disable_xr": True,
        "disable_advanced_gui": True,
        "modules_enabled_by_default": False,
        "module_gdscript_enabled": True,
        "module_freetype_enabled": True,
        "module_text_server_fb_enabled": True,
        "module_godot_physics_3d_enabled": True,
        "builtin_pcre2_with_jit": False,
        "vulkan": False,
        "opengl3": False,
        "d3d12": False,
        "metal": False,
        "angle": False,
        "accesskit": False,
        "sdl": False,
        "xaudio2": False,
    }


def configure(env: "SConsEnvironment"):
    validate_arch(env["arch"], get_name(), ["mips32"])

    pspdev = _pspdev()
    pspsdk = pspdev / "psp" / "sdk"

    env["ENV"] = os.environ.copy()
    env["CC"] = _tool("psp-gcc")
    env["CXX"] = _tool("psp-g++")
    env["AR"] = _tool("psp-ar")
    env["RANLIB"] = _tool("psp-ranlib")

    env["PROGPREFIX"] = ""
    env["PROGSUFFIX"] = ".elf"
    env["LIBPREFIX"] = "lib"
    env["LIBSUFFIX"] = ".a"
    env["OBJSUFFIX"] = ".o"

    env.Prepend(CPPPATH=["#platform/psp", str(pspsdk / "include")])
    env.Append(
        CPPDEFINES=[
            "PSP_ENABLED",
            "UNIX_ENABLED",
            "UNIX_SOCKET_UNAVAILABLE",
            "NO_STATVFS",
        ]
    )
    if env["threads"]:
        env.Append(CPPDEFINES=["PTHREAD_ENABLED", "PTHREAD_NO_RENAME"])
    env.Append(
        CCFLAGS=[
            "-march=allegrex",
            "-mabi=eabi",
            "-G0",
            "-mno-gpopt",
            "-fno-exceptions",
            "-ffunction-sections",
            "-fdata-sections",
        ]
    )
    env.Append(CXXFLAGS=["-include", "platform/psp/variant_type_compat.h"])
    env.Append(LINKFLAGS=["-Wl,--gc-sections", "-L" + str(pspsdk / "lib")])
    env.Append(
        LIBS=[
            "GL",
            "GLU",
            "glut",
            "pspdebug",
            "pspge",
            "pspdisplay",
            "pspctrl",
            "psppower",
            "pspaudio",
            "pspgu",
            "pspgum",
            "pspsdk",
            "z",
            "m",
            "c",
        ]
    )
    if env["threads"]:
        env.Append(LIBS=["pthread"])

    env.extra_suffix = ".psp" + env.extra_suffix
