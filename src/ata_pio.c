#include "ata_pio.h"



void identify()
{
  u_int8 status1 = inb(ATA_COMMAND);
  u_int8 status2;
  if(status1==0xFF) {
    throw("Floating bus"); }
  outb(ATA_DRIVE_SELECT,ATA_DRIVE_MASTER);
  outb(ATA_ADDRESS1,0);
  outb(ATA_ADDRESS2,0);
  outb(ATA_ADDRESS3,0);
  outb(ATA_COMMAND,0xEC); // Identify command
  ATA_DELAY;
  status1 = inb(ATA_COMMAND);
  if(!status1) {
    throw("The required drive does not exist."); }
  while(inb(ATA_COMMAND) & ATA_RDY) { // poll
    status1 = inb(ATA_ADDRESS2);
    status2 = inb(ATA_ADDRESS3);
    if(status1 | status2) {
      writef("%x, %x\n",status1,status2);
      return;
    }
  }
  while(inb(ATA_COMMAND) & (ATA_ERR | ATA_DRQ));
  if(inb(ATA_COMMAND & ATA_ERR)) {
    throw("Detected drive error"); }
}
