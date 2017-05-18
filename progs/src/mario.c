#include "lib.h"
#include "printer.h"

void print_offset(fd file, u_int32 size, u_int8 offset, pos_t init, \
                  u_int8 buffer[])
{
  offset %= SCREEN_WIDTH;
  u_int32 copied;
  pos_t row = ROW(init);
  set_cursor_pos(POS(row, offset));
  while(size) {
    copied = read(file, (void*) buffer, 0, 256);
    size -= copied;
    
    u_int16 i = 0;
    u_int32 in_ofs = 0;
    while(in_ofs + i < copied) {
      for(; in_ofs + i < copied /* && i < SCREEN_WIDTH - offset */; i++) {
        if(buffer[in_ofs + i] == '\n') {
          break;
        }
        writef("%c",buffer[in_ofs + i]);
      }
      /* if(i == SCREEN_WIDTH - offset) { */
      /*   set_cursor_pos(POS(row, 0)); */
      /*   for(; in_ofs + i < copied; i++) { */
      /*     if(buffer[in_ofs + i] == '\n') { */
      /*       for(int j = get_cursor_pos() % SCREEN_WIDTH; j < offset; j++) { */
      /*         writef("%c",' '); */
      /*       } */
      /*       row++; */
      /*       set_cursor_pos(POS(row, 0)); */
      /*       for(int j = 0; j < offset; j++) { */
      /*         writef("%c",' '); */
      /*       } */
      /*       in_ofs += i + 1; */
      /*       i = 0; */
      /*       break; */
      /*     } */
      /*     writef("%c",buffer[in_ofs + i]); */
      /*   } */
      /* } else  */if(buffer[in_ofs + i] == '\n') {
        for(int j = get_cursor_pos() % SCREEN_WIDTH; j < SCREEN_WIDTH; j++) {
          writef("%c",' ');
        }
        row++;
        set_cursor_pos(POS(row, 0));
        for(int j = 0; j < offset; j++) {
          writef("%c",' ');
        }
        in_ofs += i + 1;
        i = 0;
      }
    }
  }
}

int main()
{
  fd still;
  fd rev;
  stats* st1 = (void*) malloc(sizeof(stats));
  stats* st2 = (void*) malloc(sizeof(stats));
  u_int8 buffer[256];
  still = open("/data/mario_still", O_RDONLY, 0);
  rev   = open("/data/mario_rev", O_RDONLY, 0);
  if(!still || !rev) {
    printf("Mario is not set\n");
    return 1;
  }
  clear();
  fstat(still, st1);
  fstat(rev,   st2);
  pos_t pos = get_cursor_pos();
  s_int16 offset = 0;
  /* u_int16 scan = 1; */
  for(;;) {
    for(offset = 10; offset < 30; offset++) {
      print_offset(still, st1->st_size, offset, pos, buffer);
      lseek(still, 0, SEEK_SET);
    }
    for(; offset > 10; offset--) {
      print_offset(rev, st2->st_size, offset, pos, buffer);
      lseek(rev, 0, SEEK_SET);
    }
  }
  /* while(kbdus[scan] != '\n') { */
  /*   if(scan) { */
  /*     if(scan == 75) { */
  /*       clear(); */
  /*       offset = 30; */
  /*       print_offset(still, st->st_size, offset % SCREEN_WIDTH, pos, buffer); */
  /*     } else if(scan == 77) { */
  /*       clear(); */
  /*       offset = 2; */
  /*       print_offset(still, st->st_size, offset % SCREEN_WIDTH, pos, buffer); */
  /*     } */
  /*   } */
  /*   scan = keyget(); */
  /* } */
  printf("\n");
  close(still);
  return 0;
}
