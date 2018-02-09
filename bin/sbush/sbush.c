#include <stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<dirent.h>

#define COMMAND_BUFFER 2000
#define ARGUMENT_BUFFER 32
#define BG_PIDS_BUFFER 32
#define COMMAND_LENGTH 64
#define PIPE_BUFFER 16
#define LINE_BUFFER 64
#define ENV_VAR_LENGTH 256
int environ_length;
//char **environ_main;
int *bg_pids;
int bg_count = 0, bg_buffer = BG_PIDS_BUFFER;
short int background = 0;
short int pipe_flag = 0;
char *PS1_SBUSH;



int strcmp(char *str1,char *str2){
  int i=0;
  while(*(str1+i) != '\0' || *(str2+i) != '\0'){
    if(*(str1+i) == *(str2+i))
      i++;
    else{
      return *(str1+i) - *(str2+i);
    }
  }
  return 0;
}

char *strcpy(char *str1,const char *str2){
  int i=0;
  while((str1[i] = str2[i]) != '\0'){
    i++;
  }
  return str1;
}

void strcat(char *dest, char *src)
{
  (*dest)? strcat(++dest, src): (*dest++ = *src++)? strcat(dest, src): 0 ;
}

/*void exit(int rc){
    syscall((void*)(uint64_t)100,(void*)(int64_t)rc,0,0,0,0,0);
}

void free(void *ptr){
    syscall((void*)(uint64_t)89,(void*)ptr,0,0,0,0,0);
}

uint64_t malloc(uint64_t size)
{
    return (uint64_t)syscall((void*)(uint64_t)99,(void*)size,0,0,0,0,0);
}

pid_t fork(){
    return (uint64_t)syscall((void*)(uint64_t)1,0,0,0,0,0,0);
}

int execve(const char *file, char *const argv[], char *const envp[])
{
    return (ssize_t)syscall((void*)78,(void*)file,(void*)argv,(void*)envp,0,0,0);
    
}

int close(int fd)
{
    return (ssize_t) syscall((void*)3,(void*)(uint64_t)fd,0,0,0,0,0);
}

int waitpid(int pid, int *status,int options)
{
    return (ssize_t)syscall((void*)88,(void*)(uint64_t)pid,(void *)status,0,0,0,0);
}

int chdir(const char *path)
{
    
  return (ssize_t)syscall((void*)80,(void*)path,0,0,0,0,0);
    
}

void puts(char *buf){
  int count=0;
  while(buf[count]!='\0') count++;
  syscall((void*)(uint64_t)4,(void*)(uint64_t)1,(void*)buf,(void*)(uint64_t)count,0,0,0);
}

char getChar(){
    char c;
    syscall((void*)(uint64_t)0,(void*)(uint64_t)0,(void*)&c,(void*)(uint64_t)1,0,0,0);
    return c;
}*/

char *read_line(int fd){
  char *d = (char *)malloc(sizeof(char) * LINE_BUFFER);
  char *c = (char *)malloc(sizeof(char) * 1);
  int char_count = 0,char_buffer = LINE_BUFFER,rc;

  while(1){
    if(char_count >= char_buffer){
      char_buffer = char_buffer + LINE_BUFFER;
      //d=realloc(c,char_buffer);
    }
    if((rc =read(fd,c,1)) == -1){
      //printf("Error while reading file");
      exit(-1);
    }
    else if(rc>0){
      if(!(c[0] == '\n')){
        d[char_count] = c[0];
        char_count++;
      }
      else{
        d[char_count] = '\0';
        return d;
      }
    }
    else if(rc == 0){
      if(char_count > 0){
        d[char_count] = '\0';
        return d;
      }
      else
        return NULL;
    }
  }
  return d;
}


char *token(char *command, int x,int y){
  //printf("In Tokeninzing %d %d \n",x,y);
  char *argtemp = (char *)malloc(sizeof(char) * (y-x+2));//mmap(NULL,sizeof(char) * (y-x+2),PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
  if(!argtemp){
    //dedeprintf("Error: Memory Allocation failure in the sbu shell for the token buffer\n");
    exit(1);
  }
  int i=0;
  while(x<=y){
    //printf("Inside the while loop of tokenizing");
    argtemp[i++] = command[x++];
  }
  argtemp[i] = '\0';
  //printf("Argtemp in token function is: %s\n",argtemp);
  return argtemp;
}

int change_directory(char **com_args){
  if(*(com_args+1) == '\0'){
    puts("Error: expected argument to \"cd\"\n");
    return -1;
  }
  else{
    if (chdir(com_args[1]) != 0){
      //printf("COM ARGS of 1 is : %s",com_args[1]);
      //dedeprintf("Error executing cd command in sbush\n");
      return -1;
    }
  }
  return 1;
}


