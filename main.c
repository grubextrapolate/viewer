#include "viewer.h"

#define ALIGN 0
#define VIEWER 1
#define MONOVIEW 2

#define RGB 3
#define RGBA 4

/*
 * global variables
 */

int first_time = TRUE; /* first time we've called display? */

/*
 * screen dimensions (dimensions of one 'eye', so half of desktop)
 * autodetected in main, or forced via command line.
 */
int screen_x = 0;
int screen_y = 0;

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

/* either in ALIGN mode or in VIEWER mode */
int mode = VIEWER;

int zoom = 0;
int thumb_size = 50;

TEXTURE *zoomLeft = NULL;
TEXTURE *zoomRight = NULL;

int mousex1 = 0;
int mousey1 = 0;
int fine_align = 0;
int force_geom = 0;

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

   if (mode == MONOVIEW) {
      if (!force_geom) {
#ifdef OS_DARWIN
         screen_x = glutGet(GLUT_SCREEN_WIDTH)*2;
#else
         screen_x = glutGet(GLUT_SCREEN_WIDTH);
#endif
         screen_y = glutGet(GLUT_SCREEN_HEIGHT);
      }

      debug("main: screen_x = %d, screen_y = %d\n", screen_x, screen_y);

      left->x = (screen_x - left->width)/2;
      left->y = (screen_y - left->height)/2;
      calcWindow(left);
   } else {
      if (!force_geom) {
#ifdef OS_DARWIN
         screen_x = glutGet(GLUT_SCREEN_WIDTH);
#else
         screen_x = glutGet(GLUT_SCREEN_WIDTH)/2;
#endif
         screen_y = glutGet(GLUT_SCREEN_HEIGHT);
      } else {
#ifndef OS_DARWIN
         screen_x = screen_x/2;
#endif
      }

      debug("main: screen_x = %d, screen_y = %d\n", screen_x, screen_y);

      left->x = (screen_x - left->width)/2;
      left->y = (screen_y - left->height)/2;
      calcWindow(left);

      right->x = (screen_x - right->width)/2;
      right->y = (screen_y - right->height)/2;
      calcWindow(right);
   }

   /* window */
   glutInitWindowPosition(0, 0);
   glutInitWindowSize(screen_x*2, screen_y);
   glutCreateWindow("stereo viewer");

   if (mode == ALIGN) {
      debug("main: mode == ALIGN\n");
      full = (TEXTURE *)malloc(sizeof(TEXTURE));
      if (full == NULL) die("main: malloc failure\n");

      full->width = screen_x*2;
      full->height = screen_y;
      full->tex = (GLubyte *)malloc(full->width*full->height*RGBA);
      if (full->tex == NULL) die("main: malloc failure\n");

      for (i = 0; i < full->height; i++) {
         for (j = 0; j < full->width; j++) {
            for (k = 0; k < RGBA; k++) {
               *(full->tex + (RGBA*((full->height-1-i)*
                  full->width+j)+k)) = (GLubyte) 0;
            }
         }
      }

      glutDisplayFunc(displayFuncAlign);
      glutReshapeFunc(resizeFuncAlign);
      glutKeyboardFunc(keyboardFuncAlign);
      glutSpecialFunc(specialFuncAlign);
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glShadeModel(GL_FLAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      /* menu */
      mainmenu = glutCreateMenu(menuFuncAlign);
      glutAddMenuEntry("quit",99);

      glutAttachMenu(GLUT_RIGHT_BUTTON);

   } else if (mode == VIEWER) { /* mode == VIEWER */
      debug("main: mode == VIEWER\n");

      glutDisplayFunc(displayFuncView);
      glutReshapeFunc(resizeFuncView);
      glutKeyboardFunc(keyboardFuncView);
      glutSpecialFunc(specialFuncView);
      glutMouseFunc(mouseFuncView);
      glutMotionFunc(motionFuncView);
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glShadeModel(GL_FLAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      /* menu */
      mainmenu = glutCreateMenu(menuFuncView);
      glutAddMenuEntry("quit",99);

      glutAttachMenu(GLUT_RIGHT_BUTTON);

   } else { /* mode == MONOVIEW */
      debug("main: mode == MONOVIEW\n");

      glutDisplayFunc(displayFuncViewMono);
      glutReshapeFunc(resizeFuncViewMono);
      glutKeyboardFunc(keyboardFuncViewMono);
      glutSpecialFunc(specialFuncViewMono);
      glutMouseFunc(mouseFuncView);
      glutMotionFunc(motionFuncViewMono);
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glShadeModel(GL_FLAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      /* menu */
      mainmenu = glutCreateMenu(menuFuncView);
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
   printf("by: Russ Burdick <wburdick@cs.umn.edu>\n\n");
   printf("usage:\n");
   printf("viewer [basename]\n");
   printf("       will read basename-l.ppm and basename-r.ppm as left and right images\n");
   printf("       and will write basename-leftcrop.ppm, basename-rightcrop.ppm,\n");
   printf("       and basename-pair.ppm. Using basename puts you in aligner mode.\n");
   printf("viewer [options]\n");
   printf("       -i leftinfile.ppm rightinfile.ppm [viewer mode]\n");
   printf("       -v fullinfile.ppm [viewer mode] \n");
   printf("       -m monofile.ppm [mono mode]\n");
   printf("       -a leftinfile.ppm rightinfile.ppm [aligner mode]\n");
   printf("       -o fulloutfile.ppm [default = fullout.ppm]\n");
   printf("       -l leftoutfile.ppm [default = leftout.ppm]\n");
   printf("       -r rightoutfile.ppm [default = rightout.ppm]\n");
   printf("       -g WxH [forces a specific window geometry]\n");
   printf("       -h [display this help message and exit]\n");
   printf("\nmust contain either the -i, -v, -a, or -m options or only basename.\n");
   printf("if the -o, -l, or -r options are omitted default will be used\n");
   printf("\nsee manpage viewer(1) for further information\n\n");
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
      } else if (strcmp(argv[i], "-v") == 0) {
         if (i+1 < argc) {
            i++;
            readAndSplit(argv[i]);
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-h") == 0) {
         showUsage();
         exit(-1);
      } else if (strcmp(argv[i], "-m") == 0) {
         if (i+1 < argc) {
            i++;
            if (isjpeg(argv[i]))
               left = read_JPEG_file(argv[i]);
            else
               left = read_texture(argv[i]);
            mode = MONOVIEW;
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-g") == 0) {
         if (i+1 < argc) {
            i++;
            force_geom = 1;
            sscanf(argv[i], "%dx%d", &screen_x, &screen_y);
            debug("processArgs: geometry forced to %dx%d\n", screen_x, screen_y);
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
            if (isjpeg(argv[i]))
               left = read_JPEG_file(argv[i]);
            else
               left = read_texture(argv[i]);
            i++;
            if (isjpeg(argv[i]))
               right = read_JPEG_file(argv[i]);
            else
               right = read_texture(argv[i]);
         } else {
            showUsage();
            exit(-1);
         }
      } else if (strcmp(argv[i], "-a") == 0) {
         if (i+2 < argc) {
            i++;
            if (isjpeg(argv[i]))
               left = read_JPEG_file(argv[i]);
            else
               left = read_texture(argv[i]);
            i++;
            if (isjpeg(argv[i]))
               right = read_JPEG_file(argv[i]);
            else
               right = read_texture(argv[i]);
            mode = ALIGN;
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
         mode = ALIGN;
      } else {
         showUsage();
      }
      i++;
   }

}

/*
 * reads in a single image file that spans both left and right eye from
 * the specified file then splits it into the left and right image for
 * viewing.
 */
void readAndSplit(char *filename) {

   TEXTURE *tmp;
   int i = 0;

   tmp = read_texture(filename);
   if ((left = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("readAndSplit: error allocating texture image");
   }
   if ((right = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("readAndSplit: error allocating texture image");
   }

   left->width = tmp->width/2;
   left->height = tmp->height;
   right->width = tmp->width - left->width;
   right->height = tmp->height;

   debug("readAndSplit: splitting into left=%dx%d and right=%dx%d\n",
         left->width, left->height, right->width, right->height);

   left->tex = (GLubyte *)malloc(left->height*left->width*RGBA);
   if (left->tex == NULL) die("readAndSplit: error mallocing texture\n");

   right->tex = (GLubyte *)malloc(right->height*right->width*RGBA);
   if (right->tex == NULL) die("readAndSplit: error mallocing texture\n");

   for (i = 0; i < tmp->height; i++) {

      memcpy(left->tex + RGBA*(i*left->width),
             tmp->tex + RGBA*(i*tmp->width),
             RGBA*left->width);
      memcpy(right->tex + RGBA*(i*right->width),
             tmp->tex + RGBA*(i*tmp->width+left->width-1),
             RGBA*right->width);
   }
   free(tmp->tex);
   free(tmp);

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
   int img_max, img_width, img_height;
   TEXTURE *tex;
   unsigned char *ibuffer;
   int ascii = 0;

   /* open file containing texture */
   if ((infile = fopen(infilename, "r")) == NULL) {
      die("read_texture: error opening texture file %s\n", infilename);
   }

   /* read and discard first two lines */
   fgets(string, STRING_SIZE, infile);
   if (strncmp(string, "P3", 2) == 0) { /* ascii ppm */
      debug("read_texture: found ascii ppm \"%s\"\n", infilename);
      ascii = 1;
   } else if (strncmp(string, "P6", 2) == 0) { /* raw ppm */
      debug("read_texture: found raw ppm \"%s\"\n", infilename);
      ascii = 0;
   } else {
      die("read_texture: image file \"%s\" doesnt look like a ppm\n", infilename);
   }

   fgets(string, STRING_SIZE, infile);
   while (!feof(infile) && (string[0] == '#')) {
      fgets(string, STRING_SIZE, infile);
   }

   /* read image size and (?)max component value */
   sscanf(string, "%d %d", &img_width, &img_height);
   debug("read_texture: ppm \"%s\" is %dx%d\n", infilename, img_width,
         img_height);

   fgets(string, STRING_SIZE, infile);
   sscanf(string, "%d", &img_max);
   if (img_max != 255) die("read_texture: error in texture coord");
   debug("read_texture: ppm \"%s\" has img_max = %d\n", infilename,
         img_max);

   /* allocate texture array */
   if ((tex = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("read_texture: error allocating texture image");
   }

   ibuffer = (unsigned char *) malloc(img_height*img_width*3);
   if (ibuffer == NULL) die("read_texture: error mallocing buffer\n");

   tex->width = img_width;
   tex->height = img_height;
   tex->thumb = NULL;
   tex->tex = (GLubyte *)malloc(img_height*img_width*RGBA);
   if (tex->tex == NULL) die("read_texture: error mallocing texture\n");

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

   debug("write_cropped_texture: filename = \"%s\"\n", filename);

   if ((outfile = fopen(filename, "w")) == NULL) {
      die("write_cropped_texture: error opening file %s\n", filename);
   }

   ibuffer = (unsigned char *) malloc(tex->width*tex->height*RGB);
   if (ibuffer == NULL) die("write_cropped_texture: malloc failure\n");

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
      if (tex->y + tex->height > screen_y) tex->y2 = screen_y - tex->y;
      else tex->y2 = tex->height;
   }
   debug("calcWindow: x=%d, y=%d, x1=%d, x2=%d, y1=%d, y2=%d, w=%d, h=%d\n",
         tex->x, tex->y, tex->x1, tex->x2, tex->y1, tex->y2, tex->width,
         tex->height);

}

int max(int a, int b) {

   if (a > b) return a;
   else return b;
}

int min(int a, int b) {

   if (a < b) return a;
   else return b;
}

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

int isjpeg(char *str) {
   int ret = 0;
   int len = 0;

   if (str != NULL) {
      len = strlen(str);
      if (len > 4) {
         if (((tolower(*(str+len-4)) == '.') &&
              (tolower(*(str+len-3)) == 'j') &&
              (tolower(*(str+len-2)) == 'p') &&
              (tolower(*(str+len-1)) == 'g')) ||
             ((tolower(*(str+len-5)) == '.') &&
              (tolower(*(str+len-4)) == 'j') &&
              (tolower(*(str+len-3)) == 'p') &&
              (tolower(*(str+len-2)) == 'e') &&
              (tolower(*(str+len-1)) == 'g')))
            ret = 1;
      }
   }
   return(ret);

}


/*
#include <magick/api.h>

gcc `Magick-config --cflags --cppflags` demo.c `Magick-config --ldflags --libs`
*/

/*
 * based on sample code from ImageMagick docs
 */
TEXTURE *read_image(char *infilename) {

   FILE *infile = NULL;
   int i, j, k, l;
   int r, g, b;
   int img_width = 0, img_height = 0;
   TEXTURE *tex;
   unsigned char *ibuffer;
   int ascii = 0;
/*
   ExceptionInfo exception;
   Image image;
   ImageInfo image_info;

   InitializeMagic(*argv);
   GetExceptionInfo(&exception);
   image_info=CloneImageInfo((ImageInfo *) NULL);
   (void) strcpy(image_info->filename,infilename);
   image=ReadImage(image_info,&exception);
   if (image == (Image *) NULL)
      MagickError(exception.severity,exception.reason,exception.description);
*/



   if ((tex = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("read_image: error allocating texture image");
   }
   tex->width = img_width;
   tex->height = img_height;
   tex->tex = (GLubyte *)malloc(img_height*img_width*RGBA);
   if (tex->tex == NULL) die("read_image: error mallocing texture\n");

   ibuffer = (unsigned char *) malloc(img_height*img_width*3);
   if (ibuffer == NULL) die("read_image: error mallocing buffer\n");

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
 * integer version of the pow function. takes integer args and returns
 * an integer. only works for positive integer powers.
 */
int ipow(int base, int exp) {

   int ret = 0;
   int i = 0;

   if (exp == 0) {
      ret = 1;
   } else if (exp > 0) {
      ret = base;
      for (i = 1; i < exp; i++) {
         ret *= base;
      }
   } else { /* error */
      ret = -1;
   }
    
   return ret;
}

/*
 * returns the image scaled by the appropriate zoom factor.
 */
TEXTURE *zoomImage(TEXTURE *orig, int zoomfac) {

   TEXTURE *ret = NULL;
   int i, j, k, l, a, b, r, g, x, y, off;

   if ((ret = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("zoomImage: error allocating texture image");
   }

   if (zoomfac > 0) { /* zoom in. replicates single pixel into blocks */

      debug("zoomImage: zoom in, zoomfac=%d\n", zoomfac);
      a = ipow(2, abs(zoomfac));

      ret->thumb = NULL;
      ret->width = orig->width*a;
      ret->height = orig->height*a;

      ret->x = orig->x + (orig->width - ret->width)/2;
      ret->y = orig->y + (orig->height - ret->height)/2;
      calcWindow(ret);

      ret->width = ret->x2 - ret->x1;
      ret->height = ret->y2 - ret->y1;

      if ((ret->width >= 0) && (ret->height >= 0)) {
         ret->tex = (GLubyte *)malloc(ret->height*ret->width*RGBA);
         if (ret->tex == NULL) die("zoomImage: error mallocing texture\n");

         for (i = ret->y1; i < ret->y2; i++) {
            for (j = ret->x1; j < ret->x2; j++) {
               off = RGBA*((orig->height-1-(i/a))*orig->width+(j/a));
               r = (int) *(orig->tex + off);
               g = (int) *(orig->tex + off + 1);
               b = (int) *(orig->tex + off + 2);

               off = RGBA*((ret->height-1-(i-ret->y1))*ret->width+(j-ret->x1));
               *(ret->tex + off) = (GLubyte) r;
               *(ret->tex + off + 1) = (GLubyte) g;
               *(ret->tex + off + 2) = (GLubyte) b;
            }
         }

         if (ret->x < 0) ret->x = 0;
         if (ret->y < 0) ret->y = 0;
         calcWindow(ret);
      }
   } else { /* zoom out. averages blocks of pixels */

      debug("zoomImage: zoom out, zoomfac=%d\n", zoomfac);
      a = ipow(2, abs(zoomfac));

      b = orig->width % a;
      ret->width = orig->width/a;
      if (b != 0) ret->width++;

      b = orig->height % a;
      ret->height = orig->height/a;
      if (b != 0) ret->height++;

      ret->x = orig->x + (orig->width - ret->width)/2;
      ret->y = orig->y + (orig->height - ret->height)/2;

      calcWindow(ret);

      ret->width = ret->x2 - ret->x1;
      ret->height = ret->y2 - ret->y1;

      if ((ret->width >= 0) && (ret->height >= 0)) {
         ret->tex = (GLubyte *)malloc(ret->height*ret->width*RGBA);
         if (ret->tex == NULL) die("zoomImage: error mallocing texture\n");

         for (i = ret->y1; i < ret->y2; i++) {
            for (j = ret->x1; j < ret->x2; j++) {
               r = 0; g = 0; b = 0;
               x = orig->height % a;
               if (x == 0) x = a;
               y = orig->width % a;
               if (y == 0) y = a;
               for (k = 0; k < x; k++) {
                  for (l = 0; l < y; l++) {
                     off = RGBA*((orig->height-1-(i*a+k))*orig->width+(j*a+l));
                     r += (int) *(orig->tex + off);
                     g += (int) *(orig->tex + off + 1);
                     b += (int) *(orig->tex + off + 2);
                  }
               }
               r = r/(x*y);
               g = g/(x*y);
               b = b/(x*y);
               off = RGBA*((ret->height-1-(i-ret->y1))*ret->width+(j-ret->x1));
               *(ret->tex + off) = (GLubyte) r;
               *(ret->tex + off + 1) = (GLubyte) g;
               *(ret->tex + off + 2) = (GLubyte) b;
            }
         }

         if (ret->x < 0) ret->x = 0;
         if (ret->y < 0) ret->y = 0;
         calcWindow(ret);
      }
   }
   return(ret);

}

/*
 * returns a thumbnail sized version of the supplied image.
 */
TEXTURE *makeThumb(TEXTURE *orig) {

   TEXTURE *ret = NULL;
   int i, j, k, l, a, b, r, g, x, y, off;

   if (orig != NULL) {
      if ((ret = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
         die("makeThumb: error allocating texture image");
      }

      ret->x = 0;
      ret->y = 0;
      ret->x1 = 0;
      ret->x2 = 0;
      ret->y1 = 0;
      ret->y2 = 0;

      if ((orig->height < screen_y) && (orig->width < screen_x)) {
         if (screen_y > screen_x) {
            a = screen_y/thumb_size;
         } else {
            a = screen_x/thumb_size;
         }
      } else {
         if (orig->height > orig->width) {
            a = orig->height/thumb_size;
         } else {
            a = orig->width/thumb_size;
         }
      }
      ret->x = a;
      debug("makeThumb: zoomfac=%d\n", a);

      b = orig->width % a;
      ret->width = orig->width/a;
      if (b != 0) ret->width++;

      b = orig->height % a;
      ret->height = orig->height/a;
      if (b != 0) ret->height++;

      debug("makeThumb: orig->height=%d, orig->width=%d\n", 
            orig->height, orig->width);
      debug("makeThumb: ret->height=%d, ret->width=%d\n", ret->height, 
            ret->width);
      ret->tex = (GLubyte *)malloc(ret->height*ret->width*RGBA);
      if (ret->tex == NULL) die("makeThumb: error mallocing texture\n");

      for (i = 0; i < ret->height; i++) {
         for (j = 0; j < ret->width; j++) {
            r = 0; g = 0; b = 0;
            x = orig->height % a;
            if (x == 0) x = a;
            y = orig->width % a;
            if (y == 0) y = a;
            for (k = 0; k < x; k++) {
               for (l = 0; l < y; l++) {
                  off = RGBA*((orig->height-1-(i*a+k))*orig->width+(j*a+l));
                  r += (int) *(orig->tex + off);
                  g += (int) *(orig->tex + off + 1);
                  b += (int) *(orig->tex + off + 2);
               }
            }
            r = r/(x*y);
            g = g/(x*y);
            b = b/(x*y);
            off = RGBA*((ret->height-i-1)*ret->width+j);
            *(ret->tex + off) = (GLubyte) r;
            *(ret->tex + off + 1) = (GLubyte) g;
            *(ret->tex + off + 2) = (GLubyte) b;
         }
      }
   }
   return(ret);

}

/*
 * draws the shrunken view of the image and screen in upper right corner 
 * to give the user an idea where they are within the image.
 */
void showPos(TEXTURE *tex, int dx, int dy, int eye) {

   int x = 0, y = 0, fac;
   int boxx = 0, boxy = 0, boxw = 0, boxh = 0;
   int white[] = {255, 255, 255};
   int black[] = {0, 0, 0};

   if (tex != NULL) {
      if (tex->thumb == NULL) tex->thumb = makeThumb(tex);
      if (eye == LEFT) {
         x = screen_x - thumb_size - 20;
      } else {
         x = screen_x*2 - thumb_size - 20;
      }
      y = screen_y - thumb_size - 20;

      /*
       * makeThumb uses the x value (since it's not actually needed) to
       * remember the scaling factor.
       */
      fac = tex->thumb->x;

      boxw = screen_x/fac;
      if (screen_x % fac != 0) boxw++;
      boxh = screen_y/fac;
      if (screen_y % fac != 0) boxh++;

      boxx = tex->x/fac;
      if (tex->x % fac != 0) boxx++;
      boxy = tex->y/fac;
      if (tex->y % fac != 0) boxy++;

      /* draw background of screen box */
      drawFilledBox(x-boxx, y-boxy, boxw, boxh, black, eye);

      /* draw image on background */
      if (tex->thumb->tex != NULL) {
         glRasterPos2i(x, y);
         glDrawPixels(tex->thumb->width, tex->thumb->height, GL_RGBA, 
                      GL_UNSIGNED_BYTE, tex->thumb->tex);
      }

      /* draw outline around image */
      drawBox(x-1, y-1, tex->thumb->width+2, tex->thumb->height+2, 
              black, eye);

      /* draw screen outline */
      drawBox(x-boxx, y-boxy, boxw, boxh, white, eye);
   }
}

/*
 * draws an unfilled box in the indicated color (int[3] with values from 
 * 0-255) at the specified location (x,y) of the specified size (w,h).
 */
void drawBox(int x, int y, int w, int h, int *color, int eye) {

   int i = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
   GLubyte img[1][w][RGBA];

   for (i = 0; i < w; i++) {
      img[0][i][0] = color[0];
      img[0][i][1] = color[1];
      img[0][i][2] = color[2];
      img[0][i][3] = 0;
   }

   /* check boundary to make sure box stays on correct eye */
   x1 = x;
   x2 = x+w-1;
   y1 = y;
   y2 = y+h-1;
   if (eye == LEFT) {
      if (x1 < 0) x1 = 0;
      if (x1 > screen_x-1) x1 = screen_x;
      if (x2 < 0) x2 = 0;
      if (x2 > screen_x-1) x2 = screen_x;
   }else {
      if (x1 < screen_x) x1 = screen_x-1;
      if (x1 > screen_x*2-1) x1 = screen_x*2;
      if (x2 < screen_x) x2 = screen_x-1;
      if (x2 > screen_x*2-1) x2 = screen_x*2;
   }
   if (y1 < 0) y1 = 0;
   if (y1 > screen_y-1) y1 = screen_y;
   if (y2 < 0) y2 = 0;
   if (y2 > screen_y-1) y2 = screen_y;

   /* draw top of box */
   if ((y1 > 0) && (y1 < screen_y-1)) {
      glRasterPos2i(x1, y1);
      glDrawPixels(x2-x1+1, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
   }

   /* draw left side of box */
   if (((eye == LEFT) && ((x1 > 0) && (x1 < screen_x-1))) || 
       ((eye == RIGHT) && ((x1 > screen_x) && (x1 < screen_x*2-1)))) {
      for (i = y1+1; i < y2; i++) {
         glRasterPos2i(x1, i);
         glDrawPixels(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
      }
   }
   /* draw right side of box */
   if (((eye == LEFT) && ((x2 > 0) && (x2 < screen_x-1))) || 
       ((eye == RIGHT) && ((x2 > screen_x) && (x2 < screen_x*2-1)))) {
      for (i = y1+1; i < y2; i++) {
         glRasterPos2i(x2, i);
         glDrawPixels(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
      }
   }

   /* draw bottom of box */
   if ((y2 > 0) && (y2 < screen_y-1)) {
      glRasterPos2i(x1, y2);
      glDrawPixels(x2-x1+1, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
   }
}

/*
 * draws a filled box in the indicated color (int[3] with values from 
 * 0-255) at the specified location (x,y) of the specified size (w,h).
 */
void drawFilledBox(int x, int y, int w, int h, int *color, int eye) {

   int i = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
   GLubyte img[1][w][RGBA];

   for (i = 0; i < w; i++) {
      img[0][i][0] = color[0];
      img[0][i][1] = color[1];
      img[0][i][2] = color[2];
      img[0][i][3] = 0;
   }

   /* check boundary to make sure box stays on correct eye */
   x1 = x;
   x2 = x+w-1;
   y1 = y;
   y2 = y+h-1;
   if (eye == LEFT) {
      if (x1 < 0) x1 = 0;
      if (x1 > screen_x-1) x1 = screen_x;
      if (x2 < 0) x2 = 0;
      if (x2 > screen_x-1) x2 = screen_x;
   }else {
      if (x1 < screen_x) x1 = screen_x-1;
      if (x1 > screen_x*2-1) x1 = screen_x*2;
      if (x2 < screen_x) x2 = screen_x-1;
      if (x2 > screen_x*2-1) x2 = screen_x*2;
   }
   if (y1 < 0) y1 = 0;
   if (y1 > screen_y-1) y1 = screen_y;
   if (y2 < 0) y2 = 0;
   if (y2 > screen_y-1) y2 = screen_y;

   /* draw filled box */
   for (i = y1; i < y2+1; i++) {
      glRasterPos2i(x1, i);
      glDrawPixels(x2-x1+1, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
   }
}
