#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

int foreground, c2pid;

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


//Killing all processes in exit command  (Call from Exit block)

void child_killer(int pidlist[],int array_pointer){		
	
	while(array_pointer>=0){
		kill(pidlist[array_pointer],SIGUSR1);
		array_pointer--;		
	}
}

//  This is Control C signal handler //

void controlC_handler(int sig){
	if(foreground!=0){
		kill(foreground, SIGINT);
		printf("\n");
		foreground=0;
	}
}

// Killing all Backgrounds C2  , Call from Child killer , hence from exit block

void c2killer_handler(int sig) {
	kill(c2pid, SIGKILL);
}

void c1killer_handler(int sig) {  			// Parent is here for execution 
	// c1 signals parent to reap
	sleep(1);					//So that scheduler does not context switch after kill signal,at siguser2 signal and does not exit so we wait
	while(waitpid(-1, NULL, WNOHANG) > 0);	//reaps C1
}


int main(int argc, char* argv[]) {
	signal(SIGUSR1, c2killer_handler);
	signal(SIGUSR2, c1killer_handler);				
	signal(SIGINT,controlC_handler);			//Control C signal here
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;
	int array_pointer=0;
	int pid_list[100];    		//Since there are no more than 64 processes running at a time


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   
       //****************************************Code Starts here*************************************************//

		// To continue if input is not given

		if(tokens[0] == NULL){ 
			continue;
		}

		//Exit Block 

		if(strcmp(tokens[0],"exit")==0){	//Killer for exit
			child_killer(pid_list,array_pointer-1);

			// Freeing the allocated memory	
			 for(i=0;tokens[i]!=NULL;i++){
			 	free(tokens[i]);
			 }
			 free(tokens);

			usleep(5000);						//To print bg ended first and then exit 
			exit(0);							//shell exits


		}

		if(strcmp(tokens[0],"cd") == 0){
			if(tokens[2]==NULL){					//Checking for too many arguments
				int x=chdir(tokens[1]);

				if(x==-1)
				perror("");							//error message if any (No such file one)

				continue;
			}
			else{
				printf("too many arguments\n");		
				continue;
			}
		}

	   	int background_checker=0;
	   	while(tokens[background_checker++]!=	NULL);  // (i-2) contains &

	   	pid_t pid = fork();
	   
	   	if(pid == 0) {     //Enters child process


			if(tokens[background_checker-2][0] == '&'){   // Checks for background execution and removes '&'  , to avoid giving & as parameter

					setpgid(getpid(),getpid());		// Process set to same group for background only
 
					c2pid = fork();

					if(c2pid==0){
						tokens[background_checker-2]=NULL;
						execvp(tokens[0], tokens);
						exit(EXIT_FAILURE);
					}
					else{
						int wstatus;				
						wait(&wstatus);				//reaping the inception process , c2 ended here after wait by c1
						printf("Shell: Background process finished\n");
						kill(getppid(), SIGUSR2);  	// here C1 sends signal to parent(getppid) for killing it through sigUser2
						exit(0);					// C1 exits here 
					}
				
				
			}
			else{							// Normal Process Execution
				execvp(tokens[0], tokens);
				printf("Command not found\n");
				exit(EXIT_FAILURE);
			}
	   	} 
	   	else {                         	// Enters Parent
			int wstatus;
			if(tokens[background_checker-2][0]== '&'){   //Does'nt wait for processes by Wnohang and proceeds further
				pid_list[array_pointer++]=pid;
			}
			else{
				foreground=pid;							//Globally declared to Use this in Control C
				waitpid(pid, &wstatus, 0);
			}

			
	   	}

		//****************************************Code Ends here*************************************************//

       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}

		free(tokens);

	}
	return 0;
}
