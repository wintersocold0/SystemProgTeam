#include "main.h"

void run_ls() {
    WINDOW *sub_win=recreate_menu_win(NULL);
    int highlight = 1;
    int c;
    // Checkbox (-l, -a, -h)
    int opt_l = 1; // Default selected
    int opt_a = 0;
    int opt_h = 1;


    while(1) {
        werase(sub_win);
        box(sub_win, 0, 0);
        mvwprintw(sub_win, 0, 4, " ls - File List Options ");
        
        mvwprintw(sub_win, 2, 3, "Toggle Options (Press Space to select):");
        
        // Option 1: -l
        if(highlight == 1) wattron(sub_win, A_REVERSE);
        mvwprintw(sub_win, 4, 5, "[%s] -l Detailed View", opt_l ? "X" : " ");
        if(highlight == 1) wattroff(sub_win, A_REVERSE);

        // Option 2: -a
        if(highlight == 2) wattron(sub_win, A_REVERSE);
        mvwprintw(sub_win, 5, 5, "[%s] -a Show hidden files", opt_a ? "X" : " ");
        if(highlight == 2) wattroff(sub_win, A_REVERSE);

        // Option 3: -h
        if(highlight == 3) wattron(sub_win, A_REVERSE);
        mvwprintw(sub_win, 6, 5, "[%s] -h Human-readable file sizes", opt_h ? "X" : " ");
        if(highlight == 3) wattroff(sub_win, A_REVERSE);

        // Execute Button
        if(highlight == 4) wattron(sub_win, A_REVERSE);
        mvwprintw(sub_win, 9, 15, " Enter — Run ");
        if(highlight == 4) wattroff(sub_win, A_REVERSE);

        mvwprintw(sub_win, 11, 3, "↑↓ Move | Space Toggle | Esc Go Back");
        wrefresh(sub_win);

        c = wgetch(sub_win);
        switch(c) {
            case KEY_RESIZE:
                sub_win = recreate_menu_win(sub_win);
                print_menu(sub_win, highlight);
                continue;
            case KEY_UP:
                if(highlight > 1) highlight--;
                break;
            case KEY_DOWN:
                if(highlight < 4) highlight++;
                break;
            case ' ': // Space နှိပ်လျှင် checkbox အမှန်ခြစ်ပြောင်းရန်
                if(highlight == 1) opt_l = !opt_l;
                else if(highlight == 2) opt_a = !opt_a;
                else if(highlight == 3) opt_h = !opt_h;
                break;
            case 10: // Enter နှိပ်လျှင် Command ကို တည်ဆောက်၍ Run မည်
                if(highlight == 4) {
                    char cmd[100] = "ls";
                    if(opt_l || opt_a || opt_h) {
                        strcat(cmd, " -");
                        if(opt_l) strcat(cmd, "l");
                        if(opt_a) strcat(cmd, "a");
                        if(opt_h) strcat(cmd, "h");
                    }
                    strcat(cmd, " .");
                    //stop Ncurses temporarily to run the command in terminal mode
                    endwin();
                    printf("\n[실행할 명령어]: %s\n", cmd);
                    printf("----------------------------------------\n");
                    system(cmd);
                    printf("----------------------------------------\n\n");
                    printf("계속하려면 아무 키나 누르세요...");
                    getchar();
                    // Resume Ncurses
                    
                    refresh();
                }
                break;
            case 27: // if user presses Esc, return to main menu
                delwin(sub_win);
                return;
        }
    }
}