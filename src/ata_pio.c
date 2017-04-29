#include "ata_pio.h"


unsigned char disk_id = 8;


/**
 *  @name poll - Waits for the drive to be ready to transfer data
 *  @return  0 - No error
 *           1 - ERR (Error) bit set
 *           2 - DF (Drive Fault) bit set
 */
char poll()
{
  u_int8 status = inb(ATA_COMMAND);
  while(status & ATA_BSY) {
    status = inb(ATA_COMMAND);
  }
  if(status & ATA_ERR) { return 1; }
  if(status & ATA_DF ) { return 2; }
  while(!(status & ATA_DRQ)) {
    if(status & ATA_ERR) { return 1; }
    if(status & ATA_DF ) { return 2; }
    status = inb(ATA_COMMAND);
  }
  return 0;
}

void flush_cache()
{
  outb(ATA_COMMAND, 0xE7);
  while(inb(ATA_COMMAND) & ATA_BSY);
}

unsigned char identify()
{
  u_int8 status1 = inb(ATA_COMMAND);
  if(status1==0xFF) {
    return 1; // Floating bus
  }
  u_int8 status2;
  outb(ATA_DRIVE_SELECT,ATA_DRIVE_MASTER);
  ATA_DELAY; // Necessary delay after the initial choice
  outb(ATA_SECTOR_COUNT,0);
  outb(ATA_ADDRESS1,0);
  outb(ATA_ADDRESS2,0);
  outb(ATA_ADDRESS3,0);
  outb(ATA_COMMAND,0xEC); // Identify command
  status1 = inb(ATA_COMMAND);
  if(!status1) {
    return 2; // The required drive does not exist
  }
  while(inb(ATA_COMMAND) & ATA_BSY); // poll
  status1 = inb(ATA_ADDRESS2);
  status2 = inb(ATA_ADDRESS3);
  if(status1 | status2) {
    if(status1==0x14 && status2==0xEB) {
      return 3; // ATAPI disk, not ATA
    } else if(status1==0x3c && status2==0xc3) {
      return 4; // SATA disk, not ATA
    } else if(status1==0x69 && status2==0x96) {
      return 5; // SATAPI disk, not ATA
    }
    return 6; // Not an ATA disk
  }
  if(poll()) {
    return 7; // Detected drive error
  }
  u_int16 to_read[256];
  for(int i=0; i<256; i++) {
    to_read[i] = inw(ATA_DATA);
  }
  if(to_read[83] & (1<<10)) {
    writef("Support LBA48\n");
    u_int32 lba48_high = (to_read[100]<<16)+to_read[101];
    u_int32 lba48_low  = (to_read[102]<<16)+to_read[103];
    writef("48 bit LBA sectors : (%u<<32) + %u\n", lba48_high, lba48_low);
  }
  writef("UDMA mode : supported=%x, chosen=%x\n", to_read[88]&0xF, (to_read[88]>>4)&0xF);
  if(to_read[93] & (1<<11)) {
    writef("Detected 80 conductor table\n");
  }
  u_int32 lba28 = (to_read[60]<<16) + to_read[61];
  if(lba28) {
    writef("28 bit LBA sectors : %u\n", lba28);
  }
  outb(ATA_DCR,0);
  ATA_DELAY;
  outb(ATA_DCR,ATA_NIEN); // Prevents ATA IRQs from firing
  ATA_DELAY;  
  return 0;
}

void identify_throw(unsigned char flag)
{
  switch(flag) {
  case 0: {throw("There shouldn't be any error!"); break;}
  case 1: {throw("Floating bus"); break;}
  case 2: {throw("The required drive does not exist"); break;}
  case 3: {throw("ATAPI disk, not ATA"); break;}
  case 4: {throw("SATA disk, not ATA"); break;}
  case 5: {throw("SATAPI disk, not ATA"); break;}
  case 6: {throw("Not an ATA disk"); break;}
  case 7: {throw("Detected drive error"); break;}
  case 8: {throw("No identification made"); break;}
  default: {throw("Unknown response to the IDENTIFY command");}
  }
}

void check_disk()
{
  disk_id = identify();
}

void software_reset()
{
  if(disk_id) { identify_throw(disk_id); }
  outb(ATA_DCR, ATA_SRST);
  ATA_DELAY;
  outb(ATA_DCR, 0);
  ATA_DELAY;
}

void readPIO(u_int32 lba, unsigned char sector_count, u_int16 buffer[256])
{
  if(disk_id) { identify_throw(disk_id); }
  outb(ATA_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
  ATA_DELAY;
  /* outb(0x1F1, 0x00) // Supposedly useless */
  outb(ATA_SECTOR_COUNT, sector_count);
  outb(ATA_ADDRESS1, (u_int8) lba);
  outb(ATA_ADDRESS2, (u_int8) (lba>>8));
  outb(ATA_ADDRESS3, (u_int8) (lba>>16));
  outb(ATA_COMMAND, 0x20); // READ SECTORS command
  if(poll()) {
    throw("Error while reading");
  }
  for(int i=0; i<256; i++) {
    buffer[i] = inw(ATA_DATA);
  }
  ATA_DELAY;
}

void writePIO(u_int32 lba, unsigned char sector_count, u_int16 buffer[256])
{
  if(disk_id) { identify_throw(disk_id); }
  outb(ATA_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));
  ATA_DELAY;
  /* outb(0x1F1, 0x00) // Supposedly useless */
  outb(ATA_SECTOR_COUNT, sector_count);
  outb(ATA_ADDRESS1, (u_int8) lba);
  outb(ATA_ADDRESS2, (u_int8) (lba>>8));
  outb(ATA_ADDRESS3, (u_int8) (lba>>16));
  outb(ATA_COMMAND, 0x30); // WRITE SECTORS command
  if(poll()) {
    throw("Error while writing");
  }
  for(int i=0; i<256; i++) {
    outw(ATA_DATA,buffer[i]);
  }
  ATA_DELAY;
}
