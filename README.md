# SER_CLIC
A client-server architecture in C++ using sockets to transfer multiple files 
<hr>

## LINUX:
The default `gcc` compiler that comes with the linux distributions is enough to compile and run the code.<br>
>Server : `g++ -std=c++17 -pthread server.cpp -o server`<br>
>
>Client : `g++ -std=c++17 -pthread client.cpp -o client`<br>
>
> Usage : `./server` `./client <ipaddress/address_name>`<br>

## WINDOWS:
The `visual-c++` compiler can be used to compile and run the code without any additional libraries. While using the `mingw` or any `gcc` based compilers the winsock and other libraries has to be mannualy linked using explicit `lwsock32` and `lws2_32` flags for the compiler args<br>
>Server : `g++ server.cpp -o server.exe -lwsock32 -lws2_32`<br>
>
>Client :  `g++ client.cpp -o client.exe -lwsock32 -lws2_32`<br>
>
>Usage : `server`   `client <ipaddress/address_name>`<br>

### NOTE:
The code has been run and tested on linux and windows systems. But porting to other operating systems is relatively easy. If any assistance needed feel free to raise a ticket.
