#include "AFE.h"
#include "inttypes.h"
#include <cstdlib>

static void PrintReportDesc(uint32_t magic, uint32_t error_desc, uint64_t titleid, uint64_t module_base = -1)
{
	printf("Magic            : %.4s (0x%" PRIX32 ")\n", (char*)&magic, magic);
	printf("Error description: 0x%" PRIX32 "\n", error_desc);
	printf("Title ID         : %" PRIX64 "\n", titleid);
	printf("Module base addr : %" PRIX64 "\n", module_base);
}

static void PrintRegisters(uint64_t *gprs, uint64_t pc, uint64_t module_base, const IStackTraceResolver& resolver)
{
	printf("Registers:\n");
	for (uint32_t i = 0; i != 29; ++i)
		printf("	X[%02" PRIu32 "]: %s\n", i, resolver.ResolveAddress(module_base, gprs[i]).c_str());

	printf("	FP: %s\n", resolver.ResolveAddress(module_base, gprs[29]).c_str());
	printf("	LR: %s\n", resolver.ResolveAddress(module_base, gprs[30]).c_str());
	printf("	SP: %s\n", resolver.ResolveAddress(module_base, gprs[31]).c_str());
	printf("	PC: %s\n", resolver.ResolveAddress(module_base, pc).c_str());

}

static void PrintMisc(uint32_t pstate, uint32_t afsr0, uint32_t afsr1, uint32_t esr, uint64_t far, uint64_t report_identifier)
{
	printf("PSTATE           : 0x%" PRIx32 " (Processor State Register)\n", pstate);
	printf("AFSR0            : 0x%" PRIx32 " (Auxiliary Fault Status Registers)\n", afsr0);
	printf("AFSR1            : 0x%" PRIx32 " (Auxiliary Fault Status Registers)\n", afsr1);
	printf("ESR              : 0x%" PRIx32 " (Category and details of the fault)\n", esr);
	printf("FAR              : 0x%" PRIx64 " (Fault Address Register, the virtual address involved in the fault)\n", far);
	printf("Report Identifier: 0x%" PRIx64 " (Name of the report)\n", report_identifier);
}

static void PrintStackTrace(uint64_t* stack_trace, uint64_t stack_trace_size, uint64_t module_base, const IStackTraceResolver& resolver)
{
	printf("Stack trace:\n");
	for (uint64_t i = 0; i != stack_trace_size; ++i)
	{
		printf("	%02" PRIu64 " - %s\n", i, resolver.ResolveAddress(module_base, stack_trace[i]).c_str());
	}
}

static void PrintStackDump(uint8_t* stack_dump, uint64_t stack_dump_size, uint64_t stack_dump_base = 0)
{
	printf("Stack Dump:	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n"
		"---------------------------------------------------------------\n");
	for (uint64_t i = 0; i < stack_dump_size; i += 0x10)
	{
		printf("%06" PRIx64 "		%02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			i + stack_dump_base,
			stack_dump[i + 0], stack_dump[i + 1], stack_dump[i + 2], stack_dump[i + 3],
			stack_dump[i + 4], stack_dump[i + 5], stack_dump[i + 6], stack_dump[i + 7],
			stack_dump[i + 8], stack_dump[i + 9], stack_dump[i + 10], stack_dump[i + 11],
			stack_dump[i + 12], stack_dump[i + 13], stack_dump[i + 14], stack_dump[i + 15]);
	}
}

static void PrintTlsDump(uint8_t* tls_dump, uint64_t tls_dump_size = AMS_FATAL_ERROR_TLS_SIZE, uint64_t tls_dump_base = 0)
{
	printf("TLS Dump:	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n"
		"---------------------------------------------------------------\n");
	for (uint64_t i = 0; i < tls_dump_size; i += 0x10)
	{
		printf("%06" PRIx64 "		%02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			i + tls_dump_base,
			tls_dump[i + 0], tls_dump[i + 1], tls_dump[i + 2], tls_dump[i + 3],
			tls_dump[i + 4], tls_dump[i + 5], tls_dump[i + 6], tls_dump[i + 7],
			tls_dump[i + 8], tls_dump[i + 9], tls_dump[i + 10], tls_dump[i + 11],
			tls_dump[i + 12], tls_dump[i + 13], tls_dump[i + 14], tls_dump[i + 15]);
	}
}

void PrintAFE0Report(atmosphere_fatal_error_ctx_0* fatal_report, const IStackTraceResolver& resolver)
{
	printf("Fatal report (AFE0):\n");
	printf("\n");

	PrintReportDesc(fatal_report->magic, fatal_report->error_desc, fatal_report->title_id);
	printf("\n");

	PrintRegisters(fatal_report->gprs, fatal_report->pc, -1, resolver);
	printf("\n");
	
	PrintMisc(fatal_report->pstate, fatal_report->afsr0, fatal_report->afsr1, fatal_report->esr, fatal_report->far, fatal_report->report_identifier);
	printf("\n");
}

void PrintAFE1Report(atmosphere_fatal_error_ctx_1* fatal_report, const IStackTraceResolver& resolver)
{
	printf("Fatal report (AFE1):\n");
	printf("\n");
	
	PrintReportDesc(fatal_report->magic, fatal_report->error_desc, fatal_report->title_id, fatal_report->module_base);
	printf("\n");

	PrintRegisters(fatal_report->gprs, fatal_report->pc, fatal_report->module_base, resolver);
	printf("\n");

	PrintMisc(fatal_report->pstate, fatal_report->afsr0, fatal_report->afsr1, fatal_report->esr, fatal_report->far, fatal_report->report_identifier);
	printf("\n");

	PrintStackTrace(fatal_report->stack_trace, fatal_report->stack_trace_size, fatal_report->module_base, resolver);
	printf("\n");

	PrintStackDump(fatal_report->stack_dump, fatal_report->stack_dump_size);
	printf("\n");
}

void PrintAFE2Report(atmosphere_fatal_error_ctx* fatal_report, const IStackTraceResolver& resolver)
{
	printf("Fatal report (AFE2):\n");
	printf("\n");
	
	PrintReportDesc(fatal_report->magic, fatal_report->error_desc, fatal_report->title_id, fatal_report->module_base);
	printf("\n");

	PrintRegisters(fatal_report->gprs, fatal_report->pc, fatal_report->module_base, resolver);
	printf("\n");

	PrintMisc(fatal_report->pstate, fatal_report->afsr0, fatal_report->afsr1, fatal_report->esr, fatal_report->far, fatal_report->report_identifier);
	printf("\n");

	PrintStackTrace(fatal_report->stack_trace, fatal_report->stack_trace_size, fatal_report->module_base, resolver);
	printf("\n");

	PrintStackDump(fatal_report->stack_dump, fatal_report->stack_dump_size);
	printf("\n");

	PrintTlsDump(fatal_report->tls);
	printf("\n");
}