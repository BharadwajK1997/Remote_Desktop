all: listener.c
	gcc -g -Wall -o test_listener listener.c -lX11
	gcc -g -Wall -o test_tcpserver tcpserver.c -lX11
	gcc -g -Wall -o final_test finaltest.c -lX11 -pthread
clean:
	$(RM) test_listener
	$(RM) test_tcpserver
	$(RM) final_test
