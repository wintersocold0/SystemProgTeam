#include "main.h"


void print_menu(WINDOW *win, int highlight) {
    int x = 3, y = 2;
    box(win, 0, 0);
    mvwprintw(win, 0, 4, " Command Helper ");
    
    char cwd[256];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        mvwprintw(win, y,x,"current directory: %s",cwd);
    }
    y+=3;
    for(int i = 0; i < n_choices; ++i) {
        if(highlight == i + 1) {
            wattron(win, A_REVERSE);
            mvwprintw(win, y, x, "> %s", choices[i]);
            wattroff(win, A_REVERSE);
        } else {
            mvwprintw(win, y, x, "  %s", choices[i]);
        }
        y++;
    }
    y+=2;
    mvwprintw(win, y, 2, "↑↓ Move | Enter Run | Esc Exit");
    wrefresh(win);
}
