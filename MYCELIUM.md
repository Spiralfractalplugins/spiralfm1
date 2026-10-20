# Mycelium FM1 - 1.0

Forest, Goa, Psytrance, Hitech und TeKK für den M-VAVE FM1.
Firmware basiert auf SLOOP/Felucca, GPL-3.0-only. Herkunftshinweise bleiben erhalten.

HOME halten → GROOVES → Groove wählen → OCT+ → OCT+ bestätigen.
Vorher eigene Arbeit speichern: Laden ersetzt den aktuellen Loop und seine Klänge,
aber keine gespeicherten Projektplätze. Transport und Aufnahme müssen gestoppt sein.
Danach eine Taste spielen: der Beat startet. Nach bewusstem STOP startet PLAY erneut.
Ein Update ersetzt keine gespeicherte Szene: den neuen Groove ausdrücklich laden.

Sechs Grooves, jeweils 32 Steps: FOREST JAM 156 BPM, PSY ROLL 148,
PSY GALLOP 146, GOA ACID 144, HITECH 196, TEKK 170.
Forest: Viertel-Kicks, drei Bass-Sechzehntel dazwischen, Offbeat-Hats, keine Snare.
TeKK: Offbeat-Bass, Viertel-Kicks mit End-Fill, sparsame Clap.
Laden löscht Solo und stellt Drumlautstärke sowie Filterzustände wieder her.

ARP antippen: GALLOP → OFFBEAT → FULL-ON. Ersetzt nur das Basspattern;
Grundton, andere Spuren, Tempo und laufender Transport bleiben erhalten.
Während Aufnahme/Count-in oder aktivem Arranger ist der Wechsel gesperrt.
ARP halten bietet weiterhin Roll.

FX antippen und loslassen öffnet PSY FX:
1 KB LP: gemeinsamer Lowpass für trockenen Bass auf Spur 1 und Kick-Noten 35/36.
  Links offen, rechts geschlossen; feste Resonanz 25/127, rund 20 Prozent.
2 SPACE: synchrones Echo und Hall gemeinsam, Sends für Lead und FX.
  Bei 0 keine neue Einspeisung; Hallreste klingen aus.
3 FB+DRV: Echo-Feedback und Low-End-Drive gemeinsam; links beide 0,
  rechts Feedback 100/127 und Drive 127/127 (3x Soft-Sättigung vor dem Lowpass).
4 ALIEN: Ringmodulation ausschließlich auf trockenem Lead und FX (Spuren 2/3).
  Links bypass, rechts metallische Modulation bis 2400 Hz; geglättete Intensität.
  Kick, Bass und gesamte Drums bleiben unverändert. Echo-/Hall-Sends bleiben separat.
FX halten bietet weiterhin Punch-Effekte. Projektformat bleibt kompatibel.

21 kuratierte Werkspresets; drei unterschiedliche Psy-/Forest-/Hitech-Drumkits.
Kicks: Psy ONE_SHOT_KICK_01, Forest LTWB-KICK, Hitech ONE_SHOT_KICK_05.
Alle drei Halbtöne tiefer. Original LTWB bei 44.1 kHz, weitere Kicks bei 22.05 kHz;
sechs unterschiedliche Hat-/Clap-Samples bei 16 kHz, IMA-ADPCM.
Samples auf Peaks 70 Prozent (Kick), 30 Prozent (Hat), 40 Prozent (Clap) vorbereitet.
Weitere Snare-/Percussion-Sounds unterschiedlich synthetisiert. Presetlautstärken
übernehmen die gemessenen Einzelkorrekturen aus 0.6. Herkunft und Hashes:
assets/mycelium/samples/SOURCE.json und DRUM-SOURCES.json.
Lokale Loopmasters-Quellen sind für diese persönliche Anpassung autorisiert;
öffentliche Verbreitungsrechte für eingebettete Samples wurden nicht verifiziert.

Originales Mandalars/Mycelium-Logo beim Start, dunkler Hintergrund.
Spuren 1–4: Neongrün, Türkis, Neongelb, Neonpink.
USB-Name Mycelium, Hersteller Mandalars. Markenassets von Mandalars autorisiert.
USB-/TRS-MIDI: Kanal 1–3 Synths, Kanal 10 Drums, Kanal 4 ausgewählte Spur.
Noten, Velocity, Clock/Start/Stop; normale Klangregler-CCs nicht implementiert.
GLO → SYSTEM → SYNC wählt USB/TRS/INT.

