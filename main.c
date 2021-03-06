#include "viewer.h"

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

/* left image, right image, and 'full' (combined left+right images) */
TEXTURE *left = NULL;
TEXTURE *right = NULL;
TEXTURE *full = NULL;

/* output filenames */
char *fullOutfile = NULL;
char *leftOutfile = NULL;
char *rightOutfile = NULL;
char *basefilename = NULL;

/* either in ALIGN mode or in VIEWER mode */
int mode = VIEWER;

double szoom = 0;
int thumb_size = 100;
int show_time = 5;

TEXTURE *zoomLeft = NULL;
TEXTURE *zoomRight = NULL;

int mousex1 = 0;
int mousey1 = 0;
int fine_align = FALSE;
int force_geom = FALSE;

int leftDown = FALSE;
int rightDown = FALSE;
int middleDown = FALSE;

/* display thumbnails */
int nothumb = FALSE;

/* display zoom factor */
int nofac = FALSE;

PAIRLIST *list = NULL;

int clone_mode = FALSE;
int fullscreen = FALSE;

/*
 * the main function. this sets up the global variables, creates menus,
 * associates the various callback functions with their opengl event, and
 * starts things running.
 */
int main(int argc, char **argv) {

   int i, j, k;

   initList(&list);
   processArgs(argc, argv);

   glutInit(&argc, argv);

   if (clone_mode) {
      glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_STEREO);
   } else {
      glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
   }

   if (mode == MONOVIEW) {
      if (!force_geom) {
#ifdef OS_Darwin
         screen_x = glutGet(GLUT_SCREEN_WIDTH)*2;
#else
         screen_x = glutGet(GLUT_SCREEN_WIDTH);
#endif
         screen_y = glutGet(GLUT_SCREEN_HEIGHT);
      }

      debug("main: screen_x = %d, screen_y = %d\n", screen_x, screen_y);

   } else {
      if (!force_geom) {
#ifdef OS_Darwin
         screen_x = glutGet(GLUT_SCREEN_WIDTH);
#else
         screen_x = glutGet(GLUT_SCREEN_WIDTH)/2;
#endif
         screen_y = glutGet(GLUT_SCREEN_HEIGHT);

      } else {
         screen_x = screen_x/2;
      }

      if (clone_mode) screen_x *= 2;

      debug("main: screen_x = %d, screen_y = %d\n", screen_x, screen_y);

   }

   /* window */
   glutInitWindowPosition(0, 0);
   if (clone_mode) {
      glutInitWindowSize(screen_x, screen_y);
   } else {
      glutInitWindowSize(screen_x*2, screen_y);
   }
   glutCreateWindow("stereo viewer");

   if (fullscreen) {
      glutFullScreen();
   }

   if (mode == ALIGN) { /* mode == ALIGN */
      debug("main: mode == ALIGN\n");
      full = (TEXTURE *)malloc(sizeof(TEXTURE));
      if (full == NULL) die("main: malloc failure\n");

      full->width = screen_x*2;
      full->height = screen_y;
      full->tex = (GLubyte *)malloc(full->width*full->height*
                                    RGB*sizeof(GLubyte));
      if (full->tex == NULL) die("main: malloc failure\n");

      for (i = 0; i < full->height; i++) {
         for (j = 0; j < full->width; j++) {
            for (k = 0; k < RGB; k++) {
               *(full->tex + (RGB*(i*
                  full->width+j)+k)) = (GLubyte) 0;
            }
         }
      }

      glutDisplayFunc(displayFuncAlign);
      glutReshapeFunc(resizeFuncAlign);
      glutKeyboardFunc(keyboardFuncAlign);
      glutSpecialFunc(specialFuncAlign);
      glutMouseFunc(mouseFuncAlign);
      glutMotionFunc(motionFuncAlign);
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glShadeModel(GL_FLAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   } else if ((mode == VIEWER) || (mode == AUTOVIEW)) { /* mode == VIEWER */
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

   }

   if (list->cur != NULL) {
      readPair(list->cur);
   } else {
      die("main: why is list->cur == NULL?\n");
   }

   if (mode == MONOVIEW) {
      left->x = (screen_x - left->width)/2;
      left->y = (screen_y - left->height)/2;
      calcWindow(left);

   } else {

      left->x = (screen_x - left->width)/2;
      left->y = (screen_y - left->height)/2;
      calcWindow(left);

      right->x = (screen_x - right->width)/2 + list->cur->x_offset;
      right->y = (screen_y - right->height)/2 + list->cur->y_offset;
      calcWindow(right);
   }

   if (mode == AUTOVIEW) {
      glutTimerFunc(1000*show_time, slideshowFunc, 0);
   }

   glutMainLoop();

   return 0;

}

