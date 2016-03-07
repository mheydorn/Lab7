#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <iostream>
#include <vector>             // stl vector
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

using namespace std;

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define MESSAGE             ""
#define QUEUE_SIZE          5
#define MAXMSG 		    1024
#define MAX_MSG_SZ          1024

int hSocket,hServerSocket;  /* handle to socket */
struct hostent* pHostInfo;   /* holds info about a machine */
struct sockaddr_in Address; /* Internet socket address stuct */
int nAddressSize=sizeof(struct sockaddr_in);
char pBuffer[BUFFER_SIZE];
int nHostPort;
char Get[MAXMSG];
// Determine if the character is whitespace
/*void FileOrDir(char* path[300])
{
  
}*/
// Determine if the character is whitespace
bool isWhitespace(char c)
{
    switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {

        if (amtread > 0){
            messagesize += amtread;
        }
        else if( amtread == 0 )
        {
            break;
        }
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
        {
            break;
        }
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}
    
// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;
    
    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');
        
        if (s[i] == '-')
            s[i] = '_';
    }
    
}


// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 2;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat)
{

    std::cout << "1" << std::endl;
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;
    
    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        std::cout << "lin: " << tline << std::endl;
        std::cout << "len: " << strlen( tline ) << std::endl;
        if (strstr(tline, "Content-Length") || 
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, const_cast<char*>( "" ) );
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, const_cast<char*>( "HTTP_" ) );
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);                
            }
        }
        fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}
