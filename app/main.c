#include "main.h"

char *choices[] = {
    "ls   - List files and folders in the current directory",
    "cd   - Move you into anothe folder",
    "pwd  - Display the full path of the current folder location",
    "cp   - Duplicate files from one location to another",
    "grep - Search text or command outputs for specific keywords",
    "echo - Print text to standard output",
    "exit - Exit the program"
};
int n_choices = sizeof(choices) / sizeof(char *);

int main() {
    struct passwd *pw = getpwuid(getuid());
    if(pw != NULL && pw->pw_dir != NULL)    {
        if(chdir(pw->pw_dir) != 0){
            perror("Cannot set to home directory.");
        }
    }else{
            chdir("/");
    }
    
    setlocale(LC_ALL, "");
    WINDOW *my_menu_win;
    int highlight = 1;
    int choice = 0;
    int c;

    initscr();
    clear();
    noecho();
    cbreak();
    
    my_menu_win = newwin(30, 100, 3, 25);
    keypad(my_menu_win, TRUE);
    
    print_menu(my_menu_win, highlight);
    
    while(1) {
        c = wgetch(my_menu_win);
        switch(c) {
            case KEY_UP:
                if(highlight == 1) highlight = n_choices;
                else --highlight;
                break;
            case KEY_DOWN:
                if(highlight == n_choices) highlight = 1;
                else ++highlight;
                break;
            case 10: // Enter နှိပ်သောအခါ
                if(highlight == 1) { // 'ls' ကို ရွေးလိုက်လျှင်
                    endwin();
                    run_ls(); // Sub-menu သို့ သွားမည်
                    initscr();
                    print_menu(my_menu_win, highlight);
                }else if(highlight == 2) {
                    endwin();
                    run_cd();
                    initscr();
                    print_menu(my_menu_win,highlight);
                }else if(highlight == 4) {
                    endwin();
                    run_cp();
                    initscr();
                    print_menu(my_menu_win,highlight);
                }else if(highlight ==6){
                    endwin();
                    run_echo();
                    initscr();
                    print_menu(my_menu_win,highlight);
                }else if(highlight == n_choices) { // 'exit' ကို ရွေးလျှင်
                    choice = highlight;
                } else {
                    endwin();
                    printf("\n[%s] 명령창은 아직 구현되지 않았습니다.\n\n", choices[highlight-1]);
                    printf("계속하려면 Enter를 누르세요...");
                    getchar();
                    initscr();
                    print_menu(my_menu_win, highlight);
                }
                break;
            case 27:
                choice = n_choices;
                break;
        }
        print_menu(my_menu_win, highlight);
        if(choice == n_choices) break;
    }
    
    endwin();
    printf("프로그램을 종료합니다.\n");
    return 0;
} 