/*
 * displays a message about how to use the program and the commands
 * available in the user interface
 */
void showUsage() {

   printf("stereo pair viewer/aligner v%s.\n", VERSION);
   printf("by: Russ Burdick <wburdick@cs.umn.edu>\n");
   printf("see manpage or AUTHORS for list of other contributors\n\n");
   printf("usage:\n");
   printf("viewer [basename]\n");
   printf("       will read basename-l.ppm and basename-r.ppm as left and right images\n");
   printf("       and will write basename-leftcrop.ppm, basename-rightcrop.ppm,\n");
   printf("       and basename-pair.ppm. Using basename puts you in aligner mode.\n");
   printf("viewer [options]\n");
   printf("       -i, --input leftinfile.ppm rightinfile.ppm [viewer mode]\n");
   printf("       -v, --view fullinfile.ppm [viewer mode] \n");
   printf("       -m, --mono monofile.ppm [mono mode]\n");
   printf("       -a, --align leftinfile.ppm rightinfile.ppm [aligner mode]\n");
   printf("       -o, --output fulloutfile.ppm [default = fullout.ppm]\n");
   printf("       -l, --left leftoutfile.ppm [default = leftout.ppm]\n");
   printf("       -r, --right rightoutfile.ppm [default = rightout.ppm]\n");
   printf("       -g, --geom WxH [forces a specific window geometry]\n");
   printf("       -x, --off XY [specifies offset of right image relative to left]\n");
   printf("       -t, --thumbsize size [size of the square used for thumbnails]\n");
   printf("       -f, --file filename [multi-file viewer mode]\n");
   printf("       -h, --help [display this help message and exit]\n");
   printf("       -n, --nothumb [disable thumbnail view]\n");
   printf("       -z, --nofac [disable zoom%% view]\n");
   printf("       -s, --stereo [enable hardware supported stereo]\n");
   printf("       -u, --fullscreen [enable fullscreen mode]\n");
   printf("       -p, --import <file(s)> [import the file(s) in viewer mode]\n");
   printf("       -w, --show time [enter slideshow mode, rotating every 'time' seconds]\n");
   printf("\nmust contain either the -i, -v, -a, -m, -p, or -f options or only basename.\n");
   printf("if the -o, -l, or -r options are omitted default will be used\n");
   printf("\nsee manpage viewer(1) for further information\n\n");
}

/*
 * processes command line arguments.
 */
