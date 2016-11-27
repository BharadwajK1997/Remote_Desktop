/*
**listener.c -- a datagram sockets "server" demo
*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
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

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100*101*sizeof(int)


typedef struct pixelarray{
    int pixels[100][100];
    int x;
    int y;
}pixelarray;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
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

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
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

    /*Display * display = XOpenDisplay((char*) NULL);
	Window parent;
	int x, y;
	int width, height;
	int border_width, border, background;
	Window w = XCreateSimpleWindow(&display, parent, x, y, width, height, border_width, border, background);
    int width = 100;
    int height = 100;
    int depth = 32;
    int bitmap_pad = 32;
    int bytes_per_line = 0;
    Display * display = XOpenDisplay(0);
    if(display == NULL){
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
        int screen = DefaultScreen(display);
    Window w = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 100, 100, 1, BlackPixel(display, screen), WhitePixel(display, screen));
    XSetWindowBackgroundPixmap(display, w, pixelmap);
    XMapWindow(display, w);*/

    Display *d;
    Window w;
    XEvent e;
    char *msg = "Hello, World!";
    int screen;
           
    d = XOpenDisplay(NULL);
    if (d == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
    screen = DefaultScreen(d);
    w = XCreateSimpleWindow(d, RootWindow(d, screen), 0, 0, 1500, 1500, 1, BlackPixel(d, screen), WhitePixel(d, screen));
    XSelectInput(d, w, ExposureMask | KeyPressMask);
    XMapWindow(d, w);
                               
    //while (1) {
                   //unsigned char *image32=(unsigned char *)malloc(width*height*4);
            //XImage *img = XCreateImage(d, CopyFromParent, depth, XYPixmap, 0, image32, width, height, bitmap_pad, bytes_per_line);
            /*XImage *img = XGetImage(d, RootWindow(d, screen), 0, 0, 100, 100, AllPlanes, ZPixmap);
            Pixmap pixelmap = XCreatePixmap(d, w, width, height, depth);
            XGCValues gcvalues;
            GC gc = XCreateGC(d, pixelmap, 0, &gcvalues);
            XPutImage(d, w, gc, img, 100, 100, 0, 0, width, height);*/
            //XFreePixMap(d, pixelmap);

       // }
//        if (e.type == KeyPress)
  //          break;
//  }
    int height = 800;
    int width = 800;
    XImage *im = XCreateImage(d, XDefaultVisual(d, screen), DisplayPlanes(d,screen), ZPixmap, 0, (char*)NULL, height, width, 32, 0);
    im->data = malloc(im->bytes_per_line * height); 
    XInitImage(im);
    while(1){ 
        //XNextEvent(d, &e);
        /* if (e.type == Expose) {
            XFillRectangle(d, w, DefaultGC(d, screen), 20, 20, 10, 10);
            XDrawString(d, w, DefaultGC(d, screen), 10, 50, msg, strlen(msg));
         }*/
        if ((numbytes = recvfrom(sockfd, &buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            printf("recvfrom");
            exit(1);
        }
        if(numbytes != (100*100)*sizeof(int) + 2*sizeof(int)){
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
        /* 
        while(ctr < (100*100)*sizeof(int)){
            int pixel;
            memcpy(&pixel, buf + ctr, sizeof(int));
            ctr += sizeof(int);
            XPutPixel(im, xcoord + iter/height, ycoord + iter%width, pixel);
            iter++;
        }
        XPutImage(d, w, DefaultGC(d, screen), im, 0,0, xcoord, ycoord, width, height);
        */
        for(int xiter = 0; xiter<100; xiter++){
            for(int yiter = 0; yiter<100; yiter++){
                XPutPixel(im, xcoord + xiter, ycoord + yiter, buf.pixels[xiter][yiter]);
            }
        }
        XPutImage(d, w, DefaultGC(d, screen), im, 0,0, 0, 0,  width, height);

        /*printf("listener: got packet from %s\n",
        
        inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s));
        printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buf);*/
    }
    close(sockfd);
    
    XCloseDisplay(d);
    printf("closing\n");
    return 0;
}
