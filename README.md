# Simple_Shell
Built a simple shell to execute user commands, much like the bash shell in Linux. 

How to run this program
    1) On your command prompt , have to give executable permissions for run.sh 
        command --- chmod +x run.sh

    2)Now just type ./run.sh


What this program can do 

1) It can run simple commands like ls, cat, echo and sleep etc. 
    These are implemented using fork commands and wait to reap them using parent process.
    
2) Background Execution is also supported by this program , reaping logic for child processes is also implemented 
3) Also exit command , stops the shell and exits from the process
4) The Ctrl+C signal , ends the foreground process imidiately 
    Keeping all the background processes running in the background.
    
One can check all the processes spawn by shell including background and foreground , by htop command
    


