#include "kernel.h"
#include "list.h"
#include "queue.h"
#include <stdio.h>


registers get_registers(state* s)
{
    registers regs = {
        .r0 = s->registers[0],
        .r1 = s->registers[1],
        .r2 = s->registers[2],
        .r3 = s->registers[3],
        .r4 = s->registers[4],
    };
    return regs;
}

void set_registers(state* s, registers regs)
{
    s->registers[0] = regs.r0;
    s->registers[1] = regs.r1;
    s->registers[2] = regs.r2;
    s->registers[3] = regs.r3;
    s->registers[4] = regs.r4;
}


syscall* decode(state* s)
{
    syscall* sc = malloc(sizeof(syscall));
    switch (s->registers[0])
    {
        case NewChannel:
            sc->type = NewChannel;
            break;

        case Send:
            sc->type = Send;
            sc->send.chan = s->registers[1];
            sc->send.v = s->registers[2];
            break;

        case Receive:
            sc->type = Receive;
            list* l = malloc(sizeof(list));
            *l = 0;

            int chan_seen[NUM_CHANNELS] = {0};
            for (int i = 1; i <= 4; i++)
            {
                chanid c = s->registers[i];
                if (c >= 0 && c < NUM_CHANNELS && !chan_seen[c])
                {
                    append(l, c);
                    chan_seen[c] = 1;
                }
            }
            sc->recv = l;
            break;

        case Fork:
            sc->type = Fork;
            sc->fork.prio = s->registers[1];
            sc->fork.r2   = s->registers[2];
            sc->fork.r3   = s->registers[3];
            sc->fork.r4   = s->registers[4];
            break;

        case Exit:
            sc->type = Exit;
            break;

        case Wait:
            sc->type = Wait;
            break;

        default:
            sc->type = Invalid;
            break;
    }
    return sc;
}


void sc_fork(state* s, priority child_prio, value r2, value r3, value r4)
{
    // Research of a free process
    pid id = 0;
    while (id < NUM_PROCESSES && s->processes[id].state.tag != Free)
    {
        id += 1;
    }

    // One cannot create a child process with a higher priority than its own
    if (id == NUM_PROCESSES || child_prio > s->curr_prio)
    {
        s->registers[0] = 0;
        return;
    }

    // Initialization of different registers and other fields
    s->processes[id].parent_id = s->curr_pid;
    s->processes[id].state.tag = Runnable;
    s->processes[id].saved_context[0] = 2;
    s->processes[id].saved_context[1] = s->curr_pid;
    s->processes[id].saved_context[2] = r2;
    s->processes[id].saved_context[3] = r3;
    s->processes[id].saved_context[4] = r4;

    // Adding the process in the runqueue
    enqueue(s->runqueues[child_prio], id);
    
    // Setting the values of the parent process
    s->registers[0] = 1;
    s->registers[1] = id;
}

void sc_exit(state* s)
{
    pid id = s->curr_pid;
    s->processes[id].state.tag = Zombie;

    // Notifies the child processes of the exiting one
    for (pid i = 0; i < NUM_PROCESSES; i++)
    {
        if (s->processes[i].parent_id == id)
        {
            s->processes[i].parent_id = 1;
        }
    }

    // Checks whether the parent was waiting for us to die (how cruel!)
    pid parent_id = s->processes[id].parent_id;
    if (s->processes[parent_id].state.tag == Waiting)
    {
        // Freeing ourselves from zombie state
        s->processes[id].state.tag = Free;

        // Goodbye cruel world
        queue* q = s->runqueues[s->curr_prio];
        queue* temp = empty_queue();
        while (!is_empty_queue(q))
        {
            pid pid = dequeue(q);
            if (pid != id)
            {
                enqueue(temp, pid);
            }
        }
        s->runqueues[s->curr_prio] = temp;
        
        // Notifies the parent
        process* parent_proc = &(s->processes[parent_id]);
        parent_proc->state.tag = Runnable;
        parent_proc->saved_context[0] = 1;
        parent_proc->saved_context[1] = id;
        parent_proc->saved_context[2] = s->registers[1]; // Return value
    }
}

void sc_wait(state* s)
{
    pid parent_id = s->curr_pid;
    int has_children = 0;

    for (pid id = 0; id < NUM_PROCESSES; id++)
    {
        process proc = s->processes[id];
        if (proc.parent_id == parent_id)
        {
            has_children = 1;
            if (proc.state.tag == Zombie)
            {
                s->processes[id].state.tag = Free;
                s->registers[0] = 1;
                s->registers[1] = id;
                s->registers[2] = proc.saved_context[1]; // Return value

                // Goodbye cruel world
                for (priority prio_child = 0; prio_child <= MAX_PRIORITY; prio_child++)
                {
                    // SLOW! But no way of knowing the priority of our child...
                    // Possible improvement: directly look in the runqueues instead of procs
                    queue* q = s->runqueues[prio_child];
                    queue* temp = empty_queue();
                    while (!is_empty_queue(q))
                    {
                        pid pid = dequeue(q);
                        if (pid != id)
                        {
                            enqueue(temp, pid);
                        }
                    }
                    s->runqueues[prio_child] = temp;
                }

                return;
            }
        }
    }

    if (!has_children)
    {
        s->registers[0] = 0;
    }
    else
    {
        // The process has children but no Zombie ones
        s->processes[s->curr_pid].state.tag = Waiting;
    }
}



