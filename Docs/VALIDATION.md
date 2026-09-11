# Validación de R-Vocoder 0.1.0

Este documento conserva las pruebas de la versión inicial. Para los cambios actuales,
consulta [R-Vocoder 0.2.0](RELEASE-0.2.md).

Registro de la compilación local comprobada el **11 de septiembre de 2026 (UTC)**.
El plugin está implementado, compilado e instalado. Esta entrega es una versión inicial
para evaluación musical: la validación automática está aprobada, pero la aceptación
auditiva y los recorridos completos dentro de Logic y Ableton siguen pendientes.

## Entorno

- macOS 26.6 (25G72), Apple Silicon; Apple Clang 21, Command Line Tools.
- JUCE 8.0.12, commit `29396c22c93392d6738e021b83196283d6e4d850`.
- CMake 4.4.3, Ninja 1.13.2, C++20, Release.
- AU efecto `aufx/Rvoc/Rstk`; AU MIDI `aumf/Rvmi/Rstk`; VST3 y Standalone.
- Los cuatro bundles contienen `arm64` y `x86_64`; mínimo configurado: macOS 11.
- pluginval 1.0.4; semilla `0x52564f43`, nivel 10.

## Resultados ejecutados

| Prueba | Resultado | Evidencia |
|---|---|---|
| DSP nativo, silencio, gate, filtros, envolventes, MIDI, mezcla, NaN/Inf, modulación y estrés | 449,894 comprobaciones aprobadas | [DSP](Validation/dsp.txt), `Tests/DSPTests.cpp` |
| DSP x86_64 mediante Rosetta | 449,894 comprobaciones aprobadas | [Rosetta](Validation/dsp-rosetta.txt) |
| AddressSanitizer + UndefinedBehaviorSanitizer, DSP Debug | Aprobado, sin errores reportados | [Sanitizers](Validation/asan.txt) |
| Integración JUCE, presets, layouts, persistencia, concurrencia y callback | 311,665 comprobaciones en el registro final | [Integración](Validation/integration.txt), `Tests/ProcessorChecks.h` |
| Audio Unit efecto, Apple auval | AU VALIDATION SUCCEEDED | [AU efecto](Validation/au-effect-final.txt) |
| Audio Unit MIDI, Apple auval | AU VALIDATION SUCCEEDED | [AU MIDI](Validation/au-midi-final.txt) |
| VST3, pluginval nivel 10 | SUCCESS | [VST3](Validation/vst3-final.txt) |
| AU efecto, pluginval nivel 10 | SUCCESS | [AU efecto intensivo](Validation/au-effect-pluginval.txt) |
| AU MIDI, pluginval nivel 10 | SUCCESS | [AU MIDI intensivo](Validation/au-midi-pluginval.txt) |
| Bundles reales cargados por un host de prueba independiente | Latencia reportada y dry medidos: 16 muestras; sidechain produce audio | [Host](Validation/host-wrappers.txt), `Tests/HostTests.cpp` |
| 100 presets, render offline con la misma voz de referencia | 100 sonidos distintos por parámetros; salidas finitas y acotadas | [Mediciones](Validation/preset-measurements.csv) |
| GUI nativa, tamaño predeterminado y captura Retina 2x | Revisada visualmente; unidades y solapamientos corregidos | [Captura](Preview.png) |

La cantidad de comprobaciones de integración varía ligeramente con la duración de la
prueba concurrente. Es un conteo de aserciones, muchas por muestra, no un conteo de
cientos de miles de escenarios independientes.

Las pruebas de integración y pluginval cubren **44.1, 48, 88.2 y 96 kHz** y bloques
**32, 64, 128, 256, 512, 1024 y 2048**. Se verifican mono → mono, mono → estéreo,
estéreo → estéreo y auxiliar desactivado/mono/estéreo. Estéreo → mono y surround se
rechazan de forma controlada. También se comprueba que el procesador no sobrescriba
las entradas auxiliares y que intercambiar los papeles de las fuentes conserve el resultado.

pluginval incluye carga en frío/caliente, editor, editor durante reproducción,
automatización, cambio de programas, restauración de estado en segundo plano,
preparaciones repetidas, disposiciones de buses y fuzzing de parámetros.
La prueba propia intercepta `operator new` de C++ durante callbacks normales y durante
restauración concurrente: no detectó reservas. No es un perfilador universal de todas
las APIs del sistema ni un informe de ThreadSanitizer.

## Latencia y avisos de los validadores

El audio procesado tiene **16 muestras de retardo de transporte**, y el dry tiene el
mismo retardo. El host de prueba carga cada bundle real, activa su sidechain, prepara
audio a las cuatro frecuencias y comprueba tanto la propiedad de latencia como un
impulso dry. Las doce combinaciones entregan exactamente 16 muestras.

