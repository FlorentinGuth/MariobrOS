#include "ata_pio.h"



void identify()
{
  u_int8 status = inb(ATA_COMMAND(ATA_BUS_PRIMARY));
  if(status==0xFF)
    throw("Floating bus");
  outb(ATA_DRIVE_SELECT(ATA_BUS_PRIMARY),ATA_DRIVE_MASTER);
  outb(ATA_ADDRESS1(ATA_BUS_PRIMARY),0);
  outb(ATA_ADDRESS2(ATA_BUS_PRIMARY),0);
  outb(ATA_ADDRESS3(ATA_BUS_PRIMARY),0);
  outb(ATA_COMMAND(ATA_BUS_PRIMARY),0xEC); // Identify command
  ATA_SELECT_DELAY(ATA_BUS_PRIMARY);
  status = inb(ATA_COMMAND(ATA_BUS_PRIMARY));
  if(!status)
    throw("The required drive does not exist.");
  while(inb(ATA_COMMAND(ATA_BUS_PRIMARY)) & ATA_RDY) { // poll
    if(inb(ATA_ADDRESS2(ATA_BUS_PRIMARY) | ATA_ADDRESS3(ATA_BUS_PRIMARY))) {
      throw("Not an ATA drive.");
    }
  }
  while(inb(ATA_COMMAND(ATA_BUS_PRIMARY)) & (ATA_ERR | ATA_DRQ));
  if(inb(ATA_COMMAND(ATA_BUS_PRIMARY) & ATA_ERR))
    throw("Detected drive error");
}
