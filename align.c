#include "viewer.h"

int moving = -1;

/*
 * display function for ALIGN mode draws the two images (on a black
 * background if they are smaller than the screen size). uses double
 * buffering to avoid flicker of display
 */
void displayFuncAlign(void) {

   int rx, ry;
   int i, h, w, r, off;

   if (first_time) {
      glutPositionWindow(0, 0);
      first_time = FALSE;
   }

   if ((left->x > 0) || (left->x + left->width < screen_x) ||
       (left->y > 0) || (left->y + left->height < screen_y) ||
       (right->x > 0) || (right->x + right->width < screen_x) ||
       (right->y > 0) || (right->y + right->height < screen_y)) {

      /* at least one edge showing, so blank */
      debug("displayFuncAlign: blanking screen\n");
      glClear(GL_COLOR_BUFFER_BIT);

   } else { /* no edges showing, dont blank */
      debug("displayFuncAlign: no screen blank needed\n");
   }

   if ((left->width >= 0) && (left->height >= 0)) {

      if (left->x < 0) rx = 0;
      else rx = left->x;
      if (left->y < 0) ry = 0;
      else ry = left->y;

      w = left->x2 - left->x1;
      h = left->y2 - left->y1;
      r = left->width*RGBA;
      off = RGBA*(left->y1*left->width+left->x1);

      for (i = 0; i < h; i++) {
         if (clone) {
            glDrawBuffer(GL_LEFT);
            glRasterPos2i(rx, ry + i);
            glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                         left->tex+off);
         } else {
            glRasterPos2i(rx, ry + i);
            glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                         left->tex+off);
         }
         off += r;
      }
      showPos(left, 0, 0, LEFT);
   }
   if ((right->width >= 0) && (right->height >= 0)) {

      if (right->x < 0) rx = 0;
      else rx = right->x;
      if (right->y < 0) ry = 0;
      else ry = right->y;

      w = right->x2 - right->x1;
      h = right->y2 - right->y1;
      r = right->width*RGBA;
      off = RGBA*(right->y1*right->width+right->x1);

      for (i = 0; i < h; i++) {
         if (clone) {
            glDrawBuffer(GL_RIGHT);
            glRasterPos2i(rx, ry + i);
            glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                         right->tex+off);
         } else {
            glRasterPos2i(rx+screen_x, ry + i);
            glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                         right->tex+off);
         }
         off += r;
      }
      showPos(right, 0, 0, RIGHT);
   }
   glutSwapBuffers();
}

/*
 * keyboard function for ALIGN mode. handles cropping of images and
 * writing of cropped images to files and quitting from program.
 * movement (via cursor keys) is done in specialFunc.
 */
