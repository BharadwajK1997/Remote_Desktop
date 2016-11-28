all: listener.c
<<<<<<< HEAD
	gcc -g -Wall -o test_listener listener.c -lX11
	gcc -g -Wall -o test_tcpserver tcpserver.c -lX11
	gcc -g -Wall -o final_test finaltest.c -lX11 -pthread
=======
	gcc -g -Wall -o test_listener listener.c -lX11 -std=c99 -D_POSIX_SOURCE
	gcc -g -Wall -o test_tcpserver tcpserver.c -lX11 -std=c99 -D_POSIX_SOURCE
	gcc -g -Wall -o final_test final.c -lX11 -pthread -std=c99 -D_POSIX_SOURCE
>>>>>>> 0773cfe696951cd53f7c0a31e58da074b3ae071d
clean:
	$(RM) test_listener
	$(RM) test_tcpserver
	$(RM) final_test
