# Third-party notices

## JUCE 8.0.12

Copyright Raw Material Software Limited. Source:
https://github.com/juce-framework/JUCE/tree/29396c22c93392d6738e021b83196283d6e4d850

JUCE modules are dual-licensed under AGPLv3 or the commercial JUCE 8 licence.
R-Vocoder is intended to use the **commercial licensing route**, not to impose AGPL
on the original source. The product owner must have a JUCE licence appropriate to
their revenue/funding and use. No claim of purchased licence or eligibility is made.
Review the [official terms](https://juce.com/legal/juce-8-licence/) before distribution.
No license fee, account registration or purchase was performed by this project.

The exact JUCE licence and bundled dependency notices are retained in `.deps/JUCE`.
`Scripts/package-macos.sh` copies those notices into the local distribution package.
Relevant bundled components include AudioUnitSDK (Apache-2.0), VST3 SDK (MIT in this
pinned version), FLAC and Ogg/Vorbis (BSD), JPEG (IJG), PNG and zlib (zlib),
HarfBuzz (MIT), SheenBidi (Apache-2.0). Not every JUCE dependency is linked: no AAX,
ASIO, LV2 or Android target is enabled in the macOS build.

## Build and validation tools — not linked into the plugin

- CMake: BSD-3-Clause, https://cmake.org/licensing/.
- Ninja: Apache-2.0, https://github.com/ninja-build/ninja.
- pluginval 1.0.4: GPL-3.0, https://github.com/Tracktion/pluginval.
  It is run as a separate validation executable and is not embedded or distributed
  with R-Vocoder. No pluginval code is linked into the product.
- Apple SDKs and `auval`: supplied by macOS / Apple Command Line Tools.

All R-Vocoder DSP, UI, parameter definitions and factory preset recipes are original
project code. Mathematical descriptions in the research references were used to
guide the design. No third-party preset banks, graphics or recorded musical performances
are included.
