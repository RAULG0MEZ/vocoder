# Materiales de R-Vocoder 0.4

Se generaron cinco piezas independientes con **GPT Image 2.5 Sunburst**, modelo
`gpt-image-2.5-sunburst`, mediante el modo CLI/API del skill de imágenes de Codex.
Calidad alta, sin imágenes de entrada. Los prompts completos y los parámetros de
generación están en [prompts.jsonl](../../Assets/Hardware/prompts.jsonl).

Los PNG originales se integran en AU, AU MIDI, VST3 y Standalone mediante JUCE
BinaryData. El chasis, las perillas, los botones, los racks y el cristal son
componentes separados. JUCE recorta la silueta circular y dibuja encima los
indicadores, textos y medidores en tiempo real; la luz del material permanece fija
y el indicador sigue al parámetro. Los niveles y curvas provienen del motor de audio.
No hay rótulos ni valores pintados dentro de las imágenes.

| Pieza | Archivo | Tamaño | SHA-256 |
|---|---|---|---|
| chassis | [chassis.png](../../Assets/Hardware/chassis.png) | 1536x1024 | `d1ec72967394d91cc1ad0c4b0fe726d00385bfd24c7bdfeb2f4e5e9f55c30740` |
| knob | [knob.png](../../Assets/Hardware/knob.png) | 1024x1024 | `e2b6dbdf0c98bc029477a308c5308076ddef8b59bf99d5b7b026940c1d484bd1` |
| button | [button.png](../../Assets/Hardware/button.png) | 1536x1024 | `a14397f960626da0c5cf9fbb3f78e1b73948af443f25ceffe9a7b2d3ed1e184a` |
| rack | [rack.png](../../Assets/Hardware/rack.png) | 1024x1536 | `cdff4cf84486d77f808dc9a8b924263efb6fcec709940dd12eb595149d931615` |
| glass | [glass.png](../../Assets/Hardware/glass.png) | 1536x1024 | `0a0708e254c7816a25ba79514f3b25ee38216a6358d52267d72ef1c377fda145` |

La generación usa una clave privada de desarrollo; el plugin funciona sin esa clave,
sin cuentas y sin internet. No se incluyen secretos en los assets ni en los prompts.
No se usan materiales propietarios de VocalSynth. Los archivos de exploración
anteriores no forman parte de esta versión.
