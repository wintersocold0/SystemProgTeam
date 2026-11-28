#include "main.h"

char *choices[] = {
    "ls   - List files and folders in the current directory",
    "cd   - Move you into anothe folder",
    "pwd  - Display the full path of the current folder location",
    "cp   - Duplicate files from one location to another",
    "mkdir - Create a new directory",
    "rmdir - Remove an empty directory",
    "mv   - Move or rename files and folders",
    "rm   - Delete files or folders",
    "echo - Display text in the terminal",
    "grep - Search text for specific keywords",
    "find - Search files and directories",
    "cat  - Display the contents of a file",
    "head - Display the first N lines of a file",
    "tail - Display the last N lines of a file",
    "exit - Exit the program"
};
int n_choices = sizeof(choices) / sizeof(char *);

void calc_menu_rect(int *out_h, int *out_w, int *out_y, int *out_x) {
    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);

    int h = term_h - 4;
    if (h > 30) h = 30;
    if (h < 14) h = term_h;

    int w = term_w - 6;
    if (w > 100) w = 100;
    if (w < 45) w = term_w;

    int y = (term_h - h) / 2;
    int x = (term_w - w) / 2;
    if (y < 0) y = 0;
    if (x < 0) x = 0;

    *out_h = h;
    *out_w = w;
    *out_y = y;
    *out_x = x;
}

 WINDOW *recreate_menu_win(WINDOW *old_win) {
    if (old_win != NULL) {
        delwin(old_win);
    }

    int win_h, win_w, start_y, start_x;
    calc_menu_rect(&win_h, &win_w, &start_y, &start_x);

    clear();
    refresh();

    WINDOW *win = newwin(win_h, win_w, start_y, start_x);
    keypad(win, TRUE);
    return win;
}
void build_path(const char *cwd, const char *input, char *out, size_t out_size) {
    if (input[0] == '/') {
        snprintf(out, out_size, "%s", input);
    } else {
        snprintf(out, out_size, "%s/%s", cwd, input);
    }
}

void show_status(WINDOW *win, int win_w, int status_y, const char *msg) {
    int max_msg = win_w - 6;
    if (max_msg < 10) max_msg = 10;
    mvwprintw(win, status_y, 3, "%-*.*s", max_msg, max_msg, msg);
    wrefresh(win);
}

int main() {
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL && pw->pw_dir != NULL) {
        if (chdir(pw->pw_dir) != 0) {
            perror("Cannot set to home directory.");
        }
    } else {
        chdir("/");
    }

    setlocale(LC_ALL, "");

    initscr();
    
    clear();
    noecho();
    cbreak();
    refresh();

    int highlight = 1;
    int choice = 0;
    int c;

    WINDOW *my_menu_win = recreate_menu_win(NULL);
    print_menu(my_menu_win, highlight);

    while (1) {
        c = wgetch(my_menu_win);


        switch (c) {
            case KEY_RESIZE:
                my_menu_win = recreate_menu_win(my_menu_win);
                print_menu(my_menu_win, highlight);
                continue;
            case KEY_UP:
                if (highlight == 1) highlight = n_choices;
                else --highlight;
                break;

            case KEY_DOWN:
                if (highlight == n_choices) highlight = 1;
                else ++highlight;
                break;

            case 10:
                if (highlight == 1) { // ls
                    run_ls();
                    my_menu_win = recreate_menu_win(my_menu_win);
                } else if (highlight == 2) { // cd
                    run_cd();
                    my_menu_win = recreate_menu_win(my_menu_win);
                } else if (highlight == 3) { // pwd
                    endwin();
                    printf("\n[실행할 명령어]: pwd\n");
                    printf("----------------------------------------\n");
                    system("pwd");
                    printf("----------------------------------------\n\n");
                    printf("계속하려면 아무 키나 누르세요...");
                    getchar();
                    // Resume Ncurses
                    
                    refresh();
                }else if (highlight == 4) { // cp
                    run_cp();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 5) { // mkdir
                    run_mkdir();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 6) { // rmdir
                    run_rmdir();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 7) { // mv
                    run_mv();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 8) { // rm
                    run_rm();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 9) { // echo
                    run_echo();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 10) { // grep
                    run_grep();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 11) { // find
                    run_find();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 12) { // cat
                    run_cat();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 13) { // head
                    run_head();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == 14) { // tail
                    run_tail();
                    my_menu_win = recreate_menu_win(my_menu_win);
                }else if (highlight == n_choices) { // exit
                    choice = highlight;
                } else {
                    // pwd or grep (not implemented yet)
                    WINDOW *msg_win = newwin(7, 64, 8, 10);
                    box(msg_win, 0, 0);
                    mvwprintw(msg_win, 2, 3, "[%s]", choices[highlight - 1]);
                    mvwprintw(msg_win, 3, 3, "This command is not implemented yet.");
                    mvwprintw(msg_win, 5, 3, "Press any key to continue...");
                    wrefresh(msg_win);
                    wgetch(msg_win);
                    delwin(msg_win);
                    my_menu_win = recreate_menu_win(my_menu_win);
                }
                break;

            case 27: // ESC
                choice = n_choices;
                break;
        }

        if (choice == n_choices) {
            break;
        }

        print_menu(my_menu_win, highlight);
    }

    if (my_menu_win != NULL) {
        delwin(my_menu_win);
    }
    endwin();
    printf("프로그램을 종료합니다.\n");
    return 0;
} 
