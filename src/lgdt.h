/** lgdt.h:
 *  Wrapper for assembly primitive 'lgdt' and segments setting
 */

#ifndef LGDT_H
#define LGDT_H

/** load_gdt:
 *  Wrapper for ASM instruction 'lgdt'
 *
 * @param gdt The GDT to load
 */
void load_gdt(gdt_l *gdt);


/** set_segments:
 *  Setting of segment registers ds, ss, es and cs
 */
void set_segments();

#endif
