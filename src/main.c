#include "kernel.h"

int main()
{
    printf("Initial state\n");
    state* s = init();
    log_state(s);

    printf("Forking init\n");
    s->registers[0] = Fork;
    s->registers[1] = MAX_PRIORITY;
    transition(s, SysCall);
    log_state(s);

    printf("Asking for a new channel, in r4\n");
    s->registers[0] = NewChannel;
    transition(s, SysCall);
    s->registers[4] = s->registers[0];
    log_state(s);

    printf("Making init wait for a message on the channel\n");
    printf("This should switch to the child process since init is BlockedReading\n");
    s->registers[0] = Receive;
    s->registers[1] = -1;
    s->registers[2] = -1;
    s->registers[3] = -1;
    transition(s, SysCall);
    log_state(s);

    printf("Getting a new channel in r3\n");
    s->registers[0] = NewChannel;
    transition(s, SysCall);
    s->registers[3] = s->registers[0];
    log_state(s);

    printf("What about having a child of our own?\n");
    s->registers[0] = Fork;
    s->registers[1] = MAX_PRIORITY - 1;
    transition(s, SysCall);
    log_state(s);

    printf("Let's wait for him to die!\n");
    s->registers[0] = Wait;
    transition(s, SysCall);
    log_state(s);

    printf("On with the grandchild, which'll send on channel r3\n");
    s->registers[0] = Send;
    s->registers[1] = s->registers[3];
    s->registers[2] = -12;
    transition(s, SysCall);
    log_state(s);

    printf("On with idle, to listen to the grandchild!\n");
    s->registers[0] = Receive;
    s->registers[1] = 1;       /* Little hack, not supposed to know it's gonna be channel one */
    s->registers[2] = -1;
    s->registers[3] = -1;
    s->registers[4] = -1;
    transition(s, SysCall);
    log_state(s);

    printf("Letting the timer tick until we're back to the grandchild\n");
    for (int i = MAX_TIME_SLICES; i >= 0; i--)
    {
        transition(s, Timer);
    }
    log_state(s);

    printf("Hara-kiri\n");
    s->registers[0] = Exit;
    s->registers[1] = 125;
    transition(s, SysCall);
    log_state(s);

    printf("Let's speak to dad!\n");
    s->registers[0] = Send;
    s->registers[1] = s->registers[4];
    s->registers[2] = 42;
    transition(s, SysCall);
    log_state(s);

    printf("Our job is done, back to dad! (see 42 in r2?)\n");
    s->registers[0] = Exit;
    s->registers[1] = 12;       /* Return value */
    transition(s, SysCall);
    log_state(s);

    printf("Let's loot the body of our child (see 12 in r2?)\n");
    s->registers[0] = Wait;
    transition(s, SysCall);
    log_state(s);

    return 0;
}
