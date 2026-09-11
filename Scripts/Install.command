#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
for name in 'R-Vocoder.component' 'R-Vocoder MIDI.component' 'R-Vocoder.vst3'; do
  source="$PWD/Plugins/$name"
  [[ -d "$source" ]] || { echo "Falta $source"; exit 1; }
  kind=Components
  [[ "$name" != *.vst3 ]] || kind=VST3
  target="$HOME/Library/Audio/Plug-Ins/$kind/$name"
  mkdir -p "$(dirname "$target")"
  if [[ -d "$target" ]]; then
    backup="$PWD/Previous/$(date +%Y%m%d-%H%M%S)/$name"
    mkdir -p "$(dirname "$backup")"
    mv "$target" "$backup"
  fi
  ditto "$source" "$target"
done
echo "R-Vocoder está instalado. Reabre Logic o Ableton para detectar los plugins."
