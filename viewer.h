#include <GL/glut.h>
#include <GL/gl.h>
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

#define winHeight 400 /* window width */
#define winWidth 400  /* window height */

#define LARGE_NUM 9999999999
#define STRING_SIZE 132

#define DEBUG

/* defines a RGB color data type*/
typedef struct {
   float r;
   float g;
   float b;
} COLOR;

/* defines a texture data type */
typedef struct {
   COLOR **tex;
   int width;
   int height;
} TEXTURE;

float pi = 3.14159265358979; /* pi, obviously */

/*
 * this is the cleanup function. it deallocates all polygon memory and
 * exits the program.
 */
void cleanup() {

   exit(0);

}

/* displays an error message and exits the program */
void die(char *fmt, ...) {
   va_list ap;
   va_start(ap, fmt);
   vfprintf(stderr, fmt, ap);
   exit(-1);
}

/* displays a debugging message if debugging is turned on */
void debug(char *fmt, ...) {
#ifdef DEBUG
   va_list ap;
   va_start(ap,fmt);
   vfprintf(stderr, fmt, ap);
#endif
}

double sign(double arg) {
   double ret;

   if (arg < 0) ret = -1;
   else if (arg > 0) ret = 1;
   else ret = 0;

   return ret;
}

int max(int a, int b) {

   if (a > b) return a;
   else return b;
}

int min(int a, int b) {

   if (a < b) return a;
   else return b;   
}  

double fmax(double a, double b) {

   if (a > b) return a;
   else return b;
}

double fmin(double a, double b) {

   if (a < b) return a;
   else return b;   
}  

/*
 * calculates the distance between two points.
 */
float distance(int x1, int y1, int x2, int y2) { 
  
   int dx, dy;

   dx = x2 - x1;
   dy = y2 - y1;
   
   return(sqrt(dx*dx + dy*dy));
   
}

