#include "viewer.h"

#define POSITION 0
#define VIEWER 1

/*
 * some function prototypes.
 */
void displayFunc(void); /* the display function */
void displayFunc2(void); /* the display function (for viewer) */
void resizeFunc(int, int); /* the resize function */
void resizeFunc2(int, int); /* the resize function (for viewer) */
void menuFunc(int); /* the menu function */
void menuFunc2(int); /* the menu function (for viewer) */
void keyboardFunc(unsigned char, int, int);
void keyboardFunc2(unsigned char, int, int); /* (for viewer) */
void specialFunc(int, int, int);
void processArgs(int, char **);
TEXTURE *read_texture(char *);
void write_ppm(char *, COLOR ***, int, int);
void write_texture(char *, TEXTURE *);
void write_cropped_texture(char *, TEXTURE *, int, int, int, int);
void writePixel(int, int, COLOR *);

/*
 * global variables
 */

int first_time = TRUE;
int screen_x = 1024;
int screen_y = 768;
int offset_x = 0;
int offset_y = 0;

TEXTURE *left = NULL;
int left_win = 0;
int left_x = 0;
int left_y = 0;

int right_win = 0;
TEXTURE *right = NULL;
int right_x = 0;
int right_y = 0;

TEXTURE *full = NULL;

char *fullOutfile = NULL;
char *leftOutfile = NULL;
char *rightOutfile = NULL;

int mode = POSITION;

/*
 * the main function. this sets up the global variables, creates menus,
 * associates the various callback functions with their opengl event, and
 * starts things running.
 */
int main(int argc, char **argv) {

   int mainmenu; /* menu id */

   processArgs(argc, argv);

   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);

   if (mode == POSITION) {

      screen_x = glutGet(GLUT_SCREEN_WIDTH)/2;
      screen_y = glutGet(GLUT_SCREEN_HEIGHT);

      debug("screen_x = %d, screen_y = %d\n", screen_x, screen_y);

      left_x = (screen_x - left->width)/2;
      left_y = (screen_y - left->height)/2;

      /* left window */
      glutInitWindowPosition(left_x, left_y);
      glutInitWindowSize(left->width, left->height);
      left_win = glutCreateWindow("stereo viewer: left image");

      glutDisplayFunc(displayFunc);
      glutReshapeFunc(resizeFunc);
      glutKeyboardFunc(keyboardFunc);
      glutSpecialFunc(specialFunc);

      /* menu */
      mainmenu = glutCreateMenu(menuFunc);
      glutAddMenuEntry("quit",99);

      glutAttachMenu(GLUT_RIGHT_BUTTON);

      right_x = (screen_x - right->width)/2;
      right_y = (screen_y - right->height)/2;

      /* right window */
      glutInitWindowPosition(right_x + screen_x + offset_x, right_y + offset_y);
      glutInitWindowSize(right->width, right->height);
      right_win = glutCreateWindow("stereo viewer: right image");

      glutDisplayFunc(displayFunc);
      glutReshapeFunc(resizeFunc);
      glutKeyboardFunc(keyboardFunc);
      glutSpecialFunc(specialFunc);

   } else {
      glutInitWindowPosition(0, 0);
      glutInitWindowSize(full->width, full->height);
      left_win = glutCreateWindow("stereo viewer");

      glutDisplayFunc(displayFunc2);
      glutReshapeFunc(resizeFunc2);
      glutKeyboardFunc(keyboardFunc2);

      /* menu */
      mainmenu = glutCreateMenu(menuFunc2);
      glutAddMenuEntry("quit",99);

      glutAttachMenu(GLUT_RIGHT_BUTTON);

   }

   glutMainLoop();

   return 0;

}

