#ifndef __186_h__
#define __186_h__

void _wrap_0x0F(vomit_cpu_t *cpu);

void _BOUND(vomit_cpu_t *cpu);
void _ENTER(vomit_cpu_t *cpu);
void _LEAVE(vomit_cpu_t *cpu);

void _PUSHA(vomit_cpu_t *cpu);
void _POPA(vomit_cpu_t *cpu);
void _PUSH_imm8(vomit_cpu_t *cpu);
void _PUSH_imm16(vomit_cpu_t *cpu);

void _IMUL_reg16_RM16_imm8(vomit_cpu_t *cpu);

#endif /* __186_h__ */
