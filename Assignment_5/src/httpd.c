/////////////////////////////////////////////////////////////////////////
//
// To compile: 			gcc -o my_httpd my_httpd.c -lnsl -lsocket
//
// To start your server:	./my_httpd 2000 .www			
//
// To Kill your server:		kill_my_httpd
//
/////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <stdlib.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <signal.h>
#include <stdbool.h>

#define TRUE            1
#define FALSE			0

#define	ERROR_FILE	    -1
#define FILE_NOT_FOUND   0
#define REG_FILE         1
#define EXECUTABLE_FILE  2
#define DIRECTORY        3


typedef struct {
	char *HTTP_Type;    //ie. GET, POST
	char *versionNum;   //HTTP version number
	char *contentType;  //Content type requested (text/html, image/webp, ...)
	char *resultCode;   //Three digit computer-readable status code 
	char *status;       //Human-readable short status description
} HTTP_Response;


//Function prototypes
void GetMyHomeDir(char *myhome, char **environ);
int  TypeOfFile(char *fullPathToFile);
void SendDataBin(char *fileToSend, int sock, char *home, 
		         char *content, HTTP_Response *response);
void ExtractFileRequest(char *req, char *buff, HTTP_Response *response);
char *last_char(char *str);
char first_char(char *str);
void setupHeader(char *Header, HTTP_Response *response,
	             long unsigned filesize);


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void GetMyHomeDir(char *myhome, char **environ) {
        int i=0;
	while( environ[i] != NULL ) {
        	if( ! strncmp(environ[i], "HOME=", 5) ){
			strcpy(myhome, &environ[i][5]);
			return;
		}
		i++;
        }
	fprintf(stderr, "[FATAL] SHOULD NEVER COME HERE\n");
	fflush(stderr);
	exit(-1);
}


/** 
 * Tells us if the request is a directory or a regular file.
 *
 * @Param fullPathToFile
 * @Return one of the following:
 *		FILE_NOT_FOUND, REG_FILE, EXECUTABLE_FILE, DIRECTORY, ERROR_FILE
 */ 
int TypeOfFile(char *fullPathToFile) {
	struct stat buf;	/* to stat the file/dir */
	
	//First check if file/directory exists and is readable
	if (access(fullPathToFile, R_OK) < 0)
	{
		return FILE_NOT_FOUND;	
	}

	if (stat(fullPathToFile, &buf) != 0 ) 
	{
		perror("stat()");
		fprintf(stderr, "[ERROR] stat() on file: |%s|\n", 
						fullPathToFile);
		fflush(stderr);
                exit(-1);
    }


	if (S_ISREG(buf.st_mode))
	{
		// Check if the file is executable by the owner
		if (buf.st_mode & S_IEXEC)
		{
			return EXECUTABLE_FILE;
		}
		else
		{
			return REG_FILE;
		}
	}
	else if (S_ISDIR(buf.st_mode))
	{
		return DIRECTORY;
	}

	return(ERROR_FILE);
}

/**
 * Returns last character in string, or \0 if its empty.
 */
char *last_char(char *str)
{
	int len = strlen(str);
	return len > 0 ? str + len - 1 : str;
}

/**
 * Returns first character in string, or \0 if its empty.
 */
char first_char(char *str)
{
	char t = *str;
	return t;
}

/**
 * Sends the HTTP Response along with the web
 * content to the socket file descriptor.
 *
 * @param fileToSend: location of file to display to client
 * @param sock: Socket file descriptor
 * @param home: 
 * @param content:
 * @param response: Holds information regarding HTTP request
 */
