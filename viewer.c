#include "viewer.h"

#define POSITION 0
#define VIEWER 1

#define RGB 3
#define RGBA 4

/*
 * some function prototypes.
 */
void displayFunc(void); /* the display function */
void displayFunc2(void); /* the display function (for viewer) */
void resizeFunc(int, int); /* the resize function */
void menuFunc(int); /* the menu function */
void keyboardFunc(unsigned char, int, int);
void keyboardFunc2(unsigned char, int, int); /* (for viewer) */
void specialFunc(int, int, int);
void processArgs(int, char **);
TEXTURE *read_texture(char *);
void write_ppm(char *, COLOR ***, int, int);
void write_texture(char *, TEXTURE *);
void write_cropped_texture(char *, TEXTURE *, int, int, int, int);
void calcWindow(TEXTURE *);

/*
 * global variables
 */

int first_time = TRUE; /* first time we've called display? */

/* screen dimensions (dimensions of one 'eye', so half of desktop) */
int screen_x = 1024;
int screen_y = 768;

/*
 * right hand image can be 'offset' from top left corner of screen. was
 * used when second screen was offset in y direction, unused now. left
 * in for convenience and backwards compatibility.
 */
int offset_x = 0;
int offset_y = 0;

/* left image, right image, and 'full' (combined left+right images) */
TEXTURE *left = NULL;
TEXTURE *right = NULL;
TEXTURE *full = NULL;

/* output filenames */
char *fullOutfile = NULL;
char *leftOutfile = NULL;
char *rightOutfile = NULL;
char *basename = NULL;

/* either in POSITION mode or in VIEWER mode */
int mode = POSITION;

/*
 * the main function. this sets up the global variables, creates menus,
 * associates the various callback functions with their opengl event, and
 * starts things running.
 */
int main(int argc, char **argv) {

   int mainmenu; /* menu id */
   int i, j, k;

   processArgs(argc, argv);

   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);

   if (mode == POSITION) {

      screen_x = glutGet(GLUT_SCREEN_WIDTH)/2;
      screen_y = glutGet(GLUT_SCREEN_HEIGHT);

      debug("screen_x = %d, screen_y = %d\n", screen_x, screen_y);

      left->x = (screen_x - left->width)/2;
      left->y = (screen_y - left->height)/2;
      calcWindow(left);

      right->x = (screen_x - right->width)/2;
      right->y = (screen_y - right->height)/2;
      calcWindow(right);

      full = (TEXTURE *)malloc(sizeof(TEXTURE));
      if (full == NULL) die("keyboardFunc: malloc failure\n");

      full->width = screen_x*2;
      full->height = screen_y;
      full->tex = (GLubyte *)malloc(full->width*full->height*RGBA);
      if (full->tex == NULL) die("keyboardFunc: malloc failure\n");

      for (i = 0; i < full->height; i++) {
         for (j = 0; j < full->width; j++) {
            for (k = 0; k < RGBA; k++) {
               *(full->tex + (RGBA*((full->height-1-i)*
                  full->width+j)+k)) = (GLubyte) 0;
            }
         }
      }

      /* window */
      glutInitWindowPosition(0, 0);
      glutInitWindowSize(screen_x*2, screen_y);
      glutCreateWindow("stereo viewer");

      glutDisplayFunc(displayFunc);
      glutReshapeFunc(resizeFunc);
      glutKeyboardFunc(keyboardFunc);
      glutSpecialFunc(specialFunc);
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glShadeModel(GL_FLAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      /* menu */
      mainmenu = glutCreateMenu(menuFunc);
      glutAddMenuEntry("quit",99);

      glutAttachMenu(GLUT_RIGHT_BUTTON);

   } else { /* mode == VIEWER */

      glutInitWindowPosition(0, 0);
      glutInitWindowSize(full->width, full->height);
      glutCreateWindow("stereo viewer");

      glutDisplayFunc(displayFunc2);
      glutReshapeFunc(resizeFunc);
      glutKeyboardFunc(keyboardFunc2);

      /* menu */
      mainmenu = glutCreateMenu(menuFunc);
      glutAddMenuEntry("quit",99);

      glutAttachMenu(GLUT_RIGHT_BUTTON);

   }

   glutMainLoop();

   return 0;

}

/*
 * displays a message about how to use the program and the commands
 * available in the user interface
 */
