# R-Vocoder 0.2.0 · Beta para macOS

Vocoder creativo con **100 presets originales**, procesamiento de voz sin osciladores,
sintetizador MIDI y hasta 64 bandas.

**[Web y ejemplos de audio](https://raulg0mez.github.io/vocoder/)**

## Descarga e instalación

Descarga **R-Vocoder-0.2.0-macOS.zip** de los archivos de esta release.
Los ZIP/TAR marcados como “Source code” contienen el código, no el plugin instalado.

1. Descomprime el ZIP y ejecuta `Install.command`.
2. Vuelve a abrir tu DAW para que detecte los plugins.
3. En una pista de audio elige R-Vocoder. Para tocar con MIDI en Logic, carga
   R-Vocoder MIDI en una pista de instrumento y selecciona la voz en Side Chain.

Incluye **AU, AU MIDI para Logic, VST3 y Standalone**, más documentación y ejemplos.
Binarios universales para Apple Silicon e Intel; macOS 11 o posterior. No requiere
cuenta ni conexión a internet para usar el plugin. No se incluye versión de Windows.

## Lo nuevo

- **Pista / Sidechain** elige de dónde llega la voz; el recorrido interno se ajusta solo.
- **Voz** transforma la grabación original conservando sus notas, sin sintetizador.
- **Synth** conserva el vocoder clásico, con Drone o acordes MIDI; **Externo** permite
  otra fuente sonora en el AU de efecto y VST3.
- El teclado de pantalla muestra las notas MIDI recibidas.
- Cambiar de preset conserva las conexiones y el modo elegido.

## Estado de esta beta

AU y AU MIDI superaron auval; los tres plugins superaron pluginval nivel 10. El motor,
estado y conexiones se comprobaron con pruebas automatizadas y un host de prueba.
Queda pendiente completar la audición y recorrido real en Logic/Ableton.

Esta beta tiene **firma ad hoc y no está notarizada por Apple**. macOS puede bloquear
su apertura. Si confías en su origen, después de intentar abrirla revisa
Ajustes del Sistema → Privacidad y seguridad → Abrir igualmente. Consulta la
[guía de Apple](https://support.apple.com/es-mx/102445); no desactives la protección global.

[Guía del plugin](https://github.com/RAULG0MEZ/vocoder/blob/main/README.md) ·
[Validación y límites](https://github.com/RAULG0MEZ/vocoder/blob/main/Docs/RELEASE-0.2.md) ·
[Avisos de terceros](https://github.com/RAULG0MEZ/vocoder/blob/main/THIRD_PARTY_NOTICES.md)

## Verificar la descarga

Descarga también `SHA256SUMS.txt`, colócalo junto al ZIP y ejecuta:

```sh
shasum -a 256 -c SHA256SUMS.txt
```

SHA-256 del ZIP:
`6fa64cf5699d0500ac72119ca697ed12525577571dfeeb660d3b9570c34184e2`