void showUsage() {

   printf("stereo pair viewer/aligner. usage:\n");
   printf("viewer [options]\n");
   printf("       -i leftinfile.ppm rightinfile.ppm\n");
   printf("       -v fullinfile.ppm\n");
   printf("       -o fulloutfile.ppm [default = fullout.ppm]\n");
   printf("       -l leftoutfile.ppm [default = leftout.ppm]\n");
   printf("       -r rightoutfile.ppm [default = rightout.ppm]\n");
   printf("\nmust contain either the -i or -v options.\n");
   printf("if the -o, -l, or -r options are omitted default will be used\n");
   printf("\ncommands within program:\n");
   printf("cursor keys             move left image by 1 pixel\n");
   printf("cursor keys+shift       move right image by 1 pixel\n");
   printf("cursor keys+ctrl        move left image by 10 pixels\n");
   printf("cursor keys+shift+ctrl  move right image by 10 pixels\n");
   printf("esc or q                quit\n");
   printf("enter                   write images\n\n");
}

void processArgs(int argc, char **argv) {

   int i = 1;

   if (argc == 1) { /* display usage */
      showUsage();
      exit(-1);
   }

   while (i < argc) {

      if (strcmp(argv[i], "-o") == 0) {
         if (i+1 < argc) {
            i++;
            fullOutfile = argv[i];
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-l") == 0) {
         if (i+1 < argc) {
            i++;
            leftOutfile = argv[i];
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-r") == 0) {
         if (i+1 < argc) {
            i++;
            rightOutfile = argv[i];
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-i") == 0) {
         if (i+2 < argc) {
            i++;
            left = read_texture(argv[i]);
            i++;
            right = read_texture(argv[i]);
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-v") == 0) {
         if (i+1 < argc) {
            i++;
            full = read_texture(argv[i]);
            mode = VIEWER;
         } else {
            showUsage();
            exit(-1);
         }
      } else {
         showUsage();
      }
      i++;
   }

}

void displayFunc(void) {
   int i, j;

   if (first_time) {
      glutSetWindow(left_win);
      glutPositionWindow(left_x, left_y);
      glutSetWindow(right_win);
      glutPositionWindow(right_x + screen_x + offset_x, right_y + offset_y);
      first_time = FALSE;
   }

   glutSetWindow(left_win);
   for (i = 0; i < left->width; i++) {
      for (j = 0; j < left->height; j++) {
         glBegin(GL_POINTS);
            glColor3f(left->tex[j][i].r, left->tex[j][i].g, 
                      left->tex[j][i].b);
            glVertex2i(i, left->height - j);
         glEnd();
      }
   }

   glutSetWindow(right_win);
   for (i = 0; i < right->width; i++) {
      for (j = 0; j < right->height; j++) {
         glBegin(GL_POINTS);
            glColor3f(right->tex[j][i].r, right->tex[j][i].g, 
                      right->tex[j][i].b);
            glVertex2i(i, right->height - j);
         glEnd();
      }
   }
   glFlush();
}

void displayFunc2(void) {
   int i, j;

   if (first_time) {
      glutPositionWindow(0, 0);
      first_time = FALSE;
   }

   for (i = 0; i < full->width; i++) {
      for (j = 0; j < full->height; j++) {
         glBegin(GL_POINTS);
            glColor3f(full->tex[j][i].r, full->tex[j][i].g, 
                      full->tex[j][i].b);
            glVertex2i(i, full->height - j);
         glEnd();
      }
   }

   glFlush();
}

void keyboardFunc(unsigned char key, int x, int y) {

   int crop_x1, crop_x2, crop_y1, crop_y2, i, j;
   int start_x, start_y;

   debug("keyboardFunc: key is (ascii) %d\n", key);

   switch(key) {
      case 13:
         debug("return key pressed.\n");

         glutSetWindow(left_win);
         left_x = glutGet(GLUT_WINDOW_X);
         left_y = glutGet(GLUT_WINDOW_Y);
         glutSetWindow(right_win);
         right_x = glutGet(GLUT_WINDOW_X) - screen_x - offset_x;
         right_y = glutGet(GLUT_WINDOW_Y) - offset_y;

         debug("left image is %dx%d at %dx%d\n",
               left->width, left->height, left_x, left_y);
         debug("right image is %dx%d at %dx%d, offset by %dx%d\n",
               right->width, right->height, right_x, right_y, screen_x +
               offset_x, offset_y);

         crop_x1 = max(left_x, right_x);
         crop_x2 = min(left_x + left->width, right_x + right->width);
         crop_y1 = max(left_y, right_y);
         crop_y2 = min(left_y + left->height, right_y + right->height);

         debug("crop_x1 = %d, crop_x2 = %d, crop_y1 = %d, crop_y2 = %d\n",
               crop_x1, crop_x2, crop_y1, crop_y2);

         if (crop_x1 < 0) crop_x1 = 0;
         if (crop_x2 > screen_x) crop_x2 = screen_x;
         if (crop_y1 < 0) crop_y1 = 0;
         if (crop_y2 > screen_y) crop_y2 = screen_y;

         debug("crop_x1 = %d, crop_x2 = %d, crop_y1 = %d, crop_y2 = %d\n",
               crop_x1, crop_x2, crop_y1, crop_y2);

         full = (TEXTURE *)malloc(sizeof(TEXTURE));
         if (full == NULL) die("keyboardFunc: malloc failure\n");

         full->width = screen_x*2;
         full->height = screen_y;
         full->tex = (COLOR **)malloc(sizeof(COLOR *)*screen_y);
         if (full->tex == NULL) die("keyboardFunc: malloc failure\n");

         for (i = 0; i < screen_y; i++) {
            full->tex[i] = (COLOR *)malloc(sizeof(COLOR)*screen_x*2);
            if (full->tex[i] == NULL) die("keyboardFunc: malloc failure\n");
         }

         for (i = 0; i < screen_y; i++) {
            for (j = 0; j < screen_x*2; j++) {
               full->tex[i][j].r = 0;
               full->tex[i][j].g = 0;
               full->tex[i][j].b = 0;
            }
         }

         start_x = (screen_x - crop_x2 - crop_x1)/2;
         start_y = (screen_y - crop_y2 - crop_y1)/2;

         /* calc crop */
         for (i = crop_y1; i < crop_y2; i++) {
            for (j = crop_x1; j < crop_x2; j++) {
               full->tex[i+start_y][j+start_x].r = 
                        left->tex[i-left_y][j-left_x].r;
               full->tex[i+start_y][j+start_x].g = 
                        left->tex[i-left_y][j-left_x].g;
               full->tex[i+start_y][j+start_x].b = 
                        left->tex[i-left_y][j-left_x].b;

               full->tex[i+start_y][j+start_x+screen_x].r =
                        right->tex[i-right_y][j-right_x].r;
               full->tex[i+start_y][j+start_x+screen_x].g =
                        right->tex[i-right_y][j-right_x].g;
               full->tex[i+start_y][j+start_x+screen_x].b =
                        right->tex[i-right_y][j-right_x].b;
            }
         }

         /* write cropped images to file(s) */
         if (fullOutfile != NULL)
            write_texture(fullOutfile, full);
         else
            write_texture("fullout.ppm", full);

         if (leftOutfile != NULL)
            write_cropped_texture(leftOutfile, left, crop_x1 - left_x, 
                                  crop_x2 - left_x,
                                  crop_y1 - left_y, crop_y2 - left_y);
         else
            write_cropped_texture("leftout.ppm", left, crop_x1 - left_x, 
                                  crop_x2 - left_x,
                                  crop_y1 - left_y, crop_y2 - left_y);

         if (rightOutfile != NULL)
            write_cropped_texture(rightOutfile, right, crop_x1-right_x,
                                  crop_x2-right_x, crop_y1-right_y,
                                  crop_y2-right_y);
         else
            write_cropped_texture("rightout.ppm", right, crop_x1-right_x,
                                  crop_x2-right_x, crop_y1-right_y,
                                  crop_y2-right_y);

         break;

      case 'q': /* q or escape to exit */
      case 'Q':
      case 27:
         exit(0);
         break;

      default:
         break;
   }

}

void keyboardFunc2(unsigned char key, int x, int y) {

   debug("keyboardFunc2: key is (ascii) %d\n", key);

   switch(key) {
      case 'q': /* q or escape to exit */
      case 'Q':
      case 27:
         exit(0);
         break;

      default:
         break;
   }

}


void specialFunc(int key, int x, int y) {

   debug("specialFunc: key is %d\n", key);

   if ((key == GLUT_KEY_LEFT) || (key == GLUT_KEY_RIGHT) ||
       (key == GLUT_KEY_DOWN) || (key == GLUT_KEY_UP)) {
      glutSetWindow(left_win);
      left_x = glutGet(GLUT_WINDOW_X);
      left_y = glutGet(GLUT_WINDOW_Y);
      glutSetWindow(right_win);
      right_x = glutGet(GLUT_WINDOW_X) - screen_x - offset_x;
      right_y = glutGet(GLUT_WINDOW_Y) - offset_y;
   }

   switch(key) {
      case GLUT_KEY_LEFT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right_x -= 10;
            else
               right_x--;

            glutSetWindow(right_win);
            glutPositionWindow(right_x + screen_x + offset_x, right_y + offset_y);
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left_x -= 10;
            else
               left_x--;
            glutSetWindow(left_win);
            glutPositionWindow(left_x, left_y);
         }
         break;
      case GLUT_KEY_RIGHT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right_x += 10;
            else
               right_x++;
            glutSetWindow(right_win);
            glutPositionWindow(right_x + screen_x + offset_x, right_y + offset_y);
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left_x += 10;
            else
               left_x++;

            glutSetWindow(left_win);
            glutPositionWindow(left_x, left_y);
         }
         break;
      case GLUT_KEY_UP:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right_y -= 10;
            else
               right_y--;
            glutSetWindow(right_win);
            glutPositionWindow(right_x + screen_x + offset_x, right_y + offset_y);
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left_y -= 10;
            else
               left_y--;

            glutSetWindow(left_win);
            glutPositionWindow(left_x, left_y);
         }
         break;
      case GLUT_KEY_DOWN:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right_y += 10;
            else
               right_y++;
            glutSetWindow(right_win);
            glutPositionWindow(right_x + screen_x + offset_x, right_y + offset_y);
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left_y += 10;
            else
               left_y++;

            glutSetWindow(left_win);
            glutPositionWindow(left_x, left_y);
         }
         break;
      default:
         break;
   }

}

