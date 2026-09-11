# Arquitectura de R-Vocoder

## Elección de plataforma

C++20 + CMake + JUCE 8.0.12, fijado al commit `29396c22c93392d6738e021b83196283d6e4d850`.
JUCE adapta el procesador a AU, VST3 y Standalone; el DSP original no depende de JUCE y
puede probarse con un compilador C++20. Se enlaza `juce_dsp` para la infraestructura
de futuras extensiones; el motor actual implementa sus filtros, sintetizador y FIR propios.
La GUI usa dibujo vectorial nativo de JUCE, sin navegador, cuentas, servidor ni conexión.

## Recorrido del audio

1. Entradas principal y auxiliar mono/estéreo, con ganancia común de entrada.
2. Selección y transición entre fuentes: voz principal/carrier interno; voz principal/carrier
   auxiliar; voz auxiliar/carrier principal; voz auxiliar/carrier interno.
3. Gate gobernado por la voz: detector rápido, umbrales de apertura/cierre separados,
   hold y ganancia suavizada. Range determina la atenuación al cerrar.
4. Banco logarítmico de 8/12/16/24/32/48/64 bandas. Cada banda tiene dos filtros
   pasa-banda de segundo orden en cascada para análisis y para cada canal de síntesis.
   Los filtros de estado variable usan integración trapezoidal. La Q depende de la
   separación entre bandas y Formant focus. La frecuencia superior se limita según Nyquist.
5. Rectificación y seguidores con attack/release independientes. Definition comprime
   moderadamente las envolventes débiles; Clarity también contribuye a esta función.
6. Transferencia interpolada de envolventes. Formant aplica un desplazamiento logarítmico
   en semitonos; Band shift desplaza índices. Size e Identity modifican esa transformación;
   Nasal y Throat modelan zonas espectrales distintas. No hay pitch shifting de la grabación.
7. Consonantes: combinación heurística de energía por encima de 3 kHz y cruces por cero.
   Sibilance/Unvoiced conservan esa señal de voz; Breath añade la franja por encima de 7 kHz.
   Es un detector de componentes no sonoros, no reconocimiento de fonemas ni inteligencia artificial.
8. Mezclas independientes de carrier y modulator, cantidad de transferencia espectral,
   panorama alterno por bandas, anchura mid/side y movimiento de panorama.
9. Curvas Low/Body/Mid/Presence/Air; altavoz con filtrado, color vintage, ruido gobernado
   por la envolvente, reducción deliberada de resolución y frecuencia.
10. Saturación suave y distorsión, interpoladas entre dos estilos, procesadas a 2x con
    interpolación y decimación FIR. Protección suave de salida y mezcla equal-power.

## Sintetizador

12 voces MIDI, 7 osciladores de unísono por voz como máximo, ADSR, pitch bend de ±2
semitonos, CC64 sustain, CC120 all-sound-off y CC123 all-notes-off. Drone tiene 1–3 notas
con acordes Single/Fifth/Minor/Major/Octaves. Saw, square y pulse emplean PolyBLEP;
triangle usa una aproximación aditiva limitada según Nyquist. Sine y Noise completan
las formas de onda. Supersaw fuerza al menos tres osciladores. Oscillator mix mezcla
la onda elegida con una senoide de la misma nota. Pulse elimina su componente DC.

Glide suaviza cambios de tono, con una transición mínima breve contra clicks. Al cambiar
de forma de onda se cruzan ambas durante 10 ms. Las fases se reinician de forma
determinista al preparar una instancia; no se serializa la fase de una nota sostenida.

## Tiempo real y latencia

El callback usa estructuras de tamaño fijo, parámetros atómicos y una cola MIDI de
capacidad fija para el teclado de pantalla. No abre archivos, crea presets, usa mutex
ni reserva memoria. La GUI consulta medidores atómicos a 25 Hz.

Los controles continuos usan suavizado de 12 ms. La configuración de filtros se calcula
a una frecuencia de control de 1/64 del sample rate. Los cambios de bandas, rango o Q
cruzan dos bancos durante 35 ms. Las actualizaciones adicionales esperan a que termine
la transición vigente, evitando tormentas de reconstrucción durante automatización.

Dos FIR de 33 taps trabajan al doble de frecuencia. Retardo total:
`(16 + 16) / 2 = 16 muestras`, reportado al host. Se retarda el dry 16 muestras también.
Esto equivale a 0.363 ms a 44.1 kHz y 0.167 ms a 96 kHz. Los filtros de análisis y
las envolventes tienen respuesta temporal propia; no se presentan como latencia de transporte.
El bypass mantiene la compensación temporal y el motor caliente.

El sobremuestreo reduce aliasing de la saturación; el bitcrusher conserva aliasing por
diseño. No equivale a un modo de mastering de sobremuestreo elevado.

## Estado y compatibilidad

`Source/Parameters.h` es el registro permanente de IDs, rangos y orden de opciones.
Agregar parámetros al final; no reutilizar IDs ni reordenar opciones existentes.
APVTS guarda parámetros, metadatos del preset, programa y versión de esquema.
Se rechazan estados de una versión futura desconocida y datos excesivos. Se limitan
valores inválidos antes de instalarlos. Una secuencia atómica evita que el callback
adopte un preset a medio cargar.

La persistencia de presets y favoritos ocurre fuera del callback. Los presets personales
son JSON con IDs estables y nombres de archivo UUID, sin rutas derivadas del nombre visible.
Cambiar de preset conserva routing, modo Drone/MIDI, ganancias y bypass del proyecto.

## Fuentes técnicas consultadas

- [JUCE: buses y disposiciones de canales](https://juce.com/tutorials/tutorial_audio_bus_layouts/).
- [Julius O. Smith: análisis de vocoder](https://www.dsprelated.com/freebooks/filters/Vocoder_Analysis.html).
- [Julius O. Smith: parámetros de vocoder](https://www.dsprelated.com/freebooks/sasp/Computing_Vocoder_Parameters.html).
- [Cytomic: derivaciones de filtros de estado variable](https://cytomic.com/technical-papers/).
- [Apple: fuente sidechain e instrumento controlado por MIDI](https://support.apple.com/en-mide/guide/logicpro/lgsifc36886a/10.7/mac/11.0).
- [Ableton: sidechain en plugins externos](https://help.ableton.com/hc/en-us/articles/209775325-Sidechaining-a-third-party-plug-in).

Se consultaron conceptos y APIs; no se incorporaron algoritmos ni presets propietarios.
