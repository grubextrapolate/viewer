#include "viewer.h"

void readFileList(char *filename, PAIRLIST **list) {

   PAIR *itm = NULL;
   FILE *infile;
   char ptr[STRING_SIZE]; /* input buffer */
   char lfile[STRING_SIZE], rfile[STRING_SIZE];
   int xoff, yoff;

   if (!(infile = fopen(filename, "r"))) {
      debug("readFileList: cant open list file \"%s\"\n", filename);
   } else {

      while (!feof(infile)) {
         *ptr = '\0';
         fgets(ptr, sizeof(char)*STRING_SIZE, infile);
         if ((*ptr != '\0') && (*ptr != '#') && (*ptr != '\n')) {

            sscanf(ptr, "%s %s %d %d", lfile, rfile, &xoff, &yoff);
            debug("readFileList: \"%s\" \"%s\" %d %d\n", lfile, rfile, 
                  xoff, yoff);
            itm = newPair(lfile, rfile);
            itm->x_offset = xoff;
            itm->y_offset = yoff;
            addPair(itm, list);
         } else {
            debug("readFileList: ignoring blank or comment line \n");
         }
      }
   }
}

void readPair(PAIR *pair) {

   if (pair != NULL) {

      if (pair->left_file == pair->right_file) { /* -v */
         debug("readPair: reading joined image from \"%s\"\n", 
               pair->left_file);
         readAndSplit(pair->left_file);
      } else if (pair->right_file == NULL) { /* -m */
         debug("readPair: reading mono image from \"%s\"\n", 
               pair->left_file);
         if (isjpeg(pair->left_file)) {
/*            left = read_JPEG_file(pair->left_file);
*/ die("cant read jepgs yet\n");
         } else {
            left = read_texture(pair->left_file);
         }
      } else { /* -i, -a, or basename */
         debug("readPair: reading pair\n");
         if (isjpeg(pair->left_file)) {
/*            left = read_JPEG_file(pair->left_file);
*/ die("cant read jepgs yet\n");
         } else {
            left = read_texture(pair->left_file);
         }   
         if (isjpeg(pair->right_file)) {
/*            right = read_JPEG_file(pair->right_file);
*/ die("cant read jepgs yet\n");
         } else {
            right = read_texture(pair->right_file);
         }
      }

   } else {
      die("readPair: cant read null pair\n");
   }
}

void getNextPair(PAIRLIST *list) {

   if ((list != NULL) && (list->cur != NULL)) {
      if (list->cur->next != NULL) {
         list->cur = list->cur->next;

         if (left != NULL) freeTexture(&left);
         if (right != NULL) freeTexture(&right);
         readPair(list->cur);

         left->x = (screen_x - left->width)/2;
         left->y = (screen_y - left->height)/2;
         calcWindow(left);

         if (list->cur->right_file != NULL) {
            right->x = (screen_x - right->width)/2 + list->cur->x_offset;
            right->y = (screen_y - right->height)/2 + list->cur->y_offset;
            calcWindow(right);
         }
      }
   }
}

void getPrevPair(PAIRLIST *list) {

   if ((list != NULL) && (list->cur != NULL)) {
      if (list->cur->prev != NULL) {
         list->cur = list->cur->prev;

         if (left != NULL) freeTexture(&left);
         if (right != NULL) freeTexture(&right);
         readPair(list->cur);

         left->x = (screen_x - left->width)/2;
         left->y = (screen_y - left->height)/2;
         calcWindow(left);

         if (list->cur->right_file != NULL) {
            right->x = (screen_x - right->width)/2 + list->cur->x_offset;
            right->y = (screen_y - right->height)/2 + list->cur->y_offset;
            calcWindow(right);
         }
      }
   }
}

