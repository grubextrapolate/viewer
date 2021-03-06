#include "viewer.h"

double progress = 0;

/*
 * display function for MONO mode. draws the image across
 * both desktops (both eyes). uses double buffering for drawing.
 */
void displayFuncViewMono(void) {

   int rx, ry;
   int i, h, w, r, off;

   TEXTURE *zleft = NULL;

   if (first_time) {
      glutPositionWindow(0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      first_time = FALSE;
   }

   if (szoom == 0) {

      if ((left->width >= 0) && (left->height >= 0)) {

         if ((left->x > 0) || (left->x + left->width < screen_x) ||
             (left->y > 0) || (left->y + left->height < screen_y)) {

            /* at least one edge showing, so blank */
            debug("displayFuncViewMono: blanking screen\n");
            glDrawBuffer(GL_BACK);
            glClear(GL_COLOR_BUFFER_BIT);

         } else { /* no edges showing, dont blank */
            glDrawBuffer(GL_BACK);
            debug("displayFuncViewMono: no screen blank needed\n");
         }

         if (left->x < 0) rx = 0;
         else rx = left->x;
         if (left->y < 0) ry = 0;
         else ry = left->y;

         w = left->x2 - left->x1;
         h = left->y2 - left->y1;
         r = left->width*RGB;
         off = RGB*(left->y1*left->width+left->x1);

         for (i = 0; i < h; i++) {
            glRasterPos2i(rx, ry + i);
            glDrawPixels(w, 1, GL_RGB, GL_UNSIGNED_BYTE,
                         left->tex+off);
            off += r;
         }
         showPos(left, LEFT, NULL);
         showZoomfac(LEFT);
      }
   } else {

      zleft = zoomImageSmooth(left, szoom);

      if ((zleft->width >= 0) && (zleft->height >= 0)) {
         if ((zleft->x > 0) || (zleft->x + zleft->width < screen_x) ||
             (zleft->y > 0) || (zleft->y + zleft->height < screen_y)) {

            /* at least one edge showing, so blank */
            debug("displayFuncViewMono: blanking screen\n");
            glDrawBuffer(GL_BACK);
            glClear(GL_COLOR_BUFFER_BIT);

         } else { /* no edges showing, dont blank */
            glDrawBuffer(GL_BACK);
            debug("displayFuncViewMono: no screen blank needed\n");
         }

         if (zleft->x < 0) rx = 0;
         else rx = zleft->x;
         if (zleft->y < 0) ry = 0;
         else ry = zleft->y;

         w = zleft->x2 - zleft->x1;
         h = zleft->y2 - zleft->y1;
         r = zleft->width*RGB;
         off = RGB*(zleft->y1*zleft->width+zleft->x1);

         for (i = 0; i < h; i++) {
            glRasterPos2i(rx, ry + i);
            glDrawPixels(w, 1, GL_RGB, GL_UNSIGNED_BYTE,
                         zleft->tex+off);
            off += r;
         }
         showPos(zleft, LEFT, left);
         showZoomfac(LEFT);
      }
      freeTexture(&zleft);

   }

   glutSwapBuffers();
}

/*
 * keyboard function for VIEWER mode. since the stereo image should be
 * exactly the same size as the desktop there should be no need for
 * movement and the only key it watches for is quit.
 */
void keyboardFuncViewMono(unsigned char key, int x, int y) {

   debug("keyboardFuncViewMono: key is (ascii) %d", key);
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
      case 'k':
      case 'K':
         szoom += 1;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case 'x': /* zoom out */
      case 'X':
      case 'f':
      case 'F':
         szoom -= 1;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case 'v': /* small zoom in */
      case 'V':   
         szoom += 0.1;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case 'b': /* small zoom out */
      case 'B':
         szoom -= 0.1;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case 'c': /* center */
      case 'C':
         left->x = (screen_x - left->width)/2;
         left->y = (screen_y - left->height)/2;
         calcWindow(left);
         glutPostRedisplay();
         break;
      case '1': /* actual size */
         szoom = 0;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case 'd': /* double size */
      case 'D':
         szoom = 1;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case '2': /* 1/2 size */
         szoom = -1;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case '3': /* 1/4 size */
         szoom = -2;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case '4': /* 1/8 size */
         szoom = -3;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case '5': /* 1/16 size */
         szoom = -4;
         freeTexture(&(left->thumb));
         left->thumb = NULL;
         glutPostRedisplay();
         break;
      case 'h': /* home (center and un-zoomed) */
      case 'H':
         szoom = 0;
         left->x = (screen_x - left->width)/2;
         left->y = (screen_y - left->height)/2;
         calcWindow(left);
         freeTexture(&(left->thumb));
         left->thumb = NULL;
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
      case 'y': /* move up: wallview compat */
      case 'Y':
         left->y += 10;
         break;
      /* move down: wallview compat CONFLICTS SO DISABLED */
/*      case 'n':
      case 'N':
         left->y -= 10;
         break;*/
      case 'g': /* move left: wallview compat */
      case 'G':
         left->x += 10;
         break;
      case 'j': /* move right: wallview compat */
      case 'J':
         left->x -= 10;
         break;

      default:
         break;
   }

}

/*
 * function to handle movement of windows via cursor keys.
 */
void specialFuncViewMono(int key, int x, int y) {

   debug("specialFuncViewMono: key is %d\n", key);

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
         } else {
            left->x += 10;
         }
         break;
      case GLUT_KEY_RIGHT:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            left->x--;
         } else {
            left->x -= 10;
         }
         break;
      case GLUT_KEY_UP:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            left->y++;
         } else {
            left->y += 10;
         }
         break;
      case GLUT_KEY_DOWN:
         if (glutGetModifiers() & GLUT_ACTIVE_SHIFT) {
            left->y--;
         } else {
            left->y -= 10;
         }
         break;
      default:
         break;
   }

   if ((key == GLUT_KEY_LEFT) || (key == GLUT_KEY_RIGHT) ||
       (key == GLUT_KEY_DOWN) || (key == GLUT_KEY_UP)) {
      calcWindow(left);
      glutPostRedisplay();
   }
}

void motionFuncViewMono(int x, int y) {

   int dx, dy;

   dx = x - mousex1;
   dy = y - mousey1;
   mousex1 = x;
   mousey1 = y;
   debug("motionFuncViewMono: mouse moved to (%d,%d), dx=%d, dy=%d\n", x, y, dx, dy);

   if (leftDown || middleDown) {
      left->x += dx;
      left->y -= dy;
      calcWindow(left);

      glutPostRedisplay();

   } else if (rightDown) {
      szoom -= (float)dy/100;

      debug("motionFuncView: new szoom=%f\n", szoom);

      freeTexture(&(left->thumb));
      left->thumb = NULL;

      calcWindow(left);

      glutPostRedisplay();
   }

}

/*
 * resize callback function. this will simply force the window to go back
 * to its original size if the user tries to resize it.
 */
void resizeFuncViewMono(int height, int width) {

   glutReshapeWindow(screen_x, screen_y);
   glViewport(0, 0, screen_x, screen_y);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, screen_x, 0, screen_y);

}

