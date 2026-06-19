#!/usr/bin/env python3
import os
import subprocess
import tempfile
from pathlib import Path


def _tool(name):
    suffix = ".exe" if os.name == "nt" else ""
    pspdev = Path(os.environ.get("PSPDEV", "/usr/local/pspdev"))
    return str(pspdev / "bin" / (name + suffix))


def _run(args):
    subprocess.run([str(a) for a in args], check=True)


def make_eboot(target, source, env):
    output = Path(str(target[0])).resolve()
    source_elf = Path(str(source[0])).resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory(prefix="godot-psp-") as temp_dir:
        temp = Path(temp_dir)
        fixed = temp / "godot_psp.elf"
        stripped = temp / "godot_psp_stripped.elf"
        param = temp / "PARAM.SFO"

        _run([_tool("psp-fixup-imports"), "-o", fixed, source_elf])
        _run([_tool("psp-strip"), "-x", "-o", stripped, fixed])
        _run([_tool("mksfoex"), "-d", "MEMSIZE=1", "Godot 4 PSP", param])
        _run(
            [
                _tool("pack-pbp"),
                output,
                param,
                "NULL",
                "NULL",
                "NULL",
                "NULL",
                "NULL",
                "NULL",
                stripped,
                "NULL",
            ]
        )

    return 0


if __name__ == "__main__":
    raise SystemExit("This file is a SCons action, not a standalone command.")
