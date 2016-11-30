all: 
	gcc -g -Wall -o test_tcpserver tcpserver.c -lX11
	gcc -g -Wall -o final_test finaltest1.c -lX11 -pthread
clean:
	$(RM) test_listener
	$(RM) test_tcpserver
	$(RM) final_test
