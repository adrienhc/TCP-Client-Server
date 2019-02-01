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
  cout << "HOST IS " << host->h_name << endl;
  cout << "ADDR IS " << inet_ntoa( *(struct in_addr*) (host-> h_addr)) << endl;
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
  cout << "START POS IS " << ftell(fptr) << endl;


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

  int connected = connect(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if (connected == -1)
  {
    perror("connect");
    cerr << "ERROR: in connecting the socket -- Closing" << endl;
    return 1;
  }

  //cout << file_content << endl;
 
  //Write whole string file by chunks of 1000 bits!
  long k = 0;
  //int f_size = file_content.length();
  long f_size = fSize;

  cout << "FILE SIZE IS " << f_size << endl;
  char buffer[1000];

  for (k = 0; k < f_size -1000; k += 1000)
  {
      //cout << "SIZE BUF = " << sizeof(buffer) / sizeof(char) << endl;
      if (fread(buffer, 1, sizeof(buffer), fptr) != sizeof(buffer))
      {
      cerr << "ERROR: Could not read from file -- Closing" << endl;
      perror("fread");
      return 1;
    }

    int wrote = write(socketfd, buffer, 1000);
    
    //int wrote = write(socketfd, , 1000);
    if (wrote == -1)
    {
      cerr << "ERROR: in writing to the server -- Closing" << endl;
      return 1;
    }

    //fseek(fptr, 1000, SEEK_CUR);
    memset(buffer, '\0', sizeof(buffer));

  }

  //send remaining bits
  cout << "SEND LAST PART !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
  //k -= 1000;
  
  //string rest = file_content.substr(k);
  //cout << "SEND: " << endl << rest << endl;
  long rest = fSize - ftell(fptr); 
  cout << "REST IS " << rest << endl;
  char buff_end[rest];

  if( fread(buff_end, 1, sizeof(buffer), fptr) != sizeof(buff_end) )
    {
      cerr << "ERROR: Could not read from file -- Closing" << endl;
      return 1;
    }

  int wrote = write(socketfd, buff_end, rest);
  if (wrote == -1)
  {
    cerr << "ERROR: in writing last part to the server -- Closing" << endl;
    return 1;
  }  

  //cout << "SEND: " << endl << *buff_end << endl;
  

  int closed = close(socketfd);
  if (closed == -1)
  {
    cerr << "ERROR: in closing the socket -- Closing" << endl;
    return 1; 
  }

  return 0;
}

//string file_content;
  //ifstream File (file_dir, ios::in | ios::binary);
  //ifstream File(file_dir);
  //File.exceptions (ifstream::failbit | ifstream::badbit);

  /*try
  {
   // filebuf* pbuf = File.rdbuf();
   // size_t size = pbuf->pubseekof(0, File.end, File.in);
   // pbuf->pubseekof(0, File.in);

    //File.open(file_dir);
    //stringstream fileStream;
    //fileStream << File.rdbuf();
    //File.close();
    //file_content = fileStream.str();*/
  
    /*ostringstream content;
    content << File.rdbuf();
    File.close();
    file_content = content.str();*/

/*  }
  catch(ifstream::failure e)
  {
    cerr << "ERROR: in reading from the file" << endl;
    return 1;
  }*/