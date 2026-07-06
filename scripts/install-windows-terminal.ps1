# Download the Windows binary from the latest GitHub release
iwr https://github.com/PRITISH-TOMAR/spin_shell/releases/latest/download/shell-windows-x64.exe -OutFile "$env:USERPROFILE\shell.exe"

# Path to Windows Terminal's settings file
$f = "$env:LOCALAPPDATA\Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState\settings.json"

# Parse the existing settings JSON
$c = Get-Content $f -Raw | ConvertFrom-Json

# Append a new profile entry pointing to the downloaded binary
$c.profiles.list += [pscustomobject]@{ name="spin_shell"; guid="{a8f1c2d3-4e5b-6f7a-8b9c-0d1e2f3a4b5c}"; commandline="$env:USERPROFILE\shell.exe"; startingDirectory="%USERPROFILE%" }

# Save the updated settings back
$c | ConvertTo-Json -Depth 10 | Set-Content $f
