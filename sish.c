/*
	By: Sadia Khan
	To compile: gcc sish.c -o sish -Wall -Werror -std=c99 
	To execute: ./sish
	
*/




#define _GNU_SOURCE
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int const MAX_HIST = 100;
int k = 0, numPipes = -1;
int cmd=0; //pipe num
int whileRuns=0;
int arrayDeclare=0;

char* getInput(size_t length, char *history[MAX_HIST]);

void parseInput(char* userInput, char* args[], int argsNum[]);

void addToHistory(char* userInputCopy, char* history[MAX_HIST]);

void execCommands(char* args[]);

void allCommands(char *userInput, char *userInputCopy, char* args[], char* history[MAX_HIST], int argsNum[]);

void pipes(char *args[], int argsNum[]);

void err_exit(char *msg);

int
main(int argc, const char* argv[])
{
    

    //define and copy string variables
    size_t length = 0;
    char *userInput=(char*) malloc(sizeof(char)*(length+1));
    char *userInputCopy = (char*) malloc(sizeof(char)*(length+1));
    
    
    //history array
    char *history[MAX_HIST];
    
    
    //runs continious
    while (1){
        
        //display prompt
        printf("sish> ");
        
        
        
        //call user input
        userInput=getInput(length, history);

        
        //make copy so we can use without changing original
        userInputCopy = strdup(userInput);
        
        //exit command
        if ((strncmp(userInputCopy, "exit", 4))==0){
            exit(EXIT_SUCCESS);
        }
		
	    //arguments storage
	    char *args[arrayDeclare];
		int argsNum[arrayDeclare];
        
        
        
        //parse
        parseInput(userInput, args, argsNum);
		

        
        //pipes
        if(cmd!=0){
            pipes(args, argsNum);
            addToHistory(userInputCopy, history);
            
        }
		
        //history space is same as calling history
        else if(strcmp(userInputCopy, "history ")==0){
            userInput="history";
            userInputCopy = strdup(userInput);
            parseInput(userInput, args, argsNum);
            //copy to history
            addToHistory(userInputCopy, history);
            //call all commands
            allCommands(userInput, userInputCopy, args, history, argsNum);
        }
		
        //history offset
        else if((strncmp(userInputCopy, "history ", 8)==0) && strncmp(userInputCopy, "history -c", 10) != 0){
			//basically just seperating offset from history offset
		    char *index1;
            index1=strtok(userInputCopy, "history ");
            int index;
        	sscanf(index1, "%d", &index);
			
                
            //negative offset
            if(index < 0){
                printf("Index out of bounds. \n");
                //copy to history
                addToHistory(userInputCopy, history);
                
            }
            
            //if offset called history
            else if(index < k && index >= 0 && (strcmp(history[index], "history") == 0)){ 
				//call all commands
                allCommands(userInput, userInputCopy, args, history, argsNum);
                
            }
            
            //basically if history 3 points to history 1 and histoy 1 is like echo mo
            else if(index < k && index >= 0 && strncmp(history[index], "history ", 8) == 0){
                //copy to history
                addToHistory(userInputCopy, history);
                
                while(strncmp(history[index], "history ", 8) == 0){
                    char* index1;
                    index1=strtok(history[index], "history ");
                    sscanf(index1, "%d", &index);
                    userInput=history[index];
                    userInputCopy=userInput;
                }
                
                //call all commands
                parseInput(userInput, args, argsNum);


				
                allCommands(userInput, userInputCopy, args, history, argsNum);
                
            }
            
            //something elses
            else{
                //call all commands
                allCommands(userInput, userInputCopy, args, history, argsNum);
                
                //copy to history
                addToHistory(userInputCopy, history);
                
            }
            
            
        }
        //history or history clear
        else if ((strncmp(userInput, "history", 7) == 0 || strncmp(userInputCopy, "history -c", 10) == 0)){
			//copy to history
            addToHistory(userInputCopy, history);
            
            //call all commands
            allCommands(userInput, userInputCopy, args, history, argsNum);
            
            if(strncmp(userInputCopy, "history -c", 10) == 0){
                whileRuns=-1;
            }
            
            
        }
        
        //everything else
        else{
            //call all commands
            allCommands(userInput, userInputCopy, args, history, argsNum);
        
            //copy to history
            addToHistory(userInputCopy, history);
            
            
        }
    whileRuns++;
    }//while
}//main