void showUsage() {

   printf("stereo pair viewer/aligner v%d.%d.%d.\n", MAJOR, MINOR, PATCH);
   printf("usage:\n");
   printf("viewer [basename]\n");
   printf("       will read basename-l.ppm and basename-r.ppm as left and right images\n");
   printf("       and will write basename-leftcrop.ppm, basename-rightcrop.ppm,\n");
   printf("       and basename-pair.ppm\n");
   printf("viewer [options]\n");
   printf("       -i leftinfile.ppm rightinfile.ppm\n");
   printf("       -v fullinfile.ppm\n");
   printf("       -o fulloutfile.ppm [default = fullout.ppm]\n");
   printf("       -l leftoutfile.ppm [default = leftout.ppm]\n");
   printf("       -r rightoutfile.ppm [default = rightout.ppm]\n");
   printf("\nmust contain either the -i or -v options or only basename.\n");
   printf("if the -o, -l, or -r options are omitted default will be used\n");
   printf("\ncommands within program:\n");
   printf("cursor keys             move left image by 1 pixel\n");
   printf("cursor keys+shift       move right image by 1 pixel\n");
   printf("cursor keys+alt         move both images by 1 pixel\n");
   printf("cursor keys+ctrl        move left image by 10 pixels\n");
   printf("cursor keys+shift+ctrl  move right image by 10 pixels\n");
   printf("cursor keys+shift+alt   move both images by 10 pixels\n");
   printf("esc or q                quit\n");
   printf("enter                   write images\n\n");
   printf("enter+shift             write images and quit\n\n");
}

/*
 * processes command line arguments.
 */
void processArgs(int argc, char **argv) {

   int i = 1;
   char buf[2048];

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
      } else if (argc == 2) {
         basename = argv[1];
         strcpy(buf, argv[1]);
         strcat(buf, "-l.ppm");
         left = read_texture(buf);
         strcpy(buf, argv[1]);
         strcat(buf, "-r.ppm");
         right = read_texture(buf);
      } else {
         showUsage();
      }
      i++;
   }

}

/*
 * display function for POSITION mode draws the two images (on a black
 * background if they are smaller than the screen size). uses double
 * buffering to avoid flicker of display
 */
void displayFunc(void) {

   int rx, ry;
   int i, h, w, r, off;

   if (first_time) {
      glutPositionWindow(0, 0);
      first_time = FALSE;
   }

   glClear(GL_COLOR_BUFFER_BIT);

   if (left->x < 0) rx = 0;
   else rx = left->x;
   if (left->y < 0) ry = 0;
   else ry = left->y;

   w = left->x2 - left->x1;
   h = left->y2 - left->y1;
   r = left->width*RGBA;
   off = RGBA*((left->height-1-left->y1)*left->width+left->x1);

   for (i = 0; i < h; i++) {
      glRasterPos2i(rx, screen_y - (ry+i));
      glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                   left->tex+off);
      off -= r;
   }

   if (right->x < 0) rx = 0;
   else rx = right->x;
   if (right->y < 0) ry = 0;
   else ry = right->y;

   w = right->x2 - right->x1;
   h = right->y2 - right->y1;
   r = right->width*RGBA;
   off = RGBA*((right->height-1-right->y1)*right->width+right->x1);

   for (i = 0; i < h; i++) {
      glRasterPos2i(rx+screen_x, screen_y - (ry+i));
      glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                   right->tex+off);
      off -= r;
   }

   glutSwapBuffers();
}

/*
 * display function for VIEWER mode. draws the full stereo image across
 * both desktops (both eyes). uses double buffering for drawing.
 */
void displayFunc2(void) {

   if (first_time) {
      glutPositionWindow(0, 0);
      first_time = FALSE;
   }

   glClear(GL_COLOR_BUFFER_BIT);
   glRasterPos2i(0,0);
   glDrawPixels(full->width, full->height, GL_RGBA, GL_UNSIGNED_BYTE,
                full->tex);

   glutSwapBuffers();
}

/*
 * keyboard function for POSITION mode. handles cropping of images and
 * writing of cropped images to files and quitting from program.
 * movement (via cursor keys) is done in specialFunc.
 */