El apartado inicial «Plugin info» de pluginval imprime `Reported latency: 0` antes de
preparar el audio. Es el valor todavía no actualizado de la instancia anfitriona de
JUCE: al preparar, consulta al plugin y obtiene 16. La prueba de wrappers reproduce
y registra ambos valores. La derivación de los FIR se explica en [Arquitectura](ARCHITECTURE.md).

auval emite un aviso informativo para el AU `aufx` porque expone entrada MIDI y Apple
recomienda `aumf` para ese uso. La validación termina aprobada. Logic dispone de la
edición separada **R-Vocoder MIDI**, de tipo `aumf`, para tocar el sintetizador desde
una pista de instrumento; esa edición pasa sin este aviso.

## Mediciones de sonido y rendimiento

El banco de 100 presets se renderizó con una frase española generada localmente por
la voz Monica de macOS, a 48 kHz. Se ajustó una ganancia fija por preset; no hay un
control automático de volumen persiguiendo cada grabación. RMS medido: de **−21.54 a
−20.96 dBFS**; pico máximo: **−0.0062 dBFS**. No son medidas LUFS ni intersample true-peak.
Las mediciones comprueban niveles; no sustituyen escuchar voces cantadas y diferentes intérpretes.

Los presets más anchos pueden tener correlación negativa (mínimo observado: −0.286).
El medidor permite detectarlo y reducir Width/Spread. Se conserva el centro en la
transformación mid/side, pero no se promete idéntico timbre o volumen al sumar a mono.

El microbenchmark de 64 bandas a 48 kHz consumió **2.41% de un núcleo** en esta ejecución
Release nativa; el mismo ensayo x86_64 bajo Rosetta dio 4.21%. Es un ensayo DSP de
referencia, no el máximo con doce notas y siete osciladores por nota, ni una garantía
del medidor de CPU de un DAW. La cifra de CPU del registro ASan corresponde a Debug
instrumentado y no se utiliza como medida del producto.

El paquete local incluye diez WAV de ejemplo y la voz original en `Demos/`. Son renders
de evaluación generados con la misma fuente, sin posprocesamiento externo; no un banco
de interpretaciones profesionales ni una prueba de calidad musical ya aceptada.

## Comprobaciones pendientes antes de declarar producción

- **Logic Pro 12.3:** se confirmó el registro y la aparición de R-Vocoder MIDI en el
  menú del host. No se completó la reproducción, selección de sidechain, interpretación
  MIDI y reapertura de un proyecto con audio. Las llamadas de control de ventanas se
  bloquearon durante periodos prolongados. El proyecto separado de prueba está en
  `build/validation/R-Vocoder Host Test.logicx`.
- **Ableton Live 12 Suite:** instalado, pero no se pudo completar la sesión de prueba
  por el mismo bloqueo. La equivalencia del motor AU/VST3 está comprobada en el host
  de prueba; la experiencia dentro de Live permanece sin verificar.
- **Audición musical:** probar voces reales habladas/cantadas, micrófonos ruidosos,
  acordes sostenidos, cambios de preset en reproducción y mezclas completas.
- **Intel físico y otros sistemas:** el DSP x86_64 pasó bajo Rosetta. No se ejecutó en
  un Mac Intel físico, macOS 11 ni Windows. La aplicación Standalone está compilada;
  su recorrido con dispositivos físicos no forma parte de esta aceptación.
- **Distribución comercial:** resolver la licencia JUCE aplicable, firma con Developer ID
  y notarización. Los bundles entregados tienen firma ad hoc local.

Para cerrar la aceptación en cada DAW: reproducir una voz con carrier externo y luego
interno, intercambiar fuentes de sidechain, tocar un acorde MIDI, automatizar los
macros, cambiar varios presets, guardar el proyecto, cerrarlo y reabrirlo. Comparar
el audio renderizado antes/después y comprobar un bounce offline y reproducción en mono.

## Repetir las pruebas

Con los bundles compilados e instalados:

```sh
bash Scripts/validate-macos.sh
```

El script usa `.deps/pluginval/pluginval.app/Contents/MacOS/pluginval`; se puede indicar
otra instalación mediante la opción de desarrollo `RV_PLUGINVAL`. No es una variable
necesaria para arrancar ni usar R-Vocoder. Descarga oficial:
[Tracktion pluginval](https://github.com/Tracktion/pluginval/releases/tag/v1.0.4).

DSP instrumentado, sin JUCE:

```sh
cmake -S . -B build-asan -DRV_BUILD_PLUGINS=OFF -DRV_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-asan
./build-asan/rv_dsp_tests
arch -x86_64 ./build/rv_dsp_tests
```

Renders reproducibles de una voz propia:

```sh
./build/rv_plugin_tests_artefacts/Release/rv_plugin_tests --render /ruta/absoluta/voz.wav /ruta/absoluta/renders
```

Los archivos de `Validation/` son evidencia conservada de esta entrega; los scripts
escriben las ejecuciones nuevas en `build/validation/` sin reemplazar ese registro.
