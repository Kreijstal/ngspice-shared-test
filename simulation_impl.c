#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

// Global context pointer for signal handler
SimContext* g_context = NULL;

void signal_handler(int signum) {
    if (g_context && g_context->csv_file) {
        fflush(g_context->csv_file);
        fclose(g_context->csv_file);
        g_context->csv_file = NULL;
    }
    exit(signum);
}

int ng_getchar(char* outputchar, int ident, void* userdata) {
    SimContext* context = (SimContext*)userdata;
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

    printf("Status: %s (Progress: %d%%)\n", outputstat, context->current_progress);
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
        printf("\nTime = %g\n", timeValue->creal);
        
        if (!context->voltage_altered && !context->should_alter_voltage && timeValue->creal >= 6.0) {
            context->should_alter_voltage = true;
            printf("Time threshold reached at t=%g, preparing to alter voltage\n", timeValue->creal);
        }
        
        fprintf(context->csv_file, "%g", timeValue->creal);
    }
    
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
    
    return 0;
}

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
    printf("Background thread status: %s\n", !running ? "running" : "stopped");
    return 0;
}