void keyboardFunc(unsigned char key, int x, int y) {

   int crop_x1, crop_x2, crop_y1, crop_y2, i, j, k;
   int start_x, start_y;

   debug("keyboardFunc: key is (ascii) %d\n", key);

   switch(key) {
      case 13:
         debug("return key pressed.\n");

         debug("left image is %dx%d at %dx%d\n",
               left->width, left->height, left->x, left->y);
         debug("right image is %dx%d at %dx%d, offset by %dx%d\n",
               right->width, right->height, right->x, right->y, screen_x +
               offset_x, offset_y);

         crop_x1 = max(left->x, right->x);
         crop_x2 = min(left->x + left->width, right->x + right->width);
         crop_y1 = max(left->y, right->y);
         crop_y2 = min(left->y + left->height, right->y + right->height);

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
         full->tex = (GLubyte *)malloc(full->width*full->height*RGBA);
         if (full->tex == NULL) die("keyboardFunc: malloc failure\n");

         for (i = 0; i < full->height; i++) {
            for (j = 0; j < full->width; j++) {
               for (k = 0; k < RGBA; k++) {
                  *(full->tex + (RGBA*((full->height-1-i)*
                     full->width+j)+k)) = (GLubyte) 0;
               }
            }
         }

         start_x = (screen_x - crop_x2 - crop_x1)/2;
         start_y = (screen_y - crop_y2 - crop_y1)/2;

         /* calc crop */
         for (i = crop_y1; i < crop_y2; i++) {
            for (j = crop_x1; j < crop_x2; j++) {
               for (k = 0; k < RGBA; k++) {

                  *(full->tex +(RGBA*((full->height-1-i-start_y)*
                     full->width+j+start_x)+k)) =
                     *(left->tex + (RGBA*((left->height-1-i+left->y)*
                     left->width+j-left->x)+k));

                  *(full->tex +(RGBA*((full->height-1-i-start_y)*
                     full->width+j+start_x+screen_x)+k)) =
                     *(right->tex + (RGBA*((right->height-1-i+right->y)*
                     right->width+j-right->x)+k));

               }
            }
         }

         if (basename != NULL) {
            fullOutfile = (char *)malloc(sizeof(char)*(strlen(basename)+10));
            strcpy(fullOutfile, basename);
            strcat(fullOutfile, "-pair.ppm");

            leftOutfile = (char *)malloc(sizeof(char)*(strlen(basename)+14));
            strcpy(leftOutfile, basename);
            strcat(leftOutfile, "-leftcrop.ppm");

            rightOutfile = (char *)malloc(sizeof(char)*(strlen(basename)+15));
            strcpy(rightOutfile, basename);
            strcat(rightOutfile, "-rightcrop.ppm");
         }

         /* write cropped images to file(s) */
         if (fullOutfile != NULL)
            write_texture(fullOutfile, full);
         else
            write_texture("fullout.ppm", full);

         if (leftOutfile != NULL)
            write_cropped_texture(leftOutfile, left, crop_x1 - left->x, 
                                  crop_x2 - left->x,
                                  crop_y1 - left->y, crop_y2 - left->y);
         else
            write_cropped_texture("leftout.ppm", left, crop_x1 - left->x, 
                                  crop_x2 - left->x,
                                  crop_y1 - left->y, crop_y2 - left->y);

         if (rightOutfile != NULL)
            write_cropped_texture(rightOutfile, right, crop_x1-right->x,
                                  crop_x2-right->x, crop_y1-right->y,
                                  crop_y2-right->y);
         else
            write_cropped_texture("rightout.ppm", right, crop_x1-right->x,
                                  crop_x2-right->x, crop_y1-right->y,
                                  crop_y2-right->y);

         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) exit(0);

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

/*
 * keyboard function for VIEWER mode. since the stereo image should be
 * exactly the same size as the desktop there should be no need for
 * movement and the only key it watches for is quit.
 */
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

/*
 * function to handle movement of windows via cursor keys.
 */
void specialFunc(int key, int x, int y) {

   debug("specialFunc: key is %d\n", key);

   switch(key) {
      case GLUT_KEY_LEFT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right->x -= 10;
            else
               right->x--;

         } else if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL) {
               left->x -= 10;
               right->x -= 10;
            } else {
               left->x--;
               right->x--;
            }
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left->x -= 10;
            else
               left->x--;
         }
         break;
      case GLUT_KEY_RIGHT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right->x += 10;
            else
               right->x++;
         } else if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL) {
               left->x += 10;
               right->x += 10;
            } else {
               left->x++;
               right->x++;
            }
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left->x += 10;
            else
               left->x++;

         }
         break;
      case GLUT_KEY_UP:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right->y -= 10;
            else
               right->y--;
         } else if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL) {
               left->y -= 10;
               right->y -= 10;
            }else {
               left->y--;
               right->y--;
            }
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left->y -= 10;
            else
               left->y--;

         }
         break;
      case GLUT_KEY_DOWN:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               right->y += 10;
            else
               right->y++;
         } else if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL) {
               left->y += 10;
               right->y += 10;
            } else {
               right->y++;
               right->y++;
            }
         } else {
            if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
               left->y += 10;
            else
               left->y++;

         }
         break;
      default:
         break;
   }

   if ((key == GLUT_KEY_LEFT) || (key == GLUT_KEY_RIGHT) ||
       (key == GLUT_KEY_DOWN) || (key == GLUT_KEY_UP)) {
      if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
         calcWindow(right);
      } else if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
         calcWindow(left);
         calcWindow(right);
      } else {
         calcWindow(left);
      }

      glutPostRedisplay();
   }
}

