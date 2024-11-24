#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

extern SimContext* g_context;

int main() {
    // Declare the simulation context
    SimContext context;
    g_context = &context;  // Set global pointer for signal handler
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    context.simulation_finished = false;
    context.current_progress = 0;
    context.voltage_altered = false;
    context.should_alter_voltage = false;
    context.is_bg_running = false;
    context.headers_written = false;
    
    // Open CSV file for writing
    context.csv_file = fopen("simulation_data.csv", "w");
    if (!context.csv_file) {
        fprintf(stderr, "Error opening CSV file\n");
        return 1;
    }

    // Example callback function
    void handle_simulation_data(SimulationData* data, void* user_data) {
        printf("Simulation time: %f\n", data->time);
        for (int i = 0; i < data->num_signals; i++) {
            printf("  Signal '%s': %f\n", data->signal_names[i], data->signal_values[i]);
        }
    }

    // Initialize ngspice
    int ret = ngSpice_Init(ng_getchar, ng_getstat, ng_exit,
                          ng_data, ng_initdata, ng_bgrunning, &context);

    // Set up the callback
    set_simulation_callback(&context, handle_simulation_data, NULL);

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
        ".tran 0.001s 12s 0s uic",
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
        printf("current_progress %d%%  should_alter_voltage: %d %d\n",context.current_progress,context.should_alter_voltage,context.is_bg_running );
        
        // Check if we need to alter voltage
        if (context.should_alter_voltage && !context.voltage_altered && context.is_bg_running) {
            printf("Halting simulation to alter voltage...\n");
            ngSpice_Command("bg_halt");
            
            // Wait for simulation to actually halt
            while (context.is_bg_running) {
                usleep(10000);
            }
            
            printf("Simulation halted, altering voltage...\n");
            ngSpice_Command("alter Vvdc=0");
            context.voltage_altered = true;
            
            printf("Resuming simulation...\n");
            ngSpice_Command("bg_resume");
        }
        
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
