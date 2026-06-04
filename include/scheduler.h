/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software
 * Foundation, version 3.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <kernel.h>

#define TASK_RUNNING  0
#define TASK_READY    1
#define TASK_BLOCKED  2
#define TASK_DEAD     3
#define TASK_SLEEPING 4

#define MAX_TASKS   512
#define STACK_SIZE  4096

// -----------
// By Bigpower
// -----------

/************************
*  Powered By ValiantCore
*
*************************/

typedef struct {
   uint32_t id;
   uint8_t  state;
   uint8_t  priority;
   addr_t   stack_ptr;
   addr_t   stack_base;
   uint32_t time_slice;
   uint32_t ticks_used;
   uint32_t sleep_ticks;
   char     name[16];
} task_t;

void     init_scheduler();
int      task_create(addr_t entry, uint8_t priority, const char *name);
void     task_kill(uint32_t id);
void     task_block(uint32_t id);
void     task_unblock(uint32_t id);
void     task_sleep(uint32_t id, uint32_t ticks);
void     task_set_priority(uint32_t id, uint8_t priority);
void     schedule_next();
int      ipc_send(uint32_t from, uint32_t to, addr_t data, uint32_t len);
int      ipc_receive(uint32_t id, addr_t *data, uint32_t len);
uint32_t schedule_get_tick();
uint32_t scheduler_task_count();
uint32_t schedule_current();

#endif
