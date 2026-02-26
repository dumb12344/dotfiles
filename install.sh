#!/bin/bash
echo Installing script packages and updating system
sudo pacman -Syu figlet jq git base-devel niri zsh xdg-desktop-portal-gnome \
    xwayland-satellite kitty cliphist cava xdg-desktop-portal brightnessctl \
    xdg-utils vulkan-radeon vulkan-intel vulkan-headers vulkan-tools ly neovim \
    ttf-cascadia-code-nerd --needed
sudo systemctl enable ly@tty1.service
sudo systemctl disable getty@tty1.service
figlet Stuff -f big
if [ ! -f /usr/bin/yay ]; then
    echo Removing debug from makepkg config
    sudo sed -ie 's/purge debug/purge !debug/' /etc/makepkg.conf
    echo Installing yay aur helper
    dir=$(pwd)
    yaytmp=$(mktemp -d)
    git clone https://aur.archlinux.org/yay-bin.git $yaytmp
    cd $yaytmp
    makepkg -si
    cd $dir
    sudo rm -rf $yaytmp
fi
echo Installing aur packages
yay -S noctalia-shell zsh-theme-powerlevel10k-git pokeget --needed
echo Copying configs
cp -rf configs/. ~
sudo chsh test -s /bin/zsh
chown -R $(whoami) ~/*
chgrp -R $(whoami) ~/*
echo Remember to re-run if any task fails:
if [ ! -f /usr/sbin/qs ]; then
    echo Quickshell wasn\'t installed properly
fi
if [ ! -f /etc/xdg/quickshell/noctalia-shell/shell.qml ]; then
    echo Noctalia shell wasn\'t installed properly
fi
if [ ! -f /usr/share/zsh-theme-powerlevel10k/powerlevel10k.zsh-theme ]; then
    echo Powerlevel10k wasn\'t installed properly
fi