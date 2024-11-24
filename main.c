#include "plot.h"
#include "simulation.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

extern SimContext* g_context;


SignalValues get_new_values(double t, PlotConfig* config) {
    SignalValues values;
    values.values[0] = sin(t) * config->amplitude_slider.value;  // First signal: sine with slider amplitude
    values.values[1] = cos(t);  // Second signal: cosine
    return values;
}

// Callback function to update plot buffers with simulation data
typedef struct {
    PlotConfig* config;
    double** buffers;
} CallbackData;

void handle_simulation_data(SimulationData* data, void* user_data) {
    CallbackData* cb_data = (CallbackData*)user_data;
    PlotConfig* config = cb_data->config;
    double** buffers = cb_data->buffers;
    SignalValues new_values;
    
    if (!config || !buffers) return;  // Safety check
    
    // Count and process actual signals (excluding #branch)
    int plot_idx = 0;
    
    if (!buffers) return;  // Safety check
    
    // Second pass: fill in the values
    plot_idx = 0;  // Reset counter for actual data processing
    for (int i = 1; i < data->num_signals && plot_idx < 15; i++) {
        if (data->signal_names && data->signal_names[i] && 
            strstr(data->signal_names[i], "#branch") == NULL) {
            new_values.values[plot_idx] = data->signal_values[i];
            plot_idx++;
        }
    }
    
    update_buffers(buffers, new_values, config);
}

int main(int argc, char* argv[]) {
  //SDL2
    PlotConfig config = setup_config();
    config.num_signals = 2;  // Initialize with default number of signals
    double** buffers = init_buffers(&config);
    
    // Prepare callback data
    CallbackData cb_data = {
        .config = &config,
        .buffers = buffers
    };
    
    SDL_Window* window = init_sdl(&config);
    if (!window) return 1;

    SDL_Renderer* renderer = create_renderer(window);
    if (!renderer) return 1;

    int quit = 0;
    SDL_Event e;
    int useInterpolation = 1;

  //ngspice

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


    // Initialize ngspice
    int ret = ngSpice_Init(ng_getchar, ng_getstat, ng_exit,
                          ng_data, ng_initdata, ng_bgrunning, &context);

    // Set up the callback
    set_simulation_callback(&context, handle_simulation_data, &cb_data);

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
        ".tran 0.0001s 120s 0s uic",
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

 

    while (!quit) {

        // Check if slider value changed
        if (config.amplitude_slider.value_changed && context.is_bg_running) {
            printf("Halting simulation to alter voltage...\n");
            ngSpice_Command("bg_halt");
            
            // Wait for simulation to actually halt
            while (context.is_bg_running) {
                usleep(10000);
            }
            
            char alter_cmd[64];
            snprintf(alter_cmd, sizeof(alter_cmd), "alter Vvdc=%f", config.amplitude_slider.value);
            printf("Simulation halted, altering voltage to %f...\n", config.amplitude_slider.value);
            ngSpice_Command(alter_cmd);
            
            printf("Resuming simulation...\n");
            ngSpice_Command("bg_resume");
            
            config.amplitude_slider.value_changed = false;
        }

        while (SDL_PollEvent(&e) != 0) {
            handle_events(&e, &config, &quit, &useInterpolation);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_signals(renderer, buffers, &config, useInterpolation);
        draw_grid(renderer, &config);
        draw_slider(renderer, &config.amplitude_slider);
        
        //SignalValues new_values = get_new_values(t, &config);
        //t += config.time_increment;

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Cap at roughly 60 FPS
    }

    cleanup_simulation(&context);
    cleanup(renderer, window, buffers, &config);
    return 0;
}
