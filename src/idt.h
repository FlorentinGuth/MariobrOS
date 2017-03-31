#ifndef IDT_H
#define IDT_H

/* Defines an IDT entry */
typedef struct idt_entry idt_e;
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;        /* Our kernel segment goes here */
    unsigned char always0;     /* This will ALWAYS be set to 0 */
    unsigned char flags;       /* Set using the above table */
    unsigned short base_hi;
} __attribute__((packed));

typedef struct idt_ptr idt_ptr;
struct idt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

#endif
