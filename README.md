# R-Vocoder

Vocoder creativo de RSTK para macOS. Incluye AU para pistas de audio, AU controlado
por MIDI para Logic, VST3 y una aplicación independiente. Compilación universal
Apple Silicon + Intel, macOS 11 o posterior. Versión inicial 0.1.0.

**Estado:** compilado, instalado y aprobado por auval y pluginval nivel 10.
Disponible para evaluación; falta completar la audición y el recorrido real dentro
de Logic/Ableton antes de declararlo listo para producción. Consulta los
[resultados y pendientes](Docs/VALIDATION.md).

## Empieza a cantar

1. Instala los plugins y vuelve a abrir tu DAW.
2. Inserta **R-Vocoder** en una pista con voz.
3. Deja `Voice + internal`: el carrier suena automáticamente con el modo Drone.
4. Prueba **French Chrome**, **Classic Robot**, **Pop Robot** o **Wide Choir**.
5. Ajusta **Clarity**, **Formant**, **Width** y **Mix**. Si el ruido abre el efecto,
   sube **Threshold** en VOCODER → Gate.

El plugin no requiere internet, cuentas, secretos ni configuración externa.

## Instalación local

Desde una compilación del proyecto:

```sh
bash Scripts/install-macos.sh
```

Se instalan exclusivamente:

- `~/Library/Audio/Plug-Ins/Components/R-Vocoder.component`
- `~/Library/Audio/Plug-Ins/Components/R-Vocoder MIDI.component`
- `~/Library/Audio/Plug-Ins/VST3/R-Vocoder.vst3`

No hace falta administrador. La aplicación independiente queda en
`build/RVocoder_artefacts/Release/Standalone/R-Vocoder.app`.
Usa audífonos si habilitas entrada de micrófono en Standalone. El wrapper de JUCE
incluye un interruptor de entrada para evitar realimentación acústica.

La distribución local se genera con `bash Scripts/package-macos.sh` en `dist/`;
contiene los bundles, `Install.command`, documentación y licencias. Los binarios
tienen firma ad hoc para pruebas locales; **no están notarizados para distribución comercial**.

## Logic Pro: efecto en una pista de audio

En una ranura **Audio FX**, elige **Audio Units → RSTK → R-Vocoder**.
Están contemplados Mono, Mono → Stereo y Stereo.

| Sources | Pista donde insertas R-Vocoder | Selector Side Chain de Logic |
|---|---|---|
| Voice + internal | Voz | No necesario |
| Voice + sidechain carrier | Voz | Sintetizador, guitarra, acordes u otra señal armónica |
| Carrier + sidechain voice | Sintetizador o carrier | Pista vocal |
| Sidechain voice + internal | Pista auxiliar que aloje el efecto | Pista vocal |

La voz **modulator** determina las palabras; el **carrier** aporta las notas y los armónicos.
En un carrier externo convienen sonidos ricos en armónicos; una senoide tiene pocas
frecuencias con las que reconstruir palabras.

## Logic Pro: carrier mediante MIDI

1. Crea una pista **Software Instrument**.
2. En su ranura **Instrument**, abre **AU MIDI-controlled Effects → RSTK → R-Vocoder MIDI**.
   Es la ranura de instrumento, no la de MIDI FX.
3. En el selector **Side Chain** de la ventana del plugin, elige la pista de voz.
4. La edición MIDI comienza con `Sidechain voice + internal` y `Play mode: MIDI`.
5. Reproduce la voz y toca notas o acordes. Guarda la canción normalmente.

