import "@fontsource-variable/space-grotesk";
import "./style.css";

type Demo = "original" | "synth" | "voice";
const demos: Record<Demo, { label: string; caption: string }> = {
  original: {
    label: "voz sin procesar",
    caption: "La grabación de referencia, sin efectos.",
  },
  synth: {
    label: "vocoder con sintetizador",
    caption:
      "French Chrome · La voz da las palabras; el sintetizador aporta las notas y el sonido robótico.",
  },
  voice: {
    label: "modo Voz original",
    caption:
      "French Chrome · Cambia el timbre de la grabación, conserva su melodía y no añade osciladores.",
  },
};

const audio = document.querySelector<HTMLAudioElement>("#demo-audio")!;
const caption = document.querySelector<HTMLParagraphElement>("#demo-caption")!;
const error = document.querySelector<HTMLParagraphElement>("#audio-error")!;
const fileLink = document.querySelector<HTMLAnchorElement>("#audio-file")!;
const canvas = document.querySelector<HTMLCanvasElement>("#waveform")!;
const context = canvas.getContext("2d");
let selected: Demo = "original";
let waveforms: Partial<Record<Demo, number[]>> = {};
let revision = 0;

function drawWaveform() {
  if (!context) return;
  const width = canvas.clientWidth;
  const height = canvas.clientHeight;
  const ratio = window.devicePixelRatio || 1;
  canvas.width = Math.round(width * ratio);
  canvas.height = Math.round(height * ratio);
  context.scale(ratio, ratio);
  const peaks = waveforms[selected];
  if (!peaks) return;
  const step = width / peaks.length;
  const progress =
    Number.isFinite(audio.duration) && audio.duration > 0
      ? audio.currentTime / audio.duration
      : 0;
  for (const [index, peak] of peaks.entries()) {
    const barHeight = Math.max(3, peak * height * 0.85);
    context.fillStyle = index / peaks.length < progress ? "#d9f28a" : "#596348";
    context.fillRect(
      index * step,
      (height - barHeight) / 2,
      Math.max(1, step - 2),
      barHeight,
    );
  }
}

for (const input of document.querySelectorAll<HTMLInputElement>(
  'input[name="demo"]',
)) {
  input.addEventListener("change", () => {
    const next = input.value as Demo;
    if (!(next in demos)) return;
    const wasPlaying = !audio.paused && !audio.ended;
    const position = audio.currentTime;
    const change = ++revision;
    audio.pause();
    selected = next;
    audio.src = `${import.meta.env.BASE_URL}audio/${selected}.mp3`;
    audio.setAttribute("aria-label", `Ejemplo de ${demos[selected].label}`);
    caption.textContent = demos[selected].caption;
    fileLink.href = audio.src;
    error.hidden = true;
    audio.onloadedmetadata = () => {
      if (change !== revision) return;
      audio.currentTime = position < audio.duration ? position : 0;
      drawWaveform();
      if (wasPlaying) {
        void audio.play().catch(() => {
          // A browser may require another press of its native play control.
        });
      }
    };
    audio.load();
    drawWaveform();
  });
}

audio.addEventListener("error", () => {
  error.hidden = false;
});
audio.addEventListener("timeupdate", drawWaveform);
audio.addEventListener("loadedmetadata", drawWaveform);
new ResizeObserver(drawWaveform).observe(canvas);
void fetch(`${import.meta.env.BASE_URL}audio/waveforms.json`)
  .then((response) => {
    if (!response.ok) throw new Error("Waveform unavailable");
    return response.json() as Promise<Record<Demo, number[]>>;
  })
  .then((data) => {
    waveforms = data;
    drawWaveform();
  })
  .catch(() => {
    canvas.parentElement!.hidden = true;
  });
