#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"    // the port users will be connecting to
#define BUFSIZE 1024

typedef struct _chunk{
  int pixels[100][100];
  int x;
  int y;
} chunk;

typedef struct netEvent{
	int type;
	KeySym k;
	int modstate;
	int mx;
	int my;
	int buttonpress;
} NetEvent;

char * getX11coords();
int ** getChunkArray();
chunk * getChunk(int ** imgarr,int chunkX,int chunkY);
void freeChunkArray(int ** arr);

XKeyEvent createKeyEvent(Display *display, Window  * win, Window * winRoot, int press, int keycode, int modifiers);
void sendMouseEvent(int button,int press);


int main(){
//SCREENGRAB
int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    if (argc != 2) {
        fprintf(stderr,"usage: talker hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
  while(1){
    fprintf(stderr,"got here!\n");
    int ** chunkArr = getChunkArray();
    for(int chunkX = 0;chunkX < 8;chunkX++){
      for(int chunkY = 0;chunkY < 8;chunkY++){
        chunk * ch = getChunk(chunkArr,chunkX,chunkY);
          fprintf(stderr,"%d %d, ",ch->x,ch->y); 
          if ((numbytes = sendto(sockfd, ch, sizeof(chunk), 0,
                   p->ai_addr, p->ai_addrlen)) == -1) {
              perror("talker: sendto");
              exit(1);
          }
        free(ch);
        usleep(100000);
      }
    }
    fprintf(stderr,"\n"); 
    freeChunkArray(chunkArr);
  }
    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);
//INPUT
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
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
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
      printf("received stuff!\n");
			if(net.type == KeyPress){
        printf("received keypress!\n");
        XKeyEvent event = createKeyEvent(d, &winFocus, &winRoot, 1, net.k, net.modstate);
        XSendEvent(event.display, event.window, True, KeyPressMask | KeyReleaseMask, (XEvent *)&event);
        XFlush(d);
      }else if(net.type == KeyRelease){
        printf("received keyrelease!\n");
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
        printf("received mouse moved to %d %d!\n",net.mx,net.my);
				XWarpPointer(d, None, winRoot, 0, 0, 0, 0, net.mx, net.my);
      }
    }
    //printf("Echo from server: %s", buf);
    close(sockfd);

}

//SCREEN GRAB FUNCTIONS

int ** getChunkArray(){
    Display *d = XOpenDisplay((char *) NULL);
    Screen * def_screen = XDefaultScreenOfDisplay(d);
    XImage * image = XGetImage (d, RootWindow (d, DefaultScreen (d)), 0, 0, 800, 800,AllPlanes, ZPixmap);

    int ** imgarr = (int **)malloc(sizeof(int *)*800);
    for(int ctrx = 0;ctrx < 800;ctrx++){
      imgarr[ctrx] = (int *) malloc(800 *sizeof(int));
      for(int ctry=0;ctry<800;ctry++){
        imgarr[ctrx][ctry]=(int)XGetPixel(image,ctrx,ctry);
      }
    }
    XFree(image);
    return imgarr;
}

chunk * getChunk(int ** imgarr,int chunkX,int chunkY){
  chunk * c = (chunk *)malloc(sizeof(chunk));
  c->x=chunkX * 100;
  c->y=chunkY * 100;
  for(int ctrx=0;ctrx<100;ctrx++){
    for(int ctry=0;ctry<100;ctry++){
      c->pixels[ctrx][ctry] = imgarr[ctrx+c->x][ctry+c->y];
    }
  }
  return c;
}

void freeChunkArray(int ** arr){
  for(int ctr =0;ctr< 100;free(arr[ctr]),ctr++);
  free(arr);
}

//INPUT SENDING FUNCTIONS

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
  event.xbutton.subwindow = event.xbutton.root;//event.xbutton.window;
	while(event.xbutton.subwindow){
  	event.xbutton.window = event.xbutton.subwindow;
  	XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
  }

   if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0)
			fprintf(stderr, "Error\n");

    XFlush(display);



    XCloseDisplay(display);
}