/*
 * the menu handler. when a menu item is clicked this function controls
 * what happens.
 */
void menuFunc(int item) {

   switch(item) {
      case 99:
         exit(0);
         break;
      default:
   }
}

/*
 * the menu handler. when a menu item is clicked this function controls
 * what happens.
 */
void menuFunc2(int item) {

   switch(item) {
      case 99:
         exit(0);
         break;
      default:
   }
}

/*
 * resize callback function. this will simply force the window to go back
 * to its original size if the user tries to resize it.
 */
void resizeFunc(int height, int width) {

   glutSetWindow(left_win);
   glutReshapeWindow(left->width, left->height);
   glViewport(0, 0, left->width, left->height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, left->width, 0, left->height);

   glutSetWindow(right_win);
   glutReshapeWindow(right->width, right->height);
   glViewport(0, 0, right->width, right->height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, right->width, 0, right->height);

}

/*
 * resize callback function. this will simply force the window to go back
 * to its original size if the user tries to resize it.
 */
void resizeFunc2(int height, int width) {

   glutReshapeWindow(full->width, full->height);
   glViewport(0, 0, full->width, full->height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, full->width, 0, full->height);

}

/*
 * draw a point of a given size. the size is a dumb 'block' size, so size
 * of 1 prints one pixel, size of 3 prints a 3x3 block centered at the
 * given point, etc. note that sizes which are even will not center
 * properly. color is presently ignored.
 */
