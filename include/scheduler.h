#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <kernel.h>

#define TASK_RUNNING 0
#define TASK_READY   1
#define TASK_BLOCKED 2
#define TASK_DEAD    3

 // ---------------
 //  By Bıgpower
 // ---------------

 #define MAX_TASKS   64
 #define STACK_SIZE 4096


 typedef struct {
     uint32_t   id;
     uint8_t    state;
     uint8_t    priority;
     addr_t     stack_ptr;
     addr_t     stack_base;
     uint32_t   time_slice;
     uint32_t   ticks_used;
     char       name[16];
} task_t;
// -----------------------------------------------------------
// ____  _________  ____  ____ _       _______ ____  _______ _  _______ __ 
//     / __ )/  _/ ____// __ \/ __ \ | / / ____/ __ \/ / / / ____/ __ \/ |/ / ____/ / 
//    / __  |/ // / __ / /_/ / / / / |/ / __/ / /_/ / / / / __/ / /_/ /  / / __/ / /  
//   / /_/ // // /_/ // ____/ /_/ /|  / /___/ _, _/ /_/ / /___/ _, _/ /|  / /___/ /___
//  /_____/___/\____//_/    \____/ | /_____/_/ |_|\____/_____/_/ |_/_/ |_/_____/_____/
//                                 |/
// ------------------------------------------------------------


void    init_scheduler();
int     task_create(addr_t entry, uint8_t priority, const char *name);
void    task_kill(uint32_t id);
void    task_block(uint32_t id);
void    task_unblock(uint32_t id);
void    schedule_next();
uint32_t scheduler_get_tick();
uint32_t scheduler_task_count();
uint32_t scheduler_current();

#endif
