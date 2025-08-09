#pragma once
#include "panic.h"
#include <cstdio>
#include <memory>

#define MB(x) ((x) * 1024 * 1024)

class IStackTraceResolver
{
public:
	virtual ~IStackTraceResolver() = default;
	virtual void ResolveAddress(uint64_t module_base, uint64_t offset, char* output, size_t outputSize) const = 0;
};

void PrintAFE0Report(atmosphere_fatal_error_ctx_0* fatal_report);
void PrintAFE1Report(atmosphere_fatal_error_ctx_1* fatal_report, const IStackTraceResolver& resolver);
void PrintAFE2Report(atmosphere_fatal_error_ctx* fatal_report, const IStackTraceResolver& resolver);