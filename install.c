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

// idk where to put this
FILE * fptr;

char * browser = "zen-browser";
char * darkmode = "true";

// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
// https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
char * choices[] = {
    "Configure",
    "Install packages",
    "Install yay AUR helper",
    "Install AUR packages",
    "Apply configs",
    "Detect problems",
    "Reboot",
    "Exit",
    (char *) NULL,
};

char * choiceids[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    (char *) NULL,
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
    false,
};

char * concat2(char * s1, char * s2) {
    // https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
    char * result = malloc(strlen(s1) + strlen(s2) + 1);
    snprintf(result, strlen(s1) + strlen(s2) + 1, "%s%s", s1, s2);
    return result;
}

char * concat3(char * s1, char * s2, char * s3) {
    char * result = malloc(strlen(s1) + strlen(s2) + strlen(s3) + 1);
    snprintf(result, strlen(s1) + strlen(s2) + strlen(s3) + 1, "%s%s%s", s1, s2, s3);
    return result;
}

void info(char * inText) {
    puts(concat3(ANSI_COLOR_CYAN, inText, ANSI_COLOR_RESET));
}

void good(char * inText) {
    puts(concat3(ANSI_COLOR_GREEN, inText, ANSI_COLOR_RESET));
}

void bad(char * inText) {
    puts(concat3(ANSI_COLOR_RED, inText, ANSI_COLOR_RESET));
}

int execute(char * command) {
    // I can change this to a puts statemenet for debugging
    return system(command);
}

void configure() {
    browser = malloc(100);
    if (access("browser", F_OK) == 0) {
        fptr = fopen("browser", "r");
        fscanf(fptr, "%s", browser);
        fclose(fptr);
        good("Already configured browser");
    }
    else {
        info("Do you want to use Zen browser (y) or librewolf (n) (Y/n)");
        // read choice and set browser shortcut for niri
        char choice[2];
        fgets(choice, 2, stdin);
        browser = strcmp(choice, "n") == 0 ? "librewolf" : "zen-browser";
        // write browser to ./browser for config
        fptr = fopen("browser", "w");
        fprintf(fptr, "%s", browser);
        fclose(fptr);
        getchar();
    }
    char * darkmode = malloc(10);
    if (access("darkmode", F_OK) == 0) {
        fptr = fopen("darkmode", "r");
        fscanf(fptr, "%s", darkmode);
        fclose(fptr);
        good("Already configured dark mode");
    }
    else {
        info("Do you want to use Dark mode? (Y/n)");
        char choice[2];
        fgets(choice, 2, stdin);
        darkmode = strcmp(choice, "n") == 0 ? "false" : "true";
        fptr = fopen("darkmode", "w");
        fprintf(fptr, "%s", darkmode);
        fclose(fptr);
        getchar();
    }
}

void installPackages() {
    info("Installing packages and updating system");
    // install packages
    execute("sudo pacman -Syu figlet jq git base-devel niri zsh zsh-syntax-highlighting \
            xdg-desktop-portal-gnome xwayland-satellite kitty cliphist cava xdg-desktop-portal \
            brightnessctl xdg-utils vulkan-radeon vulkan-intel vulkan-headers vulkan-tools ly neovim \
            ttf-cascadia-code-nerd qt6ct qt5ct nwg-look adw-gtk-theme xorg-xrandr --needed"
    );
    // enable ly
    execute("sudo systemctl enable ly@tty1.service && sudo systemctl disable getty@tty1.service");
}

void installYay() {
    if (access("/usr/bin/yay", F_OK) == 0) {
        good("yay is already installed.");
    } else {
        bad("yay is not installed.");
        // I don't need debug packages
        info("Disabling debug packages in makepkg.conf");
        execute("sudo sed -ie 's/purge debug/purge !debug/' /etc/makepkg.conf");
        // create temp directory
        info("Creating temporary directory");
        char template[] = "/tmp/yay.XXXXXX";
        char * dir_name = mkdtemp(template);
        info(concat2("Created directory ", dir_name));
        // clone into temp directory and save current directory
        execute(concat2("git clone https://aur.archlinux.org/yay-bin.git ", dir_name));
        char dotsdir[1000];
        getcwd(dotsdir, sizeof(dotsdir));
        // make package and go back to previous directory
        chdir(dir_name);
        execute("makepkg -si");
        chdir(dotsdir);
        // remove package directory
        execute(concat2("rm -rf ", dir_name));
    }
}

void installAurPackages() {
    info("Installing AUR packages");
    // install binary for browser choice
    execute(concat3("yay -S --needed noctalia-shell zsh-theme-powerlevel10k-git pokeget ", browser, "-bin"));
}

