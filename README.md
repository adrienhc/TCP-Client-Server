# CS118 Project 1

############# REPORT ###############

Adrien HADJ-CHAIB
004800889

Client Design:
	The client first sets up the sockets, connects to the server. Once this is done, it
	 opens the file to be sent over with a C style FILE* pointer. We then compute the file 
	 of the size and fread the file 1000 bytes by 1000 bytes. Each read is placed into a 
	 1000 bytes buffer, which is written out to through the socket. I also made sure 
	 onnecting ans two successive writes werent separated by more than 15 seconds. We stop 
	 doing the 1000 bytes until we have less than 1000 bytes remaining. We then create a 
	 buffer of the size of the remainder of the file and send it over through the network.

Server Design:
	The server sets up its receiving socket in Non Blocking mode. Then we use a combination 
	of file descriptors, the functions FD_ and select() to wait for and accept incoming 
	connections. 
	Once a new connection arrives, we create a new socket specifically for the incoming 
	client and start a new thread that will take care of its file transfer in a brand new 
	file named after the client connetion order. Inside the thread we also use select and 
	only one file descriptor to detect if there is data to be read, but also to take care of 
	the 15 seconds timeout. Once the file transfer is done or failed, the separate thread 
	closes the socket, file and exits.  

Issues faced:
	The first Issue was that the file wasn't getting correctly transfered for pdf format for 
	instance. It was partially transfered and had a few discrepencies with the original one. 
	It happened because I was trying to do the transfer using C++ file streams, read the 
	file in a string stream, converiting it to a c string and send it over. However, some 
	information got lost or misinterpreted into the conversions. So, I decided to do it the 
	C way whih turned out to be more general, correct and faster.

	Another Issue was the design of the server. I first make it work without using threads, 
	only using one select() and an array of 10 possible clients. However, I was not able to 
	make it work properly because we needed to use select a second time to get a 15 seconds 
	timeout for handling the client.

	I then started to have an array of 10 clients and threads, and start a thread for each 
	client. Each thread handling the connection using a select() with a 15 sec delay. 
	However, keeping a record of clients into an array and threads made it overly 
	complicated to keep track and reuse ressources once one client was done. The biggest 
	issue was that the thread needed to be joined, forcing the server to wait and potentialy 
	loose some client connections because of that. 

	That is why I opted for the above mentioned designed, which helped me overcome the 
	challenges I faced.


Online ressources:
	https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/

	https://www.binarytides.com/multiple-socket-connections-fdset-select-linux/

	linux MAN pages

	www.cplusplus.com

	www.tutorialspoint.com


############# END OF REPORT ###############


## Makefile

This provides a couple make targets for things.
By default (all target), it makes the `server` and `client` executables.

It provides a `clean` target, and `tarball` target to create the submission file as well.

You will need to modify the `Makefile` to add your userid for the `.tar.gz` turn-in at the top of the file.

## Academic Integrity Note

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.

## Provided Files

`server.cpp` and `client.cpp` are the entry points for the server and client part of the project.

