import argparse
import os
import subprocess
from typing import Iterable, Optional, Tuple

from rich.console import Console


def run_command(command: str) -> Tuple[bool, str]:
    result = ""
    process = subprocess.Popen(
        command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True
    )
    stdout, stderr = process.communicate()
    if process.returncode != 0:
        result = f"[b red]Error[/b red] executing command: [i medium_purple1]{command}[/i medium_purple1]\n{stderr.decode()}"
        return False, result

    result = stdout.decode()

    return True, result


def create_compile_command(
    output_name: str,
    additional_files: Optional[Iterable[str]] = None,
    additional_libs: Optional[Iterable[str]] = None,
) -> Iterable[str]:
    compile_command = ["cl"]

    compile_command.extend(
        [
            "-D DEV=1",  # Development build
            "-D DEBUG=1",  # Debug build
        ]
    )

    # Compiler behavior flags
    compile_command.extend(
        [
            "-nologo",  # Suppress startup banner
            "-Oi",  # Enable intrinsic functions
            "-GR-",  # Disable run-time type information
            "-EHa-",  # Disable C++ exception handling
            "-MT",  # Use static multi-threaded runtime library
            "-Gm-",  # Disable minimal rebuild
            "-Od",  # Disable optimization
        ]
    )

    # Warning and error flags
    compile_command.extend(
        [
            "-W4",  # Set warning level to 4 (highest)
            "-wd4201",  # Disable warning 4201 (nonstandard extension used: nameless struct/union)
            "-wd4127",  # Disable warning 4127 (conditional expression is constant)
            "-wd4100",  # Disable warning 4100 (unreferenced formal parameter)
        ]
    )

    # Debugging and optimization flags
    compile_command.extend(
        [
            "-FC",  # Display full path of source code files passed to cl.exe
            "-Z7",  # Generate complete debugging information
        ]
    )

    # Output flags (map file)
    compile_command.append(f"-Fm{output_name}.map")

    # Source files
    compile_command.extend(
        [
            "../src/win32/win32-handmade-hero.cpp",  # Win32 entry point
            "../src/win32/win32-input.cpp",  # Win32 input handling
            "../src/win32/win32-file-io.cpp",  # Win32 file I/O
            "../src/win32/win32-sound.cpp",  # Win32 sound handling
            "../src/win32/win32-clock.cpp",  # Win32 clock handling
            "../src/win32/win32-display.cpp",  # Win32 display handling
            "../src/handmade-hero/handmade-hero.cpp",  # Game code
        ]
    )

    # Default libraries
    compile_command.extend(
        [
            "user32.lib",  # User interface
            "gdi32.lib",  # Graphics device interface
            "xinput.lib",  # XInput controller support
        ]
    )

    # Additional files (if provided)
    if additional_files:
        compile_command.extend(additional_files)

    # Additional libraries (if provided)
    if additional_libs:
        compile_command.extend(additional_libs)

    # Linker flags (optimize for size)
    compile_command.extend(["/link", "-opt:ref"])

    return compile_command


def lint(console: Console) -> None:
    _, output = run_command("cpplint --quiet --recursive .")
    console.print(output)


def build(
    console: Console,
    arch: str,
    additional_files: Optional[Iterable[str]] = None,
    additional_libs: Optional[Iterable[str]] = None,
    output_name: Optional[str] = None,
) -> None:
    # Create build directory
    os.makedirs("build", exist_ok=True)

    # Set up environment
    command = "call vcvarsall.bat x64"
    if arch == "x86":
        command = "call vcvarsall.bat x86"
    command = command + " > nul 2>&1"

    # Prepare compilation command
    output_name = output_name or "win32_handmade_hero"
    compile_command = create_compile_command(
        output_name, additional_files, additional_libs
    )

    # Change to build directory, run compilation, and return
    os.chdir("build")

    _, output = run_command(command + " && " + " ".join(compile_command))
    console.print(output)

    os.chdir("..")


def main() -> None:
    parser = argparse.ArgumentParser(description="Custom build script")
    parser.add_argument(
        "--arch",
        type=str,
        choices=["x86", "x64"],
        default="x64",
        help="Architecture (32 or 64 bit)",
    )
    parser.add_argument(
        "--files", nargs="*", help="Additional source files to compile"
    )
    parser.add_argument(
        "--libs", nargs="*", help="Additional libraries to link"
    )
    parser.add_argument("--output", help="Output file name (without extension)")

    args = parser.parse_args()

    console = Console()
    lint(console)
    build(console, args.arch, args.files, args.libs, args.output)


if __name__ == "__main__":
    main()