//get user input
char* getInput(size_t length, char *history[MAX_HIST]) {
    char *userInput= (char*) malloc(sizeof(char)*(length + 1));
    char *temp= (char*) malloc(sizeof(char)*(length + 1));

    arrayDeclare=getline(&temp, &length, stdin);
	
    userInput = strtok(temp, "\n");
    
    //just hit enter
    while(userInput==NULL){
          printf("sish> ");
          //call user input
          userInput=getInput(length, history);
    }
    
    //if history 0 is first thing typed over and over
    while(strcmp(userInput, "history 0")==0 && whileRuns==0){
        addToHistory(userInput, history);
        printf("This index is empty\n");
        printf("sish> ");
        //call user input
        userInput=getInput(length, history);
    }
    
    
        
    return userInput;
}

//parses input and tokenizes it for spaces and pipes
void parseInput(char* userInput, char* args[], int argsNum[]) {
    //tokenize the input by space and pipes
    char *str1, *str2, *token, *subtoken;
    char *saveptr1, *saveptr2;
    int numSubTokens = 0, j, l = 0, compare;
    
    for (j = 0, str1 = userInput; ; l++, str1 = NULL) {
        numSubTokens = 0;

        token = strtok_r(str1, "|", &saveptr1);
        if (token == NULL)
            break;
        
        
        // don't count \n as a sentence
        compare = strcmp(token, "\n");
        if (compare == 0)
            break;
        
        numPipes++;

        for (str2 = token; ; j++, str2 = NULL) {
            subtoken = strtok_r(str2, " ", &saveptr2);
            if (subtoken == NULL)
                break;
            args[j] = subtoken;
            numSubTokens++;
        }
        argsNum[l] = numSubTokens;
        cmd=l;
        
    }
    
	//make sure last index is correct
    char *temp;
    temp = strtok(args[j-1], "\n");
    args[j-1] = temp;
    args[j] = NULL;
}

//adding userInput to history: if less than 100 just add to next index if equal to 100 then shif array then add
void addToHistory(char* userInputCopy, char* history[MAX_HIST]) {
    int l;
    
	//add to history is count is less than 100
    if (k < 100){
        history[k] = userInputCopy;
        k++;
        
    }
    else if (k == 100){
        //shift array
        for (l = 0; l < 99; l++) {
            history[l] = history[l+1];
        }
        history[99] = userInputCopy; //put command into history
    
    }
    
   
}

//all commands including built in and executable commands
//when called goes through and checks which command was called
void allCommands(char *userInput, char *userInputCopy, char* args[], char* history[MAX_HIST], int argsNum[]){
    //cd
    if (strncmp(userInput, "cd", 2) == 0){
        if (chdir(args[1])== -1){
            printf("Command not found.\n");
        }
    }
    
    //if we need to clear history reset array with counter
    else if (strncmp(userInputCopy, "history -c", 10) == 0){
        k=0;
    }
    
    //if history offset is called
    else if(strncmp(userInputCopy, "history ", 8)==0){
        char *index1;
        index1=strtok(userInputCopy, "history ");
        int index= atoi(index1);
        
        if(index > k || index < 0){
            printf("Index out of bounds. \n");
        }
        
        else if(strcmp(history[index], "history") == 0){
            addToHistory(userInputCopy, history);
            
            int s;
            for (s=0; s<k; s++){
                printf("%d %s \n",s, history[s]);
            }
            
        }
        
        else{
            userInput = strdup(history[index]);
            userInputCopy = strdup(userInput);
            parseInput(userInput, args, argsNum);
            allCommands(userInput, userInputCopy, args, history, argsNum);
            
        }
    }
    
	//print out history
    else if (strncmp(userInput, "history", 7) == 0) {
        
        int s;
        for (s=0; s<k; s++){
            printf("%d %s \n",s, history[s]);
        }
        
    }
    
	//executable commands
    else{
        execCommands(args);
        
    }
    
    
    
}

//executable commands
void execCommands(char* args[]){
    //Run the executable command
    pid_t cpid1 = fork();
    
    if (cpid1 < 0) {
        printf("fork cpid1 failed");
    }
    if (cpid1 == 0) {
        
        execvp(args[0], args);
        err_exit("Command not found.");
    }
    
    waitpid(cpid1, NULL, 0); //wait for child to terminate
    
}

