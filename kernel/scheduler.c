/*
 * ValiantCore Kernel
 * Copyright (C) 2026 bigpower
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "../include/scheduler.h"

static task_t    tasks[MAX_TASKS];
static uint32_t  task_count = 0;
static uint32_t  current_task = 0;
static uint32_t  tick_count   = 0;


#define IPC_QUEUE_SIZE 16

typedef struct {
   uint32_t from;
   uint32_t to;
   addr_t   data;
   uint32_t len;
   uint8_t  used;
} ipc_msg_t;

static ipc_msg_t ipc_queue[IPC_QUEUE_SIZE];


extern void context_switch(addr_t *old_sp, addr_t new_sp);

void init_scheduler() {
  for (int i = 0; i < MAX_TASKS; i++) {
      tasks[i].state    = TASK_DEAD;
      tasks[i].id       = 0;
      tasks[i].sleep_ticks  = 0;
  }

  for (int i = 0; i < IPC_QUEUE_SIZE; i++) {
      ipc_queue[i].used = 0;
  }
    
  tasks[0].id           = 0;
  tasks[0].state        = TASK_RUNNING;
  tasks[0].priority     = 0;
  tasks[0].time_slice   = 10;
  tasks[0].ticks_used   = 0;

  task_count    = 1;
  current_task  = 0;

  kprint("SCHED Scheduler initialized.\n");
}

int task_create(addr_t entry, uint8_t priority, const char *name) {
   if (task_count >= MAX_TASKS) {
      kprint("SCHED ERROR: Max task limit reached!\n");
      return -1;
 }
if (entry == 0) {
   kprint("SCHED ERROR: İnvalid entry point!\n");
   return -1;
}

for (int i = 0; i < MAX_TASKS; i++) {
    if (tasks[i].state == TASK_DEAD) {
        tasks[i].id          = i;
        tasks[i].state       = TASK_READY;
        tasks[i].priority    = priority;
        tasks[i].stack_ptr   = entry;
        tasks[i].time_slice  = 10 - priority;
        tasks[i].ticks_used  = 0;
        tasks[i].sleep_ticks = 0;

        for (int j = 0; j < 15 && name[j]; j++)
            tasks[i].name[j] = name[j];
        tasks[i].name[15] = '\0';

        task_count++;
        kprint("[SCHED] Task created.\n");
        return i;
      }
 }
 return -1;
}

void task_kill(uint32_t id) {
    if (id >= MAX_TASKS) return;
    if (id == 0) {
       kprint("[SCHED] ERROR: Cannot kill kernel task!\n");
       return;
      }
       tasks[id].state = TASK_DEAD;
       task_count--;
       kprint("SCHED Task Killed.\n");
    }


void task_sleep(uint32_t id, uint32_t ticks) {
    if (id >= MAX_TASKS) return;
    if (tasks[id].state == TASK_DEAD) return;
    tasks[id].sleep_ticks = ticks;
    tasks[id].state       = TASK_SLEEPING;
    kprint("SCHED Task sleeping.\n");
}

void task_set_priority(uint32_t id, uint8_t priority) {
    if (id >= MAX_TASKS) return;
    if (priority > 9) priority = 9;
    tasks[id].priority    = priority;
    tasks[id].time_slice = 10 - priority;
}

// ----------
// By Bipgower
// -----------


void schedule_next() {
   tick_count++;
   tasks[current_task].ticks_used++;


   for (int i = 0; i < MAX_TASKS; i++) {
       if (tasks[i].state == TASK_SLEEPING) {
           if (tasks[i].sleep_ticks > 0)
              tasks[i].sleep_ticks--;
           if (tasks[i].sleep_ticks == 0) {
               tasks[i].state = TASK_READY;
               kprint("SCHED Task woke up.\n");
            }
         }
     }


     if (tasks[current_task].ticks_used < tasks[current_task].time_slice)
        return;

     tasks[current_task].ticks_used = 0;


     uint32_t next = current_task;
     uint8_t best = 0xFF;

     for (int i = 0; i < MAX_TASKS; i++) {
        uint32_t idx = (current_task + 1 + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_READY) {
           if (tasks[idx].priority < best) {
              best = tasks[idx].priority;
              next = idx;
           }
       }
   }

   if (next == current_task) return;

   tasks[current_task].state = TASK_READY;
   tasks[next].state         = TASK_RUNNING;

   uint32_t prev = current_task;
   current_task  = next;

   context_switch(&tasks[prev].stack_ptr, tasks[next].stack_ptr);
   }

   /* --- Block / Unblock -----------------------------*/
   void task_block(uint32_t id) {
       if (id >= MAX_TASKS) return;
       tasks[id].state = TASK_BLOCKED;
   }


   void task_unblock(uint32_t id) {
       if (id >= MAX_TASKS) return;
       if (tasks[id].state ==  TASK_BLOCKED)
          tasks[id].state = TASK_READY;
   }


   int ipc_send(uint32_t from, uint32_t to, addr_t data, uint32_t len) {
       if (from >= MAX_TASKS || to >= MAX_TASKS) return -1;
       if (tasks[to].state == TASK_DEAD) return -1;

       for (int i = 0; i < IPC_QUEUE_SIZE; i++) {
           if (!ipc_queue[i].used) {
              ipc_queue[i].from  = from;
              ipc_queue[i].to    = to;
              ipc_queue[i].data  = data;
              ipc_queue[i].len   = len;
              ipc_queue[i].used  = 1;


              if (tasks[to].state == TASK_BLOCKED)
                 tasks[to].state = TASK_READY;
              return 0;
          }
     }
     kprint("SCHED ERROR: IPC queue full!\n");
     return -1;
}


int ipc_receive(uint32_t id, addr_t *data, uint32_t *len) {
    if (id >= MAX_TASKS) return -1;


    for (int i = 0; i < IPC_QUEUE_SIZE; i++) {
        if (ipc_queue[i].used && ipc_queue[i].to == id) {
           *data = ipc_queue[i].data;
           *len  = ipc_queue[i].len;
           ipc_queue[i].used = 0;
           return 0;
         }
      }
       return -1;
}

uint32_t scheduler_get_tick()   { return tick_count;  }
uint32_t scheduler_task_count() { return task_count;  }
uint32_t scheduler_current()    { return current_task;}
  
