#include "viewer.h"

PAIR *newPair(char *left_name, char *right_name) {

   PAIR *itm = NULL;

   if (left_name != NULL) {
      if ((itm = (PAIR*)malloc(sizeof(PAIR))) == NULL) die("newPair: malloc failure\n");

      itm->left_file = NULL;
      itm->right_file = NULL;
      itm->next = NULL;
      itm->prev = NULL;
      itm->x_offset = 0;
      itm->y_offset = 0;

      itm->left_file = strdup(left_name);
      if (left_name == right_name) {
         itm->right_file = itm->left_file;
      } else if (right_name != NULL) {
         itm->right_file = strdup(right_name);
      }
   } else {
      die("newPair: oops, no left filename? something's wrong\n");
   }

   return(itm);

}

/*
 * adds a pair to the end of the list
 */
void addPair(PAIR *item, PAIRLIST **list) {

   if (item != NULL) {
      debug("addPair: adding pair left=\"%s\", right=\"%s\"\n", 
            item->left_file, item->right_file);
      if (*list == NULL) initList(list);

      if ((*list)->head == NULL) { /* no items on list yet */
         (*list)->head = item;
         (*list)->tail = item;
         (*list)->num++;
      } else { /* at least one item on list */
         item->prev = (*list)->tail;
         (*list)->tail->next = item;
         (*list)->tail = item;
         (*list)->num++;
      }

      if ((*list)->cur == NULL) (*list)->cur = (*list)->head;
   } else {
      die("addPair: cant add null pair\n");
   }
}

/*
 * removes the specified pair from the given list. first verifies that
 * the pair is actually on the list before removing it.
 */
void removePair(PAIR *item, PAIRLIST **list) {

   PAIR *cur = NULL;

   if ((*list != NULL) && (item != NULL)) {

      cur = (*list)->head; /* check to make sure item is on this list */
      while ((cur != item) && (cur != NULL))
         cur = cur->next;

      if (cur != NULL) { /* item was found on list */

         /* check and adjust list pointers if necessary */
         if ((*list)->head == item) {
            (*list)->head = item->next;
         }
         if ((*list)->tail == item) {
            (*list)->tail = item->prev;
         }

         /* adjust pointers within list */
         if (item->prev != NULL) {
            cur = item->prev;
            cur->next = item->next;
         }
         if (item->next != NULL) {
            cur = item->next;
            cur->prev = item->prev;
         }

         item->prev = NULL;
         item->next = NULL;

         (*list)->num--; /* adjust list item count */

      } else {
         debug("removePair: could not find item to remove\n");
      }

   }
}

/*
 * deletes the specified pair from the given list. first verifies that
 * the pair is actually on the list before deleting it.
 */
void deletePair(PAIR *item, PAIRLIST **list) {

   removePair(item, list);

   freePair(item); /* free list item */

}

/*
 * free the memory allocated to a list and all pairs on it.
 */
void freeList(PAIRLIST **list) {

   PAIR *ptr1 = NULL;

   if (*list != NULL) {

      ptr1 = (*list)->head;
      while (ptr1 != NULL) {

         deletePair(ptr1, list);
         ptr1 = (*list)->head;

      }
      free(*list);
      *list = NULL;
   }
}

/*
 * allocate and initialize a list
 */
void initList(PAIRLIST **list) {

   if (*list == NULL) {
      (*list) = (PAIRLIST *)malloc(sizeof(PAIRLIST));
      (*list)->head = NULL;
      (*list)->tail = NULL;
      (*list)->cur = NULL;
      (*list)->num = 0;
   }
}

/*
 * free memory allocated to a pair
 */
void freePair(PAIR *item) {

   if (item != NULL) {
      free(item->left_file);
      if (item->right_file != NULL) free(item->left_file);
   }
}
