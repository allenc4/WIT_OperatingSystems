#include <stdio.h>
#include <string.h>

#include <unistd.h>      // for exec and dup2 commands
#include <stdlib.h>      // for exit() command

int main(int argc, char *argv[]) 
{
   int pfd[2];
   int pfd2[2];
   int pid, pid2;
   char uid[10];
   
   if (argc == 2)
   {
      printf("Searching for processes for user id: %s\n", argv[1]);
      strcpy(uid, argv[1]);
   }
   else
   {
      printf("Searching for processes for user id: 1041\n");
      strcpy(uid, "1041");
   }
 

   // create the first pipe and fork for ps -ef and grep USER_ID
   if (pipe(pfd) == -1)
   {
      perror("pipe failed");
      exit(-1);
   }

   if ((pid = fork()) < 0)
   {
      perror("fork failed");
      exit(-1);
   }

   if (pid == 0)
   {
      // In child process
      // Create another pipe and fork for wc
      
         //pfd[0] stores output from ps-ef. Change stdin descriptor (0) to pfd[0].  
         close(pfd[1]);
         dup2(pfd[0], 0);
         close(pfd[0]);

      if (pipe(pfd2) == -1)
      {
         perror("pipe failed");
         exit (-1);
      }
       
      if ((pid2 = fork()) < 0)
      {
         perror("fork failed");
         exit(-1);
      }      
      
      if (pid2 == 0)
      {
         // In 2nd child process
         //pfd2[0] stores ouput from ps-ef | grep userID. Change stdin descriptor (0) to pfd2[0].
         close(pfd2[1]);
         dup2(pfd2[0], 0);
         close(pfd2[0]);
         
         execlp("wc", "wc", (char *)0);
         perror("wc failed");
         exit(-1);
      }
      else
      {
         // In 1st child process
         //write output from grep to pfd2[1] (change stdout descriptor (1) to pfd2[1])  
         close(pfd2[0]);
         dup2(pfd2[1], 1);
         close(pfd2[1]);
         
         execlp("grep", "grep", uid, (char *)0);
         perror("grep failed");
         exit(-1);
      }
   }
   else
   {   
      // In parent process. Output "ps -ef" to pfd[1]
      close(pfd[0]);
      dup2(pfd[1], 1); //descriptor 1 points to pfd[1] instead of stdout
      close(pfd[1]); 

      execlp("ps", "ps", "-ef", (char *)0); //write to descriptor 1
      perror("ps command failed");
      exit(-1);
   }       

   return 0;
}
