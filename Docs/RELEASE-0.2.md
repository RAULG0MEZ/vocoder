# R-Vocoder 0.2.0 — voz original, entradas claras y respuesta MIDI

## Qué cambia al usarlo

- **Voz desde: Pista / Sidechain** reemplaza el menú de cuatro combinaciones.
  Los botones configuran el recorrido interno. La pista externa se elige en el
  selector Side Chain del DAW, porque el plugin no controla ese selector.
- **Sonido: Voz** procesa la grabación conservando su melodía, sin osciladores y sin
  necesitar notas MIDI. Formantes, carácter, gate, claridad, anchura, tono y efectos
  siguen disponibles. No es un afinador de la voz controlado por MIDI.
- **Sonido: Synth** mantiene el vocoder con sintetizador interno y Drone/MIDI.
  **Externo** usa la otra entrada como carrier, en el AU de efecto y VST3.
- Las teclas blancas y negras reflejan las notas entrantes de cualquier canal MIDI.
  El teclado puede desplazarse por las 128 notas y muestra un indicador de actividad.
  Las notas visualizadas no vuelven a enviarse al sintetizador.
- **Mix** mezcla con la voz seleccionada, incluida la que llega por sidechain.
  El bypass del efecto conserva el audio de la pista principal, con su compensación temporal.
- Los presets conservan la entrada y el modo Voz/Synth/Externo que elegiste.
  Los controles que no participan en Voz original se atenúan para evitar confusión.
- Se corrigió la visualización de acentos y se unificó el nombre de las octavas del teclado.

En una instancia nueva del efecto se selecciona **Pista + Synth**. En el AU MIDI se
configura **Sidechain + Synth + MIDI**. Para escuchar la voz sin sintetizador basta
con pulsar **Voz**. La entrada de voz debe estar reproduciéndose o con escucha activa.

## Edición MIDI en Logic

El AU MIDI expone una sola entrada de audio, **Voice Sidechain**, en el elemento cero.
Corresponde a la entrada que utiliza Logic al cargar un efecto controlado por MIDI
en la ranura de instrumento. Se eliminó la segunda entrada innecesaria de esa edición
y el recorrido se fija automáticamente a voz de sidechain con sonido interno.

Por esa razón Pista y Externo aparecen desactivados en **R-Vocoder MIDI**. Para trabajar
con audio directo de una pista o combinar dos señales externas se usa **R-Vocoder**
en Audio FX. No se usa detección de silencio para adivinar cuál es la entrada: una
pausa en la voz no cambia las conexiones.

