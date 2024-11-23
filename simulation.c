#include <stdbool.h>
#include <ngspice/sharedspice.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Structure to hold simulation context
typedef struct {
    bool simulation_finished;
    int current_progress;
} SimContext;

// Callback function to handle character output from ngspice
int ng_getchar(char* outputchar, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    printf("%s\n", outputchar);
    return 0;
}

// Callback function to handle simulation status
int ng_getstat(char* outputstat, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    // Parse percentage from status message if available
    int percent;
    if (sscanf(outputstat, "%*[^=]=%d", &percent) == 1) {
        context->current_progress = percent;
    }
    printf("Status: %s (Progress: %d%%)\n\n", outputstat, context->current_progress);
    return 0;
}

// Callback function for controlled exit
int ng_exit(int exitstatus, NG_BOOL immediate, NG_BOOL quitexit, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    context->simulation_finished = true;
    
    if (quitexit) {
        printf("Received quit request from ngspice\n");
    } else {
        printf("Received exit request from ngspice. Status: %d\n", exitstatus);
    }
    return exitstatus;
}

// Callback function to receive simulation data
int ng_data(pvecvaluesall vecdata, int numvecs, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    
    if (!vecdata || !vecdata->vecsa) {
        printf("Error: Received null vector data\n");
        return 0;
    }

    // Print time point
    printf("\nTime = %g\n\n", vecdata->vecsa[0]->creal);
    
    // Print values for each vector
    for (int i = 0; i < vecdata->veccount; i++) {
        pvecvalues value = vecdata->vecsa[i];
        if (!value || !value->name) continue;
        
        printf("%s = \n", value->name);
        if (value->is_complex) {
            printf("%g + j%g\n", value->creal, value->cimag);
        } else {
            printf("%g\n", value->creal);
        }
        printf("%s\n\n", value->is_scale ? " (scale)" : "");
    }
    
    return 0;
}

// Callback function for initialization data
int ng_initdata(pvecinfoall initdata, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    
    if (!initdata) {
        printf("Error: Received null initialization data\n");
        return 0;
    }

    printf("\nSimulation initialization\n\n");
    printf("Plot name: %s\n\n", initdata->name ? initdata->name : "unknown");
    printf("Title: %s\n\n", initdata->title ? initdata->title : "untitled");
    printf("Date: %s\n\n", initdata->date ? initdata->date : "unknown");
    printf("Type: %s\n\n", initdata->type ? initdata->type : "unknown");
    
    printf("\nAvailable vectors:\n\n");
    for (int i = 0; i < initdata->veccount; i++) {
        pvecinfo vec = initdata->vecs[i];
        if (!vec || !vec->vecname) continue;
        
        printf("  %d: %s (%s)\n", 
               i, 
               vec->vecname,
               vec->is_real ? "real" : "complex\n");
    }
    printf("\n");
    
    return 0;
}

// Callback function for background thread status
int ng_bgrunning(NG_BOOL running, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    printf("Background thread status: %s\n\n", running ? "running" : "stopped");
    return 0;
}

int main() {
    // Declare the simulation context
    SimContext context;
    context.simulation_finished = false;
    context.current_progress = 0;

    // Initialize ngspice
    int ret = ngSpice_Init(ng_getchar, ng_getstat, ng_exit,
                          ng_data, ng_initdata, ng_bgrunning, &context);

    if (ret != 0) {
        fprintf(stderr, "Error initializing ngspice\n");
        return 1;
    }

    // Create the circuit
    const char* circuit[] = {
        ".title TB8",
        "Vvdc y 0 1.0V",
        "Ccap1 0 k 1.0 ic=0",
        "Rres1 k y 1.0Ohm",
        ".options TEMP = 25C",
        ".options TNOM = 25C",
        ".tran 0.001s 6s 0s uic",
        ".end",
        NULL
    };

    // Load the circuit
    ret = ngSpice_Circ((char**)circuit);
    if (ret != 0) {
        fprintf(stderr, "Error loading circuit\n");
        return 1;
    }

    printf("Circuit loaded successfully. Starting simulation...\n\n");
    fflush(stdout); // Ensure output is visible

    // Run the simulation in background
    ret = ngSpice_Command("bg_run");
    if (ret != 0) {
        fprintf(stderr, "Error starting simulation\n");
        return 1;
    }

    // Wait for simulation to complete
    while (!context.simulation_finished) {
        // Polling delay to reduce CPU usage
        #ifdef _WIN32
            Sleep(100); // Windows uses milliseconds
        #else
            usleep(100000); // Unix/Linux uses microseconds
        #endif
    }

    printf("Simulation completed. Final progress: %d%%\n\n", context.current_progress);
    fflush(stdout);

    // Add a small delay to allow background thread to finish completely
    #ifdef _WIN32
        Sleep(100); // Windows uses milliseconds
    #else
        usleep(100000); // Unix/Linux uses microseconds
    #endif

    return 0;
}
