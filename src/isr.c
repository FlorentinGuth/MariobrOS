#include "isr.h"
#include "error.h"
#include "gdt.h"

/* This is a very repetitive function... it's not hard, it's
 *  just annoying. As you can see, we set the first 32 entries
 *  in the IDT to the first 32 ISRs. We can't use a for loop
 *  for this, because there is no way to get the function names
 *  that correspond to that given entry. We set the access
 *  flags to 0x8E. This means that the entry is present, is
 *  running in ring 0 (kernel level), and has the lower 5 bits
 *  set to the required '14', which is represented by 'E' in
 *  hex. */
void isrs_install()
{
  idt_set_gate(0,  (unsigned)isr0,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(1,  (unsigned)isr1,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(2,  (unsigned)isr2,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(3,  (unsigned)isr3,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(4,  (unsigned)isr4,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(5,  (unsigned)isr5,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(6,  (unsigned)isr6,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(7,  (unsigned)isr7,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(8,  (unsigned)isr8,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(9,  (unsigned)isr9,  KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(10, (unsigned)isr10, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(11, (unsigned)isr11, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(12, (unsigned)isr12, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(13, (unsigned)isr13, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(14, (unsigned)isr14, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(15, (unsigned)isr15, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(16, (unsigned)isr16, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(17, (unsigned)isr17, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(18, (unsigned)isr18, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(19, (unsigned)isr19, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(20, (unsigned)isr20, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(21, (unsigned)isr21, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(21, (unsigned)isr22, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(23, (unsigned)isr23, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(24, (unsigned)isr24, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(25, (unsigned)isr25, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(26, (unsigned)isr26, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(27, (unsigned)isr27, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(28, (unsigned)isr28, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(29, (unsigned)isr29, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(30, (unsigned)isr30, KERNEL_CODE_SEGMENT, 0);
  idt_set_gate(31, (unsigned)isr31, KERNEL_CODE_SEGMENT, 0);

  kloug(100, "ISR installed\n");
}

void *isr_routines[32] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char *exception_messages[] = {
  "Division By Zero",
  "Debug",
  "Non Maskable Interrupt",
  "Breakpoint",
  "Into Detected Overflow",
  "Out of Bounds",
  "Invalid Opcode",
  "No Coprocessor",
  "Double Fault",
  "Coprocessor Segment Overrun",
  "Bad TSS",
  "Segment Not Present",
  "Stack Fault",
  "General Protection Fault",
  "Page Fault",
  "Unknown Interrupt",
  "Coprocessor Fault",
  "Alignment Check",
  "Machine Check",
  "Reserved(19)",
  "SysCall",
  "Reserved(21)",
  "Reserved(22)",
  "Reserved(23)",
  "Reserved(24)",
  "Reserved(25)",
  "Reserved(26)",
  "Reserved(27)",
  "Reserved(28)",
  "Reserved(29)",
  "Reserved(30)",
  "Reserved(31)"
};


/* This installs a custom ISR handler for the given ISR */
void isr_install_handler(int isr, void (*handler)(struct regs *r))
{
  isr_routines[isr] = handler;
}

/* This clears the handler for a given ISR */
void isr_uninstall_handler(int isr)
{
  isr_routines[isr] = 0;
}

void illegal_opcode_handler(struct regs *r)
{
  writef("Illegal Opcode Exception. EIP at %x",r->eip);
  throw("Exception. System Halted");
}

void double_fault_handler(struct regs *r)
{
  writef("Double fault! EIP at %x, ESP at %x, USERESP at %x\n", r->eip,  r->esp,  r->useresp);
  throw("System halted");
}

void gpf_handler(struct regs *r)
{
  writef("General Protection Fault!\n");
  writef("Useresp: %x, ss: %x, eip:%x, errcode: %x\n", r->useresp, r->ss, r->eip, r->err_code);
  throw("System halted");
}

/* All of our Exception handling Interrupt Service Routines will
 * point to this function. This will tell us what exception has
 * happened! Right now, we simply halt the system by hitting an
 * endless loop. All ISRs disable interrupts while they are being
 * serviced as a 'locking' mechanism to prevent an IRQ from
 * happening and messing up kernel data structures */
void fault_handler(struct regs *r)
{
  /* Is this a fault whose number is from 0 to 31? */
  if (r->int_no < 32) {
    void (*handler)(struct regs *r);
    handler = isr_routines[r->int_no];
    if(handler) {
      handler(r);
    } else {
      log_string(exception_messages[r->int_no], Error);
      write_string(exception_messages[r->int_no]);
      throw("Exception. System Halted");
    }
  }
}