void processArgs(int argc, char **argv) {

   int i = 1;
   char buf[2048];
   char buf2[2048];
   PAIR *ptr = NULL;
   int xoff = 0, yoff = 0;
   int j = 0;
   int count = 0;
   char** flist;

   if (argc == 1) { /* display usage */
      showUsage();
      exit(-1);
   }

   while (i < argc) {

      if ((strcmp(argv[i], "-o") == 0) ||
          (strcmp(argv[i], "--output") == 0)) {
         if (i+1 < argc) {
            i++;
            fullOutfile = argv[i];
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-v") == 0) ||
                 (strcmp(argv[i], "--view") == 0)) {
         if (i+1 < argc) {
            i++;
            ptr = newPair(argv[i], argv[i]);
            addPair(ptr, &list);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-h") == 0) ||
                 (strcmp(argv[i], "--help") == 0)) {
         showUsage();
         exit(-1);
      } else if ((strcmp(argv[i], "-n") == 0) ||
                 (strcmp(argv[i], "--nothumb") == 0)) {
         nothumb = TRUE;
      } else if ((strcmp(argv[i], "-z") == 0) ||
                 (strcmp(argv[i], "--nofac") == 0)) {
         nofac = TRUE;
      } else if ((strcmp(argv[i], "-m") == 0) ||
                 (strcmp(argv[i], "--mono") == 0)) {
         if (i+1 < argc) {
            i++;
            mode = MONOVIEW;
            if (clone_mode) {
               printf("stereo mode is incompatible with mono viewing. stereo option ignored.\n");
               clone_mode = FALSE;
            }
            ptr = newPair(argv[i], NULL);
            addPair(ptr, &list);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-t") == 0) ||
                 (strcmp(argv[i], "--thumbsize") == 0)) {
         if (i+1 < argc) {
            i++;
            thumb_size = atoi(argv[i]);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-w") == 0) ||
                 (strcmp(argv[i], "--show") == 0)) {
         if (i+1 < argc) {
            i++;
            mode = AUTOVIEW;
            show_time = atoi(argv[i]);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-g") == 0) ||
                 (strcmp(argv[i], "--geom") == 0)) {
         if (i+1 < argc) {
            if (fullscreen) {
               printf("fullscreen mode is incompatible with forced-geometry viewing. fullscreen option disabled\n");
               fullscreen = FALSE;
            }
            i++;
            force_geom = TRUE;
            sscanf(argv[i], "%dx%d", &screen_x, &screen_y);
            debug("processArgs: geometry forced to %dx%d\n", screen_x, screen_y);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-x") == 0) ||
                 (strcmp(argv[i], "--off") == 0)) {
         if (i+1 < argc) {
            i++;
            sscanf(argv[i], "%d%d", &xoff, &yoff);
            debug("processArgs: offset set to %+d%+d\n", xoff, yoff);
            if ((list != NULL) && (list->tail != NULL)) {
               list->tail->x_offset = xoff;
               list->tail->y_offset = yoff;
            }
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-l") == 0) ||
                 (strcmp(argv[i], "--left") == 0)) {
         if (i+1 < argc) {
            i++;
            leftOutfile = argv[i];
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-r") == 0) ||
                 (strcmp(argv[i], "--right") == 0)) {
         if (i+1 < argc) {
            i++;
            rightOutfile = argv[i];
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-f") == 0) ||
                 (strcmp(argv[i], "--file") == 0)) {
         if (i+1 < argc) {
            i++;
            readFileList(argv[i], &list);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-i") == 0) ||
                 (strcmp(argv[i], "--input") == 0)) {
         if (i+2 < argc) {
            i++;
            i++;
            ptr = newPair(argv[i-1], argv[i]);
            addPair(ptr, &list);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-a") == 0) ||
                 (strcmp(argv[i], "--align") == 0)) {
         if (i+2 < argc) {
            i++;
            i++;
            mode = ALIGN;
            ptr = newPair(argv[i-1], argv[i]);
            addPair(ptr, &list);
         } else {
            showUsage();
            exit(-1);
         }
      } else if ((strcmp(argv[i], "-s") == 0) ||
                 (strcmp(argv[i], "--stereo") == 0)) {
         /*
          * Verify that stereo actually is available.
          * this test doesnt work, so it's commented out. if you try to 
          * enable stereo mode and dont support it the system will bail 
          * out anyway.
         if (!stereoCheck())
           die("This system does not support stereo.\n");
          */

         if (mode == MONOVIEW) {
            printf("stereo mode is incompatible with mono viewing. stereo option ignored.\n");
         } else {
            clone_mode = TRUE;
         }
      } else if ((strcmp(argv[i], "-u") == 0) ||
                 (strcmp(argv[i], "--fullscreen") == 0)) {
         if (force_geom) {
            printf("fullscreen mode is incompatible with forced-geometry viewing. fullscreen option disabled\n");
         } else {
            fullscreen = TRUE;
         }
      } else if ((strcmp(argv[i], "-p") == 0) ||
                 (strcmp(argv[i], "--import") == 0)) {
         j = i+1;
         while ((j < argc) && (*(argv[j]) != '-')) {
            j++;
            count++;
         }
         debug("processArgs: counted %d images for import\n", count);
         flist = (char **)malloc(sizeof(char*)*count);
         for (j = 0; j < count; j++)
            flist[j] = argv[i+j+1];
         sortList(count, flist);
         findPairs(count, flist);
         i += count;
      } else if (argc == 2) {
         basefilename = argv[1];
         strcpy(buf, argv[1]);
         strcat(buf, "-l.ppm");
         strcpy(buf2, argv[1]);
         strcat(buf2, "-r.ppm");
         mode = ALIGN;
         ptr = newPair(buf, buf2);
         addPair(ptr, &list);
      } else {
         showUsage();
         exit(-1);
      }
      i++;
   }

   if (list->cur == NULL) {
      showUsage();
      exit(-1);
   }
}

/*
 * move down the (sorted) list of images. if the next item in the list 
 * is the matching pair, mark the pair and skip them both. if the next 
 * item does not match, ignore it and try the next item.
 */
void findPairs(int count, char **flist) {
   int i = 0;
   int j = 0;
   int left = 0;
   int right = 0;
   int differ = 0;
   char *p1 = NULL;
   char *p2 = NULL;
   PAIR *ptr = NULL;

   for (i = 0; i < (count - 1); i++) {
      j = i+1;
      right = -1;
      left = -1;
      p1 = flist[i];
      p2 = flist[j];
      debug("findPairs: p1=\"%s\"\nfindPairs: p2=\"%s\"\n", p1, p2);

      while ((*p1 != '\0') && (*p2 != '\0')) {
         if (*p1 == *p2) {
            debug("findPairs: char %d matches (%c)\n", p1 - flist[i], *p1);
         } else {
            debug("findPairs: char %d does not match (%c,%c)\n", p1 - flist[i], *p1, *p2);
            differ++;
            if (((tolower(*p1) == 'l') && (tolower(*p2) == 'r')) && 
                (differ == 1)) {
               left = i;
               right = j;
            } else if (((tolower(*p1) == 'e') && (tolower(*p2) == 'i')) && 
                (differ == 2)) {
            } else if (((tolower(*p1) == 'f') && (tolower(*p2) == 'g')) && 
                 (differ == 3)) {
            } else if (((tolower(*p1) == 't') && (tolower(*p2) == 'h')) && 
                (differ == 4)) {
               if (tolower(*(p2+1)) == 't')
                  p2++;
            } else {
               debug("findPairs: pair mismatch\n");
               p1 = p1 + strlen(p1) - 1; //drop out of loop
               left = -1;
               right = -1;
            }
         }
         p1++;
         p2++;
      }
      if ((left > -1) && (right > -1)) {
         debug("findPairs: looks like a good pair\n");
         debug("findPairs: file \"%s\" looks like left image\n", flist[left]);
         debug("findPairs: file \"%s\" looks like right image\n", flist[right]);
         ptr = newPair(flist[left], flist[right]);
         addPair(ptr, &list);
         i++;
      } else {
         debug("findPairs: \"%s\" does not have a pair\n", flist[i]);
      }
      left = -1;
      right = -1;
      differ = 0;
   }


}

/*
 * sorts a given list alphabetically. this should leave left and right 
 * image adjacent in the list.
 */
void sortList(int count, char **flist) {

   char *tmp = NULL;
   int i, j;

   if (flist == NULL) { /* insure there is a list to sort */
      debug("sortList: null list\n");
   } else if (count == 1) { /* only one item, no sorting */
      debug("sortList: one item list\n");
   } else {
      /* do a bubble sort by alphabet. */
      for (i = 0; i < (count - 1); i++) {
         for (j = i+1; j < count; j++) {

            if (strcmp(flist[i], flist[j]) > 0) {
               tmp = flist[i];
               flist[i] = flist[j];
               flist[j] = tmp;
            }

         }
      }
      debug("sortList: list sorted\n");
   }
}

