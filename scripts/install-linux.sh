#!/usr/bin/env bash

# Download the Linux binary from the latest GitHub release
curl -Lo "$HOME/shell" https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/shell-linux-x64

# Make the binary executable
chmod +x "$HOME/shell"

# Generate a unique ID for the new GNOME Terminal profile
P=$(uuidgen)

# Set the profile display name
dconf write /org/gnome/terminal/legacy/profiles:/:$P/visible-name "'spin_shell'"

# Tell GNOME Terminal to launch a custom command instead of the default shell
dconf write /org/gnome/terminal/legacy/profiles:/:$P/use-custom-command true

# Set that custom command to our shell binary
dconf write /org/gnome/terminal/legacy/profiles:/:$P/custom-command "'$HOME/shell'"

# Append the new profile ID to the list of known profiles
dconf write /org/gnome/terminal/legacy/profiles:/list \
  "$(dconf read /org/gnome/terminal/legacy/profiles:/list | sed "s/]/, '$P']/")"

echo "Done. Restart GNOME Terminal — spin_shell appears in profiles."