//commands that involve pipes; works for any number of pipes
void pipes(char *args[], int argsNum[]){
    
    //create two pipes
    int fd1[2], fd2[2];
    pid_t cpid1=0, cpid2=0, cpid3=0;
    int w=0;
    int runs=-1;
    
    while(args[w]!=NULL){
        runs++;
        //first
        if (w==0){
            if (pipe(fd1) == -1)
                err_exit("pipe1");
    

            cpid1 = fork();
            if (cpid1 < 0){
                err_exit("fork cpid1 failed");
            }
            if (cpid1 == 0){ // child 1
                close(fd1[0]); // not reading from pipe
                if (dup2(fd1[1], STDOUT_FILENO) == -1){
                    err_exit("dup2 failed in Child 1");
                }
                close(fd1[1]); // write end is no longer required because dup2
                
                int n=argsNum[runs];
                char* myargs[n+1];
        
                for (int i=0; i<n; i++){
                    myargs[i]=args[i];
                }
                myargs[n]=NULL;
        
                execvp(myargs[0], myargs);
                
                err_exit("execvp failed in Child 1");
            }
            
            //update w here becuase can't update in child
            w=argsNum[runs]+w;


        }
        
        //last command if only one pipe needed
        else if ((runs==cmd)&&(runs==1)){
            // only parent gets here
            cpid2 = fork();
            if (cpid2 < 0){
                err_exit("fork cpid1 failed");
            }
            if (cpid2 == 0){
                close(fd1[1]); // wc doesnt write to pipe
                if(dup2(fd1[0], STDIN_FILENO) == -1){
                    err_exit("dup2 in child 2 failed");
                }
                close(fd1[0]); // no longer required because dup2
                
				//getting correct arguments and command
                int n=argsNum[runs];
                char* myargs[n+1];
                int new=argsNum[runs-1];
        
                for (int i=0; i<n; i++){
                    myargs[i]=args[new];
                    new++;
            
                }
        
                myargs[n]=NULL;
                
                execvp(myargs[0], myargs);
                err_exit("execvp failed in Child 2");
            }
            
            w=argsNum[runs]+w;
            close(fd1[0]);
            close(fd1[1]); // parent no longer needs
            waitpid(cpid1, NULL, 0); // wait for child 1 to terminate
            waitpid(cpid2, NULL, 0); // wait for child 2 to terminate
            
        }
        //middle commands
        else if((args[w-1] != NULL) && (runs+1<=cmd)){
            if (pipe(fd2) == -1)
                err_exit("pipe2");

            cpid2 = fork();
            if (cpid2 < 0){
                err_exit("fork cpid2 failed");
            }
            
            if (cpid2 == 0){
                if(dup2(fd1[0], STDIN_FILENO) == -1){ //reading from pipe
                    err_exit("dup2 in child 2 failed (READ)");
                }
                close(fd1[0]); // no longer required because dup2
    
                if(dup2(fd2[1], STDOUT_FILENO) == -1){ //redirecting our output to write end of new pipe
                    err_exit("dup2 in child 2 failed (WRITE)");
                }
                close(fd2[1]); // no longer required because dup2
    
                //close additional pipes
                close(fd1[1]);
                close(fd2[0]);
                
                
				//getting correct command and arguments
                int n=argsNum[runs];
                char* myargs[n+1];
        
                for (int i=0; i<n; i++){
                    myargs[i]=args[w];
                    w++;
                }
        
                myargs[n]=NULL;
            
    
                execvp(myargs[0], myargs);
                err_exit("execvp failed in Child 2");
            }
            close(fd1[0]);
            close(fd2[1]);
            
            w=argsNum[runs]+w;
            
			//checking if more pipes and if there are then redirct output to next pipe
            if(runs+1 != cmd){
                fd1[0]=fd2[0];
            }
        }
        
		//last command
        else if (runs==cmd){
            
            //done with pipe 1
            close(fd1[0]);
            close(fd1[1]);
            
            // only parent gets here
            cpid3 = fork();
            if (cpid3 < 0){
                err_exit("fork cpid3 failed");
            }
            if (cpid3 == 0){
                close(fd2[1]); // no longer required because no more writing
                if(dup2(fd2[0], STDIN_FILENO) == -1){ //reading from pipe
                    err_exit("dup2 in child 3 failed (READ)");
                }
                close(fd2[0]); // no longer required because dup2
                
                int n=argsNum[runs];
                char* myargs[n+1];
        
                for (int i=0; i<n; i++){
                    myargs[i]=args[w];
                    w++;
                }
                
                myargs[n]=NULL;
                
                
                execvp(myargs[0], myargs);
                err_exit("Command not found.");
            }
            //close second pipe
            close(fd2[0]);
            close(fd2[1]);
            waitpid(cpid1, NULL, 0); // wait for child 1 to terminate
            waitpid(cpid2, NULL, 0); // wait for child 2 to terminate
            waitpid(cpid3, NULL, 0); // wait for child 3 to terminate
            
            w=argsNum[runs]+w;
        }

    }

}

//errors message 
void err_exit(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}