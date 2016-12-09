/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 1024

typedef struct netEvent{
	int type;
	KeySym k;
	int modstate;
	int mx;
	int my;
	int buttonpress;
} NetEvent;

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

XKeyEvent createKeyEvent(Display *display, Window  * win, Window * winRoot, int press, int keycode, int modifiers);

void sendMouseEvent(int button,int press);





int main(int argc, char **argv) {
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    NetEvent net;

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");
   Display *d;
   int s;
 
                        /* open connection with the server */
   d = XOpenDisplay(NULL);
   if (d == NULL) {
     fprintf(stderr, "Cannot open display\n");
     exit(1);
   }
 
   s = DefaultScreen(d); 

    while(1){
      n = read(sockfd, &net, sizeof(NetEvent));
      if (n < 0) {
        error("ERROR reading from socket");
      }
			Window winFocus;
			Window winRoot = RootWindow(d, s);
      int revert;
      XGetInputFocus(d, &winFocus, &revert);
			if(net.type == KeyPress){
        XKeyEvent event = createKeyEvent(d, &winFocus, &winRoot, 1, net.k, net.modstate);
        XSendEvent(event.display, event.window, True, KeyPressMask | KeyReleaseMask, (XEvent *)&event);
        XFlush(d);
      }else if(net.type == KeyRelease){
        XKeyEvent event = createKeyEvent(d, &winFocus, &winRoot, 0, net.k, net.modstate);
        XSendEvent(event.display, event.window, True, KeyPressMask | KeyReleaseMask, (XEvent *)&event);
        XFlush(d);
      }else if(net.type == ButtonPress){
        sendMouseEvent(net.buttonpress,1);
				XWarpPointer(d, None, winRoot, 0, 0, 0, 0, net.mx, net.my);
      }else if(net.type == ButtonRelease){
        sendMouseEvent(net.buttonpress,0);
				XWarpPointer(d, None, winRoot, 0, 0, 0, 0, net.mx, net.my);
      }else if(net.type == MotionNotify){
				XWarpPointer(d, None, winRoot, 0, 0, 0, 0, net.mx, net.my);
      }
    }
    close(sockfd);
    return 0;
}

XKeyEvent createKeyEvent(Display *display, Window  * win, Window * winRoot, int press, int keycode, int modifiers)
{
   XKeyEvent event;

   event.display     = display;
   event.window      = *win;
   event.root        = *winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = True;
   event.keycode     = XKeysymToKeycode(display, keycode);
   event.state       = modifiers;

   if(press)
      event.type = KeyPress;
   else
      event.type = KeyRelease;

   return event;
}

void sendMouseEvent(int button,int press){

  Display *display = XOpenDisplay(NULL);
  XEvent event;
  memset(&event, 0x00, sizeof(event));
	if(press){
		event.type = ButtonPress;
	}else{
    event.type = ButtonRelease;
    event.xbutton.state = 0x100;
	}
  event.xbutton.button = button;
  event.xbutton.same_screen = True;
	XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  event.xbutton.subwindow = event.xbutton.root;
	while(event.xbutton.subwindow){
  	event.xbutton.window = event.xbutton.subwindow;
  	XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  }

   if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0)
			fprintf(stderr, "Error\n");

    XFlush(display);



    XCloseDisplay(display);
}


