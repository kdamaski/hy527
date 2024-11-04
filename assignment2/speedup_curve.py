import matplotlib.pyplot as plt

# Sample data (replace these with your actual timing data)
# Number of threads or processes
procs_t = [1, 2, 3, 4, 5, 6, 7, 8]
procs_mpi_1 = [1, 2, 3, 4]
procs_mpi_2 = [2, 4, 6, 8]

# Speedup data (calculate these from your timing data)
# Replace with actual computed speedup values for each configuration and input size
speedup_threads = [1.0, 1.75, 2.333, 2.625, 3.5, 4.2, 4.666, 4.2]      # Speedup with threads

speedup_mpi_1 = [1.0, 2.0, 3.0, 4.0]   # Speedup with MPI_one_process_per_node
speedup_mpi_2 = [1.0, 1.7778, 2.5, 2.8]   # Speedup with MPI_one_process_per_node

# Input size example (e.g., NxN matrices)
input_size = '2000x2000'

# Plot the speedup curves
plt.figure(figsize=(10, 6))
plt.plot(procs_t, speedup_threads, 'o-', label=f'Threads (Matrix {input_size})', color='blue')
plt.plot(procs_mpi_1, speedup_mpi_1, 's-', label=f'MPI single process per node(Matrix {input_size})', color='green')
plt.plot(procs_mpi_2, speedup_mpi_2, '^-', label=f'MPI two process per node(Matrix {input_size})', color='red')

# Plot settings
plt.xlabel('Number of Threads / Processes')
plt.ylabel('Speedup')
plt.title(f'Speedup Curves for Matrix Multiplication ({input_size})')
plt.legend(loc='upper left')
plt.grid(True)
plt.xticks(procs_t)  # Ensure x-axis shows all process/thread counts
plt.ylim(1, max(max(speedup_threads), max(speedup_mpi_1), max(speedup_mpi_2)) * 1.1)

# Show the plot
plt.show()
