#!/bin/bash
echo Installing script packages and updating system
sudo pacman -Syu figlet jq git base-devel niri zsh xdg-desktop-portal-gnome xwayland-satellite kitty cliphist cava xdg-desktop-portal brightnessctl --needed
figlet Stuff -f big
echo Installing yay aur helper
yaytmp=$(mktemp -d)
git clone https://aur.archlinux.org/yay-bin.git $yaytmp
cd $yaytmp
makepkg -si
cd ~
sudo rm -rf $yaytmp
echo Installing aur packages
yay -S noctalia-shell zsh-theme-powerlevel10k-git pokeget
echo copying configs
cp -i -r configs/* ~/
chown -R $(whoami) ~/*
chgrp -R $(whoami) ~/*