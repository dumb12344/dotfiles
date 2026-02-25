#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>

int main(){
    initscr();
    noecho();
    cbreak();
    int x,y;
    getmaxyx(stdscr,x,y);
    mvwprintw(stdscr,x/8,y/2-23,"Installing script packages and updating system");
    WINDOW *win = newwin(x/2, y/2, 10, x);
    box(win,0,0);
    PANEL *test = new_panel(win);
    update_panels();
    doupdate();
    refresh();
    getch();
    endwin();
    return 0;
}