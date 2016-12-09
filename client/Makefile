all: 
	gcc -g -Wall -o test_tcpserver tcpserver.c -lX11
	gcc -g -Wall -o final_test Remote_Desktop_Client.c -lX11 -pthread
	gcc -g -Wall -o tcp_final tcpfinal.c -lX11 -pthread
clean:
	$(RM) test_listener
	$(RM) test_tcpserver
	$(RM) final_test
