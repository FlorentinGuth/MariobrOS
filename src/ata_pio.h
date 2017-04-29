#ifndef ATA_PIO_H
#define ATA_PIO_H

#include "error.h"

/* The default and seemingly universal sector size for CD-ROMs. */
#define ATAPI_SECTOR_SIZE 2048

/* The default ISA IRQ numbers of the ATA controllers. */
#define ATA_IRQ_PRIMARY     0x0E
#define ATA_IRQ_SECONDARY   0x0F

/* Valid values for "bus" */
#define ATA_BUS_PRIMARY     0x1F0
#define ATA_BUS_SECONDARY   0x170
/* Valid values for "drive" */
#define ATA_DRIVE_MASTER    0xA0
#define ATA_DRIVE_SLAVE     0xB0

/* The necessary I/O ports of the primary bus */
#define ATA_DATA         0x1F0
#define ATA_FEATURES     0x1F1
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_ADDRESS1     0x1F3 /* LBA low  */
#define ATA_ADDRESS2     0x1F4 /* LBA mid  */
#define ATA_ADDRESS3     0x1F5 /* LBA high */
#define ATA_DRIVE_SELECT 0x1F6
#define ATA_COMMAND      0x1F7
#define ATA_DCR          0x3F6 /* device control register */

#define ATA_DELAY {inb(ATA_DCR); inb(ATA_DCR); inb(ATA_DCR); inb(ATA_DCR);}

/* The necessary I/O ports, indexed by "bus". */
#define ATA_DATA_BUS(x)         (x)
#define ATA_FEATURES_BUS(x)     (x+1)
#define ATA_SECTOR_COUNT_BUS(x) (x+2)
#define ATA_ADDRESS1_BUS(x)     (x+3) /* LBA low  */
#define ATA_ADDRESS2_BUS(x)     (x+4) /* LBA mid  */
#define ATA_ADDRESS3_BUS(x)     (x+5) /* LBA high */
#define ATA_DRIVE_SELECT_BUS(x) (x+6)
#define ATA_COMMAND_BUS(x)      (x+7)
#define ATA_DCR_BUS(x)          (x+0x206) /* device control register */

/* ATA specifies a 400ns delay after drive switching -- often
 * implemented as 4 Alternative Status queries. */
#define ATA_SELECT_DELAY_BUS(bus) \
  {inb(ATA_DCR(bus));inb(ATA_DCR(bus));inb(ATA_DCR(bus));inb(ATA_DCR(bus));}

/* Status Byte layout masks */
#define ATA_ERR 0x01 // Error (except DF)
#define ATA_DRQ 0x08 // PIO data transfer
#define ATA_SRV 0x10 // Overlapped Mode Service Request
#define ATA_DF  0x20 // Drive Fault Error
#define ATA_RDY 0x40 // Ready
#define ATA_BSY 0x80 // Busy (if set, disregard the rest)

/* Control Register orders */

// They should generally remain unset
#define ATA_NIEN 0x1 // Stops the device from sending interrupts
#define ATA_SRST 0x2 // Software Reset on all drives of the bus
#define ATA_HOB  0x7 /* Read back the high order byte of the last LBA48 value
                      * sent to the IO port */


/**
 *  @name identify - IDENTIFY command for ATA PIO mode
 *  @return        - Error code
 */
unsigned char identify();

/**
 *  @name check_disk - Sets the disk_present variable accordingly
 */
void check_disk();

/**
 *  @name readPIO - Reads the hard drive using PIO with LBA 28 bits
 *
 *  @param lba          - The LBA 28 bits address
 *  @param sector_count -
 *  @param buffer       - The buffer that will contain the read words
 */
void readPIO(u_int32 lba, unsigned char sector_count, u_int16 buffer[256]);

/**
 *  @name writePIO - Writes to the hard drive using PIO with LBA 28 bits
 *
 *  @param lba          - The LBA 28 bits address
 *  @param sector_count -
 *  @param buffer       - The buffer that contains the words to write
 */
void writePIO(u_int32 lba, unsigned char sector_count, u_int16 buffer[256]);

#endif
