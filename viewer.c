#include "viewer.h"

int zooming = FALSE;
int leftDown = FALSE;
int rightDown = FALSE;
int middleDown = FALSE;

/*
 * display function for VIEWER mode. draws the full stereo image across
 * both desktops (both eyes). uses double buffering for drawing.
 */
void displayFuncView(void) {

   int rx, ry;
   int i, h, w, r, off;

   TEXTURE *zleft = NULL;
   TEXTURE *zright = NULL;

   if (first_time) {
      glutPositionWindow(0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      first_time = FALSE;
   }

   if ((zoom == 0) && (szoom == 1)) {

      if ((left->x > 0) || (left->x + left->width < screen_x) ||
          (left->y > 0) || (left->y + left->height < screen_y) ||
          (right->x > 0) || (right->x + right->width < screen_x) ||
          (right->y > 0) || (right->y + right->height < screen_y)) {

         /* at least one edge showing, so blank */
         debug("displayFuncView: blanking screen\n");

         if (clone) {
            glDrawBuffer(GL_LEFT);
            glClear(GL_COLOR_BUFFER_BIT);
            glDrawBuffer(GL_RIGHT);
            glClear(GL_COLOR_BUFFER_BIT);
         } else {
            glClear(GL_COLOR_BUFFER_BIT);
         }
      } else { /* no edges showing, dont blank */
         debug("displayFuncView: no screen blank needed\n");
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

         if (clone) {
            for (i = 0; i < h; i++) {
               glDrawBuffer(GL_LEFT);
               glRasterPos2i(rx, ry + i);
               glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            left->tex+off);
               off += r;
            }
         } else {
            for (i = 0; i < h; i++) {
               glRasterPos2i(rx, ry + i);
               glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            left->tex+off);
               off += r;
            }
         }
         showPos(left, LEFT, NULL);
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
         showPos(right, RIGHT, NULL);
      }

   } else {

      if (zoom == 0) {
         zleft = zoomImageSmooth(left, szoom);
         zright = zoomImageSmooth(right, szoom);
      } else {
         zleft = zoomImage(left, zoom);
         zright = zoomImage(right, zoom);
      }

      if ((zleft->x > 0) || (zleft->x + zleft->width < screen_x) ||
          (zleft->y > 0) || (zleft->y + zleft->height < screen_y) ||
          (zright->x > 0) || (zright->x + zright->width < screen_x) ||
          (zright->y > 0) || (zright->y + zright->height < screen_y)) {

         /* at least one edge showing, so blank */
         debug("displayFuncView: blanking screen\n");
         glClear(GL_COLOR_BUFFER_BIT);

      } else { /* no edges showing, dont blank */
         debug("displayFuncView: no screen blank needed\n");
      }
          
      if ((zleft->width >= 0) && (zleft->height >= 0)) {
         if (zleft->x < 0) rx = 0;
         else rx = zleft->x;
         if (zleft->y < 0) ry = 0;
         else ry = zleft->y;

         w = zleft->x2 - zleft->x1;
         h = zleft->y2 - zleft->y1;
         r = zleft->width*RGBA;
         off = RGBA*(zleft->y1*zleft->width+zleft->x1);

         for (i = 0; i < h; i++) {
            if (clone) {
               glDrawBuffer(GL_LEFT);
               glRasterPos2i(rx, ry + i);
               glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            zleft->tex+off);
            } else {
               glRasterPos2i(rx, ry + i);
               glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            zleft->tex+off);
            }
            off += r;
         }
         showPos(zleft, LEFT, left);
      }
      if ((zright->width >= 0) && (zright->height >= 0)) {
         if (zright->x < 0) rx = 0;
         else rx = zright->x;
         if (zright->y < 0) ry = 0;
         else ry = zright->y;

         w = zright->x2 - zright->x1;
         h = zright->y2 - zright->y1;
         r = zright->width*RGBA;
         off = RGBA*(zright->y1*zright->width+zright->x1);

         for (i = 0; i < h; i++) {
            if (clone) {
               glDrawBuffer(GL_RIGHT);
               glRasterPos2i(rx, ry + i);
               glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            zright->tex+off);
            } else {
               glRasterPos2i(rx+screen_x, ry + i);
               glDrawPixels(w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            zright->tex+off);
            }
            off += r;
         }
         showPos(zright, RIGHT, right);
      }
      free(zleft);
      free(zright);

   }

   glutSwapBuffers();
}

/*
 * keyboard function for VIEWER mode. since the stereo image should be
 * exactly the same size as the desktop there should be no need for
 * movement and the only key it watches for is quit.
 */