void writePixel(int x, int y,  COLOR *color) {

   glBegin(GL_POINTS);
      glColor3f(color->r, color->g, color->b);
      glVertex2i(x, y);
   glEnd();

   glFlush();

}

/*
 * draws a dashed line between the two given points using the midpoint
 * algorithm.
 */
void drawDashedLine(int x1, int y1, int x2, int y2, COLOR *color1,
                    COLOR *color2) {

   int max = 10;
   int count = 0;
   int first = TRUE;
   int dx, dy, d, incrE, incrNE, x, y;
   int incrN, incrW, incrS, incrNW, incrSW, incrSE;
   float m, m2;
   COLOR *color = color1;

   m = ((float)(y2 - y1))/(x2 - x1);  /* slope */
   m2 = ((float)(x2 - x1))/(y2 - y1); /* inverse slope */

   if ((m >= 0) && (m <= 1)) {

      if (x2 >= x1) { /* 0 - 45 degrees */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dy - dx;
         incrE = 2*dy;
         incrNE = 2*(dy - dx);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (x = x1+1; x <= x2; x++) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose E */
               d += incrE;
            } else { /* choose NE */
               d += incrNE;
               y++;
            }
            writePixel(x, y, color);
         }
      } else { /* 180 - 225 */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dy - dx;
         incrW = 2*dy;
         incrSW = 2*(dy - dx);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (x = x1-1; x >= x2; x--) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose W */
               d -= incrW;
            } else { /* choose SW */
               d -= incrSW;
               y--;
            }
            writePixel(x, y, color);
         }
      }
   } else if ((m2 >= 0) && (m2 <= 1)) {

      if ((x2 >= x1) && (y2 > y1)) { /* 45 - 90 degrees */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dx - dy;
         incrN = 2*dx;
         incrNE = 2*(dx - dy);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (y = y1+1; y <= y2; y++) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose N */
               d += incrN;
            } else { /* choose NE */
               d += incrNE;
               x++;
            }
            writePixel(x, y, color);
         }
      } else { /* 225 - 270 */

         dx = x2 - x1;
         dy = y2 - y1;
         d = -2*dx + dy;
         incrS = 2*dx;
         incrSW = 2*(dx - dy);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (y = y1-1; y >= y2; y--) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose S */
               d -= incrS;
            } else { /* choose SW */
               d -= incrSW;
               x--;
            }
            writePixel(x, y, color);
         }
      }
   } else if ((m <= 0) && (m >= -1)) {

      if (x2 <= x1) { /* 135 - 180 degrees */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dy - dx;
         incrW = 2*dy;
         incrNW = 2*(dy + dx);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (x = x1-1; x >= x2; x--) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose W */
               d += incrW;
            } else { /* choose NW */
               d += incrNW;
               y++;
            }
            writePixel(x, y, color);
         }
      } else { /* 315 - 360 */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dy - dx;
         incrE = 2*dy;
         incrSE = 2*(-dy - dx);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (x = x1+1; x <= x2; x++) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose E */
               d -= incrE;
            } else { /* choose SE */
               d += incrSE;
               y--;
            }
            writePixel(x, y, color);
         }
      }
   } else if ((m2 <= 0) && (m2 >= -1)) {

      if (x2 <= x1) { /* 90 - 135 degrees */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dx - dy;
         incrN = 2*dx;
         incrNW = 2*(-dx - dy);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (y = y1+1; y <= y2; y++) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose N */
               d -= incrN;
            } else { /* choose NW */
               d += incrNW;
               x--;
            }
            writePixel(x, y, color);
         }
      } else { /* 270 - 315 */

         dx = x2 - x1;
         dy = y2 - y1;
         d = 2*dx - dy;
         incrS = 2*dx;
         incrSE = 2*(-dx - dy);
         x = x1;
         y = y1;

         writePixel(x, y, color);

         for (y = y1+1; y >= y2; y--) {

            if (count > max) { /* swap color */
               count = 0;
               if (first == TRUE) {
                  color = color2;
                  first = FALSE;
               } else {
                  color = color1;
                  first = TRUE;
               }
            } else {
               count++;
            }

            if (d <= 0) { /* choose S */
               d += incrS;
            } else { /* choose SE */
               d -= incrSE;
               x++;
            }
            writePixel(x, y, color);
         }
      }
   }
}

