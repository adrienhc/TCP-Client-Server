#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

using namespace std;

#define MAX_FILE 1000

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    cerr << "ERROR: Wrong number of arguments, need 3: <HOSTNAME-OR-IP> <PORT> <FILENAME> -- Closing" << endl;
    return 1;  
  }

  const char* server_ip = argv[1];
  struct hostent *host;
  host = gethostbyname(server_ip);
  if (host == nullptr)
  {
    cerr << "ERROR: Invalid Hostname: " << server_ip << " -- Closing" << endl;
    return 1; 
  }

  int port = stoi(argv[2]);
  if (port <= 1023)
  {
    cerr << "ERROR: Port is " << port << " must be 1023 < ! -- Closing" << endl;
    return 1;  
  }

  const char* file_dir = argv[3];

  FILE * fptr = fopen(file_dir, "r");
  if (fptr == NULL)
  {
    cerr << "ERROR: in opening the file" << endl;
    return 1;
  }

  fseek(fptr, 0, SEEK_END);
  long fSize = ftell(fptr);
  rewind(fptr);


  //Setup the socket
  int socketfd = socket(PF_INET, SOCK_STREAM, 0);
  if (socketfd == -1)
  {
    cerr << "ERROR: in creating the Socket -- Closing" << endl;
    return 1;
  }


  struct sockaddr_in serv_addr;
  serv_addr.sin_family = PF_INET;
  serv_addr.sin_port = htons(port); //port provided by user
  serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa( *(struct in_addr*) (host-> h_addr))); //server ip prodived by user
  memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero));


  //GET TIME
  time_t before_connect = time(NULL);

  int connected = connect(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if (connected == -1)
  {
    perror("connect");
    cerr << "ERROR: in connecting the socket -- Closing" << endl;
    return 1;
  }
  
  time_t after_connect = time(NULL);

  //COMPARE TIME 
  if (difftime(after_connect, before_connect) > 15.0)
  {
    cerr << "ERROR: Timeout, took too long to connect -- Closing" << endl;
    int closed = close(socketfd);
    if (closed == -1)
        cerr << "ERROR: in closing the socket -- Closing" << endl; 
    return 1;
  }


  long k = 0;
  long f_size = fSize;
  char buffer[1000];

  for (k = 0; k < f_size -1000; k += 1000)
  {
      if (fread(buffer, 1, sizeof(buffer), fptr) != sizeof(buffer))
      {
      cerr << "ERROR: Could not read from file -- Closing" << endl;
      perror("fread");
      return 1;
    }

    time_t before_msg = time(NULL);

    int wrote = write(socketfd, buffer, 1000);
    if (wrote == -1)
    {
      cerr << "ERROR: in writing to the server -- Closing" << endl;
      return 1;
    }

    time_t after_msg = time(NULL);
    if (difftime(after_msg, before_msg) > 15.0)
    {
      cerr << "ERROR: Timeout, took too long to write to server -- Closing" << endl;
      int closed = close(socketfd);
      if (closed == -1)
          cerr << "ERROR: in closing the socket -- Closing" << endl; 
      return 1;
    }

    memset(buffer, '\0', sizeof(buffer));

  }

  long rest = fSize - ftell(fptr); 
  char buff_end[rest];

  if( fread(buff_end, 1, sizeof(buffer), fptr) != sizeof(buff_end) )
    {
      cerr << "ERROR: Could not read from file -- Closing" << endl;
      return 1;
    }

  time_t before_last = time(NULL);
  int wrote = write(socketfd, buff_end, rest);
  if (wrote == -1)
  {
    cerr << "ERROR: in writing last part to the server -- Closing" << endl;
    return 1;
  }  
  time_t after_last = time(NULL);

  if (difftime(after_last, before_last) > 15.0)
  {
    cerr << "ERROR: Timeout, took too long to write last msg to server -- Closing" << endl;
    int closed = close(socketfd);
    if (closed == -1)
        cerr << "ERROR: in closing the socket -- Closing" << endl; 
    return 1;
  }

  

  int closed = close(socketfd);
  if (closed == -1)
  {
    cerr << "ERROR: in closing the socket -- Closing" << endl;
    return 1; 
  }

  return 0;
}
