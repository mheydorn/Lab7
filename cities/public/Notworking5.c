#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <dirent.h>



#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5
#define MAX_MSG_SZ      1024
#define MAX_DIR_SIZE 2048

#define ok "HTTP/1.1 200 OK\r\n"
#define notfound  "HTTP/1.0 404 Not Found\r\n"
#define date  "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n"
#define plainText "Content-Type: text/plain\r\n"
#define htmlContent  "Content-Type: text/html\r\n"
#define jpegContent  "Content-Type: image/jpg\r\n"
#define gifContent  "Content-Type: image/gif\r\n"
#define endOfResponse  "\r\n"

using namespace std;

// Determine if the character is whitespace
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
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
	int expire = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
	expire++;
        if (amtread >= 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n' || expire >= 1000)
            break;
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
char *FormatHeader(char *str, const char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

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
                line = FormatHeader(tline, "HTTP_");
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

#include <sys/types.h>
#include <sys/signal.h>

void handler (int status);   /* definition of signal handler */
int  counter = 0;

void handler (int status)
{
        printf("received signal %d\n",status);
}


int main(int argc, char* argv[])
{

	
        int rc1, rc2;

        // First set up the signal handler
        struct sigaction sigold, signew;

        signew.sa_handler=handler;
        sigemptyset(&signew.sa_mask);
        sigaddset(&signew.sa_mask,SIGINT);
        signew.sa_flags = SA_RESTART;
        //sigaction(SIGINT,&signew,&sigold);
        sigaction(SIGHUP,&signew,&sigold);
        sigaction(SIGPIPE, &signew, &sigold);


    if(chdir(argv[2]) == -1){
	printf("Not a valid directory\r\n");
	exit(0);
	};
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    int nHostPort;

    if(argc < 2)
      {
        printf("\nUsage: server host-port\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
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
	//Put code here!!!!!
    /* bind to a port */
	linger lin;
	unsigned int y=sizeof(lin);
	lin.l_onoff=1;
	lin.l_linger=10;
	setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
	int optval = 1;
	setsockopt (hSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));	
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);


    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }






    for(;;)
    {
	
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
  
 	vector<char *> headerLines;
	char* directory = (char*)malloc(1024);;
	GetHeaderLines(headerLines, hSocket , false);

	 for (int i = 0; i < headerLines.size(); i++) {
	    if(strstr(headerLines[i], "HTTP_GET")) {
		     sscanf(headerLines[i], "HTTP_GET / %s", directory);
	    }
	 }

	if(strcmp(directory, "HTTP/1.1") == 0){
		cout << "HTTP thing detected!!" << endl;
		strcpy(directory,".");
	}

 	  printf("\n========================\n");
	  printf("Directory is %s\n",directory);
	  printf("=======================\n\n");

	bool finished = false;
	struct stat filestat;

	if(stat(directory, &filestat) && !finished) {

		cout <<"Neither File Nor Directory, sending not found\n";
		
		finished = true;
		string content = "<html><body><h1>404 Not Found</h1></body></html>";
		char *contentlength = (char*)malloc(1024);
		sprintf(contentlength, "Content-Length: %d\r\n", (int)content.size());
		string response = "";
		
		response.append(notfound);
		response.append(date);
		response.append(htmlContent);
		response.append(string(contentlength));
		response.append(endOfResponse);
		response.append(content);

		char responsearray[response.size()+1];
		for (int i=0;i<response.size();i++)
		{
		    responsearray[i]=response.at(i);
		}

		write(hSocket,responsearray, strlen(responsearray)+1);
		free(contentlength);
	}
	
	if(S_ISREG(filestat.st_mode) && !finished) {
		cout <<  "Given Regular File" << endl;

		FILE *fp = fopen(directory,"rb");
		if(fp == NULL){
			printf("Error opening this file with fopen\r\n");
			string content = "<html><body><h1>fopen ran into a problem opening this file</h1></body></html>";
			char *contentlength = (char*)malloc(1024);
			sprintf(contentlength, "Content-Length: %d\r\n", (int)content.size());
			string response = "";
		
			response.append(notfound);
			response.append(date);
			response.append(htmlContent);
			response.append(string(contentlength));
			response.append(endOfResponse);
			response.append(content);

			char responsearray[response.size()+1];
			for (int i=0;i<response.size();i++)
			{
			    responsearray[i]=response.at(i);
			}

			write(hSocket,responsearray, strlen(responsearray)+1);
			free(contentlength);
			free(directory);
			continue;
		}
		char *buffer = (char*)malloc(filestat.st_size);

		fseek(fp, 0, SEEK_END);
		fseek(fp, 0, SEEK_SET);
		fread(buffer, filestat.st_size,1,fp);
		
		char* content = (char*)malloc(sizeof(char)*filestat.st_size);
		char *contentlength = (char*)malloc(sizeof(char)*1024);

		for(int i = 0; i < filestat.st_size; i++){
			content[i] = buffer[i];
		}

		char filetype = 't';
		if(strstr( directory, ".html")){
			cout << "File is a html file" << endl;
			filetype = 'h';
		}else if(strstr( directory, ".gif")){
			cout << "File is a gif file" << endl;
			filetype = 'g';
		}else if(strstr( directory, ".jpg")){
			cout << "File is a jpg file" << endl;
			filetype = 'j';
		}else{
			filetype = 't';
		}


		sprintf(contentlength, "Content-Length: %d\r\n", (int)filestat.st_size);
		string response = "";
		response.append(ok);
		response.append(date);
		//Check to see what type of file it is....
		switch(filetype){
			case 't':
				response.append(plainText);
			break;
			case 'h':
				response.append(htmlContent);
			break;	
			case 'g':
				response.append(gifContent);
			break;	
			case 'j':
				response.append(jpegContent);
			break;
			default:
			break;
		}
		response.append(string(contentlength));
		response.append(endOfResponse);

		char* responsearray = (char*)malloc(response.size());
		for (int i=0;i<response.size();i++)
		{
		    responsearray[i]=response.at(i);
		}

		//Write header
		write(hSocket,responsearray, response.size());

		//Write the content
		write(hSocket,content, (int)filestat.st_size);
		free(content);
		free(contentlength);
		fclose(fp);
		finished = true;
		
	}

	if(S_ISDIR(filestat.st_mode)&& !finished) {
		cout << directory << "Give a directory \n";

		DIR *dirp;
		struct dirent *dp;

		dirp = opendir(directory);
		string dir = "";
		dir.append( "<html>");
		dir.append( "<body>");
		char* dd = (char*)malloc(sizeof(char) * 4068);
		char* ddofindex = (char*)malloc(sizeof(char) * 4068);
		bool hasindex = false;
		while ((dp = readdir(dirp)) != NULL){
			if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
				sprintf(dd, "<a href=\"/%s/%s\">%s</a>",directory,dp->d_name,dp->d_name);
				dir.append( dd);
				dir.append( "<br>");
				if(strcmp(dp->d_name, "index.html") == 0){
					sprintf(ddofindex, "/%s/%s",directory,dp->d_name);
					cout << "Found index.html" << endl;
					hasindex = true;
					break;
				}
			}
		}	
		dir.append( "</body>");
		dir.append("</html>");
		(void)closedir(dirp);
		free(dd);
		if(!hasindex){
			//Has no index, send directory listing
			char* contentlength = (char*)malloc(sizeof(char)*4068);
			sprintf(contentlength, "Content-Length: %d\r\n", (int)dir.size());
			string response = "";
			response.append(ok);
			response.append(date);
			response.append(htmlContent);
			response.append(string(contentlength));
			response.append(endOfResponse);
			response.append(dir);

			char responsearray[response.size()+1];
			for (int i=0;i<response.size();i++)
			{
			    responsearray[i]=response.at(i);
			}
			write(hSocket,responsearray, strlen(responsearray)+1);
			free(contentlength);
			free(ddofindex);
		}else{
			//Has Index, send index
			//Delete the slash
			char* noslash = (char*)malloc(sizeof(char)*strlen(ddofindex)-1);
			memset(noslash, 0, sizeof(char)*strlen(ddofindex) - 1 );		
			cout << "Copying in " << ddofindex << endl;
			for(int i = 1; i < strlen(ddofindex); i++){
				noslash[i-1] = ddofindex[i];
				
			}
			noslash[strlen(ddofindex)-1] = '\0';

			printf("Stating index file which is %s\r\n",noslash);
			struct stat filestat;

			if(!stat("index.html", &filestat)) {
				printf("Error stating index file which is %s\r\n", noslash);	
				continue;		
			}
			cout <<"DOne stating" << endl;
			char *mybuffer = (char*)malloc(sizeof(char)*filestat.st_size);
			cout << "allocating " << filestat.st_size << " bytes" << endl;
			FILE *fp = fopen(ddofindex,"rb");
			cout <<"DOne opening" << endl;
			fread(mybuffer, 0,1,fp);
			cout <<"DOne reading" << endl;
			cout << "1" << endl;
			if(fp == NULL){
				printf("Error opening this file with fopen\r\n");
				string content = "<html><body><h1>fopen ran into a problem opening this file</h1></body></html>";
				char *contentlength = (char*)malloc(1024);
				sprintf(contentlength, "Content-Length: %d\r\n", (int)content.size());
				string response = "";
		
				response.append(notfound);
				response.append(date);
				response.append(htmlContent);
				response.append(string(contentlength));
				response.append(endOfResponse);
				response.append(content);

				char responsearray[response.size()+1];
				for (int i=0;i<response.size();i++)
				{
				    responsearray[i]=response.at(i);
				}

				write(hSocket,responsearray, strlen(responsearray)+1);
				//free(contentlength);
				//ree(directory);
				continue;
			}
			
			char* contentlength = (char*)malloc(sizeof(char)*4068);
			sprintf(contentlength, "Content-Length: %d\r\n", (int)strlen(mybuffer));
			string response = "";
			response.append(ok);
			response.append(date);
			response.append(htmlContent);
			response.append(string(contentlength));
			response.append(endOfResponse);
			char* responsearray = (char*)malloc(sizeof(char)*response.size());
			cout << "2" << endl;
			for (int i=0;i<response.size();i++)
			{
			    responsearray[i]=response.at(i);
			}

			//Write header
			write(hSocket,responsearray, strlen(responsearray)+1);
			//Write content
			write(hSocket,mybuffer, strlen(mybuffer)+1);
			free(contentlength);
			free(mybuffer);
			cout << "3" << endl;
			free(ddofindex);
			free(responsearray);
		}

	}

    	printf("\nClosing the socket\r\n");
	shutdown(hSocket, SHUT_RDWR);
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
	cout << "Done closeing" << endl;
	free(directory);
    }
}
