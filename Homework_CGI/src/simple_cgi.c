#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1
#define FALSE 0


// Function prototypes
void PrintTable(FILE *fd, char *splitDelim, int maxColumns);

int main(int argc, char **argv) {
	FILE *fd =  NULL;
	int p=0;
	char buf[1024];

	printf( "<HTML>\n");
	printf( "<HEAD>\n");
	printf( "<TITLE>ps -ef cgi script</TITLE>\n");
	printf( "<STYLE>\n");
	printf( "table {border-collapse:collapse; \
			 table-layout:fixed; width:1400px;}\n");
	printf( "table td {border:solid 1px #fab; \
			 word-wrap:break-word;}\n");
	printf( "</STYLE>\n");
	printf( "</HEAD>\n");
	printf( "<BODY bgcolor=\"#dddddd\" text=\"#000000\">\n");

	printf( "<center>");
	printf( "<H1>This is a ps -ef cgi script written in C</H1>\n");
	printf( "</center>");

	printf( "<table border=1>\n");

		if( (fd = popen("ps -ef", "r")) == NULL) {
			perror("popen");
			exit(-1);
		}

		PrintTable(fd, " ", 8);

		pclose(fd);

	printf( "</table>\n");

	printf("</BODY>\n");
	printf("</HTML>\n");
	
	return 0;
}


void PrintTable(FILE *fd, char *splitDelim, int maxColumns)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	char *split, *saveptr;
	int numColumns = 1;
	int numRows = 1;

	while ((read = getline(&line, &len, fd)) != -1)
	{
		printf("<tr>");
		
		//line now contains the entire line.
		//Now, break it up by splitDelim

		char *split, *savePtr;
		numColumns = 1;
		do
		{
			if (numColumns == 1)
			{
				printf("<td>");
				split = strtok_r(line, splitDelim, &savePtr);
			}
			else if (numColumns == maxColumns)
			{
				// Print the first 7 columns based on spaces, 
				// then print the rest of the characters in
				// the 8th column
				printf("<td style=\"max-width: 700px; width:700px\">");
				split = strtok_r(NULL, "", &savePtr);
				if (numRows == 1)
				{
					printf("<H3><center>\n<b>\n%s \
							</b>\n</center>\n<H3>\n</td>\n", split);
				}
				else
				{
					printf("%s</td>\n", split);
				}
				break;
			}
			else
			{
				printf("<td>");
				split = strtok_r(NULL, splitDelim, &savePtr);
			}

			if (numRows == 1)
			{
				printf("<H3><center>\n<b>\n%s\n \
						</b>\n</center>\n</H3>\n</td>\n", split);
			}
			else
			{
				printf("%s</td>\n", split);
			}
			numColumns++;

		} while (split != NULL); // End while - columns

		printf("</tr>\n");

		numRows++;
	} // End while - rows
}
