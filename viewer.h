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
#include <unistd.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LEFT 0
#define RIGHT 1
#define BOTH 2

#define MAJOR 0
#define MINOR 7
#define PATCH 2

#define RGB 3
#define RGBA 4
#define STRING_SIZE 132

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

/* global variables */
extern int first_time;
extern int screen_x;
extern int screen_y;
extern int offset_x;
extern int offset_y;
extern TEXTURE *left;
extern TEXTURE *right;
extern TEXTURE *full;
extern TEXTURE *zoomLeft;
extern TEXTURE *zoomRight;
extern char *fullOutfile;
extern char *leftOutfile;
extern char *rightOutfile;
extern char *basename;
extern int mode;
extern int zoom;
extern int mousex1;
extern int mousey1;
extern int fine_align;
extern int thumb_size;

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
int max(int, int);
int min(int, int);
void cleanup();
void die(char *, ...);
void debug(char *, ...);
int ipow(int, int);
TEXTURE *zoomImage(TEXTURE *, int);
void showPos(TEXTURE *, int, int, int);
TEXTURE *makeThumb(TEXTURE *);
int isjpeg(char *);
void drawBox(int, int, int, int, int *color, int);
void drawFilledBox(int, int, int, int, int *color, int);

/* function prototypes for example */
TEXTURE *read_JPEG_file (char *);
void write_JPEG_file(char *, int, TEXTURE *);
void readAndSplit(char *);


