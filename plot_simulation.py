import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
df = pd.read_csv('simulation_data.csv')

# Create the plot
plt.figure(figsize=(10, 6))
plt.plot(df['Time'], df['k'], label='Voltage at node k')
plt.plot(df['Time'], df['y'], label='Input voltage (y)')

# Customize the plot
plt.title('Circuit Simulation Results')
plt.xlabel('Time (s)')
plt.ylabel('Voltage (V)')
plt.grid(True)
plt.legend()

# Save the plot
plt.savefig('simulation_plot.png')
plt.close()
