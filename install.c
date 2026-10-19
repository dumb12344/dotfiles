#include <json-c/json_object.h>
#include <json-c/json_object_iterator.h>
#include <json-c/json_util.h>
#include <ncurses.h>
#include <menu.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <json-c/json.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4
#define ANSI_COLOR_RED   "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_CYAN  "\x1b[36m"

json_object * settings;

// https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
// https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
char * choices[] = {
    "Configure",
    "Update mirrors",
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
    "9",
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
    false,
};

bool str_eq(const char * a, const char * b) {
    return strcmp(a, b) == 0;
}

char * concat2(const char * s1, const char * s2) {
    // https://stackoverflow.com/questions/8465006/how-do-i-concatenate-two-strings-in-c
    char * result = malloc(strlen(s1) + strlen(s2) + 1);
    snprintf(result, strlen(s1) + strlen(s2) + 1, "%s%s", s1, s2);
    return result;
}

char * concat3(const char * s1, const char * s2, const char * s3) {
    char * result = malloc(strlen(s1) + strlen(s2) + strlen(s3) + 1);
    snprintf(result, strlen(s1) + strlen(s2) + strlen(s3) + 1, "%s%s%s", s1, s2, s3);
    return result;
}

void info(const char * inText) {
    puts(concat3(ANSI_COLOR_CYAN, inText, ANSI_COLOR_RESET));
}

void good(const char * inText) {
    puts(concat3(ANSI_COLOR_GREEN, inText, ANSI_COLOR_RESET));
}

void bad(const char * inText) {
    puts(concat3(ANSI_COLOR_RED, inText, ANSI_COLOR_RESET));
}

int execute(const char * command) {
    // I can change this to a puts statement for debugging
    return system(command);
}

void initConfigure() {
    info("Do you want to use Zen Browser (y) or LibreWolf (n) (Y/n)");
    char choice[2];
    fgets(choice, 2, stdin);
    char * browser = !str_eq(choice, "n") ? "zen-browser-bin" : "librewolf";
    char * browserBinary = !str_eq(choice, "n") ? "zen-browser" : "librewolf";
    json_object_set_string(json_object_object_get(settings, "browser"), browser);
    json_object_set_string(json_object_object_get(settings, "browserBinary"), browserBinary);
    getchar();

    info("Do you want to use Dark mode? (Y/n)");
    fgets(choice, 2, stdin);
    bool darkMode = !str_eq(choice, "n");
    json_object_set_boolean(json_object_object_get(settings, "darkMode"), darkMode);
    getchar();

    info("Do you want Number Lock? (Y/n)");
    fgets(choice, 2, stdin);
    bool numlock = !str_eq(choice, "n");
    json_object_set_boolean(json_object_object_get(settings, "numlock"), numlock);
    getchar();

    info("Do you want to use ly? (Y/n)");
    fgets(choice, 2, stdin);
    bool ly = !str_eq(choice, "n");
    json_object_set_boolean(json_object_object_get(settings, "ly"), ly);
    getchar();
}

void configure() {
    settings = json_object_from_file("settings.json");
    if (!settings) {
        settings = json_object_new_object();
        json_object_object_add(settings, "darkMode", json_object_new_boolean(true));
        json_object_object_add(settings, "browser", json_object_new_string("zen-browser-bin"));
        json_object_object_add(settings, "browserBinary", json_object_new_string("zen-browser"));
        json_object_object_add(settings, "numlock", json_object_new_boolean(true));
        json_object_object_add(settings, "ly", json_object_new_boolean(true));
        initConfigure();
        json_object_to_file_ext("settings.json", settings, JSON_C_TO_STRING_PRETTY);
    }
}

void updateMirrors() {
    if (access("/usr/sbin/reflector", F_OK) == 0) {
        good("reflector is already installed.");
    } else {
        bad("reflector is not installed.");
        info("Installing reflector");
        execute("sudo pacman -Syu reflector");
    }
    info("Updating mirrors");
    execute("sudo reflector --latest 20 --protocol https --sort rate --save /etc/pacman.d/mirrorlist");
}

void installPackages() {
    info("Installing packages and updating system");
    // enable color and animation for pacman/yay
    execute("sudo sed -ie 's/#Color/Color\\nILoveCandy/' /etc/pacman.conf");
    execute("sudo pacman -Syu figlet jq git base-devel niri zsh zsh-syntax-highlighting \
            xdg-desktop-portal-gnome xwayland-satellite kitty cliphist cava xdg-desktop-portal \
            brightnessctl xdg-utils vulkan-radeon vulkan-intel vulkan-headers vulkan-tools ly neovim \
            ttf-cascadia-code-nerd qt6ct qt5ct nwg-look adw-gtk-theme grim slurp satty noctalia --needed"
    );
    if (json_object_get_boolean(json_object_object_get(settings, "ly"))) {
        info("Enabling ly");
        execute("sudo systemctl enable ly@tty1.service && sudo systemctl disable getty@tty1.service");
    }
    info("Changing shell to zsh");
    execute(concat3("sudo chsh ", getenv("USER"), " -s /bin/zsh"));
}