/*
 * the menu handler. when a menu item is clicked this function controls
 * what happens. only have quit menu at this time.
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
 * resize callback function. this will simply force the window to go back
 * to its original size if the user tries to resize it.
 */
void resizeFunc(int height, int width) {

   glutReshapeWindow(full->width, full->height);
   glViewport(0, 0, full->width, full->height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, full->width, 0, full->height);

}

/*
 * based on read_texture function provided in CSCI5107. modified to read
 * both raw and ascii ppm files. reads from the filename provided and
 * returns a pointer to a TEXTURE object containing the image and info
 * about it.
 */
TEXTURE *read_texture(char *infilename) {

   FILE *infile;
   char string[STRING_SIZE];
   int i, j, k, l;
   int r, g, b;
   long img_size;
   int img_max, img_width, img_height;
   TEXTURE *tex;
   unsigned char *ibuffer;
   int ascii = 0;

   /* open file containing texture */
   if ((infile = fopen(infilename, "r")) == NULL) {
      die("error opening texture file %s\n", infilename);
   }

   /* read and discard first two lines */
   fgets(string, STRING_SIZE, infile);
   if (strncmp(string, "P3", 2) == 0) { /* ascii ppm */
      debug("found ascii ppm \"%s\"\n", infilename);
      ascii = 1;
   } else if (strncmp(string, "P6", 2) == 0) { /* raw ppm */
      debug("found raw ppm \"%s\"\n", infilename);
      ascii = 0;
   } else {
      die("image file \"%s\" doesnt look like a ppm\n", infilename);
   }

   fgets(string, STRING_SIZE, infile);
   while (!feof(infile) && (string[0] == '#')) {
      fgets(string, STRING_SIZE, infile);
   }

   /* read image size and (?)max component value */
   sscanf(string, "%d %d", &img_width, &img_height);
   debug("ppm \"%s\" is %dx%d\n", infilename, img_width, img_height);

   fgets(string, STRING_SIZE, infile);
   sscanf(string, "%d", &img_max);
   if (img_max != 255) die("error in texture coord");
   debug("ppm \"%s\" has img_max = %d\n", infilename, img_max);

   /* allocate texture array */
   img_size = 4*img_height*img_width;
   if ((tex = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("error allocating texture image");
   }

   ibuffer = (unsigned char *) malloc(img_height*img_width*3);
   if (ibuffer == NULL) die("error mallocign buffer\n");

   tex->width = img_width;
   tex->height = img_height;
   tex->tex = (GLubyte *)malloc(img_height*img_width*RGBA);
   if (tex->tex == NULL) die("error mallocing texture\n");

   if (ascii) { /* ascii ppm */

      /* read image data */
      for (i=0; i<img_height; i++) {
         for (j=0; j<img_width; j++) {
            fscanf(infile, "%d %d %d", &r, &g, &b);

            *(tex->tex + (RGBA*((img_height-1-i)*img_width+j))) =
               (GLubyte) r;
            *(tex->tex + (RGBA*((img_height-1-i)*img_width+j)+1)) =
               (GLubyte) g;
            *(tex->tex + (RGBA*((img_height-1-i)*img_width+j)+2)) =
               (GLubyte) b;
         }
      }

   } else { /* raw ppm */

      fread(ibuffer, sizeof(unsigned char), img_height*img_width*RGB,
            infile);
      fclose(infile);

      l = 0;
      /* read image data */
      for (i=0; i<img_height; i++) {
         for (j=0; j<img_width; j++) {
            for (k = 0; k < RGB; k++) {
               *(tex->tex + (RGBA*((img_height-1-i)*img_width+j)+k)) =
                  (GLubyte) ibuffer[l++];
            }
         }
      }

      free(ibuffer);
   }

   return tex;

}

/*
 * based on read_texture and write_ppm from extract.c. writes the rgb data
 * from texture provided to file in raw ppm format.
 */
void write_texture(char *filename, TEXTURE *tex) {

   FILE *outfile;
   unsigned char *ibuffer;
   int i, j, k, l;

   debug("write_texture: filename = \"%s\"\n", filename);

   if ((outfile = fopen(filename, "w")) == NULL) {
      die("write_texture: error opening file %s\n", filename);
   }

   ibuffer = (unsigned char *) malloc(tex->width*tex->height*RGB);
   if (ibuffer == NULL) die("write_texture: malloc failure\n");

   fprintf(outfile, "P6\n");
   fprintf(outfile, "# CREATOR: viewer\n");
   fprintf(outfile, "%d %d\n", tex->width, tex->height);
   fprintf(outfile, "255\n");

   l=0;
   for (i = 0; i < tex->height; i++) {
      for (j = 0; j < tex->width; j++) {
         for (k = 0; k < RGB; k++) {
            ibuffer[l++] = (unsigned char)
               *(tex->tex + (RGBA*((tex->height-1-i)*tex->width+j)+k));
         }
      }
   }

   fwrite(ibuffer, sizeof(unsigned char), RGB*tex->width*tex->height, outfile);
   fclose(outfile);
   free(ibuffer);
}

/*
 * based on read_texture and write_ppm from extract.c. writes the rgb data
 * from subsection of the texture provided to file in raw ppm format
 */
void write_cropped_texture(char *filename, TEXTURE *tex, int x1, int x2,
                           int y1, int y2) {

   FILE *outfile;
   unsigned char *ibuffer;
   int i, j, k, l;

   debug("write_texture: filename = \"%s\"\n", filename);

   if ((outfile = fopen(filename, "w")) == NULL) {
      die("write_texture: error opening file %s\n", filename);
   }

   ibuffer = (unsigned char *) malloc(tex->width*tex->height*RGB);
   if (ibuffer == NULL) die("write_texture: malloc failure\n");

   fprintf(outfile, "P6\n");
   fprintf(outfile, "# CREATOR: viewer\n");
   fprintf(outfile, "%d %d\n", x2 - x1, y2 - y1);
   fprintf(outfile, "255\n");

   l=0;
   for (i = y1; i < y2; i++) {
      for (j = x1; j < x2; j++) {
         for (k = 0; k < RGB; k++) {
            ibuffer[l++] = (unsigned char)
               *(tex->tex + (RGBA*((tex->height-1-i)*tex->width+j)+k));
         }
      }
   }

   fwrite(ibuffer, sizeof(unsigned char), RGB*tex->width*tex->height, outfile);
   fclose(outfile);
   free(ibuffer);
}

/*
 * calculates the 'window' (portion of image which is visible on screen)
 * for a given texture.
 */
void calcWindow(TEXTURE *tex) {

   if (tex->x < 0) {
      tex->x1 = abs(tex->x);
      if (tex->x + tex->width > screen_x) tex->x2 = tex->x1 + screen_x;
      else tex->x2 = tex->width;
   } else {
      tex->x1 = 0;
      if (tex->x + tex->width > screen_x) tex->x2 = screen_x - tex->x;
      else tex->x2 = tex->width;
   }

   if (tex->y < 0) {
      tex->y1 = abs(tex->y);
      if (tex->y + tex->height > screen_y) tex->y2 = tex->y1 + screen_y;
      else tex->y2 = tex->height;
   } else {
      tex->y1 = 0;
      if (tex->y + tex->height > screen_y) tex->y2 = screen_y + tex->y;
      else tex->y2 = tex->height;
   }

}


