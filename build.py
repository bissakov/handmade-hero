import argparse
import os
import subprocess
from typing import Iterable, Optional


def run_command(command: str) -> str:
    process = subprocess.Popen(
        command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True
    )
    stdout, stderr = process.communicate()
    if process.returncode != 0:
        print(f"Error executing command: {command}")
        print(stderr.decode())
    return stdout.decode()


def create_compile_command(
    output_name: str,
    additional_files: Optional[Iterable[str]] = None,
    additional_libs: Optional[Iterable[str]] = None,
) -> Iterable[str]:
    compile_command = ["cl"]

    # Compiler behavior flags
    compile_command.extend(
        [
            "-nologo",  # Suppress startup banner
            "-Oi",  # Enable intrinsic functions
            "-GR-",  # Disable run-time type information
            "-EHa-",  # Disable C++ exception handling
            "-MT",  # Use static multi-threaded runtime library
            "-Gm-",  # Disable minimal rebuild
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
            "../src/win32/win32-handmade-hero.cpp",
            "../src/handmade-hero/handmade-hero.cpp",
            "../src/win32/win32-input.cpp",
            "../src/win32/win32-file-io.cpp",
        ]
    )

    # Default libraries
    compile_command.extend(
        [
            "user32.lib",
            "gdi32.lib",
            "xinput.lib",
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


def main(
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
    output_name = output_name or "wib32_handmade_hero"
    compile_command = create_compile_command(
        output_name, additional_files, additional_libs
    )

    # Change to build directory, run compilation, and return
    os.chdir("build")
    print(run_command(command + " && " + " ".join(compile_command)))
    os.chdir("..")


if __name__ == "__main__":
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

    main(args.arch, args.files, args.libs, args.output)
