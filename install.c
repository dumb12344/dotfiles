#include <ncurses.h>
#include <menu.h>
#include <string.h>
#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4
// ty to https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
char *choices[] = {
    "Choice 1",
    "Choice 2",
    "Choice 3",
    "Choice 4",
    "Choice 5",
    "Choice 6",
    "Choice 7",
    "Choice 8",
    "Choice 9",
    "Choice 10",
    "Exit",
    (char *)NULL,
};
char *choiceids[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    (char *)NULL,
};
bool finishedtasks[] = {
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
};
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);


char* concat(char *s1, char *s2){
    /*char *result = malloc(strlen(s1) + strlen(s2) + 1);
    // https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
    strcpy(result, s1);
    strcat(result, s2);
    return result;*/
    char *result;
    snprintf(result, sizeof(result), "%s%s", s1, s2);
    return result;
}

int main(){
    ITEM **my_items;
	int c;
	MENU *my_menu;
    WINDOW *my_menu_win;
    int n_choices, i;
	initscr();
	start_color();
    cbreak();
    noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
    n_choices = ARRAY_SIZE(choices);
    my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
    for(i = 0; i < n_choices; ++i){ 
        //char* prefix = concat(finishedtasks[i] ? "ee " : "ea ", choiceids[i]);
        //my_items[i] = new_item(prefix, choices[i]);
        my_items[i] = new_item(choiceids[i], choices[i]);
    }
    int x,y;
    getmaxyx(stdscr, x, y);
	my_menu = new_menu((ITEM **)my_items);
    my_menu_win = newwin(x/2, y/3, x/3, y/3);
    keypad(my_menu_win, TRUE);
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 0, 48, 3, x/3));
    set_menu_format(my_menu, 12, 1);
    set_menu_mark(my_menu, " * ");
    box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, y/3, "My Menu", COLOR_PAIR(2));
	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, y/3-2);
	mvwaddch(my_menu_win, 2, y/3-1, ACS_RTEE);
	post_menu(my_menu);
	wrefresh(my_menu_win);
	
	attron(COLOR_PAIR(2));
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (Q to Exit)");
	attroff(COLOR_PAIR(2));
	refresh();

	while(c != 'q'){
        c = wgetch(my_menu_win);
        switch(c){
            case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
				break;
            case KEY_ENTER:
                system("echo asd");
                break;
		}
        wrefresh(my_menu_win);
	}	
    unpost_menu(my_menu);
    free_menu(my_menu);
    for(i = 0; i < n_choices; ++i){ 
        free_item(my_items[i]);
    }
	endwin();
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color){
    int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)>>1;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}