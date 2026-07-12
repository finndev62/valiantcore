#include <asm/kernel.h>
#include <stdint.h>

void serror_handler(uint64_t esr, uint64_t elr, uint64_t type) {
     (void)type;
     kernel_panic_aarch64(esr, elr);
}