void SendDataBin(char *fileToSend, int sock, char *home, 
		char *content, HTTP_Response *response) {
    
	char *fullPathToFile; //[256];
	char Header[1024];
	char buffer[256];
	char *endChar;
	int size;

	bzero(Header, sizeof(Header));

	/*
	 * Build the full path to the file
	 */

	bool hasSlash;
	if ((last_char(content) == "/") || (first_char(fileToSend) == '/'))
		hasSlash = TRUE;
	else
		hasSlash = FALSE;
	
	size = strlen(home) + strlen(content) + strlen(fileToSend);
	if (hasSlash)
	{
		size += 2;
		fullPathToFile = malloc(size);
		sprintf(fullPathToFile, "%s/%s%s", home, content, fileToSend);
	}	
	else
	{
		size += 3;
		fullPathToFile = malloc(size);
		sprintf(fullPathToFile, "%s/%s/%s", home, content, fileToSend);
	}
	//int file_open = open(fullPathToFile, O_RDONLY);
	int fileType = TypeOfFile(fullPathToFile);

	printf("Filetype: %d\n", fileType);

	if ((fileType == FILE_NOT_FOUND) || (fileType == ERROR_FILE))
	{
		//Requested page not available.
		bzero(fullPathToFile, strlen(fullPathToFile));
		size = strlen(home) + strlen(content);

		if (last_char(content) == "/")
		{
			size += 16;
			fullPathToFile = realloc(fullPathToFile, size);
			sprintf(fullPathToFile, "%s/%snot_found.html", home, content);
		}
		else
		{
			size += 17;
			fullPathToFile = realloc(fullPathToFile, size);
			sprintf(fullPathToFile, "%s/%s/not_found.html", home, content);
		}
	}
	else if (fileType == DIRECTORY)
	{
		//Append index.html
		size += 10;
		fullPathToFile = realloc(fullPathToFile, size);
		strcat(fullPathToFile, "index.html");
	}
	
	printf("File to open: %s\n", fullPathToFile);
	int file_open;
	FILE *file_fd = NULL;

	if (fileType == EXECUTABLE_FILE)
	{
		if ((file_fd = popen(fullPathToFile, "r")) == NULL) 
		{
			perror("popen");
			exit(-1);
		}

		file_open = fileno(file_fd);  //Convert from FILE to file descriptor integer
	}
	else
	{
		file_open = open(fullPathToFile, O_RDONLY);
	}

	if (file_open < 0)
	{
		perror("Open file error");
		exit(-1);
	}

	long unsigned filesize  = lseek(file_open, (off_t)0, SEEK_END);
	lseek(file_open, (off_t)0, SEEK_SET); //Set back to starting position
	
	setupHeader(Header, response, filesize);

	printf("\nHeader:\n%s\n\n", Header);	

	/*
	 * Send the header, open the requested file.
	 * Send requested file, close file.
	 */

	printf("file: %s\n", fullPathToFile);
	write(sock, Header, strlen(Header));

	// If request was for HEAD, do not send the body information
	if (strncmp(response -> HTTP_Type, "HEAD", 4) == 0)
	{
		free (fullPathToFile);
		fullPathToFile = NULL;
		return;
	}


	bzero(buffer, sizeof(buffer));
	while (read(file_open, buffer, sizeof(buffer)) > 0)
	{
		write(sock, buffer, sizeof(buffer));
		bzero(buffer, sizeof(buffer));
	}
	
	if (fileType == EXECUTABLE_FILE)
		pclose(file_fd);
	else
		close(file_open);

	// Deallocate allocated pointers
	free (fullPathToFile);
	fullPathToFile = NULL;
}


/**
 * Reads and parses the file request lines received from the client. 
 * @param req: location where this method writes the requested 
 *             filename or directory.
 * @param buff: buffer where the HTTP request is stored.
 * @param response: instance of Struct HTTP_Response which this method will
 *				    populate based on the HTTP request in buff.
 */
