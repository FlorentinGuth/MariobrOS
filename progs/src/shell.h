#ifndef SHELL_H
#define SHELL_H

#include "string.h"
#include "list.h"


typedef struct command {
  string name;
  string help;
  void (*handler)(list_t args);
} command_t;

void refresh_cwd();

void shell_install();

void shell_write_char(char c);
void shell_write_string(string s);

void shell_home();
void shell_end();
void shell_up();
void shell_down();
void shell_left();
void shell_right();

void register_command(command_t c);

void keyboard_shell(u_int8 scancode);

#endif
