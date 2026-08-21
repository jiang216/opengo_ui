#!/usr/bin/env sh
# Install the graphgen global bootstrap (skill + plugin) into ~/.config/opencode/
# Usage: sh install-bootstrap.sh
# Install once per machine: afterwards any project that gets a graphgen-dist/ folder
# is auto-installed when opened with opencode.
set -e
src="$(cd "$(dirname "$0")" && pwd)"
cfg="${XDG_CONFIG_HOME:-$HOME/.config}/opencode"
sk_dir="$cfg/skills/graphgen-bootstrap"
pl_dir="$cfg/plugins"
mkdir -p "$sk_dir" "$pl_dir"
cp -f "$src/skill-graphgen-bootstrap.md" "$sk_dir/SKILL.md"
cp -f "$src/plugin-graphgen-bootstrap.ts" "$pl_dir/graphgen-bootstrap.ts"
echo "[graphgen-bootstrap] installed to $cfg:"
echo "  $sk_dir/SKILL.md"
echo "  $pl_dir/graphgen-bootstrap.ts"
echo "[graphgen-bootstrap] restart opencode to activate. Any project containing graphgen-dist/ will auto-install graphgen."