Speicher-/Prozessorbudget, Sequencer, Autostart, Menüs, FX, Basswechsel,
Drumkits, TeKK und 20.000 zufällige UI-Eingaben geprüft. 18 Audio-Prüfrenderings
aus dem echten Host-Synthesecode ohne übersteuerte Samples; keine Geräteaufnahmen.
Hörtest und Bedienprüfung auf dem FM1 stehen nach Installation an.
Dash-Glitch-Tutorials nicht detailgetreu nachgebaut; keine Kooperation behauptet.
Vor öffentlichen Veröffentlichungen geerbte Asset-Lizenzen prüfen und GPL erfüllen.

## Neue Performance-Bedienung in 0.14
ENV kurz: Klanghüllkurve. ENV mindestens 0.7 Sekunden halten: temporärer Psy-Fill
mit Hats und Claps, nur bei laufendem Beat. Loslassen beendet den Fill ohne
Patternänderung. LFO kurz: Modulation. LFO mindestens 0.7 Sekunden halten:
FOREST / GOA / HITECH-Patterns für Lead und FX im Kreis. Das ersetzt nur diese
beiden Patterns; eigene Arbeit vorher speichern. Bass, Drums und Tempo bleiben.
Aufnahme/Count-in und aktiver Song-Arranger sperren beide Halteaktionen.
HOME halten → LOGO → OCT+: Original-Logo ansehen, OCT- zurück ins Menü.
PSY FX zeigt den zuletzt gedrehten Regler dauerhaft groß mit Neonbalken und
Zielbeschriftung, auch bei ausgeschaltetem ZOOM.

## Probability und Startbildschirm in 0.15
HOME/Spurübersicht, Spur 2 und 3: KNOB 1 PROB, je Spur getrennt.
100 Prozent spielt jeden programmierten NOTE-Step; 0 Prozent keinen; dazwischen
wird bei jedem NOTE-Step neu entschieden. Akkorde und Ratchets gehören gemeinsam
zu dieser Entscheidung. Ausgelassene Steps lösen keine Ratchets aus, Ties würfeln
nicht erneut. Das Pattern wird nicht gelöscht. Direkt gespielte Noten bleiben
spielbar; Bass und Drums bekommen keine Probability-Gate-Funktion.
Gespeichert im vorhandenen P_APROB-Parameter, den auch der Arpeggiator nutzt.
Werkspresets starten mit 100 Prozent; Klangwechsel kann diesen Klangparameter
zurücksetzen. Auf Bass/Drums bleibt in der Spurübersicht KNOB 1 der globale Swing.
Beim Einschalten bleibt das originale Mandalars-Logo rund vier Sekunden stehen.
Der Watchdog wird während dieser Anzeige regelmäßig bedient.

## Letzte Testversion 0.16
Nach Installation: eigene Arbeit vorher speichern, dann einen neuen Factory-Groove
laden oder Werkspresets neu auswählen. Gespeicherte Szenen enthalten sonst weiterhin
ihre alten Parameter. Das Update ersetzt keine gespeicherten Projekte.
Bass: Spur 1 → ENV kurz → ATK/DEC/SUS/REL.
Kick: Spur 4 → ENV kurz → ATK/DEC/SUS/REL. Das erste Drehen aktiviert die separate
Kick-Hüllkurve und beginnt mit Attack 0, Decay 70, Sustain 127, Release 127.
Kick-Hüllkurve wirkt nur auf Noten 35/36, nicht auf Hats/Claps/Snares. Ohne Aktivierung
bleiben Sample-Kicks unverändert. Wiederholte Kicks starten die Hüllkurve neu.
Release folgt dem Note-off oder dem automatischen One-Shot-Gate: ein Viertel bei
aktuellem Tempo, multipliziert mit dem vorhandenen Drum-GATE-Wert (SGATE/128).
ENV halten bleibt der vorübergehende Psy-Fill, kurz drücken öffnet die Hüllkurve.
FX KNOB 4 ALIEN+: Ringmodulation und Stutter zusammen auf Lead und FX. Tiefe steigt
von 0 bis 127, Stutter-Raster wechselt 1/8 → 1/16 → 1/32. Regler 0 schaltet Stutter
aus. Übernimmt die Slicer-Einstellungen der Spuren 2/3; Bass und Drums bleiben außen.
16 Lead-/FX-/Atmosphären-Hüllkurven überarbeitet: längere Ausklänge, hörbarer Sustain
bei Leads und schnellerer Attack bei Atmosphären. Percussive Hits bleiben bei Sustain 0.
Alle 21 Presets neu gemessen, Einzeltrims mit -20-LUFS-Ziel und Peakreserve korrigiert.
Host-Prüfungen bestanden; physischer Hör-/Bedientest durch Lars steht noch aus.

