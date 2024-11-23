# ngspice Simulation Project

Dieses Projekt demonstriert die Verwendung von ngspice zur Simulation eines einfachen Schaltkreises. Die Simulation wird in C geschrieben und verwendet die ngspice Shared Library.

## Voraussetzungen

- ngspice
- gcc oder clang
- make

## Kompilierung

Verwenden Sie `make`, um das Projekt zu kompilieren:

```bash
make
```

Sie können auch eigene Compiler- und Linker-Flags verwenden:

```bash
make CFLAGS="-I/path/to/include" LDFLAGS="-L/path/to/lib"
```

## Ausführung

Führen Sie die Simulation mit dem folgenden Befehl aus:

```bash
./simulation
```

Die Simulationsergebnisse werden in der Datei `simulation_data.csv` gespeichert.

## Plotten der Ergebnisse

Verwenden Sie das Python-Skript `plot_simulation.py`, um die Ergebnisse zu plotten:

```bash
python3 plot_simulation.py
```

Dies erzeugt eine PNG-Datei namens `simulation_plot.png` mit den Simulationsergebnissen.
