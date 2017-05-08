#include "shell.h"
#include "printer.h"
#include "malloc.h"
#include "logging.h"
#include "filesystem.h"


#define buffer_size 128
char blank_command[buffer_size] = {' '};
int  pos = 0, length = 0, max_length = 0;
pos_t start_of_command;

#define history_size 100
string history[history_size] = {0};
int history_length = 0, history_pos = 0;
bool modified_since_history = FALSE;

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
char user[] = "MarcPouzet";
char machine[] = "MarioBrOS";
string path;
void echo_thingy()
{
  writef("%f%s@%s:%f%s%f$ ", Green, user, machine, LightBlue, path, White);
}


/* Commands */
list_t commands = 0;
void register_command(command_t c)
{
  command_t *copy = mem_alloc(sizeof(command_t));
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
  writef("%f%s%f\t%s\n", LightRed, c->name, White, c->help);
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
        writef("%f%s%f\t%s\n", LightRed, func, White, "Unknown command");
      }

      mem_free(func);
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
    writef("%s ", word);
    mem_free(word);
  }
  writef("\b\n");  /* Deletes last space */
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
      string relative_path = (string)pop(curr_arg);
      string absolute_path = str_cat(path, relative_path);

      writef("%s:\n", relative_path);
      ls_dir(open_file(absolute_path));
      writef("\n");

      mem_free(absolute_path);
      mem_free(relative_path);
    }
  } else {
    ls_dir(open_file(path));
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
    string new_path = (string)args->head;

    if (args->tail) {
      writef("Too many arguments for %fcd%f\n", LightRed, White);
    } else {
      if (new_path[0] == '/') {
        /* Absolute path */
        mem_free(path);
        path = new_path;
      } else {
        /* Relative path */
        string new_new_path = str_cat(path, new_path);  /* TODO: /? */
        mem_free(path);
        mem_free(new_path);
        path = new_new_path;
      }
    }
  } else {
    /* New path defaults to root */
    mem_free(path);
    string new_path = (string)mem_alloc(sizeof("/"));
    new_path[0] = '/'; new_path[1] = '\0';
    path = new_path;
  }
}
command_t cd_cmd = {
  .name = "cd",
  .help = "Go to the given absolute or relative path",
  .handler = *cd_handler,
};


#pragma GCC diagnostic ignored "-Wunused-parameter"
void splash_screen(list_t args)
{
  clear();
  writef("%f%b\n\
   ____    ____                  _         ______             ___     ______    \
  |_   \\  /   _|                (_)       |_   _ \\          .'   `. .' ____ \\   \
    |   \\/   |   ,--.   _ .--.  __   .--.   | |_) | _ .--. /  .-.  \\| (___ \\_|  \
    | |\\  /| |  `'_\\ : [ `/'`\\][  |/ .'`\\ \\ |  __'.[ `/'`\\]| |   | | _.____`.   \
   _| |_\\/_| |_ // | |, | |     | || \\__. |_| |__) || |    \\  `-'  /| \\____) |  \
  |_____||_____|\\'-;__/[___]   [___]'.__.'|_______/[___]    `.___.'  \\______.'  \
\n\n", White, Red);

  write_box(POS(0,0), POS(8,SCREEN_WIDTH-1));

  writef("%f%b", Red, Black);

  for (unsigned char c = 178; c >= 176; c--) {
    for (int i = 0; i < 80; i++) {
      writef("%c", c);
    }
  }

  writef("%f%b\n", White, Black);
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
  for (int c = 0; c < 256; c++) {
    writef("%d:%c ", c, c);
  }
  writef("\n");
}
#pragma GCC diagnostic pop
command_t ascii_cmd = {
  .name = "ascii",
  .help = "Displays the ascii table (ignores its arguments)",
  .handler = *display_ascii,
};


void shell_install()
{
  path = (string)mem_alloc(sizeof("/"));
  path[0] = '/'; path[1] = '\0';

  register_command(splash_cmd);
  register_command(ls_cmd);
  register_command(help_cmd);
  register_command(echo_cmd);
  register_command(cd_cmd);
  register_command(ascii_cmd);

  /* display_ascii(); */
  splash_screen(NULL);
  echo_thingy();

  start_of_command = get_cursor_pos();
  history[0] = mem_alloc(buffer_size);
  str_fill(history[0], '\0', buffer_size);
}


void echo_command()
{
  set_cursor_pos(start_of_command);
  blank_command[max_length] = '\0';
  write_string(blank_command);
  blank_command[max_length] = ' ';

  set_cursor_pos(start_of_command);
  write_string(history[history_pos]);
  set_pos();
}

void send_command()
{
  /* Parsing of the command */
  list_t words = str_split(history[history_pos], ' ', FALSE);
  kloug(100 + buffer_size, "Executing command %s %x\n", history[history_pos], words);

  if (words) {
    string command_name = (string)words->head;
    list_t args = words->tail;
    command_t *cmd = find_command(command_name);

    if (cmd) {
      /* Run! */
      cmd->handler(args);
    } else {
      writef("Unknown command: %f%s%f\n", LightRed, command_name, White);
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
    write_char('\n');

    send_command();
    echo_thingy();

    if (history_length == history_size - 1) {
      /* Full history */
      kloug(100, "Full history %d %d\n", history_length, history_pos);
      mem_free(history[0]);
      for (int i = 1; i < history_size; i++) {
        history[i-1] = history[i];
      }
      history_pos = history_length;
    } else {
      /* Save in history */
      kloug(100, "Non-full history %d %d\n", history_length, history_pos);
      str_copy(history[history_pos], history[history_length]);

      history_length++;
      history_pos = history_length;
    }

    history[history_pos] = mem_alloc(buffer_size);
    str_fill(history[history_pos], '\0', buffer_size);

    pos = 0;
    start_of_command = get_cursor_pos();
    length = 0;
    max_length = 0;
    kloug(100, "Command sent\n");
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


