/*
** talker.c -- a datagram "client" demo
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

#define SERVERPORT "4950"    // the port users will be connecting to

typedef struct _chunk{
  int x;
  int y;
  int pixels[100][100];
} chunk;

typedef struct str{
    int a;
    int b;
} stru;


char * getX11coords();
int ** getChunkArray();
chunk * getChunk(int ** imgarr,int chunkX,int chunkY);
void freeChunkArray(int ** arr);

int main(int argc, char *argv[])
{
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
    int ** chunkArr = getChunkArray();
    for(int chunkX = 0;chunkX < 8;chunkX++){
      for(int chunkY = 0;chunkY < 8;chunkY++){
        chunk * ch = getChunk(chunkArr,chunkX,chunkY);
        chunk ch2= *ch;
          if ((numbytes = sendto(sockfd, &ch2, sizeof(chunk), 0,
                   p->ai_addr, p->ai_addrlen)) == -1) {
              perror("talker: sendto");
              exit(1);
          }
        free(ch);
        usleep(100000);
      }
    }
    freeChunkArray(chunkArr);
  }
    freeaddrinfo(servinfo);

    close(sockfd);

    return 0;
}

int ** getChunkArray(){
    Display *d = XOpenDisplay((char *) NULL);
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
