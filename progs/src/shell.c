#include "shell.h"
#include "lib.h"
#include "string.h"
#include "printer.h"

/* TODO: free unused args */


#define buffer_size 128
char blank_command[buffer_size] = {' '};
int  pos = 0, length = 0, max_length = 0;
pos_t start_of_command;

#define history_size 100
string history[history_size] = {0};
int history_length = 0, history_pos = 0;
bool modified_since_history = FALSE;

u_int32 curr_dir = 2; // root directory

unsigned char kbdus[256] =
  {
    0,  19, /* Escape */
    '&', 130, '"', '\'', '(', '-', 138, '_', /* 9 */
    135, 133, ')', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'a', 'z', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',/* Enter key */
    0, /* 29   - Control */
    'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', /* 39 */
    151, 253, /* Square */
    0,/* Left shift */
    '*', 'w', 'x', 'c', 'v', 'b', 'n', /* 49 */
    ',', ';', ':', '!',   0, /* 54 - Right shift */
    '*',// * of the numeric pad
    0,/* 56 - Alt */
    ' ',/* Space bar */
    0,/* 58 - Caps lock */
    'F',/* 59 - F1 key ... > */
    'F',   'F',   'F',   'F',   'F',   'F',   'F',   'F',
    'F',/* < ... F10 */
    0,/* 69 - Num lock*/
    0,/* 70 - Scroll Lock */
    0,/* 71 - Home key */
    0,/* 72 - Up Arrow */
    24,/* Page Up */
    '-',
    0,/* 75 - Left Arrow */
    234,
    0,/* 77 - Right Arrow */
    '+',
    0,/* 79 - End key*/
    0,/* 80 - Down Arrow */
    25,/* Page Down */
    'I',/* Insert Key */
    '\177',/* Delete Key */
    234, 234, '<',
    'F', /* F11 Key */
    'F', /* F12 Key */
    234,234,5, /* Windows key */
    234,219, /* Menu */
    234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,

    /* With shift */
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', 0, '+', '\b', /* Backspace */
    '\t', /* Tab */
    'A', 'Z', 'E', 'R', /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', 137, 156, '\n',/* Enter key */
    0, /* 29   - Control */
    'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', /* 39 */
    '%', 234,   0, /* Left shift */
    230, 'W', 'X', 'C', 'V', 'B', 'N', /* 49 */
    '?', '.', '/', 21,   0, /* 54 - Right shift */
    234,
    0,/* 56 - Alt */
    ' ',/* Space bar */
    0,/* 58 - Caps lock */
    'F',/* 59 - F1 key ... > */
    'F',   'F',   'F',   'F',   'F',   'F',   'F',   'F',
    'H',/* < ... F10 */
    0,/* 69 - Num lock*/
    0,/* 70 - Scroll Lock */
    0,/* 71 - Home key */
    0,/* 72 - Up Arrow */
    24,/* Page Up */
    '-',
    0,/* 75 - Left Arrow */
    '?',
    0,/* 77 - Right Arrow */
    '+',
    0,/* 79 - End key*/
    0,/* 80 - Down Arrow */
    25,/* Page Down */
    'I',/* Insert Key */
    '\177',/* Delete Key */
    '?',   '?',   '>',
    'F', /* F11 Key */
    'F', /* F12 Key */
    234,234,5, /* Windows key */
    234,219, /* Menu */
    234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,234,
  };

bool k_caps_lock = FALSE;
bool k_num_lock = FALSE;
bool k_scroll_lock = FALSE;
unsigned char k_shift_count = 0;
bool k_shift = FALSE;
unsigned char k_meta = 0;
unsigned char k_ctrl = 0;
unsigned char k_lights = 0x00;
bool k_dead_hat = FALSE;
bool k_dead_trema = FALSE;

/* String and cursor handling */
void set_pos()
{
  set_cursor_pos(start_of_command + pos);
}

void update_max_length()
{
  if (length > max_length) {
    max_length = length;
  }
}


/* Prompt */
char user[] = "TimPouzet";
char machine[] = "MarioBrOS";
string path;
void echo_thingy()
{
  printf("%f%s@%s:%f%s%f$ ", Green, user, machine, LightBlue, path, White);
}


/* Commands */
list_t commands = 0;
void register_command(command_t c)
{
  command_t *copy = malloc(sizeof(command_t));
  *copy = c;
  push(&commands, (u_int32)copy);
}

