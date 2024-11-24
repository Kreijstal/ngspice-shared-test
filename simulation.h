#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include <ngspice/sharedspice.h>
#include <stdio.h>

// Structure to hold simulation context
typedef struct {
    bool simulation_finished;
    int current_progress;
    FILE* csv_file;
    bool voltage_altered;
    bool should_alter_voltage;
    bool is_bg_running;
    bool headers_written;
} SimContext;

// Global context pointer declaration
extern SimContext* g_context;

// Function declarations
void signal_handler(int signum);
int ng_getchar(char* outputchar, int ident, void* userdata);
int ng_getstat(char* outputstat, int ident, void* userdata);
int ng_exit(int exitstatus, NG_BOOL immediate, NG_BOOL quitexit, int ident, void* userdata);
int ng_data(pvecvaluesall vecdata, int numvecs, int ident, void* userdata);
int ng_initdata(pvecinfoall initdata, int ident, void* userdata);
int ng_bgrunning(NG_BOOL running, int ident, void* userdata);

#endif // SIMULATION_H