void applyConfigs() {
    info("Applying configs");
    info("Copying user configs");
    execute("cp -rf home/. ~");
    execute("sed -ie \"s/browserchoice/$(cat browser)/g\" ~/.config/niri/config.kdl");
    // enable color for pacman/yay
    execute("sudo sed -ie 's/#Color/Color/' /etc/pacman.conf");
    execute("mkdir -p ~/Pictures/Wallpapers");
    if (strcmp(darkmode, "true") == 0) {
        execute("cp -rf wallpapers/darkmodewallpapers/* ~/Pictures/Wallpapers");
    }
    else {
        execute("sed -ie 's/\"darkMode\": true,/\"darkMode\": false,/' ~/.config/noctalia/settings.json");
        execute("cp -rf wallpapers/lightmodewallpapers/* ~/Pictures/Wallpapers");
    }
    info("Changing shell to zsh");
    execute(concat3("sudo chsh ", getenv("USER"), " -s /bin/zsh"));
    /*
    fputs(ANSI_COLOR_CYAN "Do you want to apply dark mode (y) or light mode wallpapers (n) (Y/n) " ANSI_COLOR_RESET, stdout);
    char test[2];
    fgets(test, 2, stdin);
    if (strcmp(test, "n") == 0) {
        execute("ln -sf ~/Pictures/lightmodewallpapers ~/Pictures/Wallpapers");
    }
    else {
        execute("ln -sf ~/Pictures/darkmodewallpapers ~/Pictures/Wallpapers");
    }*/
    info("Applying noctalia configs");
    execute(concat3("sed -ie 's/username/", getenv("USER"), "/' ~/.config/noctalia/settings.json"));
    info("Applying display scaling");
    int e = execute("xrandr > /dev/null");
    if (e != 0) {
        bad("Display not found");
        info("Remember to re-run config application in niri to apply display scaling.");
    }
    else {
        good("Display found");
        // definitely not vibecoded
        execute("DISP=$(xrandr | sed -n '2p' | awk '{print $1}'); sed -i \"s/eDP-1/$DISP/g\" ~/.config/niri/config.kdl");
        execute("DISP=$(xrandr | sed -n '2p' | awk '{print $1}'); sed -i \"s/eDP-1/$DISP/g\" ~/.config/noctalia/settings.json");
    }
}

void detectProblems() {
    info("Detecting problems");
    // check if binaries exist
    if (access("/usr/sbin/qs", F_OK) == 0) {
        good("Quickshell is installed properly");
    }
    else {
        bad("Quickshell wasn\'t installed properly");
    }
    if (access("/etc/xdg/quickshell/noctalia-shell/shell.qml", F_OK) == 0) {
        good("Noctalia Shell is installed properly");
    }
    else {
        bad("Noctalia Shell wasn\'t installed properly");
    }
    if (access("/usr/share/zsh-theme-powerlevel10k/powerlevel10k.zsh-theme", F_OK) == 0) {
        good("Powerlevel10k is installed properly");
    }
    else {
        bad("Powerlevel10k wasn\'t installed properly");
    }
}

bool handleSelection(int id, MENU * menu) {
    execute("clear");
    switch(id) {
        case 1:
            configure();
            break;
        case 2:
            installPackages();
            break;
        case 3:
            installYay();
            break;
        case 4:
            installAurPackages();
            break;
        case 5:
            applyConfigs();
            break;
        case 6:
            detectProblems();
            break;
        case 7:
            execute("reboot");
            return true;
            break;
        case 8:
            return true;
            break;
    }
    execute(concat2("read -n 1 -p \"", ANSI_COLOR_CYAN "Press any key to continue...\"" ANSI_COLOR_RESET));
    menu_driver(menu, REQ_TOGGLE_ITEM);
    return false;
}

void print_in_middle(WINDOW * win, int starty, int startx, int width, char * string, chtype color) {
    int length, x, y;
	float temp;

	if (win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if (startx != 0)
		x = startx;
	if (starty != 0)
		y = starty;
	if (width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length) >> 1;
	x = startx + (int) temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
}


void determineitems(ITEM ** items, int n_choices) {
    for(int i = 0; i < n_choices; ++i) { 
        char * prefix = choiceids[i];
        if (i <= n_choices - 2) {
            prefix = concat2(finishedtasks[i] ? "(Finished) " : "           ", prefix);
        }
        items[i] = new_item(prefix, choices[i]);
    }
}

int main() {
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
    int x, y;
    getmaxyx(stdscr, x, y);
	menu = new_menu((ITEM **) items);
    menu_opts_off(menu, O_ONEVALUE);
    menu_win = newwin(x / 2, y / 3, x / 3, y / 3);
    keypad(menu_win, TRUE);
    set_menu_win(menu, menu_win);
    set_menu_sub(menu, derwin(menu_win, 7, 40, 3, x / 5));
    set_menu_format(menu, 12, 1);
    set_menu_mark(menu, " * ");
    box(menu_win, 0, 0);
	print_in_middle(menu_win, 1, 0, y / 3, "Dotfiles", COLOR_PAIR(2));
	mvwaddch(menu_win, 2, 0, ACS_LTEE);
	mvwhline(menu_win, 2, 1, ACS_HLINE, y / 3 - 2);
	mvwaddch(menu_win, 2, y / 3 - 1, ACS_RTEE);
	wrefresh(menu_win);
	
	attron(COLOR_PAIR(2));
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (Q to Exit and ENTER to Select)");
	attroff(COLOR_PAIR(2));
    post_menu(menu);
	refresh();

	while(c != 'q' && !end) {
        c = wgetch(menu_win);
        switch(c) {
            case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
            // enter key
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
    for(int i = 0; i < n_choices; ++i)
        free_item(items[i]);
    free_menu(menu);
	endwin();
}
