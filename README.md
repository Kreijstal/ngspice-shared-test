# ngspice Echtzeit-Simulationsprojekt

Dieses Projekt demonstriert die Verwendung von ngspice zur Echtzeit-Simulation eines Schaltkreises. Die Simulation wird in C geschrieben und verwendet die ngspice Shared Library mit einer interaktiven SDL2-Benutzeroberfläche zur Echtzeitvisualisierung.

## Voraussetzungen

- ngspice mit Shared Library Support
- gcc oder clang
- make
- SDL2 und SDL2_gfx Entwicklungsbibliotheken

Unter Ubuntu/Debian können Sie die benötigten Pakete wie folgt installieren:
```bash
sudo apt install build-essential libngspice0 libngspice0-dev libsdl2-dev libsdl2-gfx-dev
```

Unter Arch Linux installieren Sie die benötigten Pakete mit:
```bash
sudo pacman -S ngspice sdl2_gfx base-devel --needed
```

Unter Windows mit MSYS2 installieren Sie die benötigten Pakete mit:
```bash
pacboy -S ngspice:p SDL2_gfx:p cc:p pkgconf:p --needed 
```

Mit brew
```bash
brew install ngspice sdl2_gfx pkgconf
```

## Kompilierung

Das Projekt wird mit `make` kompiliert:

```bash
make
```

## Ausführung

Starten Sie die interaktive Simulation mit:

```bash
./simulation_plot
```

Die Simulation wird in Echtzeit ausgeführt und in einem SDL2-Fenster angezeigt. Sie können die Simulationsparameter über die Schieberegler in der Benutzeroberfläche anpassen.