## Release 1.0
Am 08.10.2026 von Lars freigegeben. Grundlage: 0.16-test. Nur die Versionsanzeige wurde auf MYCELIUM 1.0 geändert. Sounds, Patterns und Bedienung bleiben unverändert. 1.0-Paket erstellt, Versionsumbenennung noch nicht am Gerät installiert.

## Aktueller 1.0-Stand: eigene Drums und epische Songs
Die historische LTWB-/Loopmasters-Auswahl ist in dieser Revision ersetzt.
15 eigene Drums: Oszillatoren, Pitch-Hüllkurven, Rauschen und Sättigung.
Generator und SHA256-Provenienz in tools/gen_mandalars_drums.py und assets/mycelium/samples/ORIGINAL-SOUNDS.json.
Keine externen Audio-Eingänge. Zusatz-Hip-Hop-Pack nicht mehr im Release-Quellbaum.
Firmwarebasis GPL und separat gekennzeichnete CC0-Klangbank bleiben erhalten.
HOME halten → SONGS: Forest Odyssey / Psy Ascension / Tekk Reactor / Hitech Portal.
Jeder Song 240 Takte, 16 Abschnitte; A Atmosphäre, B Aufbau, C Drop, D Break.
Virtuelle Szenen überschreiben keine Projektplätze. USER SONG stellt die eigene Reihenfolge wieder her.
Factory-Songs sind schreibgeschützt, nach Neustart neu auswählen.
ARP mindestens 700 ms halten: Roll-Modus und Logo einrasten. Erneut ARP: verlassen ohne Basswechsel.
Spielnoten rollen nur, solange ihre Taste gehalten ist. Vier komplette Songs und UI-Fuzz geprüft.
Noch nicht am Gerät installiert; neuer physischer Hörtest erforderlich.

Installation: aktuelles 1.0-Paket vollständig übertragen, Neustart und laufende MIDI-Gerätekennung verifiziert. Eigene Drums, Songs und ARP-Latch sind installiert; physischer Hörtest der neuen Funktionen ausstehend.

## Direktes SAVE und Distortion
SAVE kurz aus Übersicht, Drum-Seite oder SONG: vier fertige Factory-Song-Bänke. USER SONG öffnet eigene Abschnitte; dort EDIT zum Sichern der Kette. KNOB 4 in TRACKS: P_DIST für Bass/Drums, P_PAN für Synthspuren 2/3. Drum-KIT KNOB 4: P_DIST, Drum-GRID weiterhin Hit-Level. Drum-Distortion vor Pan/Reverb und Kick/Bass-Tiefpass, Nullwert unverändert. FX-KNOB 4 bleibt ALIEN+. Sechs Grooves bei maximaler Bass-/Drum-Distortion ohne Clipping geprüft.

## MODERN MOTION: 16 neue Werkspresets
Zehn Jahre moderne Soft-Synth-Club-Sounds auf der FM-1: acht ANALOG- und acht DIGITAL-(FM-)Presets
nach dem Charakter der Bewegungs-Bässe, Wobbles, Growls, Risers, Big-Room-Leads und Glass-Plucks
aus Vital und Serum 2 — eigene Entwürfe, keine importierten Preset-Daten (die Lizenzen dieser
Synths erlauben keinen Import; referenziert wird nur die Klangrichtung). ANALOG 21–28: MODERN
PLUCK, GROWL VA, BIG ROOM LD, TRANCE RISE, DARK MOTION, MIDNIGHT PAD, VAPOR KEY, TECH SNAP.
DIGITAL 29–36: FM WOBBLE, MODERN GROWL, GLASS PLUCK, CINEMA HIT, NEON SHIM, RISER FX, VOCAL FM,
DROP SUB. Alle 16 sind im kuratierten BANK-Browser des Geräts (PRESETS-Dreher) und im
meta/MYCELIUM-SOUNDBANK.json erfasst; Lautstärke-Trims gemessen mit
tests/mycelium_preset_level.c (BS.1770, Ziel −20 LUFS wie bei 0.16), Prüfung:
tests/mycelium_motion_pack_test.c in tests/run_tests.sh.
