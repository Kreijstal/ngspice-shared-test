#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include <ngspice/sharedspice.h>
#include <stdio.h>

// Debug levels
typedef enum {
    DEBUG_NONE = 0,
    DEBUG_ERROR = 1,
    DEBUG_WARN = 2,
    DEBUG_INFO = 3,
    DEBUG_VERBOSE = 4,
    DEBUG_TRACE = 5
} DebugLevel;

extern DebugLevel current_debug_level;

#define DEBUG_PRINT(level, fmt, ...) \
    do { \
        if (level <= current_debug_level) { \
            printf("[%s] " fmt "\n", \
                   level == DEBUG_ERROR ? "ERROR" : \
                   level == DEBUG_WARN ? "WARN" : \
                   level == DEBUG_INFO ? "INFO" : \
                   level == DEBUG_VERBOSE ? "VERBOSE" : \
                   level == DEBUG_TRACE ? "TRACE" : "UNKNOWN", \
                   ##__VA_ARGS__); \
        } \
    } while (0)

// Structure to hold simulation context
// Structure to hold simulation vector data
typedef struct {
    double time;           // Current simulation time
    int num_signals;       // Number of signals (excluding time)
    char** signal_names;   // Array of signal names
    double* signal_values; // Array of signal values at current time
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
