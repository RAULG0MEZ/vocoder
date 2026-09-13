# Web de descarga

URL pública: https://raulg0mez.github.io/vocoder/

La web vive en `website/` y usa Vite y TypeScript. GitHub Pages publica únicamente
`website/dist`, mediante `.github/workflows/pages.yml`. No hay servidor, variables
de entorno obligatorias, credenciales en el proyecto ni servicios de pago.

## Trabajo local

Con Node.js 22.19 o posterior de la rama 22:

```sh
cd website
npm ci
npm run dev
```

Vite sirve la web en `/vocoder/`, la misma ruta de GitHub Pages. Para comprobar
el resultado de producción:

```sh
npm run build
npx playwright install chromium
npm test
```

Las pruebas comprueban reproducción y cambio de audio, recuperación ante un archivo
que no carga, descarga sin JavaScript, imágenes y el diseño a 320, 390, 768 y 1440 px.
La vista previa de producción usa el puerto 4174 durante las pruebas.

## Publicación automática

Cada cambio a `website/` o al workflow integrado en `main` compila la página, ejecuta
las pruebas en Chromium y despliega con las acciones oficiales de Pages. Un pull
request ejecuta la misma validación, sin publicar. También puede ejecutarse desde
Actions → Publish website → Run workflow.

El repositorio debe tener Settings → Pages → Source: GitHub Actions. El workflow
usa el token temporal de GitHub con lectura de contenido para compilar y permisos
`pages: write` / `id-token: write` únicamente en el trabajo de despliegue. No hace
falta crear un token personal ni un secret del repositorio.

## Descarga del plugin

El botón apunta al archivo de una release explícita, no al código fuente ni a un
artefacto de Actions que caduque o requiera iniciar sesión:

https://github.com/RAULG0MEZ/vocoder/releases/download/v0.4.0/R-Vocoder-0.4.0-macOS.zip

La release incluye `SHA256SUMS.txt` para verificar el ZIP. El paquete 0.4.0 es el
binario universal validado con racks simultáneos, presets compactos, medidores circulares y materiales integrados.
Conserva firma ad hoc y sigue sin notarización de Apple.
Los binarios se alojan en Releases y no se añaden al historial de Git.

Para una versión nueva:

1. Compilar, validar y empaquetar el plugin siguiendo las instrucciones del proyecto.
2. Crear la release y subir su ZIP y checksum. Comprobar la descarga pública.
3. Actualizar versión, tamaño y enlaces en `website/index.html`, `README.md` y la
   prueba de descarga en `website/tests/site.spec.ts`.
4. Integrar en `main`; GitHub Actions publica la página una vez aprobadas las pruebas.

No volver a empaquetar ni reemplazar silenciosamente una versión publicada. Si cambia
el binario, asignar una versión nueva para conservar enlaces y checksums reproducibles.

## Contenido y alcance

- Las capturas provienen de la interfaz real de la versión 0.4.0.
- Los audios son MP3 a 192 kb/s de la frase sintética de validación y sus renders
  French Chrome. Se ofrecen la referencia, Synth y Voz original. Las formas de onda
  se calculan a partir de los archivos, no representan procesamiento en el navegador.
- La tipografía se sirve localmente e incluye su licencia. No hay cookies ni analítica.
- Se presenta como beta para macOS, con advertencia de firma local/no notarización,
  los formatos reales y los límites de validación. No se anuncia una versión de Windows.