void sc_new_channel(state* s)
{
    // Researching an unused channel
    for (chanid c = 0; c < NUM_CHANNELS; c++)
    {
        channel_state chan = s->channels[c];
        if (chan.tag == Unused)
        {
            s->registers[0] = c;
            return;
        }
    }

    s->registers[0] = -1;
}

void sc_send(state* s, chanid chanid, value v)
{
    if (chanid < 0 || chanid >= NUM_CHANNELS || s->channels[chanid].tag == Sender)
    {
        s->registers[0] = 0;
        return;
    }

    channel_state* chan = &(s->channels[chanid]);
    if (chan->tag == Receivers)
    {
        // There are processses listening to this channel, we pick the most prioritary one
        cs_receivers* receivers = chan->receivers;
        priority max_prio = (*receivers)->prio;
        pid      max_pid  = (*receivers)->id;

        // Computation of the maximum priority
        struct CsReceivers* curr = (*receivers)->tail;
        while (curr)
        {
            if (curr->prio > max_prio)
            {
                max_prio = curr->prio;
                max_pid  = curr->id;
            }
            curr = curr->tail;
        }

        // Extraction of the maximum priority receiver
        if ((*receivers)->prio == max_prio)
        {
            struct CsReceivers* tail = (*receivers)->tail;
            free(*receivers);
            *receivers = (*receivers)->tail;
        }
        else
        {
            curr = *receivers;
            while (curr->tail && curr->tail->prio != max_prio)
            {
                curr = curr->tail;
            }
            if (curr->tail)
            {
                struct CsReceivers* tail = curr->tail->tail;
                free(curr->tail);
                curr->tail = tail;
            }
        }

        // Unblocking the selected process
        process_state state = {.tag = Runnable};
        s->processes[max_pid].state = state;
        s->processes[max_pid].saved_context[0] = 1;
        s->processes[max_pid].saved_context[1] = chanid;
        s->processes[max_pid].saved_context[2] = v;

        // Checking whether the channel is now empty
        if (!*receivers)
        {
            chan->tag = Unused;
        }
    }
    else
    {
        // Nobody is currently listening to the channel, so we wait
        pid curr_proc = s->curr_pid;
        process_state state = {.tag = BlockedWriting};
        s->processes[curr_proc].state = state;

        // Changing the channel state
        chan->tag = Sender;
        cs_sender sender = {.id = curr_proc, .prio = s->curr_prio, .v = v};
        chan->sender = sender;
    }

    s->registers[0] = 1;
}

void sc_receive(state* s, list* chans)
{
    // Checking whether we have channels or not
    if (*chans)
    {
        // Search for a channel where somebody is sending something
        struct List* curr = *chans;
        while (curr && s->channels[curr->head].tag != Sender)
        {
            curr = curr->tail;
        }

        if (curr)
        {
            // We found a sending channel: we set it to unused
            chanid chanid = curr->head;
            channel_state* state = &(s->channels[chanid]);
            cs_sender sender = state->sender;
            state->tag = Unused;

            // Notify the sender
            process* proc = &(s->processes[sender.id]);
            process_state proc_state = {.tag = Runnable};
            proc->state = proc_state;

            // Get the return value
            s->registers[0] = 1;
            s->registers[1] = chanid;
            s->registers[2] = sender.v;
        }
        else
        {
            // No sending channel, we wait
            process_state state = {.tag = BlockedReading};
            s->processes[s->curr_pid].state = state;

            // Notifiying the channels that we are listening to them
            for (struct List* curr = *chans; curr; curr = curr->tail)
            {
                struct CsReceivers* elt = malloc(sizeof(struct CsReceivers));
                elt->id = s->curr_pid;
                elt->prio = s->curr_prio;

                if (s->channels[curr->head].tag == Unused)
                {
                    s->channels[curr->head].tag = Receivers;
                    s->channels[curr->head].receivers = malloc(sizeof(cs_receivers));
                    elt->tail = NULL;
                }
                else
                {
                    // Channel already on Receivers mode, we add this process to the list
                    elt->tail = *(s->channels[curr->head].receivers);
                }
                *(s->channels[curr->head].receivers) = elt;
            }
        }
    }
    else
    {
        // No channels
        s->registers[0] = 0;
    }
}