void keyboardFuncAlign(unsigned char key, int x, int y) {

   int crop_x1, crop_x2, crop_y1, crop_y2, i, j, k;
   int start_x, start_y;

   debug("keyboardFuncAlign: key is (ascii) %d\n", key);

   switch(key) {
      case 'n': /* next pair */
      case 'N':
      case ' ':
         getNextPair(list);
         glutPostRedisplay();
         break;
      case 'p': /* prev pair */
      case 'P':
      case 8: /* backspace */
         getPrevPair(list);
         glutPostRedisplay();
         break;
      case 13:
         debug("keyboardFuncAlign: return key pressed.\n");

         debug("keyboardFuncAlign: left image is %dx%d at %dx%d\n",
               left->width, left->height, left->x, left->y);
         debug("keyboardFuncAlign: right image is %dx%d at %dx%d, offset by %dx%d\n",
               right->width, right->height, right->x, right->y, screen_x +
               list->cur->x_offset, list->cur->y_offset);

         crop_x1 = MAX(left->x, right->x);
         crop_x2 = MIN(left->x + left->width, right->x + right->width);
         crop_y1 = MAX(left->y, right->y);
         crop_y2 = MIN(left->y + left->height, right->y + right->height);

         debug("keyboardFuncAlign: crop_x1=%d, crop_x2=%d, crop_y1=%d, crop_y2=%d\n",
               crop_x1, crop_x2, crop_y1, crop_y2);

         if (crop_x1 < 0) crop_x1 = 0;
         if (crop_x2 > screen_x) crop_x2 = screen_x;
         if (crop_y1 < 0) crop_y1 = 0;
         if (crop_y2 > screen_y) crop_y2 = screen_y;

         debug("keyboardFuncAlign: crop_x1=%d, crop_x2=%d, crop_y1=%d, crop_y2=%d\n",
               crop_x1, crop_x2, crop_y1, crop_y2);

         full = (TEXTURE *)malloc(sizeof(TEXTURE));
         if (full == NULL) die("keyboardFuncAlign: malloc failure\n");

         full->width = screen_x*2;
         full->height = screen_y;
         full->tex = (GLubyte *)malloc(full->width*full->height*
                                       RGBA*sizeof(GLubyte));
         if (full->tex == NULL) die("keyboardFuncAlign: malloc failure\n");

         for (i = 0; i < full->height; i++) {
            for (j = 0; j < full->width; j++) {
               for (k = 0; k < RGBA; k++) {
                  *(full->tex + (RGBA*(i*
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

                  *(full->tex +(RGBA*((i+start_y)*
                     full->width+j+start_x)+k)) =
                     *(left->tex + (RGBA*((i-left->y)*
                     left->width+j-left->x)+k));

                  *(full->tex +(RGBA*((i+start_y)*
                     full->width+j+start_x+screen_x)+k)) =
                     *(right->tex + (RGBA*((i-right->y)*
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
 * function to handle movement of windows via cursor keys.
 */
void specialFuncAlign(int key, int x, int y) {

   debug("specialFuncAlign: key is %d\n", key);

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
               left->y++;
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
void menuFuncAlign(int item) {

   switch(item) {
      case 99:
         exit(0);
         break;
      default:
         ;
   }
}

/*
 * resize callback function. this will simply force the window to go back
 * to its original size if the user tries to resize it.
 */
void resizeFuncAlign(int height, int width) {

   if (clone) {
      glutReshapeWindow(screen_x, screen_y);
      glViewport(0, 0, screen_x, screen_y);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0, screen_x, 0, screen_y);
   } else {
      glutReshapeWindow(screen_x*2, screen_y);
      glViewport(0, 0, screen_x*2, screen_y);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0, screen_x*2, 0, screen_y);
   }
}

void mouseFuncAlign(int button, int state, int x, int y) {

   if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
      debug("mouseFuncAlign: mouse middle down at (%d,%d)\n", x, y);
      mousex1 = x;
      mousey1 = y;
      moving = BOTH;
   } else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
      debug("mouseFuncAlign: mouse middle up at (%d,%d)\n", x, y);
      moving = -1;
   }

   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
      debug("mouseFuncAlign: mouse left down at (%d,%d)\n", x, y);
      mousex1 = x;
      mousey1 = y;
      if (x < screen_x) moving = LEFT;
      else moving = RIGHT;
   } else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
      debug("mouseFuncAlign: mouse left up at (%d,%d)\n", x, y);
      moving = -1;
   }
}

void motionFuncAlign(int x, int y) {

   int dx, dy;

   dx = x - mousex1;
   dy = y - mousey1;
   mousex1 = x;
   mousey1 = y;
   debug("motionFuncAlign: mouse moved to (%d,%d), dx=%d, dy=%d\n", x, y, dx, dy);

   if (moving == LEFT) {
      left->x += dx;
      left->y -= dy;
      calcWindow(left);
   } else if (moving == RIGHT) {
      right->x += dx;
      right->y -= dy;
      calcWindow(right);
   } else if (moving == BOTH) {
      left->x += dx;
      left->y -= dy;
      calcWindow(left);
      right->x += dx;
      right->y -= dy;
      calcWindow(right);
   }

   glutPostRedisplay();
}

