#include <ncurses.h>
#include <menu.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
// ty to https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
char * choices[] = {
    "Install packages",
    "Install yay AUR helper",
    "Install AUR packages",
    "Apply configs",
    "Detect problems",
    "Reboot",
    "Exit",
    (char *)NULL,
};

char * choiceids[] = {
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

char * concat2(char * s1, char * s2){
    // https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
    char * result = malloc(strlen(s1) + strlen(s2) + 1);
    //strcpy(result, s1);
    //strcat(result, s2);
    snprintf(result, strlen(s1) + strlen(s2) + 1, "%s%s", s1, s2);
    return result;
}

char * concat3(char * s1, char * s2, char * s3){
    char * result = malloc(strlen(s1) + strlen(s2) + strlen(s3) + 1);
    //strcpy(result, s1);
    //strcat(result, s2);
    //strcat(result, s3);
    snprintf(result, strlen(s1) + strlen(s2) + strlen(s3) + 1, "%s%s%s", s1, s2, s3);
    return result;
}

void info(char * inText){
    puts(concat3(ANSI_COLOR_CYAN, inText, ANSI_COLOR_RESET));
}

void good(char * inText){
    puts(concat3(ANSI_COLOR_GREEN, inText, ANSI_COLOR_RESET));
}

void bad(char * inText){
    puts(concat3(ANSI_COLOR_RED, inText, ANSI_COLOR_RESET));
}

bool handleSelection(int id, MENU * menu){
    system("clear");
    switch(id){
        case 1:
            info("Installing packages and updating system");
            //install packages
            system("sudo pacman -Syu figlet jq git base-devel niri zsh xdg-desktop-portal-gnome \
                            xwayland-satellite kitty cliphist cava xdg-desktop-portal brightnessctl \
                            xdg-utils vulkan-radeon vulkan-intel vulkan-headers vulkan-tools ly neovim \
                            ttf-cascadia-code-nerd qt6ct qt5ct nwg-look adw-gtk-theme xorg-xrandr --needed"
            );
            //enable ly
            system("sudo systemctl enable ly@tty1.service && sudo systemctl disable getty@tty1.service");
            break;
        case 2:
            if (access("/usr/bin/yay", F_OK) == 0) {
                good("yay is already installed.");
            } else {
                bad("yay is not installed.");
                //I don't need debug packages
                info("Disabling debug in makepkg.conf");
                system("sudo sed -ie 's/purge debug/purge !debug/' /etc/makepkg.conf");
                //create temp directory
                info("Creating temporary directory");
                char template[] = "/tmp/yay.XXXXXX";
                char * dir_name = mkdtemp(template);
                info(concat2("Created directory ", dir_name));
                //clone into temp directory and save current directory
                system(concat2("git clone https://aur.archlinux.org/yay-bin.git ", dir_name));
                char dotsdir[1000];
                getcwd(dotsdir, sizeof(dotsdir));
                //make package and go back to previous directory
                chdir(dir_name);
                system("makepkg -si");
                chdir(dotsdir);
                //remove package directory
                system(concat2("rm -rf ", dir_name));
            }
            break;
        case 3:
            info("Installing AUR packages");
            system("yay -S noctalia-shell zsh-theme-powerlevel10k-git pokeget --needed");
            break;
        case 4:
            info("Applying configs");
            info("Changing shell to zsh");
            system("sudo chsh test -s /bin/zsh");
            info("Copying user configs");
            system("cp -rf configs/. ~");
            info("Do you want to use dark mode (y) or light mode wallpapers (n) (Y/n)");
            char test[2];
            fgets(test,2,stdin);;
            if(strcmp(test,"n") == 0){
                system("cp -rf wallpapers/lightmodewallpapers/* ~/Pictures/Wallpapers");
            }
            else{
                system("cp -rf wallpapers/darkmodewallpapers/* ~/Pictures/Wallpapers");
            }
            /*
            fputs(ANSI_COLOR_CYAN "Do you want to apply dark mode (y) or light mode wallpapers (n) (Y/n) " ANSI_COLOR_RESET, stdout);
            char test[2];
            fgets(test,2,stdin);
            if(strcmp(test,"n") == 0){
                system("ln -sf ~/Pictures/lightmodewallpapers ~/Pictures/Wallpapers");
            }
            else{
                system("ln -sf ~/Pictures/darkmodewallpapers ~/Pictures/Wallpapers");
            }*/
            info("Applying noctalia configs");
            system(concat3("sed -ie 's/username/",getenv("USER"),"/' ~/.config/noctalia/settings.json"));
            info("Applying display scaling");
            int e = system("xrandr");
            if(e != 0){
                info("Remember to re-run config application in desktop to apply display scaling.");
            }
            else{
                //definately not vibecoded
                system("DISP=$(xrandr | sed -n '2p' | awk '{print $1}'); sed -i \"s/eDP-1/$DISP/\" ~/.config/niri/config.kdl");
            }
            break;
        case 5:
            info("Detecting problems");
            if (access("/usr/sbin/qs", F_OK) == 0){
                good("Quickshell is installed properly");
            }
            else{
                bad("Quickshell wasn\'t installed properly");
            }
            if (access("/etc/xdg/quickshell/noctalia-shell/shell.qml", F_OK) == 0){
                good("Noctalia Shell is installed properly");
            }
            else{
                bad("Noctalia Shell wasn\'t installed properly");
            }
            if (access("/usr/share/zsh-theme-powerlevel10k/powerlevel10k.zsh-theme", F_OK) == 0){
                good("Powerlevel10k is installed properly");
            }
            else{
                bad("Powerlevel10k wasn\'t installed properly");
            }
            break;
        case 6:
            system("reboot");
            return true;
            break;
        case 7:
            return true;
            break;
    }
    system(concat2("read -n 1 -p \"",ANSI_COLOR_CYAN "Press any key to continue...\"" ANSI_COLOR_RESET));
    menu_driver(menu, REQ_TOGGLE_ITEM);
    return false;
}

void print_in_middle(WINDOW * win, int starty, int startx, int width, char * string, chtype color){
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


void determineitems(ITEM ** items, int n_choices){
    for(int i = 0; i < n_choices; ++i){ 
        char * prefix=choiceids[i];
        if(i <= n_choices - 2){
            prefix = concat2(finishedtasks[i] ? "(Finished) " : "           ", prefix);
        }
        items[i] = new_item(prefix, choices[i]);
        //items[i] = new_item(choiceids[i], choices[i]);
    }
}

int main(){
    ITEM ** items;
	int c;
	MENU * menu;
    WINDOW * menu_win;
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
    set_menu_sub(menu, derwin(menu_win, 7, 40, 3, x/5));
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
                char * endptr;
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
