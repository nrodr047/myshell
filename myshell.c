/*
Student Name: Natalie Rodriguez
Panther ID: 2919853

Program Description: Program is a shell made to handle I/O redirection with the use of pipelines and multi-processes.
The program is able to execute input, output, append, and pipe redirections by using the symbols > < >> |. 
Program runs with unix commands as expected.

Statement of Affirmation: I affirm that I completed this program by myself with no help from outside resources.

 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 1024


//Function reads userinput
char* getInput() 
{
 	char* input = malloc(sizeof(char) * BUFSIZE);
 	int i = 0;
  	char c;

	//loop to retrieve user input
	while((c = getchar()) != '\n' && c != EOF) 
	{
    		input[i] = c;
    		i++;

		if(i >= BUFSIZE - 1) 
		{
			perror("Error: input too large.\n");
			exit(-1);
		}
  	}
	//last is always NULL
 	 input[i] = '\0';
 	 return input;
}

//Function separates and tokenizes commands
//@param is user input from commandline
char** getToken(char* input) 
{
	int i = 0;
  	char** args = malloc(sizeof(char) * BUFSIZE);
  	char* arg = strtok(input, " ");
	
	//loops through arguments, separates arguments, and tokenizes
  	while(arg != NULL) 
	{
    		args[i] = arg;
    		i++;
    		arg = strtok(NULL, " ");
  	}
 
	 args[i] = NULL;
	 return args;
}

//Function finds I/O redirection
//raises flags for > >> < | 
//creates processes for I/O redirection according to flag
//@param is tokenized arguments
int execute(char** args) {
	int inFlag = 0; //redirect input flag
  	int outFlag = 0; //redirect output flag
 	int i, j;
	int q = 1; //quit when 0
  	int stdinDup, stdoutDup, infile, outfile;

  	//if there are no arguments
  	if(args[0] == NULL)
	{
		perror("No arguments found.");
    		exit(-1);
  	}

  	i = 0;
 	while(args[i] != NULL) 
	{
		//if input is found, open file with proper permissions
		//raise input flag
  		if(strcmp(args[i], "<") == 0) 
		{
      			infile = open(args[i+1], O_RDONLY);
      			args[i] = NULL;
      			inFlag = 1;
    		}
		//if output is found, open file with proper permissions
		//output flag is raised
   		else if(strcmp(args[i], ">") == 0) 
		{
      			outfile = open(args[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | O_CLOEXEC);
     			args[i] = NULL;
      			outFlag = 1;
    		} 
		//if append flag is found, open file with proper permissions
		//append flag is raised
    		else if(strcmp(args[i], ">>") == 0) 
		{
      			outfile = open(args[i+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | O_CLOEXEC);
      			args[i] = NULL;
      			outFlag = 1;
    		}
		//if a pipe is found, set arg to null
		//create a pipe of before and after arguments
    		else if(strcmp(args[i], "|") == 0) {
      			args[i] = NULL;
      			q = executePipe(&args[0], &args[i+1]);
      			return q;
    		}
    		i++;
  	}//WHILE LOOP ENDS

	//execute commands with child/parent
	pid_t pid;
    	pid = fork();

	//if fork fails, exit error
    	if(pid < 0) 
	{
     		perror("Fork failed.\n");
		exit(-1);
    	}
    	else if(pid == 0) 
	{
      		//children process for redirect I/O according to flag
      		if(inFlag == 1) 
		{
			//duplicate for stdin file
        		dup2(infile, STDIN_FILENO);
      		}
      		if(outFlag == 1) 
		{
			//duplicate for stdout file
        		dup2(outfile, STDOUT_FILENO);
      		}
      		//exec command
      		execvp(args[0], args);
      		//if error
     		perror("exec failed.\n");
		exit(-1);
    		}
    	else 
	{
      		//parent process
      		//wait for dead child
        	waitpid(pid, NULL, 0);
    	}

  return q;
}

//Function creates a pipe based on the commandline arguments
//@param are the arguments before and after the '|' pipe symbol
int executePipe(char** args1, char** args2) 
{
 	pid_t pid1, pid2;
  	int fd[2];
  	//create pipe
  	pipe(fd);

  	//fork to exec first program
  	pid1 = fork();
  	
	//if fork fails, exit error
	if(pid1 < 0) 
	{
    		perror("Fork failed.\n");
		exit(-1);
  	}
  	else if(pid1 == 0) 
	{
    		//stdout to write end of pipe
    		dup2(fd[1], STDOUT_FILENO);
    		//close read end of pipe
    		close(fd[0]);
    		execute(args1);
    		exit(0);
  	}
  	else 
	{
    		//returned to parent process
    		//fork to exe second program
    		pid2 = fork();
    		if(pid2 < 0) 
		{
     			 perror("Fork failed.\n");
    		}
    		else if(pid2 == 0)
		{
      			//send stdin to read end of pipe
      			dup2(fd[0], STDIN_FILENO);
      			//close write end of pipe
      			close(fd[1]);
      			execute(args2);
      			exit(0);
    		}
   		else 
		{
     			//parent process
      			//close the pipes
      			close(fd[0]);
      			close(fd[1]);
      			//wait for dead children
      			waitpid(pid1, NULL, 0);
      			waitpid(pid2, NULL, 0);
    		}
  	}

  return 1;
}


//Main function: the shell
int main(int argc, char*argv[]) 
{
  	int a = 1; //return value of executed program
  	while(a != 0) 
	{
    		printf("COP4338$ "); //prompt
    		char* input = getInput(); //get's user input
   		char** inputArray = getToken(input); //tokenize command arguments
   		a = execute(inputArray); //execute commands
  	}
  return 0;
}