La observación de Logic está documentada por desarrolladores en el
[foro de JUCE](https://forum.juce.com/t/input-bus-layout-in-aumf-au-effects-in-logic/59902).
El [modelo de buses de JUCE](https://juce.com/tutorials/tutorial_audio_bus_layouts/)
define la primera entrada como bus principal; el nombre visible Side Chain del host
no implica que la señal llegue al índice uno en una ranura de instrumento.

## Validación ejecutada el 11 de septiembre de 2026

Mismo entorno macOS/Apple Silicon, JUCE 8.0.12 y compilador que la entrega inicial.
Compilación universal arm64 + x86_64, AU, AU MIDI, VST3 y Standalone.

| Comprobación | Resultado | Registro |
|---|---|---|
| DSP y nuevo modo de voz original | 489,992 aserciones aprobadas | [DSP](Validation/dsp-0.2.txt) |
| DSP con ASan + UBSan | Aprobado, sin errores reportados | [Sanitizers](Validation/asan-0.2.txt) |
| DSP x86_64 mediante Rosetta | 489,992 aserciones aprobadas | [Rosetta](Validation/rosetta-0.2.txt) |
| Integración, estado, presets y teclado | Aprobado; conteo variable por prueba concurrente | [Integración](Validation/integration-0.2.txt) |
| Bundles reales, voz por sidechain, acordes MIDI y modo Voz | Aprobado a 44.1/48/88.2/96 kHz | [Host de prueba](Validation/host-wrappers-0.2.txt) |
| AU normal, auval | AU VALIDATION SUCCEEDED, versión 0.2.0 | [AU](Validation/au-effect-0.2.txt) |
| AU MIDI, auval | AU VALIDATION SUCCEEDED, una entrada de voz | [AU MIDI](Validation/au-midi-0.2.txt) |
| VST3, pluginval nivel 10 | SUCCESS | [VST3](Validation/vst3-pluginval-0.2.txt) |
| AU normal, pluginval nivel 10 | SUCCESS | [AU intensivo](Validation/au-effect-pluginval-0.2.txt) |
| AU MIDI, pluginval nivel 10 | SUCCESS | [AU MIDI intensivo](Validation/au-midi-pluginval-0.2.txt) |
| 100 presets en modo Voz, render offline | Salidas finitas y limitadas | [Mediciones](Validation/original-voice-0.2.csv) |

pluginval se ejecutó con semilla `0x52564f43`, cuatro frecuencias y bloques de
32/64/128/256/512/1024/2048 muestras. El host independiente comprobó los acordes MIDI
en VST3 y AU MIDI; el AU de efecto normal no recibe MIDI mediante ese host, como indica
`acceptsMidi=0` en el registro. Su audio y modo Voz sí se comprobaron.

La prueba de teclado compara píxeles renderizados antes y después de recibir una nota,
verifica liberación y canales simultáneos, y comprueba que no se creen notas duplicadas.
Incluye MIDI en bloques sin muestras, all-notes-off y limpieza al reiniciar. La captura
de regresión inicialmente detectó un buffer del propio test con menos canales que los
negociados; se corrigió el test y se añadió una protección del procesador y su regresión
para responder con silencio ante ese contrato inválido, conservando los eventos MIDI.

La prueba de Voz compara dos motores con distintos osciladores, notas, unísono,
Carrier mix y compensación de presets: la salida vocal es idéntica. También verifica
la corrección espectral neutra, el efecto de Formant, el dry de sidechain y la latencia
de 16 muestras. Esto no implementa ni afirma transposición vocal mediante MIDI.

Los renders de voz usaron la misma frase sintetizada localmente que la primera entrega.
En modo Voz, los efectos dan RMS de −19.37 a −8.73 dBFS y el pico máximo fue −0.204 dBFS.
No hay normalización automática entre estos sonidos; los efectos más agresivos pueden
cambiar su nivel. Son mediciones offline, no una evaluación auditiva con voces cantadas.

Se revisaron las capturas nativas a escala Retina, incluyendo el tamaño mínimo:
[modo Voz](Preview-0.2.png), [notas MIDI iluminadas](MIDI-0.2.png).

## Instalación y comprobación en tu sesión

La versión 0.2.0 se instaló en las mismas rutas e identificadores de la versión anterior.
Guarda tu proyecto, cierra Logic completamente y vuelve a abrirlo. La esquina inferior
izquierda del plugin debe mostrar **0.2.0**. Una ventana que seguía abierta durante la
instalación conserva el código anterior hasta que el DAW vuelva a cargar el plugin.

En R-Vocoder MIDI selecciona la pista vocal en Side Chain y pulsa Voz para procesar su
audio original. Para el vocoder con acordes, pulsa Synth, elige MIDI y toca. En el AU de
Audio FX o VST3 puedes alternar Pista y Sidechain desde los nuevos botones.

Esta actualización se validó automáticamente con los bundles instalados. No se volvió
a controlar tu sesión de Logic mediante herramientas de ventanas; la comprobación
musical final dentro de esa sesión requiere cargar la versión nueva. Continúan los
límites de certificación de Intel físico, otros sistemas y distribución comercial
descritos en la [validación inicial](VALIDATION.md).

Los IDs antiguos se mantienen y `voiceMode` se añade al final con valor predeterminado
cero. Los sonidos guardados antes de este modo siguen abriendo como vocoder. La edición
MIDI aplica su entrada correcta aunque el estado antiguo contenga otra combinación de
routing. El estado del DAW conserva la selección nueva.

## Repetir las pruebas y escuchar ejemplos

```sh
bash Scripts/build.sh
bash Scripts/install-macos.sh
bash Scripts/validate-macos.sh
./build/rv_plugin_tests_artefacts/Release/rv_plugin_tests --screenshots
./build/rv_plugin_tests_artefacts/Release/rv_plugin_tests --render-voice /ruta/absoluta/voz.wav /ruta/absoluta/renders
```

Las nuevas ejecuciones se escriben en `build/validation/`. Los registros de `Docs/Validation/`
se conservan como evidencia de esta entrega. El paquete incluye la voz sin procesar y
diez ejemplos de Voz original en `Demos/`.
