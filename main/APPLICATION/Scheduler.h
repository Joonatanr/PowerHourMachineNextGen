/*
 * Scheduler.h
 *
 *  Created on: 16. m�rts 2018
 *      Author: JRE
 */

#ifndef LOGIC_SCHEDULER_H_
#define LOGIC_SCHEDULER_H_

#include "typedefs.h"

#define SCHEDULER_PERIOD 50u //Define this at 50 milliseconds.

typedef void (*Scheduler_TaskFunc)(void);


typedef enum
{
    APPLICATION_POWER_HOUR,
    APPLICATION_DEDICATION,
    NUMBER_OF_APPLICATIONS
} Scheduler_LogicModuleEnum;


typedef enum
{
    TASK_CYCLIC100MS_BUZZER,
    TASK_CYCLIC100MS_BUTTONS,
    TASK_CYCLIC100MS_BITMAP_LOADER,
    TASK_CYCLIC100MS_MESSAGEBOX,

    NUMBER_OF_SCHEDULER_TASKS
} Scheduler_LogicTaskEnum;

typedef struct
{
    U16 period;
    Scheduler_TaskFunc init_fptr;
    Scheduler_TaskFunc start_fptr;
    Scheduler_TaskFunc stop_fptr;
    Scheduler_TaskFunc cyclic_fptr;
} Scheduler_LogicTask;

extern void Scheduler_initTasks(void);
extern void Scheduler_cyclic(void);
extern void Scheduler_StartTasks(void);

extern void Scheduler_SetActiveApplication(Scheduler_LogicModuleEnum task);
extern void Scheduler_StopActiveApplication(void);
extern void Scheduler_SetActiveApplicationPause(Boolean pause);

extern void returnToMain(void);

#endif /* LOGIC_SCHEDULER_H_ */
