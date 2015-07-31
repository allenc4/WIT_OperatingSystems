/****************************************************
 * Chris Allen
 * Assignment 1
 * 6/2/2015
 * 
 * Inputs: argv[1] - text file to read from
 *         argv[2] - text file to write to
 *         argv[3] - optional seek flag (SEEK_SET, SEEK_CUR, SEEK_END). SEEK_SET is default 
 * Outputs: writes input file backwards to argv[2] file
 * Description: This function reads an input file one byte at a time from the last character
 *              to the first character and prints it to the argv[2] output file. Can use three
 *              seperate SEEK flags based on argv[3] (SEEK_SET, SEEK_CUR, SEEK_END).
 ****************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main ( int argc, char *argv[]) {
	int i;
	int rr;
	int infile, outfile, filesize;
	char buffer[1];
        int seekFlag;

	if ( (argc !=4) && (argc != 3) )
        {
	    fprintf(stderr, "USAGE: %s inputFile outputFile {SEEK flag}. \n", argv[0]);
	    return(-1);
	}
        
        if (argc == 4) 
        {

            if (strcmp(argv[3],"SEEK_END") == 0) {
                seekFlag = 0;
            } else if (strcmp(argv[3],"SEEK_SET") == 0) {
                seekFlag = 1;
            } else if (strcmp(argv[3],"SEEK_CUR") == 0) {
                seekFlag = 2;
            } else {
                fprintf(stderr, "%s cannot be parsed. SEEK_SET selected as default lseek flag.\n", argv[3]);
                seekFlag = 1;
            }
        } else 
        {
            fprintf(stdout, "SEEK_SET selected as default lseek flag.\n");
            seekFlag = 1;
        } 

	if( (infile = open(argv[1], O_RDONLY)) == -1)	
	    return(-1);

	if((outfile = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0644))==-1)
        {
	    close(infile);
	    return (-2);
	}

	filesize=lseek(infile, (off_t)0, SEEK_END);

	for(i=filesize-1; i>=0; i--) 
        {

		/* 
		 * use lseek() to move the file pointer to the ith position
		 * To set the file pointer to a position use the SEEK_SET flag
		 * in lseek().
		 */
                
                switch (seekFlag) 
                {
                   case 0:
                      lseek(infile, (off_t)i-filesize, SEEK_END);
                      break;
                   case 2:
                      if (i == filesize - 1) {
                              lseek(infile, (off_t)-1, SEEK_CUR);
		      } else {
                      	      lseek(infile, (off_t)-2, SEEK_CUR);
                      }
                      break;
                   //Default case is SEEK_SET
                   default:
                      lseek(infile, (off_t)i, SEEK_SET);
                      break;
                }  
		
                rr = read(infile, buffer, 1);	/* read one byte */

		if( rr != 1 ) 
                {
	            fprintf(stderr, "Couldn't read 1 byte [%d]\n", rr);
		    return(-1);
		}

		rr = write(outfile, buffer, 1); /* write the byte to the file*/

		if( rr != 1 )
                {
		    fprintf(stderr, "Couldn't write 1 byte [%d]\n", rr);
		    return(-1);
		}
	} //end for loop

	close(infile);
	close(outfile);

        fprintf(stdout, "Successfully reversed %s, witten to %s.\n", argv[1], argv[2]);

	return(0);
}