/*
bool isWhitespace(char c)
{ switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}
// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAXMSG];
    char *line;
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAXMSG)
    {
        if (amtread >= 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}
// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;
    
    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');
        
        if (s[i] == '-')
            s[i] = '_';
    }
}
char *FormatHeader(char *str, const char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;
    
    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") || 
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, "");
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
	    {
                line = FormatHeader(tline, "HTTP_");
            }
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);                
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}
*/
void getRequest(int currentSocket)
{
  //int fd;
  vector<char *> headerLines;
  char buffer[MAXMSG];
  char contentLength[MAXMSG];
  
  //printf("Web Program Tools Example\n\n");

  // This shows how you could use these tools to implement a web client
  // We will talk about how to use them for the server too

  // Open the file that will simulate the socket
  //fd = open(hSocket, O_RDONLY);
  //if(fd < 0) {
       // perror("open of sample.txt failed");
       // exit(0);
  //}

  // First read the status line
   //char *startline = GetLine(hSocket);
   //printf("Status line %s\n\n",startline);
  // Read the header lines
  GetHeaderLines(headerLines, hSocket, false);

  // Now print them out
    for (int i = 0; i < headerLines.size(); i++) {
      printf("%s\n",headerLines[i]);
      if(strstr(headerLines[i], "HTTP_GET")) {
             sscanf(headerLines[i], "HTTP_GET %s", Get);
	     printf("This is the path: %s\n", Get);
      }
      else if(strstr(headerLines[i], "GET")) {
             sscanf(headerLines[i], "GET %s", Get);
	     printf("This is the path: %s\n", Get);
      }
  }
  cout << endl;

  printf("\n=======================\n");
  printf("Headers are finished, now read the file\n");
  printf("The path is %s\n", Get);
  printf("=======================\n\n");
}
void readAndWrite(char pBuffer[BUFFER_SIZE], int hSocket)
{
	/* number returned by read() and write() is the number of bytes
        ** read or written, with -1 being that an error occured
        ** write what we received back to the server */
        
        /* read from socket into buffer */
        //vector<char*> headers;
	//GetHeaderLines(headers, hSocket, false);
	getRequest(hSocket);
        
	struct stat filestat;
    	char *tline;

        if(stat(Get, &filestat)) {
          cout <<"ERROR in stat\n";// send html of 404.
        }
        	
        if(S_ISREG(filestat.st_mode)) {
          cout << Get << " is a regular file \n";
          cout << "file size = "<<filestat.st_size <<"\n";
          FILE *fp = fopen(Get, "rb");
	  if(fp == NULL)
          {
             cout << "File Not Found" << endl;
          }
          char *buffer = (char*)malloc(filestat.st_size);
          fread(buffer, filestat.st_size, 1, fp);
          cout << buffer[0] << buffer[1] << buffer[2] << buffer[3] << buffer[4] << endl; 
	  //write(hSocket, status, strlen(status)+1);
	  if(strstr(Get, ".txt")) 
	  {
	     //cout << "here2" << endl;
	     //char contentType[1000] = "\nContent-Type: %s", 
	     char result2[BUFFER_SIZE]; 
	     char result1[] = "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
	     sprintf(result2, "Content-Length: %ld\n\n", filestat.st_size);
	     printf("%s", result1);
	     printf("%s", result2);
	     write(hSocket, result1, strlen(result1)+1);
	     write(hSocket, result2, strlen(result2)+1);
	     write(hSocket, buffer,  filestat.st_size+1);	     
 	     //write(hSocket, tline, strlen(tline)+1);
             //write(hSocket, buffer, strlen(buffer)+1);
	     //if(write(hSocket, buffer, strlen(buffer)+1))
             //{
               //printf("\nRead Error\n");
             //}
          } 
          else if(strstr(Get, ".html"))
	  {
	     char result2[BUFFER_SIZE]; 
  	     char result1[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n";
	     sprintf(result2, "Content-Length: %ld\n\n", filestat.st_size);
	     printf("%s", result1);
	     printf("%s", result2);
	     write(hSocket, result1, strlen(result1)+1);
	     write(hSocket, result2, strlen(result2)+1);
	     write(hSocket, buffer, filestat.st_size+1);
 	  }
	  else if(strstr(Get, ".gif"))
	  {
	     char result2[BUFFER_SIZE]; 
  	     char result1[] = "HTTP/1.1 200 OK\nContent-Type: image/gif\n";
	     sprintf(result2, "Content-Length: %ld\n\n", filestat.st_size);
	     printf("%s", result1);
	     printf("%s", result2);
	     write(hSocket, result1, strlen(result1)+1);
	     write(hSocket, result2, strlen(result2)+1);
	     write(hSocket, buffer,  filestat.st_size+1);
     	  }
	  else if(strstr(Get, ".jpg"))
	  {
	     char result2[BUFFER_SIZE]; 
	     cout << "here jpg" << endl;
             char result1[] = "HTTP/1.1 200 OK\nContent-Type: image/jpg\n";
	     sprintf(result2, "Content-Length: %ld\n\n", filestat.st_size);
	     printf("%s", result1);
	     printf("%s", result2);
	     write(hSocket, result1, strlen(result1));
	     write(hSocket, result2, strlen(result2));
	     write(hSocket, buffer,  filestat.st_size+1);
	  }
          else
          {
	     char result2[BUFFER_SIZE]; 
             char result1[] = "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
	     sprintf(result2, "Content-Length: %ld\n\n", filestat.st_size);
	     printf("%s", result1);
	     printf("%s", result2);
	     write(hSocket, result1, strlen(result1)+1);
	     write(hSocket, result2, strlen(result2)+1);
	     write(hSocket, buffer, strlen(buffer)+1);
          }
          //printf("File:\n%s\n", buffer);
          free(buffer);
          fclose(fp);
  	}
 	 if(S_ISDIR(filestat.st_mode)) {
    	  cout << Get << " is a directory \n";
    	  int len;
    	  DIR *dirp;
    	  struct dirent *dp;

    	  dirp = opendir(".");
          printf("<html> \n <body>");
    	 /* while ((dp = readdir(dirp)) != NULL)
		"<a href="http://www.w3schools.com">This is a link</a
		<h1>My First Heading</h1>

		<p>My first paragraph.</p>

		</body>
		</html>"
      	  	printf("name %s\n", dp->d_name);
          printf("</html> \n <body>");
    	  (void)closedir(dirp);   */ 
   	}

        //memset(pBuffer,0,sizeof(pBuffer));
        //if(read(hSocket,pBuffer,BUFFER_SIZE) == -1)
        //{
          //printf("\nRead Error\n");
        //}
	//cout << "here" << endl;

	//write(hSocket,pBuffer,strlen(pBuffer)+1);
        //if(write(hSocket,pBuffer,strlen(pBuffer)+1) == -1)
        //{
          //printf("\nRead Error\n");
        //}
        //printf(pBuffer, "Hello");
	/*"HTTP/1.1 200 OK\r\n\r\n 
	<html><ul>\n
        <li> file1.html</li>\n
        <li> file2.html</li>\n
        </ul>\n
        Hello</html>"); */

        //if(strcmp(pBuffer,MESSAGE) == 0){
            //printf("\nThe messages match");
        //}
        //else{
            //printf("\nSomething was changed in the message");
	//}
}

int main(int argc, char* argv[])
{
    //sigaction(SIGHUP,&signew,&sigold);
    //sigaction(SIGPIPE,&signew,&sigold);
   
    if(argc < 3)
      {
        printf("\nUsage: server host-port\n");
        return 0;
      }
    else if(argc == 3)
      {
        nHostPort=atoi(argv[1]);
      }
    else 
      {
        printf("\nTwo Many Arguments\n");
      }

    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

     printf("\nBinding to port %d",nHostPort);
        /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname(hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }
    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
        strcpy(pBuffer,MESSAGE);

        printf("\nSending \"%s\" to client",pBuffer);
        readAndWrite(pBuffer, hSocket);
   
        linger lin;
	unsigned int y=sizeof(lin);
	lin.l_onoff=1;
	lin.l_linger=10;
	setsockopt(hSocket, SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
        shutdown(hSocket, SHUT_RDWR);
        
        printf("\nClosing the socket");
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
