#!/usr/bin/env python3
"""Build compact Godot 4 PSP templates with PSPDEV."""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BIN = ROOT / "bin"


def find_scons():
    executable = shutil.which("scons")
    if executable:
        return [executable]
    try:
        import SCons  # noqa: F401
    except ImportError as exc:
        raise SystemExit("SCons was not found. Install it with 'python3 -m pip install scons'.") from exc
    return [sys.executable, "-m", "SCons"]


def common_arguments(target):
    return [
        "platform=psp",
        f"target={target}",
        "arch=mips32",
        "threads=no",
        "modules_enabled_by_default=no",
        "module_gdscript_enabled=yes",
        "module_freetype_enabled=yes",
        "module_text_server_fb_enabled=yes",
        "module_godot_physics_3d_enabled=yes",
        "disable_3d=no",
        "disable_physics_3d=no",
        "disable_navigation_3d=yes",
        "disable_xr=yes",
        "disable_advanced_gui=yes",
        "optimize=size",
        "lto=none",
        "eboot",
    ]


def build(configuration, jobs, template_dir):
    target = f"template_{configuration}"
    command = find_scons() + [f"-j{jobs}"] + common_arguments(target)
    subprocess.run(command, cwd=ROOT, check=True)

    source = BIN / "EBOOT.PBP"
    destination = BIN / f"psp_{configuration}.pbp"
    if not source.is_file():
        raise SystemExit(f"Expected build artifact was not created: {source}")
    shutil.copy2(source, destination)
    print(f"Created {destination}")

    if template_dir:
        template_dir.mkdir(parents=True, exist_ok=True)
        installed = template_dir / destination.name
        shutil.copy2(destination, installed)
        print(f"Installed {installed}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--configuration", choices=("debug", "release", "both"), default="both")
    parser.add_argument("--jobs", type=int, default=max(1, os.cpu_count() or 1))
    parser.add_argument("--install-templates", type=Path)
    args = parser.parse_args()

    pspdev = Path(os.environ.get("PSPDEV", "/usr/local/pspdev"))
    compiler = pspdev / "bin" / "psp-g++"
    if not compiler.is_file():
        raise SystemExit(f"PSP compiler not found at {compiler}; set PSPDEV first.")

    configurations = ("debug", "release") if args.configuration == "both" else (args.configuration,)
    for configuration in configurations:
        build(configuration, args.jobs, args.install_templates)


if __name__ == "__main__":
    main()
