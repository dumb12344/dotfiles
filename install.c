#include <ncurses.h>
#include <menu.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

// ty to https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
char *choices[] = {
    "Install packages",
    "Install yay AUR helper",
    "Install AUR packages",
    "Apply configs",
    "Detect problems",
    "Reboot",
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
    (char *)NULL,
};

bool finishedtasks[] = {
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
};

bool handleSelection(int id, MENU* menu){
    switch(id){
        case 1:
            system("clear");
            printw("Installing packages and updating system");
            //install packages
            system("sudo pacman -Syu figlet jq git base-devel niri zsh xdg-desktop-portal-gnome \
                            xwayland-satellite kitty cliphist cava xdg-desktop-portal brightnessctl \
                            xdg-utils vulkan-radeon vulkan-intel vulkan-headers vulkan-tools ly neovim \
                            ttf-cascadia-code-nerd --needed"
            );
            //enable ly
            system("sudo systemctl enable ly@tty1.service && sudo systemctl disable getty@tty1.service");
            system("read -n 1 -p \"Press any key to continue...\"");
            menu_driver(menu, REQ_TOGGLE_ITEM);
            break;
        case 2:
            if (access("/usr/bin/yay", F_OK) == 0) {
                system("echo yay is already installed.");
            } else {
                system("echo yay is not installed.");
                //I don't need debug packages
                system("echo Disabling debug in makepkg.conf");
                system("sudo sed -ie 's/purge debug/purge !debug/' /etc/makepkg.conf");
                //create temp directory
                system("echo Creating temporary directory");
                char template[] = "/tmp/tmpdir.XXXXXX";
                char *dir_name = mkdtemp(template);
                char tmp[1000] = "echo Created directory ";
                strcat(tmp, dir_name);
                system(tmp);
                //clone into temp directory and save current directory
                char command[1000] = "git clone https://aur.archlinux.org/yay-bin.git ";
                strcat(command, dir_name);
                system(command);
                char dotsdir[1000];
                getcwd(dotsdir, sizeof(dotsdir));
                //make package and go back to previous directory
                chdir(dir_name);
                system("makepkg -si");
                chdir(dotsdir);
                //remove package directory
                char command2[1000] = "rm -rf ";
                strcat(command2, dir_name);
                system(command2);
            }
            system("read -n 1 -p \"Press any key to continue...\"");
            menu_driver(menu, REQ_TOGGLE_ITEM);
            break;
        case 3:
            system("echo Installing AUR packages");
            system("yay -S noctalia-shell zsh-theme-powerlevel10k-git pokeget --needed");
            system("read -n 1 -p \"Press any key to continue...\"");
            menu_driver(menu, REQ_TOGGLE_ITEM);
            break;
        case 4:
            system("echo Applying configs");
            system("echo Changing shell to zsh");
            system("sudo chsh test -s /bin/zsh");
            system("echo Copying user configs");
            system("cp -rf configs/. ~");
            system("read -n 1 -p \"Press any key to continue...\"");
            menu_driver(menu, REQ_TOGGLE_ITEM);
            break;
        case 5:
            system("echo Detecting problems");
            if (access("/usr/sbin/qs", F_OK) == 0){
                system("echo Quickshell is installed properly");
            }
            else{
                system("echo Quickshell wasn\'t installed properly");
            }
            if (access("/etc/xdg/quickshell/noctalia-shell/shell.qml", F_OK) == 0){
                system("echo Noctalia Shell is installed properly");
            }
            else{
                system("echo Noctalia Shell wasn\'t installed properly");
            }
            if (access("/usr/share/zsh-theme-powerlevel10k/powerlevel10k.zsh-theme", F_OK) == 0){
                system("echo Powerlevel10k is installed properly");
            }
            else{
                system("echo Powerlevel10k wasn\'t installed properly");
            }
            system("read -n 1 -p \"Press any key to continue...\"");
            menu_driver(menu, REQ_TOGGLE_ITEM);
            break;
        case 6:
            system("reboot");
            break;
        case 7:
            return true;
            break;
    }
    return false;
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

char* concat(char *s1, char *s2){
    // https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
    char *result = malloc(sizeof(s1) + sizeof(s2));
    snprintf(result, sizeof(s1) + sizeof(s2), "%s%s", s1, s2);
    return result;
}

void determineitems(ITEM** items, int n_choices){
    for(int i = 0; i < n_choices; ++i){ 
        char* prefix=choiceids[i];
        if(i <= n_choices - 2){
            prefix = concat(finishedtasks[i]? "(Finished) " : "           ",prefix);
        }
        items[i] = new_item(prefix, choices[i]);
        //items[i] = new_item(choiceids[i], choices[i]);
    }
}

int main(){
    ITEM **items;
	int c;
	MENU *menu;
    WINDOW *menu_win;
    int n_choices, i;
    bool end = false;
	initscr();
	start_color();
    cbreak();
    noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
    n_choices = ARRAY_SIZE(choices);
    items = (ITEM **) calloc(n_choices, sizeof(ITEM *));
    determineitems(items, n_choices);
    int x,y;
    getmaxyx(stdscr, x, y);
	menu = new_menu((ITEM **) items);
    menu_opts_off(menu, O_ONEVALUE);
    menu_win = newwin(x / 2, y / 3, x / 3, y / 3);
    keypad(menu_win, TRUE);
    set_menu_win(menu, menu_win);
    set_menu_sub(menu, derwin(menu_win, 0, 48, 3, x/5));
    set_menu_format(menu, 12, 1);
    set_menu_mark(menu, " * ");
    box(menu_win, 0, 0);
	print_in_middle(menu_win, 1, 0, y/3, "Dotfiles", COLOR_PAIR(2));
	mvwaddch(menu_win, 2, 0, ACS_LTEE);
	mvwhline(menu_win, 2, 1, ACS_HLINE, y / 3 - 2);
	mvwaddch(menu_win, 2, y / 3 - 1, ACS_RTEE);
	wrefresh(menu_win);
	
	attron(COLOR_PAIR(2));
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (Q to Exit and ENTER to Select)");
	attroff(COLOR_PAIR(2));
    post_menu(menu);
	refresh();

	while(c != 'q' && !end){
        c = wgetch(menu_win);
        switch(c){
            case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
            case 10:
				endwin();
                char *endptr;
                int id;
                id = strtol(item_name(current_item(menu)), &endptr, 10);
                end = handleSelection(id, menu);
                break;
		}
        wrefresh(menu_win);
	}
    unpost_menu(menu);
    for(int i = 0; i < n_choices; ++i){ 
        free_item(items[i]);
    }
    free_menu(menu);
	endwin();
}
