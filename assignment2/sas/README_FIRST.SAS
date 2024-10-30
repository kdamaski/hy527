Compilation information
=======================

Log on to piranha96 by typing on shark: 
    "ssh piranha96"

Copy the SAS application directory to you account:

     "cp -r /home1/cs527/sas/application ."

In the directory of each program there is a Makefile. To compile go to
a directory holding the makefile and type: 

     "make"

Use the makefiles as templates for compiling your own applications.

General setup
============

You can use up to 8 cores in piranha96 for your experiments.

Running applications
====================

For example, to run FFT on two cores, type: 
fft -m10 -p2 -t
