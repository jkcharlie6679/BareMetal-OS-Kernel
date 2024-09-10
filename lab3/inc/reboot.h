#ifndef REBOOT_H
#define REBOOT_H

#ifdef __cpluscplus
extern "C" {
#endif

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();

#ifdef __cpluscplus
}
#endif

#endif