/*
 * draws a dashed box. args are upper left and lower right corners as well
 * as the colors to alternate between and the line thickness.
 */
void drawDashedBox(int x1, int y1, int x2, int y2, COLOR *color1,
                   COLOR *color2) {

   drawDashedLine(x1, y1, x2, y1, color1, color2);
   drawDashedLine(x1, y1, x1, y2, color1, color2);
   drawDashedLine(x2, y1, x2, y2, color1, color2);
   drawDashedLine(x1, y2, x2, y2, color1, color2);

}

/* from the course website */
TEXTURE *read_texture(char *infilename) {

   FILE *infile;
   char string[STRING_SIZE];
   int i, j;
   long img_size;
   int img_max, img_width, img_height;
   int r, g, b;
   TEXTURE *tex;

   /* open file containing texture */
   if ((infile = fopen(infilename, "r")) == NULL) {
      die("error opening texture file %s\n", infilename);
   }

   /* read and discard first two lines */
   fgets(string, STRING_SIZE, infile);
   fgets(string, STRING_SIZE, infile);

   /* read image size and (?)max component value */
   fscanf(infile, "%d %d", &img_width, &img_height);
   fscanf(infile, "%d", &img_max);
   if (img_max != 255) die("error in texture coord");

   /* allocate texture array */
   img_size = 4*img_height*img_width;
   if ((tex = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("error allocating texture image");
   }

   tex->width = img_width;
   tex->height = img_height;
   tex->tex = (COLOR **)malloc(sizeof(COLOR *)*img_height);
   if (tex->tex == NULL) die("error mallocing texture\n");

   /* read image data */
   for (i=0; i<img_height; i++) {
      tex->tex[i] = (COLOR *)malloc(sizeof(COLOR)*img_width);
      for (j=0; j<img_width; j++) {
         fscanf(infile, "%d %d %d", &r, &g, &b);
         tex->tex[i][j].r = (double) r/256;
         tex->tex[i][j].g = (double) g/256;
         tex->tex[i][j].b = (double) b/256;
      }
   }

   return tex;

}

/*
 * based on read_texture and write_ppm from extract.c. writes the rgb data
 * from image[height][width] to file in ascii ppm format.
 */
void write_texture(char *filename, TEXTURE *tex) {

   FILE *outfile;
   int i, j;

   debug("write_texture: filename = \"%s\"\n", filename);

   if ((outfile = fopen(filename, "w")) == NULL) {
      die("write_texture: error opening file %s\n", filename);
   }

   fprintf(outfile, "P3\n");
   fprintf(outfile, "# CREATOR: viewer\n");
   fprintf(outfile, "%d %d\n", tex->width, tex->height);
   fprintf(outfile, "255\n");

   for (i = 0; i < tex->height; i++) {
      for (j = 0; j < tex->width; j++) {
         fprintf(outfile, "%d %d %d ", (int)(255*tex->tex[i][j].r),
                 (int)(255*tex->tex[i][j].g), (int)(255*tex->tex[i][j].b));
         if ((j+1)%5 == 0) fprintf(outfile, "\n");
      }
   }
   fprintf(outfile, "\n");
   fclose(outfile);
}

/*
 * based on read_texture and write_ppm from extract.c. writes the rgb data
 * from image[height][width] to file in ascii ppm format.
 */
void write_cropped_texture(char *filename, TEXTURE *tex, int x1, int x2,
                           int y1, int y2) {

   FILE *outfile;
   int i, j;

   debug("write_cropped_texture: filename = \"%s\", x = %d..%d, y = %d..%d\n", 
         filename, x1, x2, y1, y2);

   if ((outfile = fopen(filename, "w")) == NULL) {
      die("write_texture: error opening file %s\n", filename);
   }

   fprintf(outfile, "P3\n");
   fprintf(outfile, "# CREATOR: viewer\n");
   fprintf(outfile, "%d %d\n", x2 - x1, y2 - y1);
   fprintf(outfile, "255\n");

   for (i = y1; i < y2; i++) {
      for (j = x1; j < x2; j++) {
         fprintf(outfile, "%d %d %d ", (int)(255*tex->tex[i][j].r),
                 (int)(255*tex->tex[i][j].g), (int)(255*tex->tex[i][j].b));
         if ((j+1)%5 == 0) fprintf(outfile, "\n");
      }
   }
   fprintf(outfile, "\n");
   fclose(outfile);
}


