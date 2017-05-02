#ifndef KERNEL_H
#define KERNEL_H

#include <stdlib.h>
#include "list.h"
#include "queue.h"


#define NUM_CHANNELS    128

typedef int chanid;
typedef int value;
typedef int interrupt;


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






typedef enum {
    Timer, 
    SysCall,
} event;

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


void sc_new_channel(state*);
void sc_send(state*, chanid, value);
void sc_receive(state*, list*); 



#endif
