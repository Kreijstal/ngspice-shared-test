#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include <ngspice/sharedspice.h>
#include <stdio.h>

// Structure to hold simulation context
// Structure to hold simulation vector data
typedef struct {
    double time;
    double* values;
    char** names;
    int count;
} SimulationData;

// Callback function type
typedef void (*SimDataCallback)(SimulationData* data, void* user_data);

typedef struct {
    bool simulation_finished;
    int current_progress;
    FILE* csv_file;
    bool voltage_altered;
    bool should_alter_voltage;
    bool is_bg_running;
    bool headers_written;
    SimDataCallback data_callback;  // Callback function pointer
    void* callback_data;           // User data for callback
} SimContext;

// Function to set the callback
void set_simulation_callback(SimContext* context, SimDataCallback callback, void* user_data);

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
