# R-Vocoder 0.3.0 · Beta para macOS

**[Descarga y ejemplos](https://raulg0mez.github.io/vocoder/)**

Una vista para tocar: teclado siempre visible, ocho macros, entradas claras y ajustes
al lado. El gráfico permite mover Formant/Character o los límites del espectro.

## Lo nuevo

- **Con teclas + Cortar al soltar:** toda la salida responde al MIDI, incluida la
  mezcla de voz original, las consonantes y los efectos. Soporta acordes y sustain.
- **Solo efecto:** evita que los presets vuelvan a añadir voz limpia a la mezcla.
- **Key release:** cierre suave de 5 a 600 ms, con 70 ms por defecto.
- Los ajustes detallados ya no esconden el teclado ni los controles principales.
- El gráfico controla parámetros reales que puedes automatizar. Doble clic reinicia;
  las flechas permiten ajustes precisos.
- Los medidores dejan de mostrar actividad vieja al detenerse el procesamiento.

En canciones anteriores, activa Con teclas, Cortar al soltar y Solo efecto para usar
el comportamiento nuevo. Se conservan los ajustes anteriores al restaurar una canción.
Si la voz original se oye por otra pista que sale al master, ajusta también esa ruta
en tu DAW; el plugin controla únicamente su propia salida.

## Instalar

Descarga **R-Vocoder-0.3.0-macOS.zip**, descomprímelo y abre **Install.command** con doble
clic. Instala AU, AU MIDI para Logic y VST3 en tu carpeta de usuario, sin escribir
comandos. Incluye también la aplicación Standalone, documentación y ejemplos.

Guarda tu canción y cierra completamente Logic/Ableton antes de abrirlos de nuevo
para cargar la versión nueva. Binarios universales Apple Silicon + Intel; macOS 11+.

Esta beta tiene firma local y **no está notarizada por Apple**. Si macOS bloquea la
apertura, consulta la sección de instalación en la web; no desactives la protección global.

## Comprobaciones

DSP probado con ASan/UBSan y mediante Rosetta; integración de MIDI, estados, presets e
interfaz; host de prueba con los plugins reales; auval y pluginval nivel 10.
[Resultados y límites](https://github.com/RAULG0MEZ/vocoder/blob/main/Docs/RELEASE-0.3.md).
Continúa pendiente completar el recorrido y audición dentro de Logic/Ableton.

Para verificar el ZIP, descarga también SHA256SUMS.txt en la misma carpeta y ejecuta:

```sh
shasum -a 256 -c SHA256SUMS.txt
```
