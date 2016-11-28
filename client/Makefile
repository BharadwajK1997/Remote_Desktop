all: listener.c
	gcc -g -Wall -o test_listener listener.c -lX11 -std=c99 -D_POSIX_SOURCE
	gcc -g -Wall -o test_tcpserver tcpserver.c -lX11 -std=c99 -D_POSIX_SOURCE
	gcc -g -Wall -o final_test final.c -lX11 -pthread -std=c99 -D_POSIX_SOURCE
clean:
	$(RM) test_listener
	$(RM) test_tcpserver
	$(RM) final_test