El AU de efecto normal comparte el motor con el VST3, pero Logic enruta MIDI a la
edición **R-Vocoder MIDI** desde una pista de instrumento. La guía de Apple documenta
este patrón de instrumento y voz por sidechain:
[fuente de análisis e instrumento MIDI](https://support.apple.com/en-mide/guide/logicpro/lgsifc36886a/10.7/mac/11.0).

## Ableton Live: VST3 y sidechain

1. En Plug-ins, carga **VST3 → RSTK → R-Vocoder** en la pista que va a procesarse.
2. Despliega el panel de sidechain del dispositivo, actívalo y elige **Audio From**.
3. Usa la tabla anterior para elegir **Sources** dentro de R-Vocoder.
4. Para tocar el carrier, selecciona `MIDI` en SYNTH y enruta una pista MIDI hacia
   la pista que contiene R-Vocoder; elige el plugin como destino cuando Live lo ofrezca.
   Si tu configuración no ofrece ese destino, usa Drone o un carrier externo.

Live presenta el selector de entrada auxiliar del plugin en su propio dispositivo:
[guía oficial de sidechain](https://help.ableton.com/hc/en-us/articles/209775325-Sidechaining-a-third-party-plug-in).

## Controles

- **MAIN:** ocho macros y actividad de las bandas. Clarity refuerza envolventes débiles
  y consonantes; Character aumenta saturación y densidad; Motion escala los destinos de movimiento.
- **VOCODER:** análisis, identidad vocal, consonantes, gate y mezclas. Preset level
  compensa el volumen propio del preset sin cambiar la ganancia de tu proyecto.
- **SYNTH:** siete ondas, mezcla con senoide, unísono, detune, filtro resonante, ADSR,
  glide, octava y acordes del Drone. El teclado de pantalla envía notas en modo MIDI.
- **FX:** anchura, spread, movimiento estéreo, distorsión, crusher, speaker, vintage,
  modern, warmth, exciter, ruido, tono y ganancias de entrada/salida.
- **MOTION:** LFO libre o sincronizado, divisiones de 1/1 a 1/16, puntillo y tresillo,
  seguidor de envolvente y destinos de bandas, filtro, formantes, panorama y anchura.

**Mix** usa curvas equal-power. **Vocoder amount** combina transferencia espectral con
carrier gobernado por el volumen de la voz. **Voice mix** agrega voz original y
**Carrier mix** agrega carrier. Todos respetan el gate del modulador en la cadena procesada.

El medidor de correlación ayuda a vigilar la compatibilidad mono. Un valor negativo
indica mucho contenido lateral: reduce Width/Spread/Unison width si el sonido pierde
demasiado cuerpo al escuchar en mono. Formant, Size e Identity son transformaciones
de la envolvente espectral, no conversión de voz neuronal.

## Presets personales

La biblioteca incluye **100 presets originales en 10 categorías**, búsqueda por nombre,
categoría o tags, anterior/siguiente, Shuffle y favoritos. **Save** guarda un sonido con
el nombre que escribas; **Delete** permite borrar únicamente presets personales y pide
confirmación. **Reset** recupera el sonido inicial conservando tus conexiones y ganancias.

Al cambiar de preset se conservan Sources, Drone/MIDI, las ganancias de entrada/salida
y Bypass. Las notas/acordes del Drone sí forman parte del sonido. Los proyectos del DAW
guardan sus parámetros completos aunque luego borres un preset personal.

Los presets de usuario y favoritos se guardan en
`~/Library/Application Support/RSTK/R-Vocoder/`. No es necesario copiar los presets
para recuperar un proyecto guardado por el DAW.

## Compilar

Requisitos: compilador C++20, Apple Command Line Tools en macOS, CMake 3.22 o superior
y conexión únicamente para descargar JUCE la primera vez. No se necesita Xcode completo.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

Si se usó el entorno privado incluido en esta estación, sustituye `cmake`/`ctest` por
`.venv/bin/cmake`/`.venv/bin/ctest`. `bash Scripts/build.sh` lo detecta automáticamente.
JUCE está fijado por commit; puede estar en `.deps/JUCE` o descargarse con FetchContent.
Las dependencias y compilaciones pertenecen únicamente a este proyecto.

Pruebas DSP sin JUCE:

```sh
cmake -S . -B build-dsp -DRV_BUILD_PLUGINS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-dsp
ctest --test-dir build-dsp --output-on-failure
```

En Windows usa Visual Studio 2022 con C++20 y CMake, sin la opción de arquitecturas macOS.
CMake configura VST3 y Standalone; AU queda excluido. El port está preparado, pero
no se certifica una compilación de Windows que no se haya ejecutado.

## Validación y arquitectura

- [Resultados y límites de las pruebas](Docs/VALIDATION.md).
- [Arquitectura DSP, sincronía y latencia](Docs/ARCHITECTURE.md).
- [Ubicación de datos y configuración](Docs/CONFIGURATION.md).
- [Dependencias y licencias](THIRD_PARTY_NOTICES.md).

Los IDs de automatización son permanentes y APVTS guarda el estado de la sesión.
Se reportan **16 muestras de latencia**; la señal dry recibe la misma compensación.
El motor no reserva memoria ni toma locks en `processBlock`.

JUCE no es MIT: hay que disponer de la licencia comercial que corresponda para
mantener un producto cerrado. Consulta [los términos oficiales](https://juce.com/legal/juce-8-licence/)
antes de distribuirlo. No se incorporaron presets, interfaces ni código propietario de otros vocoders.
