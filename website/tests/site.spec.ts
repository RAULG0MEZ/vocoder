import { test, expect } from "@playwright/test";

test("download is accessible without JavaScript and assets resolve below the repository path", async ({
  browser,
}) => {
  const page = await browser.newPage({ javaScriptEnabled: false });
  await page.goto("./");
  await expect(page.getByRole("heading", { level: 1 })).toContainText("TU VOZ");
  await expect(
    page.getByRole("link", { name: "Descargar para macOS" }),
  ).toHaveAttribute(
    "href",
    "https://github.com/RAULG0MEZ/vocoder/releases/download/v0.4.1/R-Vocoder-0.4.1-macOS.zip",
  );
  for (const image of await page.locator("img").all()) {
    await image.scrollIntoViewIfNeeded();
    await expect
      .poll(() => image.evaluate((el: HTMLImageElement) => el.naturalWidth))
      .toBeGreaterThan(0);
  }
  await page
    .getByText("macOS muestra una advertencia al abrirlo", { exact: true })
    .click();
  await expect(page.getByText(/Esta beta tiene firma local/)).toBeVisible();
  await page.close();
});

test("audio plays and switches versions at the same position; failed audio gives a useful fallback", async ({
  page,
}) => {
  const errors: string[] = [];
  page.on("pageerror", (error) => errors.push(error.message));
  await page.goto("./");
  const audio = page.locator("audio");
  await expect
    .poll(() => audio.evaluate((el: HTMLAudioElement) => el.readyState))
    .toBeGreaterThan(0);
  await audio.evaluate(async (el: HTMLAudioElement) => {
    el.currentTime = 2;
    await el.play();
  });
  await page.getByText("02 Vocoder + Synth", { exact: false }).click();
  await expect(audio).toHaveAttribute("src", "/vocoder/audio/synth.mp3");
  await expect
    .poll(() =>
      audio.evaluate(
        (el: HTMLAudioElement) => !el.paused && el.currentTime > 1.8,
      ),
    )
    .toBe(true);
  await audio.evaluate((el: HTMLAudioElement) => el.pause());
  await page.getByText("03 Modo Voz", { exact: false }).click();
  await expect(audio).toHaveAttribute("src", "/vocoder/audio/voice.mp3");
  await expect
    .poll(() => audio.evaluate((el: HTMLAudioElement) => el.readyState))
    .toBeGreaterThan(0);
  expect(await audio.evaluate((el: HTMLAudioElement) => el.paused)).toBe(true);
  await page.route("**/audio/original.mp3", (route) => route.abort());
  await page.getByText("01 Sin procesar", { exact: false }).click();
  await expect(page.locator("#audio-error")).toBeVisible();
  await expect(page.locator("#audio-file")).toHaveAttribute(
    "href",
    /\/vocoder\/audio\/original.mp3$/,
  );
  expect(errors).toEqual([]);
});

test("layout stays within desktop, tablet and narrow mobile screens", async ({
  page,
}) => {
  for (const width of [1440, 768, 390, 320]) {
    await page.setViewportSize({ width, height: 900 });
    await page.goto("./");
    await page.evaluate(() => document.fonts.ready);
    expect(
      await page.evaluate(
        () => document.documentElement.scrollWidth <= window.innerWidth,
      ),
    ).toBe(true);
    await expect(
      page.getByRole("link", { name: "Descargar para macOS" }),
    ).toBeVisible();
    await page.screenshot({
      path: `test-results/site-${width}.png`,
      fullPage: true,
    });
  }
});
