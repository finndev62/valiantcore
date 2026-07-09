#ifndef SCHEDULER64_H
#define SCHEDULER64_H

#include "kernel.h"

#define TASK_RUNNING  0
#define TASK_READY    1
#define TASK_BLOCKED  2
#define TASK_DEAD     3
#define TASK_SLEEPING 4

#define MAX_TASKS  512
#define STACK_SIZE  4096

/* ValiantCore Aarch64 #define definitions end-of-line */

typedef struct {
   uint32_t id;
   uint8_t state;
   uint8_t priority;
   addr_t stack_ptr;
   addr_t stack_base;
   uint32_t time_slice;
   uint32_t ticks_used;
   uint32_t sleep_ticks;
   char     name[16];
} task_t;
/*==============================================*/

void    init_scheduler(void);
int     task_create(addr_t entry, uint8_t priority, const char *name);
void    task_kill(uint32_t id);
void    task_block(uint32_t id);
void    task_unblock(uint32_t id);
void    task_sleep(uint32_t id, uint32_t ticks);
void    task_set_priority(uint32_t id, uint8_t priority);
void    schedule_next(void);
int     ipc_send(uint32_t from, uint32_t to, addr_t data, uint32_t len);
int     ipc_receive(uint32_t id, addr_t *data, uint32_t *len);
uint32_t scheduler_get_tick(void);
uint32_t scheduler_task_count(void);
uint32_t scheduler_current(void);

#endif /* SCHEDULER64_H */
