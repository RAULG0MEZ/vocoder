# R-Vocoder 0.4.0 — cuatro racks, una ventana

## Qué cambia al usarlo

- Teclado MIDI, selección de entrada, modo, ocho macros y ganancias siempre visibles.
- Cuatro racks juntos: VOZ, SYNTH, FX y MOTION. Cada uno cambia su grupo de ajustes
  mediante un selector compacto, sin desplazar paneles.
- Presets arriba, agrupados por categoría, con flechas anterior/siguiente y estrella
  de favoritos. El menú … reúne guardar, borrar, favoritos, azar y reinicio.
- Anillos de nivel alrededor de Entrada y Salida, con caída suave y retención de picos.
  La raya de la perilla indica ganancia; el anillo indica audio. Voz y Carrier tienen
  sus propios medidores circulares. Mono permanece junto al gráfico.
- Cinco materiales independientes: chasis, cuerpo de perilla, botón, rack y cristal,
  generados con GPT Image 2.5 Sunburst e incluidos dentro del plugin. Las indicaciones
  y curvas siguen al sonido real. [Assets y prompts](Design/ASSET-PROMPTS-0.4.md).
- Ventana inicial de 1320 × 840, mínimo 1080 × 760 y máximo 1800 × 1100. Los valores
  se pueden escribir; doble clic en una perilla recupera su valor inicial.

El gráfico XY sigue modificando Formant y Character; los bordes del Espectro cambian
el rango del banco de filtros. El teclado muestra las notas entrantes. Los controles
que no participan en el modo elegido aparecen atenuados.

## Sonido y compatibilidad

El motor de audio, sus identificadores de automatización, rangos, valores iniciales,
rutas, presets y formato de estado no cambian respecto a 0.3.0. Tampoco cambia la
latencia de 16 muestras. Los proyectos del DAW conservan su sonido.

Cortar al soltar y Solo efecto siguen presentes encima del teclado. Para sonar solo
con MIDI, elige Con teclas y activa ambos. Key release está ahora en VOZ → Gate de voz;
las mezclas avanzadas están en FX → Mezcla. Entrada y Salida permanecen arriba a la derecha.

Las sesiones anteriores a 0.3 conservan sus interruptores apagados hasta que el usuario
los active. El plugin solo puede cortar su propia salida: una pista que también llegue
al master por otra ruta continuará oyéndose por esa ruta.

## Validación de esta compilación

| Comprobación | Registro |
|---|---|
| DSP y cierre por MIDI, sustain, canales y mezcla | [DSP](Validation/dsp-0.4.txt) |
| Memoria y comportamiento indefinido del DSP | [ASan/UBSan](Validation/asan-0.4.txt) |
| DSP x86_64 mediante Rosetta | [Rosetta](Validation/rosetta-0.4.txt) |
| Presets, estado, automatización, UI y notas en posición de muestra | [Integración](Validation/integration-0.4.txt) |
| Audio y corte entre notas en los formatos reales | [Host](Validation/host-wrappers-0.4.txt) |
| AU y AU MIDI | [auval](Validation/au-effect-0.4.txt), [MIDI](Validation/au-midi-0.4.txt) |
| pluginval nivel 10 | [VST3](Validation/vst3-pluginval-0.4.txt), [AU](Validation/au-effect-pluginval-0.4.txt), [AU MIDI](Validation/au-midi-pluginval-0.4.txt) |
| Web: audio, imágenes, descarga sin JavaScript y tamaños móviles | [Playwright](Validation/website-0.4.txt) |

Las pruebas de interfaz comprueban que cada parámetro tenga un único control, que las
cinco imágenes estén integradas, que no existan Viewports ni ListBoxes y que todos
los grupos de los cuatro racks quepan a tres tamaños. Comprueban también flechas de
presets, selección por categoría y arrastre del gráfico sobre parámetros reales.
Se revisaron capturas Retina de la ventana normal y mínima, MIDI, Voz y Espectro.

Compilación universal arm64 + x86_64: AU, AU MIDI, VST3 y Standalone. pluginval prueba
44.1, 48, 88.2 y 96 kHz con bloques entre 32 y 2048 muestras. Los ejemplos de audio
se conservan de 0.3 porque esta versión no cambia el procesamiento.

Continúa como beta: estas validaciones no sustituyen una audición y recorrido
completos dentro de Logic/Ableton. No se declara prueba física en Intel ni un binario
Windows. La firma es ad hoc; los plugins y el instalador no están notarizados por Apple.

## Instalar

Descomprime el ZIP, ejecuta Install.command y vuelve a abrir tu DAW. Instala únicamente
los plugins R-Vocoder en las carpetas de tu usuario, sin pedir administrador.
La aplicación independiente y la documentación también vienen en el paquete.
El plugin funciona sin cuentas, internet ni claves API.
