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


#define BUFFER_SIZE 100
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

char nextCharNeeded(int index)
{
    std::string base = "\r\n\r\n";
    return base.at(index);
}
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
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
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



int main(int argc, char* argv[])
{
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
	    char pBuffer[BUFFER_SIZE];
    for(;;)//Forever loop
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));


	    int foundSoFar = 0;
   std::string soFar = "";	

	    bool endOfHeader = false;
	    int headerLength = 0;
	    string header = "";
	    int saveSpot = 0;
	    int nReadAmount = 0;

	    while (!endOfHeader) {

		memset(pBuffer, 0, BUFFER_SIZE);
		nReadAmount = read(hSocket, pBuffer, BUFFER_SIZE);
					//    std::cout << nReadAmount << std::endl;
		//Parse What we just read, looking for the \r\n\r\n
		for (int i = 0; i < nReadAmount; i++) {
		    

		    
		    headerLength++;
		    header += pBuffer[i];
		    if (nextCharNeeded(foundSoFar) == pBuffer[i]) {

			foundSoFar++;
		    }
		    else {
			foundSoFar = 0;
		    }
		    if (foundSoFar == 4) {
			endOfHeader = true;
			break;
		    }
		}
	    }



	//Now extract the directory from all this jazz
	


        //strcpy(pBuffer,MESSAGE);
        //printf("\nSending \"%s\" to client",pBuffer);
        /* number returned by read() and write() is the number of bytes
        ** read or written, with -1 being that an error occured
        ** write what we received back to the server */

        /* read from socket into buffer */
	/*
	//Need To Read the entire request
 	 vector<char *> headerLines;
*/
	// char directory[MAX_DIR_SIZE];

	//char directory[MAX_DIR_SIZE];
	string directorystring = "";
	cout << "1" << endl;
	bool done = false;
	int i = 0;
	//Make a char* for scan f
	while(!done && i < header.length()){
				cout << "foo1";
		if(header.at(i) == 'G'){
		if(header.at(i+1) == 'E'){
		if(header.at(i+2) == 'T'){
			int j = i+2;
			while(header.at(j) != '/'){

				cout << header.at(j);
				j++;
			}
			while(header.at(j) != ' '){
				cout << "foo3";
				directorystring += header.at(j);
				j++;
			}
			done = true;
		
		
		}}}
		i++;
	}
	char* directory = (char*)malloc(directorystring.length()
);
	for(int k = 0; k < directorystring.length(); k++){
		if(k == 0){
			//Do nothing
		}else{
			directory[k-1] = directorystring.at(k);
		}
	}
	
	//GetHeaderLines(headerLines, hSocket , false);
	/*
	  cout << "2" << endl;
	  // Now print them out
	  for (int i = 0; i < headerLines.size(); i++) {
		cout << "3.5" << endl;
		cout << "getting header lines " << endl;
	    printf("%s\n", headerLines[i]);
	    if(strstr(headerLines[i], "HTTP_GET")) {
		     sscanf(headerLines[i], "HTTP_GET / %s", directory);
	    }
	  }
	*/
	  		cout << "3" << endl;
	  printf("\n=======================\n");
	  printf("Directory is %s\n",directory);
	  printf("=======================\n\n");

	//Now parse the request

	//Now construct something to return and return it
	//We'll make the header for the response first



	//First check if directory is empty, because this is a special case
	cout << "4" << endl;
	bool indexflag = false;
	if(strcmp(directory, "HTTP/1.1") == 0){
		cout << "You gave me no directory path !!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		//Check if there is an index.html to return
		struct stat filestat;
		char index[] = "index.html";
		strcpy(directory,index);
		if(stat(directory, &filestat)) {
			cout <<"No index.html found\n" << "was looking for:";
			printf("%s\n", directory);
				
		}
		indexflag = true;
	}


	bool finished = false;
	struct stat filestat;

		cout << "5" << endl;
	if(stat(directory, &filestat) && !finished) {

		cout <<"ERROR in stat\n";
		
		finished = true;
		//Send a not found
		string content2 = "<html><body><h1>404 Not Found</h1></body></html>";

		char *contentlength = (char*)malloc(30);
		sprintf(contentlength, "Content-Length: %d\r\n", (int)content2.size());

		string response = "";
		
		//Now write file to socket
		response.append(notfound);
		response.append(date);
		response.append(htmlContent);
		response.append(string(contentlength));
		response.append(endOfResponse);
		response.append(content2);


		//Now write the response to the socket

		char responsearray[response.size()+1];
		for (int i=0;i<response.size();i++)
		{
		    responsearray[i]=response.at(i);
		}

		write(hSocket,responsearray, strlen(responsearray)+1);
		free(contentlength);
	}
	
	if(S_ISREG(filestat.st_mode) && !finished) {
		cout <<  " is a regular file \n";
		cout << "file size = "<<filestat.st_size <<"\n";
		FILE *fp = fopen(directory,"rb");
		char *buffer = (char*)malloc(filestat.st_size);
		//char buffer[filestat.st_size];


		fseek(fp, 0, SEEK_END);

		fseek(fp, 0, SEEK_SET);


		fread(buffer, filestat.st_size,1,fp);
		

		char *content = (char*)malloc(filestat.st_size);
		for(int i = 0; i < filestat.st_size; i++){
			content[i] = buffer[i];
		}
		char filetype = 't';
		//Now determine what type of file it is
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

		char *contentlength = (char*)malloc(30);
		sprintf(contentlength, "Content-Length: %d\r\n", (int)filestat.st_size);

		string response = "";

		//Now write file to socket

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

		//First write the response, then the content


		//response.append(content);


		//Now write the response to the socket  
		
		//char responsearray[response.size()+1];
		//First write the response
		char *responsearray = (char*)malloc(response.size());
		for (int i=0;i<response.size();i++)
		{
		    responsearray[i]=response.at(i);
		}
		int readsofar = 0;

		readsofar += write(hSocket,responsearray, response.size());
		//fwrite(responsearray , 1 , strlen(responsearray) , hSocket );	

		//Now write the content
		readsofar += write(hSocket,content, (int)filestat.st_size);

	 
		cout << "Wrote a total of " << readsofar << " bytes!" << endl;
		free(contentlength);
		free(responsearray);
		free(buffer);
		free(content);
		fclose(fp);
		finished = true;
		
	}
		cout << "6" << endl;
	if(S_ISDIR(filestat.st_mode)&& !finished) {
		cout << directory << " is a directory \n";

	  DIR *dirp;
	  struct dirent *dp;

	  dirp = opendir(directory);
	  while ((dp = readdir(dirp)) != NULL)
	    printf("name %s\n", dp->d_name);

	  (void)closedir(dirp);
	}
		cout << "7" << endl;

    	printf("\nClosing the socket");
        /* close socket */
	shutdown(hSocket, SHUT_RDWR);

        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
	free(directory);
    }
}