command_t *find_command(string command_name)
{
  bool is_my_cmd(u_int32 x)
  {
    command_t *c = (command_t *)x;
    return str_cmp(c->name, command_name);
  }

  return (command_t*)find(&commands, is_my_cmd, FALSE);
}


void print_command(command_t *c)
{
  printf("%f%s%f\t%s\n", LightRed, c->name, White, c->help);
}


/* The help command */
void help_handler(list_t args)
{
  if (args) {
    /* Print help for asked functions */
    list_t *curr_arg = &args;
    while (!is_empty_list(curr_arg)) {
      string func = (string)pop(curr_arg);
      command_t *cmd = find_command(func);

      if (cmd) {
        print_command(cmd);
      } else {
        printf("%f%s%f\t%s\n", LightRed, func, White, "Unknown command");
      }

      free(func);
    }
  } else {
    /* By default, print help for all functions */
    for (list_t cmds = commands; cmds; cmds = cmds->tail) {
      command_t *c = (command_t *)cmds->head;
      print_command(c);
    }
  }
}
command_t help_cmd = {
  .name = "help",
  .help = "Prints help of available functions, or of its arguments",
  .handler = *help_handler,
};

/* The echo command */
void echo_handler(list_t args)
{
  list_t *curr_arg = &args;
  while (!is_empty_list(curr_arg)) {
    string word = (string)pop(curr_arg);
    printf("%s ", word);
    free(word);
  }
  printf("\b\n");  /* Deletes last space */
}
command_t echo_cmd = {
  .name = "echo",
  .help = "Prints its arguments, separated by spaces",
  .handler = *echo_handler,
};

/* The ls command */
void ls_handler(list_t args)
{
  if (args) {
    list_t *curr_arg = &args;
    while(!is_empty_list(curr_arg)) {
      string s = (void*) pop(curr_arg);
      printf("%s:\n", s);
      if(s[0] == '/') {
        ls(s, 2);
      } else {
        ls(s, curr_dir);
      }
      free(s);
    }
  } else {
    ls(path, 2);
  }
}
command_t ls_cmd = {
  .name = "ls",
  .help = "Prints the contents of the current directory, or its arguments",
  .handler = *ls_handler,
};

/* The cd command */
void cd_handler(list_t args)
{
  if (args) {
    string new_path = (void*) args->head;
    u_int32 i = 1;
    for(; new_path[i] != '\0'; i++);
    if(i!=1 && new_path[i-1] == '/') {
      new_path[i-1] = '\0';
    }

    if (args->tail) {
      while(args->tail) {
        free((void*)args->head);
        pop(&args);
      }
      printf("Too many arguments for %fcd%f\n", LightRed, White);
      return;
    }
    if (new_path[0] == '/') {
      /* Absolute path */
      u_int32 inode = find_dir(new_path + 1, 2);
      if(!inode) {
        printf("No such directory\n");
        return;
      }
      curr_dir = inode;
    } else {
      /* Relative path */
      u_int32 inode = find_dir(new_path, curr_dir);
      if(!inode) {
        printf("No such directory\n");
        return;
      }
      curr_dir = inode;
    }
  } else {
    /* New path defaults to root */
    curr_dir = 2;
  }
  refresh_cwd();
}
command_t cd_cmd = {
  .name = "cd",
  .help = "Go to the given absolute or relative path",
  .handler = *cd_handler,
};

/**
 *  @name get_cwd - Sets the global variable "path" according to "curr_dir"
 */
