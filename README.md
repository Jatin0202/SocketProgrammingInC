Application ID 4: Multi-stage DNS Resolving System using Client-Server socket programming

There are three files:
server.c
proxy.c
client.c

Symbols Used and value for example
<IP_Address_1> : IP Address on which server.c is running. (ex. 127.0.0.1)
<IP_Address_2> : IP Address on which proxy.c is running. (ex. 127.0.0.1)
<port_no_1>	   : Port no. on which server.c is running.	 (ex. 2000)
<port_no_2>	   : Port no. on which procy.c is running.	(ex. 3000)

For example I'm using loopback for both IP Address, 2000 for port_no_1 and 3000 for <port_no_2>

Step1: Run "gcc server.c -o server"
		   "gcc proxy.c -o proxy"
		   "gcc client.c -o client"  
	Upper 3 commands will create 3 object files namely server, proxy, client

Step2: Run "./server <port_no_1>"    example: ("./server 2000")	 
	This will start server code on 2000 port.

Step3:  Open another terminal, Run "./proxy <port_no_2> <ip_address_1> <port_no_1>"    example("./proxy 3000 127.0.0.1 2000")
	This will start proxy code on 3000 port. 
	<ip_address_1> and <port_no_1> are provided because proxy has to act as client(Sometime). So, Information about server.c(where it is running) is provided.
	<port_no_2> is port no on which proxy is running.
	
Step4: Open another terminal, Run "./client <ip_address_2> <port_no_2>"   example("./client 127.0.0.1 3000")
	This will start client code. 
	<ip_address_2> and <port_no_2> are provided as information on which proxy(as server for client) is running.

Now after that on 3 terminals, 3 codes are running.
There are 2 types of request(as mentioned in question)	
Type1: DomainName(Request) and IP Address(Response)
Type2: IP Address(Request) and DomainName(Response)	
ex. of Type 1 request: '1 www.google.com' 	Response (0.0.0.0)
ex. of Type 2 request: '2 1.1.1.1'	Response(www.fb.com)
ERROR: If your request start from anynumber except 1 or 2, there is a error from server side(No request of this type)