state* init()
{
    state* s = malloc(sizeof(state));

    // Creating idle process
    process* idle = &(s->processes[0]);
    process_state runnable = {.tag = Runnable};
    idle->state = runnable;
    idle->parent_id = 0;

    // Creating init process
    process* init = &(s->processes[1]);
    init->state = runnable;
    init->parent_id = 1;

    // Initialization of state
    s->curr_pid = 1;
    s->curr_prio = 15;

    for (int r = 0; r < NUM_REGISTERS; r++)
    {
        s->registers[r] = 0;
    }

    for (chanid id = 0; id < NUM_CHANNELS; id++)
    {
        s->channels[id].tag = Unused;
    }

    for (priority prio = 0; prio <= MAX_PRIORITY; prio++)
    {
        s->runqueues[prio] = empty_queue();
    }
    enqueue(s->runqueues[0],  0);
    enqueue(s->runqueues[MAX_PRIORITY], 1);

    return s;
}

void select_new_process(state* s)
{
    process* proc = &(s->processes[s->curr_pid]);

    // Saving context
    for (int r = 0; r < NUM_REGISTERS; r++)
    {
        proc->saved_context[r] = s->registers[r];
    }

    // Searching for a runnable process
    int found = 0;
    for (priority prio = MAX_PRIORITY; prio >= 0 && !found; prio--)
    {
        queue* q = s->runqueues[prio];
        queue* temp = empty_queue();

        while (!is_empty_queue(q) && !found)
        {
            pid pid = dequeue(q);
            enqueue(temp, pid);

            if (s->processes[pid].state.tag == Runnable)
            {
                // We found a runnable process: we tell the state
                s->curr_pid = pid;
                s->curr_prio = prio;

                // Restoring context
                process* new_proc = &(s->processes[s->curr_pid]);
                for (int r = 0; r < NUM_REGISTERS; r++)
                {
                    s->registers[r] = new_proc->saved_context[r];
                }
                new_proc->slices_left = MAX_TIME_SLICES;

                found = 1;
            }
        }

        // Restoring the runqueue
        while (!is_empty_queue(temp))
        {
            enqueue(q, dequeue(temp));
        }
        free(temp);
    }
}

void transition(state* s, event e)
{
    switch (e)
    {
        case Timer: ; // Welcome to the C language : a declaration cannot follow a label
            process* proc = &(s->processes[s->curr_pid]);

            if (proc->slices_left == 0)
            {
                select_new_process(s);
            }
            else
            {
                proc->slices_left -= 1;
            }

            break;

        case SysCall: ;
            syscall* sc = decode(s);
            switch (sc->type)
            {
                case NewChannel:
                    sc_new_channel(s);
                    break;

                case Send: ;
                    sc_send(s, sc->send.chan, sc->send.v);
                    break;

                case Receive:
                    sc_receive(s, sc->recv);
                    break;

                case Fork:
                    sc_fork(s, sc->fork.prio, sc->fork.r2, sc->fork.r3, sc->fork.r4);
                    break;
                
                case Exit:
                    sc_exit(s);
                    break;
                
                case Wait: ;
                    sc_wait(s);
                    break;
            }

            if (s->processes[s->curr_pid].state.tag != Runnable)
            {
                select_new_process(s);
            }

            break;
    }
}


char* process_to_str(process p)
{
    char* state = malloc(100 * sizeof(char));
    switch (p.state.tag)
    {
        case Free:
            sprintf(state, "Free");
            break;
        
        case BlockedWriting:
            sprintf(state, "BlockedWriting (on chan %d)", p.state.chan);
            break;
        
        case BlockedReading:
            sprintf(state, "BlockedReading");
            break;

        case Waiting:
            sprintf(state, "Waiting");
            break;
            
        case Runnable:
            sprintf(state, "Runnable");
            break;

        case Zombie:
            sprintf(state, "Zombie");
            break;
    }
    
    return state;
}

char* channel_to_str(channel_state c)
{
    char* state = malloc(100 * sizeof(char));
    switch (c.tag)
    {
        case Unused:
            sprintf(state, "Unused");
            break;

        case Sender:
            sprintf(state, "%d is sending the value %d", c.sender.id, c.sender.v);
            break;

        case Receivers:
            sprintf(state, "Receivers");
            break;
    }
    return state;
}

void log_state(state* s)
{
    printf("Current process is %d (priority %d, %d slices left)\n", 
        s->curr_pid, s->curr_prio, s->processes[s->curr_pid].slices_left);
    printf("Registers are: r0=%d, r1=%d, r2=%d, r3=%d, r4=%d\n", 
        s->registers[0], s->registers[1], s->registers[2], s->registers[3], s->registers[4]);

    printf("\nRunqueues:\n");
    for (priority prio = MAX_PRIORITY; prio >= 0; prio--)
    {
        queue* q = s->runqueues[prio];
        if (*q)
        {
            printf("Priority %d:\n", prio);
            printf("%d(%s)\n", (*q)->value, process_to_str(s->processes[(*q)->value]));
            for (struct Queue* curr = (*q)->next; curr != *q; curr = curr->next)
            {
                printf("%d(%s)\n", curr->value, process_to_str(s->processes[curr->value]));
            }
        }
    }

    printf("\nChannels:\n");
    for (chanid c = 0; c < NUM_CHANNELS; c++)
    {
        if (s->channels[c].tag != Unused)
        {
            printf("%d: %s\n", c, channel_to_str(s->channels[c]));
        }
    }
    printf("\n\n");
}