void installYay() {
    if (access("/usr/bin/yay", F_OK) == 0) {
        good("yay is already installed.");
    } else {
        bad("yay is not installed.");
        info("Disabling debug packages in makepkg.conf");
        execute("sudo sed -ie 's/purge debug/purge !debug/' /etc/makepkg.conf");

        info("Creating temporary directory");
        char template[] = "/tmp/yay.XXXXXX";
        char * dir_name = mkdtemp(template);
        info(concat2("Created directory ", dir_name));

        execute(concat2("git clone https://aur.archlinux.org/yay-bin.git ", dir_name));
        char dotsdir[1000];
        getcwd(dotsdir, sizeof(dotsdir));

        chdir(dir_name);
        execute("makepkg -si");
        chdir(dotsdir);

        execute(concat2("rm -rf ", dir_name));
    }
}

void installAurPackages() {
    info("Installing AUR packages");
    configure();
    execute(concat2("yay -Syu --needed zsh-theme-powerlevel10k pokeget ", json_object_get_string(json_object_object_get(settings, "browser"))));
}

void applyConfigs() {
    info("Applying configs");
    configure();
    info("Copying user configs");
    execute("cp -rf home/. ~");
    execute(concat3("sed -ie \"s/browserchoice/", json_object_get_string(json_object_object_get(settings, "browserBinary")), "/g\" ~/.config/niri/config.kdl"));
    execute("mkdir -p ~/Pictures/Wallpapers");
    if (json_object_get_boolean(json_object_object_get(settings, "darkMode"))) {
        execute("cp -rf wallpapers/darkmodewallpapers/* ~/Pictures/Wallpapers");
    }
    else {
        // execute("sed -ie 's/\"darkMode\": true,/\"darkMode\": false,/' ~/.config/noctalia/settings.json");
        execute("sed -ie 's/mode = \"dark\"/mode = \"light\"/' ~/.config/noctalia/settings.toml");
        execute("cp -rf wallpapers/lightmodewallpapers/* ~/Pictures/Wallpapers");
    }
    /*
    fputs(ANSI_COLOR_CYAN "Do you want to apply dark mode (y) or light mode wallpapers (n) (Y/n) " ANSI_COLOR_RESET, stdout);
    char test[2];
    fgets(test, 2, stdin);
    if (strcmp(test, "n") == 0) {
        execute("ln -sf ~/Pictures/lightmodewallpapers ~/Pictures/Wallpapers");
    }
    else {\
        execute("ln -sf ~/Pictures/darkmodewallpapers ~/Pictures/Wallpapers");
    }*/
    info("Applying noctalia configs");
    // execute(concat3("sed -ie 's/username/", getenv("USER"), "/' ~/.config/noctalia/settings.json"));
    execute(concat3("sed -ie 's/username/", getenv("USER"), "/' ~/.config/noctalia/settings.toml"));
    info("Applying display scaling");
    int e = execute("niri msg outputs > /dev/null");
    if (e != 0) {
        bad("Display not found");
        info("Remember to re-run config application in niri to apply display scaling.");
    }
    else {
        good("Display found");
        // definitely not vibecoded
        execute("DISP=$(niri msg -j outputs | jq -r \"keys[0]\"); sed -i \"s/eDP-1/$DISP/g\" ~/.config/niri/config.kdl");
        // execute("DISP=$(niri msg -j outputs | jq -r \"keys[0]\"); sed -i \"s/eDP-1/$DISP/g\" ~/.config/noctalia/settings.json");
        execute("DISP=$(niri msg -j outputs | jq -r \"keys[0]\"); sed -i \"s/eDP-1/$DISP/g\" ~/.config/noctalia/settings.toml");
    }
    if (!json_object_get_boolean(json_object_object_get(settings, "numlock"))) {
        info("Disabling number lock");
        execute("sed -i \"s/numlock//\" ~/.config/niri/config.kdl");
    }
    info("Settings file explorer");
    execute("xdg-mime default org.kde.dolphin.desktop inode/directory");
}

void detectProblems() {
    info("Detecting problems");
    // check if binaries exist
    if (access("/usr/sbin/noctalia", F_OK) == 0) {
        good("Noctalia is installed properly");
    }
    else {
        bad("Noctalia wasn\'t installed properly");
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
            updateMirrors();
            break;
        case 3:
            installPackages();
            break;
        case 4:
            installYay();
            break;
        case 5:
            installAurPackages();
            break;
        case 6:
            applyConfigs();
            break;
        case 7:
            detectProblems();
            break;
        case 8:
            execute("reboot");
            return true;
            break;
        case 9:
            return true;
            break;
    }
    execute(concat2("read -n 1 -p \"", ANSI_COLOR_CYAN "Press any key to continue...\"" ANSI_COLOR_RESET));
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
    for (int i = 0; i < n_choices; ++i) {
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
    int menuwidth = 41;
	menu = new_menu((ITEM **) items);
    menu_win = newwin(x / 2, menuwidth, x / 3, y / 2 - 20);
    keypad(menu_win, TRUE);
    set_menu_win(menu, menu_win);
    set_menu_sub(menu, derwin(menu_win, ARRAY_SIZE(choiceids), 40, 3, 1));
    set_menu_format(menu, 12, 1);
    set_menu_mark(menu, "*");
    box(menu_win, 0, 0);
	print_in_middle(menu_win, 1, 0, menuwidth, "Dotfiles", COLOR_PAIR(2));
	mvwaddch(menu_win, 2, 0, ACS_LTEE);
	mvwhline(menu_win, 2, 1, ACS_HLINE, menuwidth - 2);
	mvwaddch(menu_win, 2, menuwidth - 1, ACS_RTEE);
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
	json_object_put(settings);
}
