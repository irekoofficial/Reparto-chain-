#!/bin/bash
# Reparto Chain by Ireko - compilar en Mac (Apple Silicon e Intel). Uso:  bash build-mac.sh
set -e
cd "$(dirname "$0")"
if ! xcode-select -p >/dev/null 2>&1; then echo "Instalando herramientas de Apple... termina la instalacion y vuelve a correr este script."; xcode-select --install; exit 1; fi
if ! command -v cmake >/dev/null 2>&1; then
  if command -v brew >/dev/null 2>&1; then brew install cmake
  else echo "Falta CMake. Descargalo en https://cmake.org/download/ (macOS universal) y vuelve a correr el script."; exit 1; fi
fi
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13
cmake --build build --config Release --parallel
rm -rf pkgroot out zip; mkdir -p zip
mkdir -p pkgroot/Library/Audio/Plug-Ins/VST3 pkgroot/Library/Audio/Plug-Ins/Components pkgroot/Applications out zip
find build -maxdepth 6 -name "*.vst3" -print0 | while IFS= read -r -d '' b; do codesign --force --deep -s - "$b"; cp -R "$b" pkgroot/Library/Audio/Plug-Ins/VST3/; cp -R "$b" zip/; done
find build -maxdepth 6 -name "*.component" -print0 | while IFS= read -r -d '' b; do codesign --force --deep -s - "$b"; cp -R "$b" pkgroot/Library/Audio/Plug-Ins/Components/; cp -R "$b" zip/; done
find build -maxdepth 6 -name "*.app" -print0 | while IFS= read -r -d '' b; do codesign --force --deep -s - "$b"; cp -R "$b" pkgroot/Applications/; cp -R "$b" zip/; done
pkgbuild --root pkgroot --identifier com.ireko.repartochain --version 1.1.0 --install-location / out/RepartoChain-Mac.pkg
ditto -c -k --keepParent zip out/RepartoChain-Mac-manual.zip
echo; echo "LISTO. Archivos en la carpeta: $(pwd)/out"; open out
