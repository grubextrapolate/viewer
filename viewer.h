#ifdef OS_Darwin
  #include <glut.h>
  #include <gl.h>
#else
  #include <GL/glut.h>
  #include <GL/gl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef _WIN32
   #include <unistd.h>
#endif

#define ALIGN 0
#define VIEWER 1
#define MONOVIEW 2

#define RGB 3
#define RGBA 4

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LEFT 0
#define RIGHT 1
#define BOTH 2

#define RGB 3
#define RGBA 4
#define STRING_SIZE 132

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* defines a RGB color data type*/
typedef struct {
   float r;
   float g;
   float b;
} COLOR;

/* defines a texture data type */
typedef struct TEXTURE {
   GLubyte *tex;
   int width;
   int height;
   int x;
   int y;
   int x1;
   int x2;
   int y1;
   int y2;
   struct TEXTURE *thumb;
} TEXTURE;

/* defines each pair in a slideshow */
typedef struct PAIR {
   char *left_file;
   char *right_file;
   int x_offset;
   int y_offset;
   struct PAIR *next;
   struct PAIR *prev;
} PAIR;

/* a list of pairs */
typedef struct PAIRLIST {
   PAIR *head;
   PAIR *tail;
   PAIR *cur;
   int num;
} PAIRLIST;

/* global variables */
extern int first_time;
extern int screen_x;
extern int screen_y;
extern TEXTURE *left;
extern TEXTURE *right;
extern TEXTURE *full;
extern TEXTURE *zoomLeft;
extern TEXTURE *zoomRight;
extern char *fullOutfile;
extern char *leftOutfile;
extern char *rightOutfile;
extern char *basefilename;
extern int mode;
extern int zoom;
extern double szoom;
extern int mousex1;
extern int mousey1;
extern int fine_align;
extern int thumb_size;
extern int nothumb;
extern int x_offset;
extern int y_offset;
extern PAIRLIST *list;
extern int clone_mode;
extern int fullscreen;

extern int leftDown;
extern int middleDown;
extern int rightDown;

/* function prototypes for viewer */
void displayFuncView(void); /* the display function (for viewer) */
void resizeFuncView(int, int); /* the resize function */
void menuFuncView(int); /* the menu function */
void keyboardFuncView(unsigned char, int, int); /* (for viewer) */
void specialFuncView(int, int, int);
void mouseFuncView(int, int, int, int);
void motionFuncView(int, int);

/* function prototypes for mono viewer */
void displayFuncViewMono(void); /* the display function (for viewer) */
void keyboardFuncViewMono(unsigned char, int, int); /* (for viewer) */
void specialFuncViewMono(int, int, int);
void motionFuncViewMono(int, int);
void resizeFuncViewMono(int, int); /* the resize function */

/* function prototypes for aligner */
void displayFuncAlign(void); /* the display function */
void keyboardFuncAlign(unsigned char, int, int);
void specialFuncAlign(int, int, int);
void menuFuncAlign(int); /* the menu function */
void resizeFuncAlign(int, int); /* the resize function */
void mouseFuncAlign(int, int, int, int);
void motionFuncAlign(int, int);

/* function prototypes for main */
void processArgs(int, char **);
TEXTURE *read_texture(char *); 
void write_ppm(char *, COLOR ***, int, int);
void write_texture(char *, TEXTURE *);
void write_cropped_texture(char *, TEXTURE *, int, int, int, int);
void calcWindow(TEXTURE *);
void cleanup();
void die(char *, ...);
void debug(char *, ...);
int ipow(int, int);
TEXTURE *zoomImage(TEXTURE *, int);
TEXTURE *zoomImageSmooth(TEXTURE *, double);
void showPos(TEXTURE *, int, TEXTURE *);
TEXTURE *makeThumb(TEXTURE *);
int isjpeg(char *);
void drawBox(int, int, int, int, int *color, int);
void drawFilledBox(int, int, int, int, int *color, int);
void readPair(PAIR *);
void readFileList(char *, PAIRLIST **);
void getNextPair(PAIRLIST *);
void getPrevPair(PAIRLIST *);
void freeTexture(TEXTURE **);
int stereoCheck();
void findPairs(int, char **);
void sortList(int, char **);

/* function prototypes for example */
TEXTURE *read_JPEG_file (char *);
void write_JPEG_file(char *, int, TEXTURE *);
void readAndSplit(char *);

/* function prototypes for list */
extern PAIR *newPair(char *, char *);
extern void addPair(PAIR *, PAIRLIST **);
extern void removePair(PAIR *, PAIRLIST **);
extern void deletePair(PAIR *, PAIRLIST **);
extern void freeList(PAIRLIST **);
extern void initList(PAIRLIST **);
extern void freePair(PAIR *);
