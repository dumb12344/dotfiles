#!/bin/bash
echo Installing script packages and updating system
sudo pacman -Syu figlet jq git base-devel niri zsh xdg-desktop-portal-gnome xwayland-satellite kitty cliphist cava xdg-desktop-portal brightnessctl ly neovim --needed
sudo systemctl enable ly@tty1.service
sudo systemctl disable getty@tty1.service
figlet Stuff -f big
if [ ! -f /usr/bin/yay ]; then
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
echo copying configs
cp -rf configs/. ~
chown -R $(whoami) ~/*
chgrp -R $(whoami) ~/*