#include "main.h"
#include <fcntl.h>
#include <time.h>
#include <utmp.h>

static void draw_who_frame(WINDOW *win, int width);
static ssize_t read_who_record(int fd, struct utmp *record);
static void show_who_record(WINDOW *win, int row, int width,
                            const struct utmp *record);

void run_who(void) {
    WINDOW *who_win = recreate_menu_win(NULL);
    if (who_win == NULL) {
        return;
    }

    int height, width;
    getmaxyx(who_win, height, width);
    if (height < 6 || width < 10) {
        delwin(who_win);
        return;
    }

    const int first_row = 2;
    const int status_y = height - 3;
    const int footer_y = height - 2;
    int row = first_row;
    char status[160] = "";
    draw_who_frame(who_win, width);

    int fd = open(UTMP_FILE, O_RDONLY);
    if (fd == -1) {
        snprintf(status, sizeof(status), "Cannot open utmp: %s", strerror(errno));
    } else {
        struct utmp record;
        ssize_t bytes;

        while ((bytes = read_who_record(fd, &record)) == (ssize_t)sizeof(record)) {
            if (record.ut_type != USER_PROCESS || record.ut_user[0] == '\0') {
                continue;
            }

            if (row == status_y) {
                mvwaddnstr(who_win, footer_y, 3, "Any key: next page | Esc: back",
                           width - 6);
                wrefresh(who_win);
                int key = wgetch(who_win);
                if (key == 27 || key == KEY_RESIZE || key == ERR) {
                    close(fd);
                    delwin(who_win);
                    return;
                }
                draw_who_frame(who_win, width);
                row = first_row;
            }

            show_who_record(who_win, row, width, &record);
            row++;
        }

        if (bytes == -1) {
            snprintf(status, sizeof(status), "Cannot read utmp: %s", strerror(errno));
        } else if (bytes > 0) {
            snprintf(status, sizeof(status), "Incomplete utmp record.");
        } else if (row == first_row) {
            snprintf(status, sizeof(status), "No login sessions in utmp.");
        }
        close(fd);
    }

    mvwaddnstr(who_win, status_y, 3, status, width - 6);
    mvwaddnstr(who_win, footer_y, 3, "Press any key to return.", width - 6);
    wrefresh(who_win);
    wgetch(who_win);
    delwin(who_win);
}

static void draw_who_frame(WINDOW *win, int width) {
    werase(win);
    box(win, 0, 0);
    mvwaddnstr(win, 0, 4, " who - Logged-in Users ", width - 6);
}

static ssize_t read_who_record(int fd, struct utmp *record) {
    char *buffer = (char *)record;
    size_t offset = 0;

    while (offset < sizeof(*record)) {
        ssize_t bytes = read(fd, buffer + offset, sizeof(*record) - offset);
        if (bytes == -1 && errno == EINTR) {
            continue;
        }
        if (bytes <= 0) {
            return bytes == -1 ? -1 : (ssize_t)offset;
        }
        offset += (size_t)bytes;
    }
    return (ssize_t)offset;
}

static void show_who_record(WINDOW *win, int row, int width,
                            const struct utmp *record) {
    time_t seconds = (time_t)record->ut_tv.tv_sec;
    struct tm *local = localtime(&seconds);
    char login_time[32];

    if (local == NULL ||
        strftime(login_time, sizeof(login_time), "%Y-%m-%d %H:%M", local) == 0) {
        strcpy(login_time, "?");
    }

    // A full-width utmp field may not end in a null byte.
    int has_host = record->ut_host[0] != '\0';
    char line[sizeof(record->ut_user) + sizeof(record->ut_line) +
              sizeof(record->ut_host) + sizeof(login_time) + 32];
    snprintf(line, sizeof(line), "%-12.*s %-12.*s %s%s%.*s%s",
             (int)sizeof(record->ut_user), record->ut_user,
             (int)sizeof(record->ut_line), record->ut_line,
             login_time, has_host ? " (" : "",
             (int)sizeof(record->ut_host), record->ut_host,
             has_host ? ")" : "");
    mvwaddnstr(win, row, 3, line, width - 6);
}