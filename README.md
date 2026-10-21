# Reparto Vocal Suite (v1.0) — plugin original

Plugin VST3 hecho desde cero (no usa código ni archivos de Fresh Vocal ni de ningún otro plugin comercial).

## Qué trae
**Cadena vocal:** ganancia de entrada, low cut, de-esser, EQ (cuerpo / barro / presencia / aire),
compresor, saturación, delay ping-pong, reverb con pre-delay, nivel de salida.
**Cadena master (activable aparte):** EQ de graves y agudos, compresor "glue", bajos en mono,
ancho estéreo, drive, soft clipper, limitador con lookahead y techo garantizado.

## Presets (menú de presets del plugin / de FL Studio)
- Reparto - Voz Principal / Agresivo (Trap) / Coros-Doble
- Voz Cantada Limpia
- Master - Dj Honda, Befocus (x2), Dichavate, El Bandolero, Mauro, Lobo Malo, Dave Produce + Master - Suave
- Voz Reparto + Master Honda (las dos cadenas)

## Análisis del tema de referencia (decodificado de YouTube AAC 128k)
- ~ -5.6 LUFS integrado, cresta ~12.5 dB, LRA ~3.6 (muy comprimido y fuerte)
- Estéreo moderado: correlación 0.74, lado 8 dB bajo el centro; <150 Hz casi mono (lado -16 dB)
- Energía pareja entre 40 Hz y 1.6 kHz, caída de agudos desde 3 kHz (en parte por el AAC)
- Tempo ~71 BPM (medio tiempo, ~142 en doble)
Prueba: el tema bajado 9 dB pasado por el preset "Master - Estilo Honda" sale a -5.6 LUFS con pico -0.3 dBFS.
`tools/analyze_reference.py` repite la medición (`python3 -I analyze_reference.py archivo.raw`).

## Instalador .exe
El workflow de GitHub Actions ahora también genera `RepartoVocalSuite-Setup.exe` (artifact `RepartoVocalSuite-Setup`).
Al ejecutarlo instala el .vst3 en C:\Program Files\Common Files\VST3. Después, en FL Studio: Options > Manage plugins > Find plugins.
(Windows puede mostrar aviso de SmartScreen porque el instalador no está firmado: "Más información" > "Ejecutar de todos modos".)

## Cómo compilar en Windows
Opción A (sin instalar nada pesado): sube esta carpeta a un repositorio de GitHub; la pestaña
Actions compila sola y te da `RepartoVocalSuite-Windows-VST3` (carpeta .vst3 + .exe standalone).
Opción B (local): instala Visual Studio Community (carga "Desarrollo para escritorio con C++") y CMake, luego:
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
El .vst3 queda en build\RepartoVocalSuite_artefacts\Release\VST3 → cópialo a
C:\Program Files\Common Files\VST3 y en FL Studio: Options > Manage plugins > Find plugins.

## Notas
- Latencia fija de ~2 ms (limitador lookahead), el plugin la reporta al host.
- Para voz y master a la vez en un solo canal normalmente conviene usar el plugin en la pista vocal
  (cadena vocal) y otra instancia en el Master (solo cadena master).

## Presets de master por referencia (LUFS medido de cada tema)
Dj Honda -5.6 · Befocus Un Tin -7.9 · Befocus Cosita -6.4 · Dichavate -6.8 · El Bandolero (Se Picó) -7.8 ·
Mauro El Código Secreto -9.0 · Lobo Malo -7.0 · Dave Produce (Bionica) -7.3.
Cada preset ajusta: EQ de 6 bandas (diferencia de cada tema frente al promedio de todas las referencias),
ancho estéreo, bajos en mono, compresión glue, soft clipper y drive hasta llegar al volumen del tema.
`tools/` tiene los scripts para sacar los mismos datos de nuevas referencias.
