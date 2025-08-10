#include "panic.h"
#include "AFE.h"
#include "inttypes.h"
#include <cstdio>
#include <fstream>
#include <cstring>
#include <filesystem>

#define ADDR2LINE_DEFAULT_PATH "/opt/devkitpro/devkitA64/bin/aarch64-none-elf-addr2line.exe"

class Addr2LineResolver : public IStackTraceResolver
{
private:
	const char* addr2linePath;
	const char* elfPath;
public:
	Addr2LineResolver(const char* addr2linePath, const char* elfPath)
		: addr2linePath(addr2linePath), elfPath(elfPath) {}

	static uint64_t GetRelativeAddress(uint64_t module_base, uint64_t offset)
	{
		/*
		    On AArch64, the PC register contains the address of the current instruction,
		    which, for return addresses, usually points to the instruction after the call.
		    This means the reported address is actually the return site, not the call itself.
		    To correctly identify the call location in the stack trace, subtract 4 bytes
		    from the reported PC value.
		*/

		if (offset >= module_base && offset - module_base < MB(16)) // Assuming 16MB is the max size of a module
			return (offset - module_base) - 4;
		return offset - 4;
	}

	void ResolveAddress(uint64_t module_base, uint64_t offset, char* output, size_t outputSize) const override
	{
		char addr2lineOutput[256] = "Failed to run addr2line";
		char command[256];
		snprintf(command, sizeof(command), "\"%s\" -e \"%s\" -f -p -C -i 0x%" PRIx64, addr2linePath, elfPath, GetRelativeAddress(module_base, offset));

#if defined(_WIN32)
        FILE* pipe = _popen(command, "r");
#else
        FILE* pipe = popen(command, "r");
#endif
		if (pipe) 
		{
			if (fgets(addr2lineOutput, sizeof(addr2lineOutput), pipe)) 
			{
            	addr2lineOutput[strcspn(addr2lineOutput, "\n")] = '\0'; // Remove newline
        	}
	#if defined(_WIN32)
			_pclose(pipe);
	#else
			pclose(pipe);
	#endif
		}

		snprintf(output, outputSize, "0x%" PRIx64 " (MOD_BASE + 0x%08" PRIx64 ") - %s", offset, offset - module_base, addr2lineOutput);
	}
};

class NoneResolver : public IStackTraceResolver
{
public:
	void ResolveAddress(uint64_t module_base, uint64_t offset, char* output, size_t outputSize) const override
	{
		snprintf(output, outputSize, "0x%" PRIx64 " (MOD_BASE + 0x%" PRIx64 ")", offset, offset - module_base);
	}
};


void usage(const char* programName)
{
	printf("Usage: %s -report <report.bin> -elf <report.elf> [-addr2line <path/to/addr2line>]\n", programName);
	printf("Options:\n");
	printf("  -report <report.bin>   Path to the AFE report file.\n");
	printf("  -elf <report.elf>      Path to the ELF file for symbol resolution.\n");
	printf("  -addr2line <path>      Optional path to addr2line executable (default: %s).\n", ADDR2LINE_DEFAULT_PATH);
	printf("  -h, --help             Show this help message.\n");
}

int main(int argc, char* argv[])
{
	const char *reportPath = NULL;
    const char *elfPath = NULL;
	const char *addr2linePath = NULL;
	std::unique_ptr<IStackTraceResolver> resolver;

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-report") == 0 && i + 1 < argc)
        {
            reportPath = argv[++i];
        }
        else if (strcmp(argv[i], "-elf") == 0 && i + 1 < argc)
        {
            elfPath = argv[++i];
        }
		else if (strcmp(argv[i], "-addr2line") == 0 && i + 1 < argc)
        {
            addr2linePath = argv[++i];
        }
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			usage(argv[0]);
			return 0;
		}
    }

    // Validate
    if (!reportPath || !elfPath)
    {
        usage(argv[0]);
        return -1;
    }

	if (!addr2linePath && std::filesystem::exists(ADDR2LINE_DEFAULT_PATH))
		addr2linePath = ADDR2LINE_DEFAULT_PATH; // Default addr2line command

	resolver = std::make_unique<NoneResolver>();
	if (addr2linePath)
		resolver = std::make_unique<Addr2LineResolver>(addr2linePath, elfPath);

    printf("Report file    : %s\n", reportPath);
    printf("ELF file       : %s\n", elfPath);
	printf("Resolver type  : %s (%s)\n", resolver ? "Addr2LineResolver" : "NoneResolver", addr2linePath ? addr2linePath : "");
	printf("---------------------------------------------------------------\n");

	std::fstream file;
	file.open(reportPath, std::fstream::in | std::fstream::binary);

	//Get file magic
	uint32_t fatal_magic;
	file.read((char*)&fatal_magic, sizeof(fatal_magic));
	file.seekp(std::ios_base::beg);

	//AFE2
	if (fatal_magic == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC)
	{
		atmosphere_fatal_error_ctx fatal_report;
		file.read((char*)&fatal_report, sizeof(fatal_report));
		PrintAFE2Report(&fatal_report, *resolver);
	}
	//AFE1
	else if (fatal_magic == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC_1)
	{
		atmosphere_fatal_error_ctx_1 fatal_report;
		file.read((char*)&fatal_report, sizeof(fatal_report));
		PrintAFE1Report(&fatal_report, *resolver);
	}
	//AFE0
	else if (fatal_magic == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC_0)
	{
		atmosphere_fatal_error_ctx_0 fatal_report;
		file.read((char*)&fatal_report, sizeof(fatal_report));
		PrintAFE0Report(&fatal_report);
	}
	else if ((fatal_magic & 0xF0FFFFFF) == ATMOSPHERE_REBOOT_TO_FATAL_MAGIC_0)
	{
		printf("Passed file contains an unknown AFE version:  %.4s (0x%" PRIX32 ")\n", (char*)&fatal_magic, fatal_magic);
	}
	else
	{
		printf("Passed file is not a valid AFE file\n");
	}
	file.close();
}