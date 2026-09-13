#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/validation
./build/rv_dsp_tests | tee build/validation/dsp.txt
./build/rv_plugin_tests_artefacts/Release/rv_plugin_tests | tee build/validation/integration.txt
./build/rv_host_tests "$PWD/build/RVocoder_artefacts/Release/VST3/R-Vocoder.vst3" \
 "$HOME/Library/Audio/Plug-Ins/Components/R-Vocoder.component" \
 "$HOME/Library/Audio/Plug-Ins/Components/R-Vocoder MIDI.component" | tee build/validation/host-wrappers.txt
auval -v aufx Rvoc Rstk | tee build/validation/au-effect-final.txt
auval -v aumf Rvmi Rstk | tee build/validation/au-midi-final.txt
python3 - <<'PY'
from pathlib import Path
import plistlib, re
for bundle, log in [
 ("build/RVocoder_artefacts/Release/AU/R-Vocoder.component", "au-effect-final.txt"),
 ("build/RVocoderMIDI_artefacts/Release/AU/R-Vocoder MIDI.component", "au-midi-final.txt")
]:
 expected = plistlib.loads((Path(bundle)/"Contents/Info.plist").read_bytes())["CFBundleShortVersionString"]
 found = re.search(r"Component Version: ([0-9.]+)", (Path("build/validation")/log).read_text())
 if not found or found.group(1) != expected:
  raise SystemExit("AU cache/version mismatch: install this build, refresh AudioComponentRegistrar for your user, then rerun validation.")
PY
validator="${RV_PLUGINVAL:-.deps/pluginval/pluginval.app/Contents/MacOS/pluginval}"
if [[ -x "$validator" ]]; then
 options=(--strictness-level 10 --random-seed 0x52564f43 --sample-rates 44100,48000,88200,96000 --block-sizes 32,64,128,256,512,1024,2048)
 "$validator" "${options[@]}" --validate "$PWD/build/RVocoder_artefacts/Release/VST3/R-Vocoder.vst3" | tee build/validation/vst3-final.txt
 "$validator" "${options[@]}" --validate "$HOME/Library/Audio/Plug-Ins/Components/R-Vocoder.component" | tee build/validation/au-effect-pluginval.txt
 "$validator" "${options[@]}" --validate "$HOME/Library/Audio/Plug-Ins/Components/R-Vocoder MIDI.component" | tee build/validation/au-midi-pluginval.txt
else
 echo "pluginval no está instalado; consulta Docs/VALIDATION.md."
 exit 1
fi
