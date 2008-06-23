/* fpu.c
 *
 * Actually, this file only contains the "ESCAPE" instruction, which handles
 * all FPU instructions by swallowing all ModR/M data and logging a little.
 *
 * Someday.
 *
 */

#include "vomit.h"
#include "debug.h"

void
_ESCAPE()
{
	byte rm;
	vlog( VM_CPUMSG, "%04X:%04X FPU escape via %02X /%u", cpu.base_CS, cpu.base_IP, cpu_opcode, rmreg( mem_getbyte(cpu.base_CS, cpu.base_IP + 1) ));
	dump_all();
	dump_ivt();

	rm = cpu_pfq_getbyte();
	(void) modrm_read16( rm );

	ui_kill();
	vm_debug();
	ui_show();
#if 0
	printf( "Swallowed %d bytes: ", cpu.IP - cpu.base_IP );
	for( int i = 0; i < cpu.IP - cpu.base_IP; ++i )
		printf( "%02X ", mem_getbyte(cpu.base_CS, cpu.base_IP + i));
	printf("\n");
#endif

#if 0
	/* 80286+: Coprocessor not available exception. */
	int_call( 7 );
#endif
}
