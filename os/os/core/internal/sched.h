
#pragma once

#include "osthread.h"

void sched_state_clear(OSThread* t);
void sched_state_setRunnable(OSThread* t);
void sched_state_setSleeping(OSThread* t);
void sched_state_setBlockedResource(OSThread* t);
//void sched_state_setBlockedInterrupt(OSThread* t);
void sched_state_setBlockedRecv(OSThread* t);
void sched_state_setBlockedSend(OSThread* t);

uint8_t sched_state_isBlockedRecv(OSThread* t);
//uint8_t sched_state_isBlockedInterrupt(Thread t);

void sched_yield(void) __attribute__((naked));
void sched_yieldFromNowhere(void) __attribute__((naked));

OSThread* sched_currentThread(void);

uint8_t sched_isAlive(OSThread* t);
void sched_assignNullOwnerToSpawnOf(OSThread* terminatedThread);

void sched_init(void);
