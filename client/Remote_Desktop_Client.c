#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
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
#include <pthread.h>
#include <sys/wait.h>

#define UDPPORT "4950"    // UDP Port for receiving chunks
#define TCPPORT "1234"
#define MAXBUFLEN chunksize*(chunksize+1)*sizeof(int)

#define height_image 1500
#define width_image 3000
#define scale_factor 2
#define chunksize 125
typedef struct pixelarray{
    int pixels[chunksize][chunksize];
    int x;
    int y;
}pixelarray;

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

Window window_from_name_search(Display *display, Window current, char const *needle) {
  Window retval, root, parent, *children;
  unsigned children_count;
  char *name = NULL;

  /* Check if this window has the name we seek */
  if(XFetchName(display, current, &name) > 0) {
    int r = strcmp(needle, name);
    XFree(name);
    if(r == 0) {
      return current;
    }
  }

  retval = 0;

  /* If it does not: check all subwindows recursively. */
  if(0 != XQueryTree(display, current, &root, &parent, &children, &children_count)) {
    unsigned i;
    for(i = 0; i < children_count; ++i) {
      Window win = window_from_name_search(display, children[i], needle);

      if(win != 0) {
        retval = win;
        break;
      }
    }

    XFree(children);
  }

  return retval;
}

// open display connection, start searching from the root window.
Window window_from_name(char const *name) {
  Display *display = XOpenDisplay(NULL);
  Window w = window_from_name_search(display, XDefaultRootWindow(display), name);
  XCloseDisplay(display);
  return w;
}

void tcpserver(){
    Display * d;
    XEvent e;
    int screen;

    d = XOpenDisplay(NULL);
    if(d == NULL){
        perror("display\n");
        exit(1);
    }
    screen = DefaultScreen(d);
    Window w = window_from_name("helloworld");
    XSelectInput(d, w, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask);
    XMapWindow(d,w);  
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    s = getaddrinfo(NULL, TCPPORT, &hints, &result);
    if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(1);
    }
    int optval = 1;
    int ctr = 0;
    ctr++;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &optval, sizeof(optval));
    int ret = bind(sock_fd, result->ai_addr, result->ai_addrlen);
    if (ret != 0) {
        close(sock_fd);
        ctr++;

        perror("bind() heyy");
    }

    if (listen(sock_fd, 10) != 0) {
        close(sock_fd);
        perror("listen()");
    }

    struct sockaddr_in *result_addr = (struct sockaddr_in *) result->ai_addr;
    printf("Listening on file descriptor %d, port %d\n", sock_fd, ntohs(result_addr->sin_port));
    printf("Waiting for connection...\n");
    int client_fd = accept(sock_fd, NULL, NULL);
    printf("Connection made: client_fd=%d\n", client_fd);
   
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
            data.x = e.xbutton.x*scale_factor;
            data.y = e.xbutton.y*scale_factor;
            data.buttonpress = e.xbutton.button;
            data.type = e.type;
            printf("writing: %d %d %d %d\n", data.x, data.y, data.buttonpress, data.type);
            write(client_fd, &data, sizeof(myEvent));
        }
        else if(e.type == ButtonRelease){
            data.type = e.type;
            data.x = e.xbutton.x*scale_factor;
            data.y = e.xbutton.y*scale_factor;
            data.buttonpress = e.xbutton.button;
            printf("writing: %d %d %d %d\n", data.x, data.y, data.buttonpress, data.type);
            write(client_fd, &data, sizeof(myEvent));
        }
	else if(e.type == MotionNotify){
	    data.x = e.xmotion.x*scale_factor;
	    data.y = e.xmotion.y*scale_factor;
	    data.type = e.type;
	    write(client_fd, &data, sizeof(myEvent));
	}
    }
    close(sock_fd);
}

int main(void)
{

    Display *d;
    int screen;
           
    d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
    screen = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, screen), 0, 0, width_image/scale_factor, height_image/scale_factor, 1, BlackPixel(d, screen), WhitePixel(d, screen));
    char * windowname;
    XStoreName(d,w, "helloworld");
    XFetchName(d,w, &windowname);
    int child = fork();
    if(child == 0){
        tcpserver();
        return 0;
    }
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    
    pixelarray buf;

    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, UDPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            printf("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            printf("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof(their_addr);
    XSelectInput(d, w, KeyPressMask);
    XMapWindow(d, w);
                               
    int height = height_image/scale_factor;
    int width = width_image/scale_factor;
    XImage *im = XCreateImage(d, XDefaultVisual(d, screen), DisplayPlanes(d,screen), ZPixmap, 0, (char*)NULL, width, height, 32, 0);
    im->data = malloc(im->bytes_per_line * height); 
    XInitImage(im);
    

    while(1){ 
        if ((numbytes = recvfrom(sockfd, &buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            printf("recvfrom");
            exit(1);
        }
        if(numbytes != (chunksize*chunksize)*sizeof(int) + 2*sizeof(int)){
            printf("didn't get the right number of bytes bruh\n");
            continue;
        }     
        int ctr = 0;
        int xcoord;
        xcoord = buf.x;
        printf("xcoord %d\n", xcoord);
        int ycoord;
        ycoord = buf.y;
        printf("ycoord %d\n", ycoord);
        for(int xiter = 0; xiter<chunksize; xiter+=2){
            for(int yiter = 0; yiter<chunksize; yiter+=2){
                XPutPixel(im, (xcoord + xiter)/2, (ycoord + yiter)/2, buf.pixels[xiter][yiter]);
            }
        }
        XPutImage(d, w, DefaultGC(d, screen), im, 0,0, 0, 0,  width, height);
    }
    int status;
    waitpid(child, &status, 0);
    close(sockfd);
    XCloseDisplay(d);
    return 0;
}