char **parse_command(char *command){
    char **com_args = (char **)malloc(ARGUMENT_BUFFER*sizeof(char*));
    int buffer = ARGUMENT_BUFFER;
    
    if(!com_args){
        exit(1);
    }
    
    int i,start,end,n;
    
    i=start=end=n=0;
    
    if (*command == '\0')
        return NULL;
    
    while(*(command+i) != '\0'){
        if(n>buffer){
            buffer = buffer+ARGUMENT_BUFFER;
        }
        if(command[i] != ' ' && command[i] != '\t' && command[i] != '\a' && command[i] != '\r' && command[i] != '\n'){
            end = i;
        }
        else{
            if((start>end) && (command[i] == ' ' || command[i] == '\t' || command[i] == '\a' || command[i] == '\r' || command[i] == '\n')){
                start++;
            }
            else if(end == 0)
                start++;
            else{
                //printf("Tokenizing");
                //printf("Command before tokenizing: %s\n",command);
                if(strcmp(com_args[n++] = token(command,start,end),"|") == 0){
                    pipe_flag = 1 ;
                    //printf("Pipe flag is set to true");
                }
                start=i+1;
                //printf("After Tokeninzing");
            }
        }
        i++;
    }
    if(start<=end){
        //printf("n before last arg: %d",n);
        //com_args[n] = token(command,start,end);
        //printf("n after com_args: %d, com_args[n] at last = %s",n,com_args[n]);
        if(strcmp(com_args[n] = token(command,start,end),"|") == 0 ){
            pipe_flag = 1;
        }
    }
    else{
        n--;
    }
    //printf("here1");
    if(strcmp(com_args[n],"&") == 0){
        background = 1;
        //printf("here2");
        com_args[n] = '\0';
    }
    else{
        //printf("here3");
        n++;
        com_args[n] = '\0';
    }
    return com_args;
}


char *read_command(){
    char *inputStr = (char*)malloc(2000);
    int i=0;
        
    while(1)
    {
        char c = getchar();
        if(c =='\0' && i>0) break;
        else if(c==' ' && i==0) continue;
        else if(i>2000) break;
        else if(c != '\0')
        {
          inputStr[i]=c;
          i++;
        }    
    }
    inputStr[i]='\0';
    
    return inputStr;
}

int fork_execution(char **com_args,short int background,char **envp){
  pid_t pid,wait_pid;
  int com_status;
  //bg_pids = malloc(sizeof(int) * BG_PIDS);
  pid = fork();
  
  if(pid == 0){
    if(background){
      //close(0);
    }
    if(execve(com_args[0],com_args,envp) == -1){
      //dedeprintf("Error while executing the command\n");
      //exit(1);//need to find out whether to use _exit or exit
    }
  }
  else if(pid < 0){
    //dedeprintf("Error while forking\n");//its good to use perror instead of printf to print the error.
  }
  else{
    if (background){
      wait_pid = waitpid(pid,&com_status,WNOHANG);
      if(wait_pid == -1){
        //puts("Error in waitpid\n");
        //exit(-1);
      }
    }
    else{
      do{ 
        wait_pid = waitpid(pid,&com_status,WUNTRACED);
        // if(wait_pid == -1) break;
      }while(!WIFEXITED(com_status));
      //}while(!WIFEXITED(com_status) && !WIFSIGNALED(com_status));
    }
  }
  return com_status;
}


int execute_command(char **com_args,short int background,char **envp){
  //printf("%s\n",com_args[0]);
  //printf("%s\n",com_args[1]);
  if(com_args == NULL){
    //printf("No Command Passed\n");
    return 1;
  }
  int rc = 0;
  if(strcmp(*com_args,"cd") == 0){
  rc = change_directory(com_args);
  }
  else if(strcmp(*com_args,"exit") == 0){
    //int x=0,y=0;
    if(*(com_args+1) == '\0'){
    //printf("Error: expected argument to \"exit\"\n");
    exit(0);
    }
    else{
      int x = 0, y=0;
      while(com_args[1][y] != '\0'){
        if((com_args[1][y] >= '0') && (com_args[1][y] <= '9')){
          x = x * 10 + (com_args[1][y] - '0');
          y++;
        }
        else{
          //dedeprintf("Not a valid return code");
          return -1;
        }
      }
      exit(y);
    }  
  }
  else if(strcmp(*com_args,"export") == 0){
    if(*(com_args+1) == '\0'){
      //dedeprintf("Error: expected argument to set Environmental variable\n");
      return -1;
    }
   else{
       env(com_args);
    }
  }
  else{
    rc =fork_execution(com_args,background,envp);
  }
  return rc;
}


int main(int argc, char *argv[], char *envp[]){

    if(argc >= 2){
    int fp = open(argv[1],O_RDONLY);
    char *line = read_line(fp);
    while((line = read_line(fp)) != NULL){ 
      char **com_args = parse_command(line);
      if(pipe_flag == 1){
        //pipe_execution(com_args,background);
      }
      else
        execute_command(com_args,background,envp);
      free(com_args);
      background = 0;
      pipe_flag = 0;
    }
    return 0;
    }
    
    while(1)
    {
        //read a input line
        
        char *ps1=getenv("PS1");
        //printf("Hi.. :D\n");
        //char *ps1=NULL;
        if(ps1==NULL){
          puts("sbush>");
        }
        else{
          puts(ps1);
          puts(">");
        }
        char *command = read_command();
        if(*command == '\0'){
          free(command);
          continue;
        }
        char **com_args = parse_command(command);
    
        if(pipe_flag == 1){
            //pipe_execution(com_args,background);
            ;
        }
        else{
            //environ_main=envp;
            execute_command(com_args,background,envp);
        }
        free(command);
        free(com_args);
        background = 0;//resetting this background flag for next command
        pipe_flag = 0;
    }
    
    while(1);
}
