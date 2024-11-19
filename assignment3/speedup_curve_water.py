import matplotlib.pyplot as plt

# Number of threads or processes
procs_t = [1, 2]

speedup_threads = [1.0, 1.82]      # Speedup with threads

# Input size example (e.g., NxN matrices)
input_size = '1e-15'
num_locks = '4096'

# Plot the speedup curves
plt.figure(figsize=(10, 6))
plt.plot(procs_t, speedup_threads, 'o-', label=f'Threads (Molecules {input_size}, Locks {num_locks})', color='blue')

# Plot settings
plt.xlabel('Number of Threads (using mcslocks)')
plt.ylabel('Speedup')
plt.title(f'Speedup Curves for WaterNsquared Splash2 Benchmark ({input_size})')
plt.legend(loc='upper left')
plt.grid(True)
plt.xticks(procs_t)  # Ensure x-axis shows all process/thread counts

# Show the plot
plt.show()
