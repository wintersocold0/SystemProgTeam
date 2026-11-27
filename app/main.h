#pragma once

#define _XOPEN_SOURCE_EXTENDED
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ncursesw/ncurses.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#define MAX_ITEMS 256
#define PATH_MAX_LEN 1024

#ifndef MENU_H
#define MENU_H

extern char *choices[];
extern int n_choices;
#endif

void print_menu(WINDOW *,int);
void run_ls();
void run_cd();
void run_cp();
void run_echo();
