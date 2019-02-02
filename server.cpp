#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h> 
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h> 
#include <pthread.h>
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;

#define MAX_FILE 1000 

//ADD SIGNALS SIG
void sighandler_term(int);
void sighandler_quit(int);

struct thread_args 
{
    int thread_id;
    int index;
    int fds;
    string file_path;
};











void* runner(void* struct_arg)
{
    struct thread_args *args = (struct thread_args*) struct_arg;
    int thread_id = args->thread_id;
    int index = args->index;
    int fds = args->fds; 
    string file_path = args->file_path;

    cout << "My index is = " << index << endl;

    //CREATE FILE / Open a file
    string file_string = file_path + "/" + to_string(index+1) + ".file";
    const char* file_name = file_string.c_str();
    cout << "FILE NAME = " << file_name << endl;

    FILE * fptr = fopen(file_name, "w");
    if (fptr == NULL)
    {
        cerr << "ERROR: in opening the file" << endl;

        int closed = close(fds);
        if (closed == -1)
        {
          cerr << "ERROR: in closing the client socket -- Closing" << endl;
        }

        pthread_exit(NULL);
        
    }


    //SETUP BUFFER  
    char buf[MAX_FILE] = {0};

    //TIMEOUT
    struct timeval timeout;

    while (1)
    {

        fd_set thread_read;
        FD_ZERO(&thread_read);
        FD_SET(fds, &thread_read);

        timeout.tv_sec = 15;
        timeout.tv_usec = 0;

        int selected = select( fds + 1 , &thread_read , NULL , NULL , &timeout);   //wait for incoming connection
       
        if ((selected < 0)) // && (errno!=EINTR))   
        {   
             cerr << "ERROR: in select -- Closing thread" << endl;
             break;
        }  


        if (!FD_ISSET(fds, &thread_read))
        {
            cerr << "ERROR: Client timed out -- Closing thread" << endl;
             fptr = fopen(file_name, "w");
             if (fptr == NULL)
             {
                cerr << "ERROR: could not truncate file -- Closing thread" << endl;
                fclose(fptr);
                int closed = close(fds);
                if (closed == -1)
                {
                  cerr << "ERROR: in closing the client socket -- Closing" << endl;
                }

                pthread_exit(NULL);
             }

             string error = "ERROR";
             const char* c_error = error.c_str();
             if (fwrite(c_error, 1, 5, fptr) < 0)
             {
                cerr << "ERROR: Could not write to file -- Closing thread" << endl;
             }
             
            fclose(fptr);
            int closed = close(fds);
            if (closed == -1)
            {
              cerr << "ERROR: in closing the client socket -- Closing" << endl;
            }

            pthread_exit(NULL);
        }


        int _read = read(fds, buf, (size_t) sizeof(buf));
        if (_read == 0) //if nothing to read anymore             
        {
            cout << "Nothing left to read from client -- Closing thread " << thread_id << endl;
            break;
        }
        else // perform the reading and store it into the file 
        {
            if (fwrite(buf, 1, _read, fptr) < 0)
            {
                cerr << "ERROR: Could not write to file -- Closing thread" << endl;
                break;
            }
            memset(buf, '\0', sizeof(buf));
        }
    }

    //CLEANUP
    fclose(fptr);
    int closed = close(fds);
    if (closed == -1)
    {
      cerr << "ERROR: in closing the client socket -- Closing" << endl;
    }

    pthread_exit(NULL);
}














int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    cerr << "ERROR: Wrong number of arguments, need 2: <PORT> <FILE-DIR> -- Closing" << endl;
    return 1;  
  }

  int port = stoi(argv[1]);
  string file_dir = argv[2]; 
  if (port <= 1023)
  {
    cerr << "ERROR: Port is " << port << "must be 1023 < ! -- Closing" << endl;
    return 1;  
  }


  //CREATE DIR / CHECK IF EXISTS
  DIR* dptr = opendir(file_dir.c_str());
  if (dptr)
  {
    cout << "Directory already exists, closing it" << endl;
    if (closedir(dptr) < 0)
    {
      cerr << "ERROR: Could Not Close Directory -- Closing" << endl;
      return 1;
    }
  }
  else if (ENOENT == errno)
  {
    if(mkdir(file_dir.c_str(), 0777) < 0)
    {
      cerr << "ERROR: could not create directory -- Closing" << endl;
      return 1;
    }
    else
      cout << "Successfully Created the Directory" << endl;
  }
  else
  {
    cerr << "Could Not Open the Directory -- Closing" << endl;
    return 1;
  }

