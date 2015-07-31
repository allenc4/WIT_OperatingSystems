/***************************************************************************************
 * Chris Allen                                                                         *
 * 6/9/2015                                                                            *
 * Assignment 2                                                                        *
 *                                                                                     *
 * Description: Implementation of UNIX cp command (ignoring any parameters the actual  *
 * cp command accepts. Only uses the following system calls: open, read, write, close. *
 * Inputs: argv[1] as the source file to copy, argv[2] as the location to copy to.     *
 * Outputs: makes a dirrect copy of file at argv[1] to destination argv[2].            *
 * Usage: executable file_to_copy_source destination_to_copy_to
 *        executable list_of_files_to_copy destination_directory
 ***************************************************************************************/

#include <stdio.h>
#include <fcntl.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[])
{
   int i, readValue = 0, writeValue = 0;
   int inFile, outFile, numArgs, startArgs;
   char buffer[256];
   char outFileName[512];
   int copyToDirectory = FALSE;
   
   struct stat s;
   int err;

   err = stat(argv[argc-1], &s);
   //Check to see if argument exists (file or directory)...
   if (err < 0)
   {
      //Directory (or file) does not exist
      copyToDirectory = FALSE;
      numArgs = argc;
   }
   else
   {
      //Check to see if argument is a directory or not
      if (S_ISDIR(s.st_mode))
      {
         //Is a directory
         copyToDirectory = TRUE;
         numArgs = argc - 1;
      }
      else
      {
         //Is a file
         copyToDirectory = FALSE;
         numArgs = argc;
      }
   }
   
   if ((copyToDirectory == FALSE) && (argc != 3))
   {
      fprintf(stderr, "Usage: %s input_file output_file\nor\nUsage: input_file1 input_file2 ... directory_to_copy_to\n", argv[0]);
      return(-1);
   }

   //read the file, size of the buffer allocation (256 bytes) at a time
   for (i = 1; i < argc - 1; i++)
   { 
      if ((inFile = open(argv[i], O_RDONLY)) == -1)
      {
         perror("Input file");
         return (-1);
      } 

      //If copying files to directory, get name of output file
      if (copyToDirectory == TRUE)
      {
          strcpy(outFileName, argv[argc-1]);
          strcat(outFileName, "/");
          strcat(outFileName, argv[i]);           
          printf("%s\n",outFileName);
       }
       else
       {
          strcpy(outFileName, argv[i+1]);
       }
       
       //Check to see if we can open the output file for write-only
       if ((outFile = open(outFileName, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
       {
          perror("Output file");
          close(inFile);
          return(-1);
       }

       do
       {    
          readValue = read(inFile, buffer, sizeof(buffer)); //read size of buffer
          if (readValue < 0)
          {
             perror("Read error");
             close(inFile);
             return(-1);
          }
 
          writeValue = write(outFile, buffer, readValue); //write size of readValue
          if (writeValue < 0)
          {
             perror("Write error");
             close(inFile);
             close(outFile);
             return(-1);
          } 
       } while (readValue == sizeof(buffer));
   
   }  //exit for loop

   close(inFile);
   close(outFile);

   return 0;
}
