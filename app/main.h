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
#include <errno.h>
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
void run_mkdir();
void run_rmdir();
void run_mv();
void run_rm();
void run_echo();
void run_grep();
void run_find();

#define CD_MAX_ITEMS 256

typedef struct {
    char name[256];
    int is_dir;
} CdFileItem;

int load_current_dir_items(CdFileItem *items, int max_items, int *max_name_len);
void draw_dir_items(WINDOW *win, int win_width, int start_y, int start_x,
                    const CdFileItem *items, int n_items, int max_name_len);
int read_line_with_tab(WINDOW *win, int y, int x, int width,
                       char *buf, size_t buf_size,
                       const CdFileItem *items, int n_items);

void calc_menu_rect(int *out_h, int *out_w, int *out_y, int *out_x);
WINDOW *recreate_menu_win(WINDOW *old_win);


void build_path(const char *cwd, const char *input, char *out, size_t out_size);

void show_status(WINDOW *win, int win_w, int status_y, const char *msg);
