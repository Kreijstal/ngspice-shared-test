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
    FILE* csv_file;
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
    printf("Status: %s (Progress: %d%%)\n", outputstat, context->current_progress);
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

    // Find the time vector
    pvecvalues timeValue = NULL;
    for (int i = 0; i < vecdata->veccount; i++) {
        if (vecdata->vecsa[i] && vecdata->vecsa[i]->name && 
            strcmp(vecdata->vecsa[i]->name, "time") == 0) {
            timeValue = vecdata->vecsa[i];
            break;
        }
    }
    
    if (timeValue) {
        // Print and write time point
        printf("\nTime = %g\n", timeValue->creal);
        
        // Write data to CSV
        fprintf(context->csv_file, "%g", timeValue->creal);
    }
    
    // Print and write values for each vector
    for (int i = 0; i < vecdata->veccount; i++) {
        pvecvalues value = vecdata->vecsa[i];
        if (!value || !value->name) continue;
        
        printf("%s = ", value->name);
        if (value->is_complex) {
            printf("%g + j%g", value->creal, value->cimag);
        } else {
            printf("%g", value->creal);
        }
        printf("%s\n", value->is_scale ? " (scale)" : "");
        
        // Write to CSV, skip the time vector since we already have it as first column
        if (strcmp(value->name, "time") != 0) {
            if (value->is_complex) {
                fprintf(context->csv_file, ",%g+j%g", value->creal, value->cimag);
            } else {
                fprintf(context->csv_file, ",%g", value->creal);
            }
        }
    }
    fprintf(context->csv_file, "\n");
    
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
    printf("Plot name: %s\n", initdata->name ? initdata->name : "unknown");
    printf("Title: %s\n", initdata->title ? initdata->title : "untitled");
    printf("Date: %s\n", initdata->date ? initdata->date : "unknown");
    printf("Type: %s\n", initdata->type ? initdata->type : "unknown");
    
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
    
    // Write CSV headers
    fprintf(context->csv_file, "Time");
    for (int i = 0; i < initdata->veccount; i++) {
        pvecinfo vec = initdata->vecs[i];
        if (!vec || !vec->vecname) continue;
        // Skip the time vector since we already have it as the first column
        if (strcmp(vec->vecname, "time") != 0) {
            fprintf(context->csv_file, ",%s", vec->vecname);
        }
    }
    fprintf(context->csv_file, "\n");
    
    return 0;
}

// Callback function for background thread status
int ng_bgrunning(NG_BOOL running, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    printf("Background thread status: %s\n", running ? "running" : "stopped");
    return 0;
}

int main() {
    // Declare the simulation context
    SimContext context;
    context.simulation_finished = false;
    context.current_progress = 0;
    
    // Open CSV file for writing
    context.csv_file = fopen("simulation_data.csv", "w");
    if (!context.csv_file) {
        fprintf(stderr, "Error opening CSV file\n");
        return 1;
    }

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

    printf("Simulation completed. Final progress: %d%%\n", context.current_progress);
    fflush(stdout);

    // Add a small delay to allow background thread to finish completely
    #ifdef _WIN32
        Sleep(100); // Windows uses milliseconds
    #else
        usleep(100000); // Unix/Linux uses microseconds
    #endif

    // Close CSV file
    if (context.csv_file) {
        fclose(context.csv_file);
        context.csv_file = NULL;
    }

    return 0;
}
