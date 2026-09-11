#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
install_bundle() {
 local source="$1" destination="$2"
 [[ -d "$source" ]] || { echo "Falta compilar: $source"; exit 1; }
 mkdir -p "$(dirname "$destination")"
 # Replacement is restricted to our exact bundle; no other plugins are touched.
 if [[ -d "$destination" ]]; then
  local backup="$PWD/build/plugin-backups/$(date +%Y%m%d-%H%M%S)/$(basename "$destination")"
  mkdir -p "$(dirname "$backup")"
  mv "$destination" "$backup"
 fi
 ditto "$source" "$destination"
 codesign --force --deep --sign - "$destination"
}
install_bundle "build/RVocoder_artefacts/Release/AU/R-Vocoder.component" "$HOME/Library/Audio/Plug-Ins/Components/R-Vocoder.component"
install_bundle "build/RVocoderMIDI_artefacts/Release/AU/R-Vocoder MIDI.component" "$HOME/Library/Audio/Plug-Ins/Components/R-Vocoder MIDI.component"
install_bundle "build/RVocoder_artefacts/Release/VST3/R-Vocoder.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/R-Vocoder.vst3"
echo "R-Vocoder instalado para tu usuario. Reabre Logic/Ableton para escanearlo."
