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













