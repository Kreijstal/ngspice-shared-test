#include "simulation.h"
#include <stdlib.h>

// Initialize debug level to ERROR by default
DebugLevel current_debug_level = DEBUG_ERROR;
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

// Global context pointer for signal handler
SimContext* g_context = NULL;

void set_simulation_callback(SimContext* context, SimDataCallback callback, void* user_data) {
    context->data_callback = callback;
    context->callback_data = user_data;
}

void cleanup_simulation(SimContext* context) {
    if (!context) return;

    // Halt any running simulation
    if (context->is_bg_running) {
        ngSpice_Command("bg_halt");
        // Small delay to allow halt to complete
        usleep(10000);
    }

    // Close CSV file if open
    if (context->csv_file) {
        fflush(context->csv_file);
        fclose(context->csv_file);
        context->csv_file = NULL;
    }
}

void signal_handler(int signum) {
    if (g_context) {
        cleanup_simulation(g_context);
    }
    signal(signum, SIG_DFL);  // Reset to default handler
    raise(signum);  // Re-raise the signal
}

int ng_getchar(char* outputchar, int ident, void* userdata) {
    (void)userdata;  // Suppress unused parameter warning
    (void)ident;     // Suppress unused parameter warning
    printf("%s\n", outputchar);
    return 0;
}

int ng_getstat(char* outputstat, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    
    // Check for completion
    if (strcmp(outputstat, "--ready--") == 0) {
        context->simulation_finished = true;
        context->current_progress = 100;
        return 0;
    }

    // Parse percentage - look for any XX.X% pattern in the string
    float percent;
    char* percent_pos = strstr(outputstat, "%");
    if (percent_pos) {
        // Scan backwards from % to find the number
        char* num_start = percent_pos;
        while (num_start > outputstat && 
              (isdigit(*(num_start-1)) || *(num_start-1) == '.')) {
            num_start--;
        }
        if (sscanf(num_start, "%f%%", &percent) == 1) {
            context->current_progress = (int)percent;
        }
    }

    DEBUG_PRINT(DEBUG_INFO, "Status: %s (Progress: %d%%)", outputstat, context->current_progress);
    return 0;
}

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

int ng_data(pvecvaluesall vecdata, int numvecs, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    
    if (!vecdata || !vecdata->vecsa) {
        DEBUG_PRINT(DEBUG_ERROR, "Received null vector data");
        return 0;
    }

    // Prepare simulation data for callback
    SimulationData sim_data = {0};
    sim_data.num_signals = vecdata->veccount - 1;  // Subtract 1 to exclude time
    sim_data.signal_values = malloc(sim_data.num_signals * sizeof(double));
    sim_data.signal_names = malloc(sim_data.num_signals * sizeof(char*));

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
        DEBUG_PRINT(DEBUG_VERBOSE, "Time = %g", timeValue->creal);
        
        if (!context->voltage_altered && !context->should_alter_voltage && timeValue->creal >= 6.0) {
            context->should_alter_voltage = true;
            DEBUG_PRINT(DEBUG_INFO, "Time threshold reached at t=%g, preparing to alter voltage", timeValue->creal);
        }
        
        fprintf(context->csv_file, "%g", timeValue->creal);
    }
    
    for (int i = 0; i < vecdata->veccount; i++) {
        pvecvalues value = vecdata->vecsa[i];
        if (!value || !value->name) continue;
        
        if (value->is_complex) {
            DEBUG_PRINT(DEBUG_VERBOSE, "%s = %g + j%g%s", value->name, 
                       value->creal, value->cimag, 
                       value->is_scale ? " (scale)" : "");
        } else {
            DEBUG_PRINT(DEBUG_VERBOSE, "%s = %g%s", value->name, 
                       value->creal,
                       value->is_scale ? " (scale)" : "");
        }
        
        if (strcmp(value->name, "time") != 0) {
            if (value->is_complex) {
                fprintf(context->csv_file, ",%g+j%g", value->creal, value->cimag);
            } else {
                fprintf(context->csv_file, ",%g", value->creal);
            }
        }
    }
    fprintf(context->csv_file, "\n");
    fflush(context->csv_file);

    // Fill simulation data
    int signal_index = 0;
    for (int i = 0; i < vecdata->veccount; i++) {
        pvecvalues value = vecdata->vecsa[i];
        if (!value || !value->name) continue;

        if (strcmp(value->name, "time") == 0) {
            sim_data.time = value->creal;
        } else {
            sim_data.signal_values[signal_index] = value->creal;
            sim_data.signal_names[signal_index] = strdup(value->name);
            signal_index++;
        }
    }

    // Call the callback if set
    if (context->data_callback) {
        context->data_callback(&sim_data, context->callback_data);
    }

    // Clean up
    for (int i = 0; i < sim_data.num_signals; i++) {
        free(sim_data.signal_names[i]);
    }
    free(sim_data.signal_values);
    free(sim_data.signal_names);
    
    return 0;
}

int ng_initdata(pvecinfoall initdata, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    
    if (!initdata) {
        DEBUG_PRINT(DEBUG_ERROR, "Received null initialization data");
        return 0;
    }

    DEBUG_PRINT(DEBUG_INFO, "Simulation initialization");
    DEBUG_PRINT(DEBUG_INFO, "Plot name: %s", initdata->name ? initdata->name : "unknown");
    DEBUG_PRINT(DEBUG_INFO, "Title: %s", initdata->title ? initdata->title : "untitled");
    DEBUG_PRINT(DEBUG_INFO, "Date: %s", initdata->date ? initdata->date : "unknown");
    DEBUG_PRINT(DEBUG_INFO, "Type: %s", initdata->type ? initdata->type : "unknown");
    
    DEBUG_PRINT(DEBUG_VERBOSE, "Available vectors:");
    for (int i = 0; i < initdata->veccount; i++) {
        pvecinfo vec = initdata->vecs[i];
        if (!vec || !vec->vecname) continue;
        
        DEBUG_PRINT(DEBUG_VERBOSE, "  %d: %s (%s)", 
                   i, 
                   vec->vecname,
                   vec->is_real ? "real" : "complex");
    }
    printf("\n");
    
    if (!context->headers_written) {
        fprintf(context->csv_file, "Time");
        for (int i = 0; i < initdata->veccount; i++) {
            pvecinfo vec = initdata->vecs[i];
            if (!vec || !vec->vecname) continue;
            if (strcmp(vec->vecname, "time") != 0) {
                fprintf(context->csv_file, ",%s", vec->vecname);
            }
        }
        fprintf(context->csv_file, "\n");
        context->headers_written = true;
    }
    
    return 0;
}

int ng_bgrunning(NG_BOOL running, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
    context->is_bg_running = !running;
    DEBUG_PRINT(DEBUG_INFO, "Background thread status: %s", !running ? "running" : "stopped");
    return 0;
}
