#!/usr/bin/env bash
# Download binary
curl -Lo "$HOME/shell" https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/shell-macos-arm64

# Make it executable
chmod +x "$HOME/shell"

# Remove macOS quarantine flag so Gatekeeper doesn't block it
xattr -d com.apple.quarantine "$HOME/shell" 2>/dev/null || true

# Create a Terminal.app profile file pointing to the shell binary
cat > /tmp/spin_shell.terminal <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
  <key>CommandString</key><string>$HOME/shell</string>
  <key>name</key><string>spin_shell</string>
  <key>RunCommandAsShell</key><false/>
</dict></plist>
EOF

# Open the profile — Terminal.app imports it automatically
open /tmp/spin_shell.terminal

echo "Done. spin_shell profile added to Terminal.app."