void keyboardFuncView(unsigned char key, int x, int y) {

   debug("keyboardFuncView: key is (ascii) %d", key);
   if (isprint(key)) debug("=\"%c\"\n", key);
   else debug("\n");

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

      case 'z': /* zoom in */
      case 'Z':
         zoom++;
         glutPostRedisplay();
         break;
      case 'x': /* zoom out */
      case 'X':
         zoom--;
         glutPostRedisplay();
         break;
      case 'v': /* smooth zoom in */
      case 'V':
         szoom += 0.1;
         glutPostRedisplay();
         break;
      case 'b': /* smooth zoom out */
      case 'B':
         szoom -= 0.1;
         glutPostRedisplay();
         break;

      case 'a': /* toggle fine_align mode */
      case 'A':
         if (fine_align)
            glutSpecialFunc(specialFuncView);
         else
            glutSpecialFunc(specialFuncAlign);
         fine_align = ! fine_align;
         break;

      case 'c': /* center */
      case 'C':
         left->x = (screen_x - left->width)/2;
         left->y = (screen_y - left->height)/2;
         calcWindow(left);
         right->x = (screen_x - right->width)/2;
         right->y = (screen_y - right->height)/2;
         calcWindow(right);
         glutPostRedisplay();
         break;
      case 'h': /* home (center and un-zoomed) */
      case 'H':
         szoom = 1;
         left->x = (screen_x - left->width)/2;
         left->y = (screen_y - left->height)/2;
         calcWindow(left);
         right->x = (screen_x - right->width)/2;
         right->y = (screen_y - right->height)/2;
         calcWindow(right);
         glutPostRedisplay();
         break;

      case 'q': /* q or escape to exit */
      case 'Q':
      case 27:
         exit(0);
         break;

      case 'r': /* r to re-display at (0,0) */
      case 'R':
         glutPositionWindow(0, 0);
         break;

      case '1': /* actual size */
         szoom = 1;
         glutPostRedisplay();
         break;

      case 'd': /* double size */
      case 'D':
         szoom = 2;
         glutPostRedisplay();
         break;

      case '2': /* 1/2 size */
         szoom = 0.5;
         glutPostRedisplay();
         break;

      case '3': /* 1/4 size */
         szoom = 0.25;
         glutPostRedisplay();
         break;

      case '4': /* 1/8 size */
         szoom = 0.125;
         glutPostRedisplay();
         break;

      case '5': /* 1/16 size */
         szoom = 0.0625;
         glutPostRedisplay();
         break;

      default:
         break;
   }

}

/*
 * function to handle movement of windows via cursor keys.
 */
void specialFuncView(int key, int x, int y) {

   debug("specialFuncView: key is %d\n", key);

   switch(key) {
      case GLUT_KEY_PAGE_UP: /* prev pair */
         getPrevPair(list);
         glutPostRedisplay();
         break;
      case GLUT_KEY_PAGE_DOWN: /* next pair */
         getNextPair(list);
         glutPostRedisplay();
         break;
      case GLUT_KEY_LEFT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            left->x++;
            right->x++;
         } else {
            left->x += 10;
            right->x += 10;
         }
         break;
      case GLUT_KEY_RIGHT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
             left->x--;
             right->x--;
         } else {
             left->x -= 10;
             right->x -= 10;
         }
         break;
      case GLUT_KEY_UP:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            left->y++;
            right->y++;
         } else {
            left->y += 10;
            right->y += 10;
         }
         break;
      case GLUT_KEY_DOWN:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            left->y--;
            right->y--;
         } else {
            left->y -= 10;
            right->y -= 10;
         }
         break;
      default:
         break;
   }

   if ((key == GLUT_KEY_LEFT) || (key == GLUT_KEY_RIGHT) ||
       (key == GLUT_KEY_DOWN) || (key == GLUT_KEY_UP)) {
      calcWindow(left);
      calcWindow(right);
      glutPostRedisplay();
   }
}

/*
 * the menu handler. when a menu item is clicked this function controls
 * what happens. only have quit menu at this time.
 */
void menuFuncView(int item) {

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
void resizeFuncView(int height, int width) {

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

void mouseFuncView(int button, int state, int x, int y) {

   if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
      debug("mouseFuncView: mouse middle down at (%d,%d)\n", x, y);
      middleDown = TRUE;
      mousex1 = x;
      mousey1 = y;
   } else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
      debug("mouseFuncView: mouse middle up at (%d,%d)\n", x, y);
      middleDown = FALSE;
   }

   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
      debug("mouseFuncView: mouse left down at (%d,%d)\n", x, y);
      leftDown = TRUE;
      mousex1 = x;
      mousey1 = y;
   } else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
      debug("mouseFuncView: mouse left up at (%d,%d)\n", x, y);
      leftDown = FALSE;
   }

   if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
      debug("mouseFuncView: mouse right down at (%d,%d)\n", x, y);
      rightDown = TRUE;
      mousex1 = x;
      mousey1 = y;
   } else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
      debug("mouseFuncView: mouse right up at (%d,%d)\n", x, y);
      rightDown = FALSE;
   }
}

void motionFuncView(int x, int y) {

   int dx, dy;

   dx = x - mousex1;
   dy = y - mousey1;
   mousex1 = x;
   mousey1 = y;
   debug("motionFuncView: mouse moved to (%d,%d), dx=%d, dy=%d\n", x, y, dx, dy);

   if (leftDown || middleDown) {
      left->x += dx;
      right->x += dx;
      left->y -= dy;
      right->y -= dy;

      calcWindow(left);
      calcWindow(right);

      glutPostRedisplay();

   } else if (rightDown) {

      szoom += (float)dy/100;

      calcWindow(left);
      calcWindow(right);

      glutPostRedisplay();
   }
}
