#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
package="$PWD/dist/R-Vocoder-0.2.0-macOS"
mkdir -p "$package/Plugins" "$package/ThirdParty" "$package/Docs"
ditto 'build/RVocoder_artefacts/Release/AU/R-Vocoder.component' "$package/Plugins/R-Vocoder.component"
ditto 'build/RVocoderMIDI_artefacts/Release/AU/R-Vocoder MIDI.component' "$package/Plugins/R-Vocoder MIDI.component"
ditto 'build/RVocoder_artefacts/Release/VST3/R-Vocoder.vst3' "$package/Plugins/R-Vocoder.vst3"
ditto 'build/RVocoder_artefacts/Release/Standalone/R-Vocoder.app' "$package/R-Vocoder.app"
for bundle in "$package/Plugins/"* "$package/R-Vocoder.app"; do codesign --force --deep --sign - "$bundle"; done
cp README.md THIRD_PARTY_NOTICES.md "$package/"
ditto Docs "$package/Docs"
if [[ -d build/demo/calibrated ]]; then
 mkdir -p "$package/Demos"
 cp build/demo/calibrated/*.wav "$package/Demos/"
 cp build/demo/voice.wav "$package/Demos/Original voice.wav"
fi
if [[ -d build/demo/original-voice ]]; then
 mkdir -p "$package/Demos/Voz original"
 cp build/demo/original-voice/*.wav "$package/Demos/Voz original/"
 cp build/demo/voice.wav "$package/Demos/Sin procesar.wav"
fi
cp Scripts/Install.command "$package/Install.command"
chmod +x "$package/Install.command"
python3 - "$package" <<'PY'
from pathlib import Path
import shutil,sys
source=Path('.deps/JUCE')
destination=Path(sys.argv[1])/'ThirdParty'/'JUCE'
for p in source.rglob('*'):
    if p.is_file() and (p.name.lower().startswith(('license','licence','copying')) or p.name=='LICENSE.md'):
        output=destination/p.relative_to(source)
        output.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(p,output)
PY
ditto -c -k --sequesterRsrc --keepParent "$package" "$PWD/dist/R-Vocoder-0.2.0-macOS.zip"
(cd dist && shasum -a 256 R-Vocoder-0.2.0-macOS.zip > SHA256SUMS.txt)
echo "Paquete listo: $package"
