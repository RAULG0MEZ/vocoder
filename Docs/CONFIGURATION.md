# Configuración y datos

No hay credenciales, secretos, variables obligatorias, cuentas ni base de datos.
El plugin arranca después de instalar el bundle AU/VST3.

| Dato | Dónde se guarda |
|---|---|
| Sonido, routing, síntesis, movimiento, mezcla y preset activo | Estado del plugin dentro del proyecto del DAW |
| Presets de fábrica | Compilados desde `Source/Presets/FactoryPresets.h` |
| Presets personales en macOS | `~/Library/Application Support/RSTK/R-Vocoder/*.rvpreset` |
| Favoritos en macOS | `~/Library/Application Support/RSTK/R-Vocoder/favorites.txt` |
| Dependencias de desarrollo | `.deps/` y `.venv/`, excluidas de Git |
| Compilaciones y resultados de pruebas | `build*/`, excluidos de Git |

El software no envía telemetría ni sube audio. Las rutas de datos en Windows utilizan
el directorio de aplicación del usuario que proporciona JUCE. La distribución firmada
para terceros requeriría certificados del desarrollador fuera del repositorio; este
proyecto no contiene ninguno.
