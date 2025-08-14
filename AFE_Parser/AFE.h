#pragma once
#include "panic.h"
#include <cstdio>
#include <memory>
#include <string>

#define MB(x) ((x) * 1024 * 1024)

class IStackTraceResolver
{
public:
	virtual ~IStackTraceResolver() = default;
	virtual std::string ResolveAddress(uint64_t module_base, uint64_t offset) const = 0;
};

void PrintAFE0Report(atmosphere_fatal_error_ctx_0* fatal_report, const IStackTraceResolver& resolver);
void PrintAFE1Report(atmosphere_fatal_error_ctx_1* fatal_report, const IStackTraceResolver& resolver);
void PrintAFE2Report(atmosphere_fatal_error_ctx* fatal_report, const IStackTraceResolver& resolver);