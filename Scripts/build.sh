#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake_bin="${RV_CMAKE:-cmake}"
if [[ -x .venv/bin/cmake ]]; then cmake_bin="$PWD/.venv/bin/cmake"; fi
args=(-S . -B build -DCMAKE_BUILD_TYPE=Release)
if [[ "$(uname -s)" == Darwin ]]; then args+=('-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'); fi
"$cmake_bin" "${args[@]}"
"$cmake_bin" --build build --config Release --parallel 4
"$cmake_bin" --build build --target test --config Release
