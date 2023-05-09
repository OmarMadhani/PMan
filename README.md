# Process Manager (PMan)

Name: Omar Madhani

PMan allows you to start and manage multiple processes at a time.
Even if a process is running, PMan will continue to accept user input.

Ensure that PMan.c, PMan.h, and the Makefile are all in the same directory.
To compile PMan.c and create an executable, simply type "make" in your command line (within the same directory).
To run the program, type "./PMan"

Congrats! You've launched PMan.

(To quit PMan and terminate any processes created by it, type and enter "q")

To start a background process, type "bg" followed by the process name. 
For example, if I wanted to run a program called test, I would type bg ./test
PMan assumes that you already have created an executable for the program.
PMan will indicate that a process has been created by stating "Process (pid) created".

PMan also supports the execution of linux commands with options. Make sure to type "bg" before the linux command.
Ex) bg ls -l -a

To view a list of current background processes, type "bglist".
You will see the process ID and file path of each executable.
You will also see the total number of background jobs.
If a process has terminated, you will get an indication the next time you type in bglist.

You can temporarily stop a process by typing "bgstop" followed by its process ID.
Ex) bgstop 12345

Alternatively, you can resume a process by typing "bgstart" followed by its process ID.
Ex) bgstart 12345

You can kill a process by typing "bgkill" followed by its process ID.
Ex) bgkill 12345

PMan allows the user to obtain information about a process.
(comm, state, utime, stime, rss, voluntary context switches, nonvoluntary context switches)
Type "pstat" followed by a process ID to view this info.

If the process does not exist or you do not provide a process ID, an error message will be displayed.

Thank you!