void refresh_cwd()
{
  string tmp = get_cwd(curr_dir, path);
  if(!tmp) {
    free(path);
    path = malloc(sizeof("/"));
    path[0] = '/'; path[1] = '\0';
  } else {
    path = tmp;
  }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void pwd_handler(list_t args)
{
  refresh_cwd();
  printf("%s\n", path);
}
#pragma GCC diagnostic pop
command_t pwd_cmd = {
  .name = "pwd",
  .help = "Prints the path to the current directory",
  .handler = *pwd_handler,
};


#pragma GCC diagnostic ignored "-Wunused-parameter"
void splash_screen(list_t args)
{
  clear();
  printf("%f%b\n\
   ____    ____                  _         ______             ___     ______    \
  |_   \\  /   _|                (_)       |_   _ \\          .'   `. .' ____ \\   \
    |   \\/   |   ,--.   _ .--.  __   .--.   | |_) | _ .--. /  .-.  \\| (___ \\_|  \
    | |\\  /| |  `'_\\ : [ `/'`\\][  |/ .'`\\ \\ |  __'.[ `/'`\\]| |   | | _.____`.   \
   _| |_\\/_| |_ // | |, | |     | || \\__. |_| |__) || |    \\  `-'  /| \\____) |  \
  |_____||_____|\\'-;__/[___]   [___]'.__.'|_______/[___]    `.___.'  \\______.'  \
\n\n", White, Red);

  print_box(POS(0,0), POS(8,SCREEN_WIDTH-1));

  printf("%f%b", Red, Black);

  for (unsigned char c = 178; c >= 176; c--) {
    for (int i = 0; i < 80; i++) {
      printf("%c", c);
    }
  }

  printf("%f%b\n", White, Black);
}
#pragma GCC diagnostic pop
command_t splash_cmd = {
  .name = "splash",
  .help = "Clears the screen and displays the splash screen (ignores its arguments)",
  .handler = *splash_screen,
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
void display_ascii(list_t args)
{
  printf("0: 1:%c\t", 1);
  for (int c = 2; c < 128; c++) {
    if(c==10) {
      printf("10:\n");
    } else if(c < 10) {
      printf("%d:%c\t\t",c,c);
    } else {
      printf("%d:%c\t", c, c);
    }
  }
  printf("\n");
  for(int c = 128; c < 256; c++) {
    printf("%d:%c ", c, c);
  }
  printf("\n");
}
#pragma GCC diagnostic pop
command_t ascii_cmd = {
  .name = "ascii",
  .help = "Displays the ascii table (ignores its arguments)",
  .handler = *display_ascii,
};


/* The run command */
void run_handler(list_t args)
{
  if (is_empty_list(&args)) {
    printf("%frun:%f\tNo arguments given\n", LightRed, White);
  } else {
    string prog = (string)pop(&args);

    if (is_empty_list(&args)) {
      run_program(prog);
    } else {
      printf("%frun:%f\tToo many arguments\n", LightRed, White);
    }

    free(prog);
    delete_list(&args, TRUE);
  }
}
command_t run_cmd = {
  .name = "run",
  .help = "Runs the given program",
  .handler = *run_handler,
};

/* The mkdir command */
void mkdir_handler(list_t args)
{
  if (is_empty_list(&args)) {
    printf("%fmkdir:%f\tNo arguments given\n", LightRed, White);
  } else {
    while(!(is_empty_list(&args))) {
      string dir = (string) pop(&args);
      if(mkdir(dir, curr_dir)) {
        printf("Impossible to create directory %s\n", dir);
      }
    }
  }
}
command_t mkdir_cmd = {
  .name = "mkdir",
  .help = "Creates a subdirectory of the current directory",
  .handler = *mkdir_handler,
};

/* The rm command */
void rm_handler(list_t args)
{
  if (is_empty_list(&args)) {
    printf("%frm:%f\tNo arguments given\n", LightRed, White);
  } else {
    bool rec = FALSE;
    while(!(is_empty_list(&args))) {
      string file = (string) pop(&args);
      if(file[0] == '-' && file[1] == 'r') {
        rec = TRUE;
        continue;
      }
      u_int8 error = rm(file, curr_dir, rec);

      switch(error) {
      case 0: { continue; }
      case 1: {
        printf("File %s does not exist\n", file);
        continue;
      }
      case 2: {
        printf("An error was encountered while deleting %s directory\n",file);
        continue;
      }
      case 3: {
        printf("%s is not a file but a directory\n", file);
        continue;
      }
      case 4: {
        printf("An error was encountered while deleting %s\n", file);
        continue;
      }
      }
    }
  }
}
command_t rm_cmd = {
  .name = "rm",
  .help = "Deletes a file (option -r to delete a directory)",
  .handler = *rm_handler,
};

#pragma GCC diagnostic ignored "-Wunused-parameter"
void clear_handler(list_t args)
{
  clear();
}
#pragma GCC diagnostic pop
command_t clear_cmd = {
  .name = "clear",
  .help = "Clears the framebuffer",
  .handler = *clear_handler,
};

void shell_install()
{
  path = (void*) malloc(4);
  path[0] = '/'; path[1] = '\0';

  register_command(splash_cmd);
  register_command(run_cmd);
  register_command(ls_cmd);
  register_command(help_cmd);
  register_command(echo_cmd);
  register_command(cd_cmd);
  register_command(pwd_cmd);
  register_command(ascii_cmd);
  register_command(mkdir_cmd);
  register_command(rm_cmd);
  register_command(clear_cmd);

  /* display_ascii(); */
  splash_screen(NULL);
  echo_thingy();

  start_of_command = get_cursor_pos();
  history[0] = malloc(buffer_size);
  str_fill(history[0], '\0', buffer_size);
}


void echo_command()
{
  set_cursor_pos(start_of_command);
  blank_command[max_length] = '\0';
  printf("%s", blank_command);
  blank_command[max_length] = ' ';

  set_cursor_pos(start_of_command);
  int state = 0; /* 0 for first spaces, 1 for first word, 2 for after */
  for (int i = 0; history[history_pos][i] != '\0'; i++) {
    char c = history[history_pos][i];
    if (state == 0 && c != ' ') {
      /* First word! */
      printf("%f", LightRed);
      state = 1;
    } else if (state == 1 && c == ' ') {
      /* After first word */
      printf("%f", White);
      state = 2;
    }
    printf("%c",c);
  }
  printf("%f", White);  /* In case the command has no spaces */
  set_pos();
}

void send_command()
{
  /* Parsing of the command */
  list_t words = str_split(history[history_pos], ' ', FALSE);

  if (words) {
    string command_name = (string)words->head;
    list_t args = words->tail;
    command_t *cmd = find_command(command_name);

    if (cmd) {
      /* Run! */
      cmd->handler(args);
    } else {
      printf("Unknown command: %f%s%f\n", LightRed, command_name, White);
    }
  }
}


void shell_write_char(char c)
{
  switch (c) {

  case '\0':
    return;

  case '\n':
    set_cursor_pos(start_of_command + length);
    printf("\n");

    send_command();
    echo_thingy();

    if (history_length == history_size - 1) {
      /* Full history */
      free(history[0]);
      for (int i = 1; i < history_size; i++) {
        history[i-1] = history[i];
      }
      history_pos = history_length;
    } else {
      /* Save in history */
      str_copy(history[history_pos], history[history_length]);

      history_length++;
      history_pos = history_length;
    }

    history[history_pos] = malloc(buffer_size);
    str_fill(history[history_pos], '\0', buffer_size);

    pos = 0;
    start_of_command = get_cursor_pos();
    length = 0;
    max_length = 0;
    return;

  case '\t':
    /* TODO: autocomplete! */
    return;

  case '\b': /* backspace */
    if (pos > 0) {
      pos--;
      /* go to delete! */
    } else {
      return;
    }

  case '\177': /* delete */
    for (int i = pos; i < length; i++) {
      history[history_pos][i] = history[history_pos][i+1];
    }
    if (pos < length)
      length--;
    break;

  default:  /* write real char */
    if (length < buffer_size - 1) {  /* Account for '\0' at the end */
      /* First, we move the text */
      for (int i = length; i > pos; i--) {
        history[history_pos][i] = history[history_pos][i-1];
      }
      history[history_pos][pos] = c;
      pos++;
      length++;
      update_max_length();

      /* Take care of the scrolling */
      if (start_of_command + length >= SCREEN_HEIGHT * SCREEN_WIDTH) {
        scroll();
        start_of_command -= SCREEN_WIDTH;
      }
    }
  }

  echo_command();
}

void shell_write_string(string s)
{
  for (int i = 0; s[i] != '\0'; i++) {
    shell_write_char(s[i]);
  }
}


void shell_home()
{
  pos = 0;
  set_pos();
}

void shell_end()
{
  pos = length;
  set_pos();
}

void shell_up()
{
  if (history_pos > 0) {
    history_pos--;
    pos = length = str_length(history[history_pos]);
    update_max_length();
    /* Take care of the scrolling */
    if (start_of_command + length >= SCREEN_HEIGHT * SCREEN_WIDTH) {
      scroll();
      start_of_command -= SCREEN_WIDTH;
    }
    echo_command();
  }
}

void shell_down()
{
  if (history_pos < history_length) {
    history_pos++;
    pos = length = str_length(history[history_pos]);
    update_max_length();
    /* Take care of the scrolling */
    if (start_of_command + length >= SCREEN_HEIGHT * SCREEN_WIDTH) {
      scroll();
      start_of_command -= SCREEN_WIDTH;
    }
    echo_command();
  }
}

void shell_left()
{
  if (pos > 0)
    pos--;
  set_pos();
}

void shell_right()
{
  if (pos < buffer_size - 1 && history[history_pos][pos] != '\0')
    pos++;
  set_pos();
}

void keyboard_shell(u_int8 scancode)
{
  /* If the top bit of the byte we read from the keyboard is
   * set, that means that a key has just been released */
  if (scancode & 0x80)
    {
      switch(scancode^0x80) {

      case 29: { // Ctrl
        k_ctrl--;
        break;
      }
      case 42: { // Left Shift
        k_shift_count--;
        k_shift = ((k_lights&0x04)>>2) ^ k_shift_count;
        // k_shift = k_caps_lock ^ k_shift_count
        break;
      }
      case 54: { // Right Shift
        k_shift_count--;
        k_shift = ((k_lights&0x04)>>2) ^ k_shift_count;
        // k_shift = k_caps_lock ^ k_shift_count
        break;
      }
      case 56: { // Alt
        k_meta--;
        break;
      }

      }
    }
  else
    {
      switch (scancode) {

      case 26: { // Dead ^
        if(k_shift) {
          k_dead_hat = FALSE;
          k_dead_trema = TRUE; }
        else if(k_dead_hat) {
          shell_write_char('^');
          k_dead_hat = FALSE; }
        else
          k_dead_hat = TRUE;
        break;
      }
      case 29: { // Ctrl
        k_ctrl++;
        break;
      }
      case 42: { // Left Shift
        k_shift_count++;
        k_shift = ((k_lights&0x04) ^ 0x04)>>2;
        break;
      }
      case 54: { // Right Shift
        k_shift_count++;
        k_shift = ((k_lights&0x04) ^ 0x04)>>2;
        break;
      }
      case 56: { // Alt
        k_meta++;;
        break;
      }
      case 58: { // Caps lock
        k_lights = k_lights ^ 0x04;
        k_shift =!k_shift;
        /* keyboard_set_lights(k_lights); */
        break;
      }
      case 69: { // Num lock
        k_lights = k_lights ^ 0x02;
        /* keyboard_set_lights(k_lights); */
        break;
      }
      case 70: { // Scroll lock
        k_lights = k_lights ^ 0x01;
        /* keyboard_set_lights(k_lights); */
        break;
      }
        /* Next cases are only of use in the framebuffer */
      case 71: { // Home
        shell_home();
        break;
      }
      case 72: { // Up
        shell_up();
        break;
      }
      case 75: { // Left
        shell_left();
        break;
      }
      case 77: { // Right
        shell_right();
        break;
      }
      case 79: { // End
        shell_end();
        break;
      }
      case 80: { // Down
        shell_down();
        break;
      }
      default: {
        char c = kbdus[(scancode+(k_shift*128))];
        if(k_dead_hat) {
          switch(c) {
          case 'a': { shell_write_string("â"); break; }
          case 'e': { shell_write_string("ê"); break; }
          case 'i': { shell_write_string("î"); break; }
          case 'o': { shell_write_string("ô"); break; }
          case 'u': { shell_write_string("û"); break; }
          default: shell_write_char(c);
          }
        } else if(k_dead_trema){
          switch(c) {
          case 'a': { shell_write_string("ä"); break; }
          case 'e': { shell_write_string("ë"); break; }
          case 'i': { shell_write_string("ï"); break; }
          case 'o': { shell_write_string("ö"); break; }
          case 'u': { shell_write_string("ü"); break; }
          case 'y': { shell_write_string("ÿ"); break; }
          case 'A': { shell_write_string("Ä"); break; }
          case 'O': { shell_write_string("Ö"); break; }
          case 'U': { shell_write_string("Ü"); break; }
          default: shell_write_char(c);
          }
        } else
          shell_write_char(c);
      }
        k_dead_hat = FALSE;
        k_dead_trema = FALSE;
      }
    }
}

void main()
{
  shell_install();
  u_int8 scancode;
  for(;;) {
    scancode = keyget();
    if(scancode) {
      keyboard_shell(scancode);
    }
  }
}
