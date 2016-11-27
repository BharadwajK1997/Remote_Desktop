#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct event{
    // key stuff
    int type;
    KeySym k;
    int modstate;
    // mouse stuff
    int x;
    int y;
    int buttonpress;
}myEvent;

int main(int argc, char **argv)
{

    Display * d;
    Window w;
    XEvent e;
    int screen;

    d = XOpenDisplay(NULL);
    if(d == NULL){
        perror("display\n");
        exit(1);
    }

    screen = DefaultScreen(d);
    w = XCreateSimpleWindow(d, RootWindow(d,screen), 10, 10, 100, 100, 1, BlackPixel(d,screen), WhitePixel(d,screen));
	XSelectInput(d, w, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask);
	XMapWindow(d,w);  
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, "1234", &hints, &result);
    if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(1);
    }
    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(sock_fd, 10) != 0) {
        perror("listen()");
        exit(1);
    }

    struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
    printf("Waiting for connection...\n");
    int client_fd = accept(sock_fd, NULL, NULL);
    printf("Connection made: client_fd=%d\n", client_fd);
   
    /*char buffer[1000];
    int len = read(client_fd, buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    printf("Read %d chars\n", len);
    printf("===\n");
    printf("%s\n", buffer);
    */
    printf("hi\n");
    while(1){
        XNextEvent(d, &e);
        myEvent data;
        printf("processing event\n");
        if(e.type == KeyPress || e.type == KeyRelease){
            KeySym k;
            char stuff[10];
            XLookupString(&(e.xkey), (char*) stuff, 10, &k, NULL);
            data.k = k;
            data.type = e.type;
            data.modstate = e.xkey.state;
            printf("writing: %d %d %c\n", data.type, data.modstate,stuff[0]);
            write(client_fd, &data, sizeof(myEvent));
        }
        else if(e.type == ButtonPress){
            data.x = e.xbutton.x;
            data.y = e.xbutton.y;
            data.buttonpress = e.xbutton.button;
            data.type = e.type;
            printf("writing: %d %d %d %d\n", data.x, data.y, data.buttonpress, data.type);
            write(client_fd, &data, sizeof(myEvent));
        }
        else if(e.type == ButtonRelease){
            data.type = e.type;
            data.x = e.xbutton.x;
            data.y = e.xbutton.y;
            data.buttonpress = e.xbutton.button;
            printf("writing: %d %d %d %d\n", data.x, data.y, data.buttonpress, data.type);
            write(client_fd, &data, sizeof(myEvent));
        }
        else if(e.type == MotionNotify){
            data.x = e.xmotion.x;
            data.y = e.xmotion.y;
            data.type = e.type;
            printf("writing: %d %d %d\n", data.x, data.y, data.type);
            write(client_fd, &data, sizeof(myEvent));
        }
    }
    close(sock_fd);
	/*
    char buffer[1000] = "Hello World";
    buffer[11] = '\0';
    write(client_fd, buffer, 11);*/
    return 0;
}
