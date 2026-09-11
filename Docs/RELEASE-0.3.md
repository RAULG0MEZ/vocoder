# R-Vocoder 0.3.0 — tocar, ver y dar forma a la voz

## Al abrirlo

El teclado aparece desde el inicio y permanece visible. La entrada, el tipo de sonido,
la activación y ocho macros comparten la vista principal. VOZ, SYNTH, FX, MOTION y
SALIDA abren ajustes en el inspector lateral sin sustituir la vista de interpretación.
La ventana se adapta entre 1080 × 760 y 1800 × 1200, con 1320 × 840 como tamaño inicial.

**Con teclas** selecciona MIDI y activa **Cortar al soltar**. La salida completa abre
con la primera nota y cierra después de soltar la última. El pedal de sustain sostiene
la salida hasta levantarlo. La apertura tiene una rampa de 3 ms; el cierre se ajusta
con Key release en SALIDA, de 5 a 600 ms (70 ms de fábrica).

**Solo efecto** evita que Mix o Voice mix de un preset añadan la grabación sin procesar.
El control Mix indica 100% y se atenúa mientras está activo. La reconstrucción de
consonantes sigue disponible como parte del vocoder. En Voz se transforma la grabación
conservando su melodía; MIDI abre/cierra el audio, sin convertirlo en un afinador.

Ambas protecciones y la duración del cierre se conservan al cambiar de preset o usar
Reset. Silenciar libera las notas y el sustain. Pulsar el teclado en pantalla activa
Con teclas automáticamente; las notas MIDI externas continúan visibles sin duplicarse.

## Un gráfico que controla el sonido

- XY: horizontal modifica Formant y vertical modifica Character.
- Espectro: arrastrar los bordes ajusta las frecuencias inferior y superior del banco.
- Los movimientos actualizan los controles reales y notifican gestos de automatización.
- Doble clic restablece el par de controles; las flechas permiten ajustes pequeños.
- La curva representa las envolventes medidas de las bandas. No es una animación ficticia.

El resto del motor de vocoder, las rutas y los 100 presets originales se mantienen.
La interfaz es propia; no se incorpora código, gráficos ni módulos de VocalSynth.

## Corrección de la voz que pasaba entre notas

La edición anterior aplicaba MIDI al sintetizador. Mix, Voice mix, las consonantes y
algunos efectos podían producir audio aunque no hubiera osciladores sonando.

La nueva puerta se aplica después de todas esas mezclas y efectos, antes del bypass.
Al terminar la rampa llega exactamente a cero, sin cola exponencial residual. Sigue
notas por canal independientemente de las 12 voces del sintetizador, por lo que robar
una voz no pierde el estado de las teclas. Soporta note-off con velocidad cero, sustain,
CC120/123, reset de controladores y reset MIDI. No añade reservas de memoria ni locks
al hilo de audio. Los medidores se limpian al parar o reiniciar el procesamiento.

## Compatibilidad

Los identificadores existentes no cambian. Se añaden al final `midiGate`,
`midiGateRelease` y `wetOnly`. Las sesiones anteriores sin los nuevos interruptores
se restauran con ambos apagados para conservar su sonido. Para adoptar el cambio:
elige Con teclas, activa Cortar al soltar y Solo efecto, y guarda de nuevo tu canción.

Una instancia nueva trae ambas protecciones activas. El AU MIDI comienza en Con teclas
y el efecto normal en Continuo. Bypass pasa deliberadamente la señal de la pista.
La pista que alimenta el sidechain puede tener otra salida hacia el master; R-Vocoder
no puede silenciar esa ruta externa.

## Validación

Compilación universal arm64 + x86_64 para AU, AU MIDI, VST3 y Standalone.

| Comprobación | Registro |
|---|---|
| DSP, puerta completa, rampas, sustain, canales y Solo efecto | [DSP](Validation/dsp-0.3.txt) |
| Memoria y comportamiento indefinido en el DSP | [ASan/UBSan](Validation/asan-0.3.txt) |
| Mismo DSP x86_64 mediante Rosetta | [Rosetta](Validation/rosetta-0.3.txt) |
| MIDI en posición de muestra, restauración, presets y UI a tres tamaños | [Integración](Validation/integration-0.3.txt) |
| Sonido y silencio entre notas a través de los plugins reales | [Host](Validation/host-wrappers-0.3.txt) |
| AU y AU MIDI | [auval](Validation/au-effect-0.3.txt), [auval MIDI](Validation/au-midi-0.3.txt) |
| pluginval, nivel 10 | [VST3](Validation/vst3-pluginval-0.3.txt), [AU](Validation/au-effect-pluginval-0.3.txt), [AU MIDI](Validation/au-midi-pluginval-0.3.txt) |
| 100 presets, renders de voz en ambos modos | [Synth](Validation/presets-synth-0.3.csv), [Voz](Validation/presets-voice-0.3.csv) |

Las pruebas de UI comprueban arrastre real de eventos, actualización de parámetros,
límites, ajustes por flechas y teclado visible en todos los inspectores y tamaños.
Se comprobaron 44.1, 48, 88.2 y 96 kHz y bloques de 32 a 2048 muestras con pluginval.
Se mantienen las 16 muestras de latencia del motor; la puerta no añade latencia.

Continúa como beta: las pruebas automatizadas de los wrappers no sustituyen una
audición y recorrido completos dentro de Logic/Ableton. No se declara prueba física
en Intel, certificación en todas las versiones de macOS ni un binario de Windows.
Los plugins tienen firma ad hoc y no están notarizados por Apple.