// SIGNALS
  signal(SIGTERM, sighandler_term);
  signal(SIGQUIT, sighandler_quit);


//# THREADS 
  int curr_threads = 0;
  int max_threads = 10;
//# CLIENTS
  int max_clients = 10;
//FDS
  int max_sd;
  fd_set readfds;


  //Setup the socket
  int socketfd = socket(PF_INET, SOCK_STREAM, 0);
  if (socketfd == -1)
  {
    cerr << "ERROR: in creating the Socket -- Closing" << endl;
    return 1;
  }

  int non_block = fcntl(socketfd, F_SETFL, O_NONBLOCK);
  if (non_block == -1)
  {
    cerr << "ERROR: in making socket non_block -- Closing" << endl;
    return 1;
  }

  struct sockaddr_in addr;
  addr.sin_family = PF_INET;
  addr.sin_port = htons(port); //port provided by user
  addr.sin_addr.s_addr = INADDR_ANY;
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
  int addrlen = sizeof(addr);

  int bound = bind(socketfd, (struct sockaddr*)&addr, addrlen);
  if (bound == -1)
  {
    cerr << "ERROR: in binding socket -- Closing" << endl;
    return 1;
  }


  int listener = listen(socketfd, max_clients); //up to 10 simultaneous connections
  if (listener == -1)
  {
    cerr << "ERROR: in getting in listening mode -- Closing" << endl;
    return 1;
  }

  while(1)
  {
      FD_ZERO(&readfds); //clear the set of sockets
      FD_SET(socketfd, &readfds); //add server socket  
      max_sd = socketfd;
      
      int selected = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   //wait for incoming connection
       
      if ((selected == -1) && (errno!=EINTR))   
      {  
         cout << "Selected = " << selected << endl;
         perror("select") ;
         cerr << "ERROR: in selecting new client connections -- Closing" << endl;
         return 1;
      }   
      

    if (FD_ISSET(socketfd, &readfds)) //if Server Socket changed, have an incoming connection -- create thread there
    {
          int clientSocketfd = accept(socketfd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
          if (clientSocketfd == -1) 
          {
            perror("accept");
           cerr << "ERROR: in accepting new client connections -- Closing" << endl;
           return 1;
          } 

          int client_non_block = fcntl(clientSocketfd, F_SETFL, O_NONBLOCK);
          if (client_non_block == -1)
          {
            cerr << "ERROR: in making client socket non_block -- Closing" << endl;
            return 1;
          }

        pthread_t client_thread;
        struct thread_args Args;
        Args.thread_id = client_thread;
        Args.index = curr_threads;
        Args.fds = clientSocketfd;
        Args.file_path = file_dir;

        int created = pthread_create(&client_thread, NULL, runner, (void*) &Args);
        if (created)
        {
            cerr << "ERROR: Unble to create thread -- Closing Socket" << endl;
            int closed = clientSocketfd;
            if (closed == -1)
            {
              cerr << "ERROR: in closing the client socket -- Closing" << endl;
              return 1; 
            }
        }

        if (curr_threads == max_threads-1) //hit max number of threads
            curr_threads = 0;
        else
            curr_threads++;
    }

  }

  int closed = close(socketfd);
  if (closed == -1)
  {
      cerr << "ERROR: in closing the client socket -- Closing" << endl;
  }

  return 0;
}


void sighandler_term(int signum)
{
    cout << "Caught SIGTERM -- Closing" << endl;
    exit(0);

}

void sighandler_quit(int signum)
{
    cout << "Caught SIGQUIT -- Closing" << endl;
    exit(0);
}