void ExtractFileRequest(char *req, char *buff, HTTP_Response *response ) {

	int lastPos = (int)(strchr(buff, '\n') - buff) - 1; //Newline is \r\n
	                                                      
	/* We should now have the ending position to get the following line:
	 * "GET / HTTP/1.0"
	 * So split it based on space delimeter to get URL path 
	 * and HTTP version.
	 */

	//printf("entire buffer: %s\nLast pos: %d\n", buff, lastPos);
	//printf("End of first line position: %d\n", lastPos);

	char *tempBuff = malloc(strlen(buff));
	strcpy(tempBuff, buff);

	char *split, *savePtr;
	int i = 0;
	int total = 0;
	while (total < lastPos)
	{
		if (total == 0)
		{
			split = strtok_r(tempBuff, " ", &savePtr);
		}
		else
		{
			split = strtok_r(NULL, " ", &savePtr);
		}
		int size = strlen(split);
		
		switch(i)
		{
			case 0: //Method (GET, POST, HEAD...)
				response->HTTP_Type = malloc(size + 1);
				strcpy(response -> HTTP_Type, split);
				break;
			case 1: //File content path
				strcpy(req, split);
				break;
			case 2: //HTTP Protocol (ex HTTP/1.1)
				
				/* There is no space after the version number, 
				 * only a newline character. So split again. */
				split = strtok(split, "\r\n");
				size = strlen(split);
				response->versionNum = malloc(size + 1);
				strcpy(response -> versionNum, split);
				break;
		}
		total += size + 1; //+1 to account for space
		i++;	
		printf("Split string: %s, size: %d\n", split, size); 
	}
	// Find the Accept: ... line in the get response
	strcpy(tempBuff, buff);

	split = strstr(tempBuff, "Accept: ");
	split = split + strlen("Accept: ");  //Should put us right after Accept: statement
	char *content_type = strtok(split, "\n");
	
	/* If content_type only contains one element, strtok will return NULL.
	 * If content_type has multiple elements (seperated by commas), strtok will 
	 * null terminate the first comma, so content_type will point to only the first element */	 
	strtok(content_type, ",");
	
	printf("Content-type: %s\n", content_type);
	response -> contentType = malloc(strlen(content_type) + 1);
	strcpy(response -> contentType, content_type);

	char *result = "200";
	response -> resultCode = malloc(strlen(result) + 1);
    strcpy(response -> resultCode, result);

	char *stat = "OK";	
	response -> status = malloc(strlen(stat) + 1);
	strcpy(response -> status, stat);

	/*
	 * Check if content requested (req) contains any user variables
	 * GET request contains data in content URL,
	 * POST containst data at the end of the buffer
	 */
	
	char *t = NULL;
	char *userVarStart = NULL;
    if (strcmp(response -> HTTP_Type, "POST") == 0)
	{
		//Print buffer for debug
		printf("Header:\n%s\n\n", buff);

		// Check if POST request with data AFTER the header
		printf("POST Request. Checking for user data\n");
		t = strstr(buff, "\r\n\r\n"); //CRLF is new line
	}
	else if ((t = strchr(req, '?')) != NULL)
	{
		// GET request with data in URL
		printf("GET Request. Checking for user data\n");
		*t = '\0'; //NULL '?' so file request is seperated from the user vars
		t++;
	}
	
	if (t != NULL)
	{
		userVarStart = malloc(strlen(t) + 14);
		sprintf(userVarStart, "QUERY_STRING=%s", t);
		printf("User data: %s\nFile: %s\n\n", userVarStart, req);

		// Add the user variables to QUERY_STRING environment variable
	    putenv(userVarStart); 
	}
		
}

/*
 * Creates the header information to send to the client.
 * Note: Only requests for GET, HEAD, and POST are dealt with
 *
 * @param Header: Buffer where the header string is stored
 * @param response: Structure containing most of the client
 *                  request information
 * @param filesize: Filesize of the content to send
 */
void setupHeader(char *Header, HTTP_Response *response, 
		         long unsigned filesize)
{
    /*
	 * Build the header
	 */

	// Get the date
	char dateTime[30];
	size_t i;
	struct tm tim;
	time_t now;
	now = time(NULL);
	tim = *(localtime(&now));
	i = strftime(dateTime, 30, "Date: %a, %b %d %Y %H:%M:%S %Z", &tim);

	sprintf(Header, "%s %s %s\n \
			%s\n \
	        Content-length: %lu\n \
	        Content-type: %s; charset=iso-8859-1\n \
			Connection: Closed\n\n",
	        response -> versionNum, response -> resultCode,response -> status, 
			dateTime, 
			filesize, 
			response -> contentType);
}

/**
 *
 * Usage:
 *    - port number
 *    - content directory
 */