void freeTexture(TEXTURE **tex) {

   if (*tex != NULL) {
      if ((*tex)->thumb != NULL) freeTexture(&((*tex)->thumb));

      if ((*tex)->tex != NULL) free((*tex)->tex);
      free(*tex);
      *tex = NULL;
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

   left->thumb = NULL;
   left->width = tmp->width/2;
   left->height = tmp->height;
   right->thumb = NULL;
   right->width = tmp->width - left->width;
   right->height = tmp->height;

   debug("readAndSplit: splitting into left=%dx%d and right=%dx%d\n",
         left->width, left->height, right->width, right->height);

   left->tex = (GLubyte *)malloc(left->height*left->width*
                                 RGB*sizeof(GLubyte));
   if (left->tex == NULL) die("readAndSplit: error mallocing texture\n");

   right->tex = (GLubyte *)malloc(right->height*right->width*
                                  RGB*sizeof(GLubyte));
   if (right->tex == NULL) die("readAndSplit: error mallocing texture\n");

   for (i = 0; i < tmp->height; i++) {

      memcpy(left->tex + RGB*(i*left->width),
             tmp->tex + RGB*(i*tmp->width),
             RGB*left->width);
      memcpy(right->tex + RGB*(i*right->width),
             tmp->tex + RGB*(i*tmp->width+left->width-1),
             RGB*right->width);
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

   tex->width = img_width;
   tex->height = img_height;
   tex->thumb = NULL;
   tex->tex = (GLubyte *)malloc(img_height*img_width*
                                RGB*sizeof(GLubyte));
   if (tex->tex == NULL) die("read_texture: error mallocing texture\n");

   if (ascii) { /* ascii ppm */

      /* read image data */
      for (i=0; i<img_height; i++) {
         for (j=0; j<img_width; j++) {
            fscanf(infile, "%d %d %d", &r, &g, &b);

            *(tex->tex + (RGB*((img_height-1-i)*img_width+j))) =
               (GLubyte) r;
            *(tex->tex + (RGB*((img_height-1-i)*img_width+j)+1)) =
               (GLubyte) g;
            *(tex->tex + (RGB*((img_height-1-i)*img_width+j)+2)) =
               (GLubyte) b;
         }
      }

   } else { /* raw ppm */

      ibuffer = (unsigned char *) malloc(img_height*img_width*3);
      if (ibuffer == NULL) die("read_texture: error mallocing buffer\n");

      fread(ibuffer, sizeof(unsigned char), img_height*img_width*RGB,
            infile);
      fclose(infile);

      l = 0;
      /* read image data */
      for (i=0; i<img_height; i++) {
         for (j=0; j<img_width; j++) {
            for (k = 0; k < RGB; k++) {
               *(tex->tex + (RGB*((img_height-1-i)*img_width+j)+k)) =
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
               *(tex->tex + (RGB*((tex->height-1-i)*tex->width+j)+k));
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
               *(tex->tex + (RGB*((tex->height-1-i)*tex->width+j)+k));
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
   tex->tex = (GLubyte *)malloc(img_height*img_width*
                                RGB*sizeof(GLubyte));
   if (tex->tex == NULL) die("read_image: error mallocing texture\n");

   ibuffer = (unsigned char *) malloc(img_height*img_width*3);
   if (ibuffer == NULL) die("read_image: error mallocing buffer\n");

   if (ascii) { /* ascii ppm */

      /* read image data */
      for (i=0; i<img_height; i++) {
         for (j=0; j<img_width; j++) {
            fscanf(infile, "%d %d %d", &r, &g, &b);

            *(tex->tex + (RGB*((img_height-1-i)*img_width+j))) =
               (GLubyte) r;
            *(tex->tex + (RGB*((img_height-1-i)*img_width+j)+1)) =
               (GLubyte) g;
            *(tex->tex + (RGB*((img_height-1-i)*img_width+j)+2)) =
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
               *(tex->tex + (RGB*((img_height-1-i)*img_width+j)+k)) =
                  (GLubyte) ibuffer[l++];
            }
         }
      }

      free(ibuffer);
   }

   return tex;

}

/*
 * returns the image scaled by the appropriate zoom factor.
 */
TEXTURE *zoomImageSmooth(TEXTURE *orig, double zoomfac) {

   TEXTURE *ret = NULL;
   int i, j, r, g, b, off;
   double a;

   if ((ret = (TEXTURE *)malloc(sizeof(TEXTURE))) == NULL) {
      die("zoomImageSmooth: error allocating texture image");
   }

      debug("zoomImageSmooth: zoom in, zoomfac=%f\n", zoomfac);
//      a = pow(2, fabs(zoomfac));
      a = pow(2, zoomfac);

   debug("zoomImageSmooth: zoomfac=%f\n", a);
   ret->thumb = NULL;
   ret->width = (float)orig->width*a;
   ret->height = (float)orig->height*a;
   debug("zoomImageSmooth: new width = %d, new height = %d\n", ret->width, ret->height);

   ret->x = orig->x + (orig->width - ret->width)/2;
   ret->y = orig->y + (orig->height - ret->height)/2;
   calcWindow(ret);

   ret->width = ret->x2 - ret->x1;
   ret->height = ret->y2 - ret->y1;

   if ((ret->width >= 0) && (ret->height >= 0)) {
      ret->tex = (GLubyte *)malloc(ret->height*ret->width*
                                   RGB*sizeof(GLubyte));
      if (ret->tex == NULL) die("zoomImageSmooth: error mallocing texture\n");

      for (i = ret->y1; i < ret->y2; i++) {
         for (j = ret->x1; j < ret->x2; j++) {
            off = RGB*((int) (((int) ((float)i/a))*orig->width + 
                               ((float)j/a)));
            r = (int) *(orig->tex + off);
            g = (int) *(orig->tex + off + 1);
            b = (int) *(orig->tex + off + 2);

            off = RGB*((i-ret->y1)*ret->width+(j-ret->x1));
            *(ret->tex + off) = (GLubyte) r;
            *(ret->tex + off + 1) = (GLubyte) g;
            *(ret->tex + off + 2) = (GLubyte) b;
         }
      }

      if (ret->x < 0) ret->x = 0;
      if (ret->y < 0) ret->y = 0;
      calcWindow(ret);
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

      ret->thumb = NULL;
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

//      b = orig->width % a;
//      ret->width = orig->width/a;
//      if (b != 0) ret->width++;
      ret->width = (float)orig->width/a + 0.5;

//      b = orig->height % a;
//      ret->height = orig->height/a;
//      if (b != 0) ret->height++;
      ret->height = (float)orig->height/a + 0.5;

      debug("makeThumb: orig->height=%d, orig->width=%d\n", 
            orig->height, orig->width);
      debug("makeThumb: ret->height=%d, ret->width=%d\n", ret->height, 
            ret->width);
      ret->tex = (GLubyte *)malloc(ret->height*ret->width*
                                   RGB*sizeof(GLubyte));
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
                  off = RGB*((i*a+k)*orig->width+(j*a+l));
                  r += (int) *(orig->tex + off);
                  g += (int) *(orig->tex + off + 1);
                  b += (int) *(orig->tex + off + 2);
               }
            }
            r = r/(x*y);
            g = g/(x*y);
            b = b/(x*y);
            off = RGB*(i*ret->width+j);
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
void showPos(TEXTURE *tex, int eye, TEXTURE *ext_thumb) {

   int x = 0, y = 0, fac;
   int boxx = 0, boxy = 0, boxw = 0, boxh = 0;
   int white[] = {255, 255, 255};
   int black[] = {0, 0, 0};
   TEXTURE *thumb;
   double a, b;

   if (ext_thumb != NULL) {
      thumb = ext_thumb;
   } else {
      thumb = tex;
   }
   if ((!nothumb) && (thumb != NULL)) {
      if (thumb->thumb == NULL) thumb->thumb = makeThumb(thumb);
      if (eye == LEFT || clone_mode) {
         x = screen_x - thumb_size - 20;
      } else {
         x = screen_x*2 - thumb_size - 20;
      }
      y = screen_y - thumb_size - 20;

      /*
       * makeThumb uses the x value (since it's not actually needed) to
       * remember the scaling factor.
       */
      fac = thumb->thumb->x;
      a = pow(2, szoom);

debug("showPos: tex->x=%d, tex->y=%d, fac=%d, szoom=%f, a=%f\n", tex->x, 
tex->y, fac, szoom, a);

//   ret->x = orig->x + (orig->width - ret->width)/2;
//   ret->y = orig->y + (orig->height - ret->height)/2;

      if (a == 0) {
         boxw = (float)screen_x/fac + 0.5;
         boxh = (float)screen_y/fac + 0.5;

         if (thumb->x > 0) {
            boxx = (float)thumb->x/fac + 0.5;
         } else {
            boxx = (float)thumb->x/fac - 0.5;
         }
         if (thumb->y > 0) {
            boxy = (float)thumb->y/fac + 0.5;
         } else {
            boxy = (float)thumb->y/fac - 0.5;
         }

      } else {

         b = ((double)1 - (double)1/a)/2;

         if (thumb->x > 0) {
            boxx = (float)thumb->x/fac*b + 0.5;
         } else {
            boxx = (float)thumb->x/fac*b - 0.5;
         }
         if (thumb->y > 0) {
            boxy = (float)thumb->y/fac*b + 0.5;
         } else {
            boxy = (float)thumb->y/fac*b - 0.5;
         }

         boxw = ((float)screen_x/fac + (float)screen_x/fac/a) + 0.5;
         boxh = ((float)screen_y/fac - (float)screen_y/fac/a) + 0.5;
      }

debug("showPos: boxw=%d, boxh=%d, boxx=%d, boxy=%d\n", boxw, boxh, boxx, 
boxy);

      /* draw background of screen box */
      drawFilledBox(x-boxx, y-boxy, boxw, boxh, black, eye);

      /* draw image on background */
      if (thumb->thumb->tex != NULL) {
         if (clone_mode) {
            if (eye == LEFT) {
               glDrawBuffer(GL_BACK_LEFT);
            } else { /* (eye == RIGHT) */
               glDrawBuffer(GL_BACK_RIGHT);
            }
         } else {
            glDrawBuffer(GL_BACK);
         }
         glRasterPos2i(x, y);
         glDrawPixels(thumb->thumb->width, thumb->thumb->height, GL_RGB, 
                      GL_UNSIGNED_BYTE, thumb->thumb->tex);
      }

      /* draw outline around image */
      drawBox(x-1, y-1, thumb->thumb->width+2, thumb->thumb->height+2, 
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
   GLubyte *img = NULL;

   img = (GLubyte *)malloc(w*RGB*sizeof(GLubyte));
   if (img == NULL) die("drawBox: malloc failure\n");

   for (i = 0; i < w; i++) {
      *(img + (RGB*i)) = (GLubyte) color[0];
      *(img + (RGB*i + 1)) = (GLubyte) color[1];
      *(img + (RGB*i + 2)) = (GLubyte) color[2];
      *(img + (RGB*i + 3)) = (GLubyte) 0;
   }

   if (clone_mode) {
      if (eye == LEFT) {
         glDrawBuffer(GL_BACK_LEFT);
      } else { /* (eye == RIGHT) */
         glDrawBuffer(GL_BACK_RIGHT);
      }
   } else {
      glDrawBuffer(GL_BACK);
   }

   /* check boundary to make sure box stays on correct eye */
   x1 = x;
   x2 = x+w-1;
   y1 = y;
   y2 = y+h-1;
   if (eye == LEFT || clone_mode) {
      if (x1 < 0) x1 = 0;
      if (x1 > screen_x-1) x1 = screen_x;
      if (x2 < 0) x2 = 0;
      if (x2 > screen_x-1) x2 = screen_x;
   } else {
      if (x1 < screen_x) x1 = screen_x-1;
      if (x1 > screen_x*2-1) x1 = screen_x*2;
      if (x2 < screen_x) x2 = screen_x-1;
      if (x2 > screen_x*2-1) x2 = screen_x*2;
   }
   if (y1 < 0) y1 = 0;
   if (y1 > screen_y-1) y1 = screen_y;
   if (y2 < 0) y2 = 0;
   if (y2 > screen_y-1) y2 = screen_y;

debug("drawBox: x1=%d, x2=%d, y1=%d, y2=%d\n", x1, x2, y1, y2);

   /* draw top of box */
   if ((y1 > 0) && (y1 < screen_y-1)) {
      glRasterPos2i(x1, y1);
      glDrawPixels(x2-x1+1, 1, GL_RGB, GL_UNSIGNED_BYTE, img);
   }

   /* draw left side of box */
   if (((eye == LEFT || clone_mode) && ((x1 > 0) && (x1 < screen_x-1))) || 
       ((eye == RIGHT) && ((x1 > screen_x) && (x1 < screen_x*2-1)))) {
      for (i = y1+1; i < y2; i++) {
         glRasterPos2i(x1, i);
         glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, img);
      }
   }
   /* draw right side of box */
   if (((eye == LEFT || clone_mode) && ((x2 > 0) && (x2 < screen_x-1))) || 
       ((eye == RIGHT) && ((x2 > screen_x) && (x2 < screen_x*2-1)))) {
      for (i = y1+1; i < y2; i++) {
         glRasterPos2i(x2, i);
         glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, img);
      }
   }

   /* draw bottom of box */
   if ((y2 > 0) && (y2 < screen_y-1)) {
      glRasterPos2i(x1, y2);
      glDrawPixels(x2-x1+1, 1, GL_RGB, GL_UNSIGNED_BYTE, img);
   }

   free(img);
}

/*
 * draws a filled box in the indicated color (int[3] with values from 
 * 0-255) at the specified location (x,y) of the specified size (w,h).
 */
void drawFilledBox(int x, int y, int w, int h, int *color, int eye) {

   int i = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
   GLubyte *img = NULL;

   img = (GLubyte *)malloc(w*RGB*sizeof(GLubyte));
   if (img == NULL) die("drawBox: malloc failure\n");

   for (i = 0; i < w; i++) {
      *(img + (RGB*i)) = (GLubyte) color[0];
      *(img + (RGB*i + 1)) = (GLubyte) color[1];
      *(img + (RGB*i + 2)) = (GLubyte) color[2];
      *(img + (RGB*i + 3)) = (GLubyte) 0;
   }

   if (clone_mode) {
      if (eye == LEFT) {
         glDrawBuffer(GL_BACK_LEFT);
      } else { /* (eye == RIGHT) */
         glDrawBuffer(GL_BACK_RIGHT);
      }
   } else {
      glDrawBuffer(GL_BACK);
   }

   /* check boundary to make sure box stays on correct eye */
   x1 = x;
   x2 = x+w-1;
   y1 = y;
   y2 = y+h-1;
   if (eye == LEFT || clone_mode) {
      if (x1 < 0) x1 = 0;
      if (x1 > screen_x-1) x1 = screen_x;
      if (x2 < 0) x2 = 0;
      if (x2 > screen_x-1) x2 = screen_x;
   } else {
      if (x1 < screen_x) x1 = screen_x-1;
      if (x1 > screen_x*2-1) x1 = screen_x*2;
      if (x2 < screen_x) x2 = screen_x-1;
      if (x2 > screen_x*2-1) x2 = screen_x*2;
   }
   if (y1 < 0) y1 = 0;
   if (y1 > screen_y-1) y1 = screen_y;
   if (y2 < 0) y2 = 0;
   if (y2 > screen_y-1) y2 = screen_y;

debug("drawFilledBox: x1=%d, x2=%d, y1=%d, y2=%d\n", x1, x2, y1, y2);

   /* draw filled box */
   for (i = y1; i < y2+1; i++) {
      glRasterPos2i(x1, i);
      glDrawPixels(x2-x1+1, 1, GL_RGB, GL_UNSIGNED_BYTE, img);
   }
}

/* int stereoCheck()
 *
 * Checks whether left and right buffers for stereo are supported.
 *
 * If stereo buffers are available, 1 is returned;
 * else, 0 is returned.
 */
int stereoCheck() {
 
   unsigned char state;
   int ret = 0;

   glGetBooleanv(GL_STEREO, &state);

   if (state)
      ret = 1;
 
   return ret;
}
