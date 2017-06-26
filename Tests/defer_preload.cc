#include "streamtrace.hh"
#include <assert.h>

using cheri::streamtrace::debug_trace_entry;
using cheri::streamtrace::register_set;

static uint64_t pcs[] = {
	0xffffffff8024d188,
	0xffffffff8024d18c,
	0xffffffff8024d190,
	0xffffffff8024d194,
	0xffffffff8024d198
};
static uint32_t instrs[] = {
	0x1800b3df,
	0x1000b2df,
	0x800b1df,
	0xb0df,
	0x800e003
};

int main()
{
	// Test opening a trace with deferred preloading
	auto trace = cheri::streamtrace::trace::open(SOURCE_PATH "/short.trace",
						     nullptr, true);
	uint64_t count = 0;
	bool regs_tested = false;

	auto scan_function = [&](const debug_trace_entry &e, const register_set &regs, uint64_t idx) {
		assert(e.pc == pcs[idx]);
		assert(e.inst == instrs[idx]);
		if (idx == 4) {
			auto expect_regval = [&](int reg, uint64_t val) {
				// Registers are indexed from 1 ($zero is not stored)
				reg -= 1;
				assert(regs.gpr[reg] == val);
				assert(regs.valid_gprs[reg]);
			};
			regs_tested = true;
			expect_regval(19, 0x7fffffe1a0LL);
			expect_regval(18, 0x9800000002b3e000LL);
			expect_regval(17, 0xc0000000150b7780LL);
			expect_regval(16, 0xc0000000150b7530LL);
		}
		return false;
	};

	trace->scan(scan_function, 0, 4, cheri::streamtrace::trace::forewards);
	assert(regs_tested);
	count = 0;

	trace->scan([&](const debug_trace_entry &e, const register_set &regs, uint64_t idx) {
			count++;
			assert(e.pc == pcs[idx]);
			assert(e.inst == instrs[idx]);
			return false;
		}, 2, 4, cheri::streamtrace::trace::backwards);
	assert(count == 3);

	return 0;
}
