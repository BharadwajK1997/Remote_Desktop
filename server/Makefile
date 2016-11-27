all: network.c final.c tcp.c 
	gcc -g -Wall -o test_network network.c -lX11
	gcc -g -Wall -o test_tcp tcp.c -lX11
	gcc -g -Wall -o final final.c -lX11
clean:
	$(RM) test_tcp
	$(RM) test_network
	$(RM) tcp
