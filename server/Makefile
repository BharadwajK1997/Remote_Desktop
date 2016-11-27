all: network.c x11_test.c x11.c 
	gcc -g -Wall -o test_x11 x11_test.c -lX11 -pthread -lXext
	gcc -g -Wall -o test_network network.c -lX11
	gcc -g -Wall -o x11 x11.c -lX11
	gcc -g -Wall -o mouse_x11 x11_mouse.c -lX11
	gcc -g -Wall -o tcp tcp.c -lX11
clean:
	$(RM) test_x11
	$(RM) test_network
	$(RM) x11