int main(int argc, char **argv, char **environ) {

	const int QUEUE_LENGTH = 5;  /* Buffer for incoming connections. 
								  * If # of simultaneous incoming connections 
								  * > QUEUE_LENGTH, then exit */

	pid_t pid;		//pid of child 
	int sockfd;		//Our initial socket 
	int PORT;		//Port number, used by 'bind'
	char content[128];	/* Your directory that contains your web 
	        			 * content such as .www in your home directory */

	char myhome[128];	/* Your home directory *
					     * (gets filled in by GetMyHomeDir()) */

	// sockaddr_in structure is used for bind, accept..
  	struct sockaddr_in server_addr, client_addr; 

	char file_request[256];	//Where we store the requested file name
	int one=1;	//Used to set socket options


	//Get my home directory from the environment 
	GetMyHomeDir(myhome, environ);	

	if( argc != 3 ) {
		fprintf(stderr, "USAGE: %s <port number> <content directory>\n", 
								argv[0]);
		exit(-1);
	}

	PORT = atoi(argv[1]);		//Get the port number
	strcpy(content, argv[2]);	//Get the content directory
	
	if ( (pid = fork()) < 0) {
		perror("Cannot fork (for daemon)");
		exit(0);
  	}
	else if (pid != 0) {
	  	// I am the parent
		char t[128];
		sprintf(t, "echo %d > %s.pid\n", pid, argv[0]);
		system(t);
    	exit(0);
  	}

  	// setsid();
  	// chdir("/");
  	// umask(0);

	/*
	 * TODO --- Done
	 * Create our socket, bind it, listen
	 */

	// Set up server_addr structure
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT); //Standard data sent as BIG ENDIAN
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Create a TCP IPv4 socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Problem creating a socket");
		exit(-1);
	}
	
	printf("PORT: %d\n", server_addr.sin_port);

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
	{
		perror("Problem on binding");
		exit(-1);
	}

	listen(sockfd, QUEUE_LENGTH);
	

	signal(SIGCHLD, SIG_IGN);


	/* 
	 * - accept a new connection and fork.
	 * - If you are the child process,  process the request and exit.
	 * - If you are the parent close the socket and come back to 
     *   accept another connection
	 */
  	while (1) {

		/* 
		 * socket that will be used for communication
		 * between the client and this server (the child) 
		 */
		int newsock;		

		/*
		 * Accept a connection from a client (a web browser)
		 * accept the new connection. newsock will be used for the 
		 * child to communicate to the client (browser)
		 */
		 /* TODO 2 --- DONE */

		int client_len = sizeof(client_addr);

		newsock = accept(sockfd, (struct sockaddr *)&client_addr, &client_len); 
   		if (newsock < 0) {
			perror("accept");
			exit(-1);
		}

		if ( (pid = fork()) < 0) {
			perror("Cannot fork");
			exit(0);
  		}
		
		
		if( pid == 0 ) {
			// In child process
			int r;
   			char buf[1024];
			int read_so_far = 0;
			char ref[1024], rowRef[1024];
			HTTP_Response response;

			close(sockfd);

			memset(buf, 0, 1024);

			do 
			{

				if (r = read(newsock, buf, sizeof(buf)) < 0)
				{
					perror("Error reading from socket");
					close(newsock);
					exit(-1);
				}
			} while (r == sizeof(buf));
//
// What you may get from the client:
//			GET / HTTP/1.0
//			Connection: Keep-Alive
//			User-Agent: Mozilla/3.0 (X11; I; SunOS 5.5 sun4m)
//			Host: spiff:6789
//			Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, * /*
//
//
// Write to client
//
//			You should write to the client an HTTP response and then the
//			requested file, if appropriate. A response may look like:
//
//			HTTP/1.0 200 OK
//			Content-length: 2032
//			Content-type: text/html
//			[single blank line necessary here]
//			[document follows]
//



			ExtractFileRequest(file_request, buf, &response);

			printf("** File Requested: |%s|\n", file_request);
			fflush(stdout);

			SendDataBin(file_request, newsock, myhome, content, &response);
			shutdown(newsock, 1);
			close(newsock);
			exit(0);
    	}
		/*
		 * I am the Parent
		 */
		close(newsock);	/* Parent handed off this connection to its child,
			               doesn't care about this socket */
  	}
}
