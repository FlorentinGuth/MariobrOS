#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include "list.h"
#include "queue.h"


#define MAX_TIME_SLICES 5
#define MAX_PRIORITY    15
#define NUM_PROCESSES   32
#define NUM_CHANNELS    128
#define NUM_REGISTERS   5

typedef int pid;
typedef int chanid;
typedef int value;
typedef int interrupt;
typedef int priority;

typedef struct {
    value r0;
    value r1;
    value r2;
    value r3;
    value r4;
} registers;


typedef struct {
    enum {Free, BlockedWriting, BlockedReading, Waiting, Runnable, Zombie} tag;
    union {
        chanid chan;    // for BlockedWriting
        list chans;     // for BlockedReading
    };      
} process_state;

typedef struct {
    pid parent_id;
    process_state state;
    int slices_left;
    int saved_context[NUM_REGISTERS];
} process;


typedef struct {
    pid id;
    priority prio;
    value v;
} cs_sender;
struct CsReceivers {
    pid id;
    priority prio;
    struct CsReceivers* tail;
};
typedef struct CsReceivers* cs_receivers;
typedef struct {
    enum {Unused, Sender, Receivers} tag;
    union {
        cs_sender sender;
        cs_receivers* receivers;
      };
} channel_state;



typedef struct {
    pid curr_pid;
    priority curr_prio;
    int registers[NUM_REGISTERS];
    process processes[NUM_PROCESSES];
    channel_state channels[NUM_CHANNELS];
    queue* runqueues[MAX_PRIORITY + 1]; // Set of processes (pids) ordered by priority
} state;

registers get_registers(state*);
void      set_registers(state*, registers);


typedef enum {
    Timer, 
    SysCall,
} event;

typedef enum {
    NewChannel = 0,
    Send = 1,
    Receive = 2,
    Fork = 3,
    Exit = 4,
    Wait = 5,    
    Invalid,
} syscall_type;

typedef struct {
    chanid chan;
    value v;
} syscall_send;
typedef struct {
    priority prio;
    value r2;
    value r3;
    value r4;
} syscall_fork;
typedef struct {
    syscall_type type;
    union {
        syscall_send send;
        list* recv; // chanid list
        syscall_fork fork;
    };
} syscall;

syscall* decode(state*);

void sc_fork(state*, priority, value, value, value);
void sc_exit(state*);
void sc_wait(state*);

void sc_new_channel(state*);
void sc_send(state*, chanid, value);
void sc_receive(state*, list*); 


state* init();
void transition(state*, event);

void log_state(state*);

#endif