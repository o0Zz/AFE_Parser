# Atmosphere Fatal Error Parser
Console application to interpret Atmosphere's fatal error "report_xxxxxx.bin" files (AFE).

# How to use
Download the latest build from the [releases page.](https://github.com/o0Zz/AFE_Parser/releases) 

Usage:
```
Usage: AFE_Parser -report <report.bin> -elf <binary.elf> [-addr2line <path/to/addr2line>]
Options:
  -report <report.bin>   Path to the AFE report file.
  -elf <binary.elf>      Path to the ELF file for symbol resolution.
  -addr2line <path>      Optional path to addr2line executable to resolve addresses(default: /opt/devkitpro/devkitA64/bin/aarch64-none-elf-addr2line.exe).
  -h, --help             Show this help message.

```

# Supported AFE formats
- AFE2
- AFE1
- AFE0

# Building
Enter the project directory and run `make`. 
On Windows, you will need to install MinGW and have MinGW/bin in your PATH environment variable.
