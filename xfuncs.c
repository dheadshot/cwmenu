#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <string.h>

#include "xfuncs.h"


struct WinPropNode *winsroot = NULL, *winsptr = NULL;
struct ItemPropNode *itemsroot = NULL; *itemsptr = NULL;

unsigned long itemidmax = 0;
unsigned long itemclicked = 0;
int mouseclickdown = 0;


unsigned long get_colour(char *acolour, Display *dis, int screen)
{
  XColor tmp;
  
  XParseColor(dis, DefaultColormap(dis, screen), acolour, &tmp);
  XAllocColor(dis, DefaultColormap(dis, screen), &tmp);
  return tmp.pixel;
}


int GetTextLength(Display *disp, GC agc, char *sometext)
{
  GContext gcx;
  XFontStruct *fontdata;
  int textcount;
  int tl;
  
  gcx = XGContextFromGC(agc);
  fontdata = XQueryFont(disp, (XID) gcx);
  textcount = strlen(sometext);
  tl = XTextWidth(fontdata, sometext, textcount);
  
  XFreeFontInfo(NULL, fontdata, 0);
  return tl;
}


int GetTextAscent(Display *disp, GC agc, char *sometext)
{
  GContext gcx;
  XFontStruct *fontdata;
  int textcount;
  XCharStruct tdata;
  int dirhint, ta, td;
  
  gcx = XGContextFromGC(agc);
  fontdata = XQueryFont(disp, (XID) gcx);
  textcount = strlen(sometext);
  
  XTextExtents(fontdata, sometext, textcount, &dirhint, &ta, &td, &tdata);
  
  XFreeFontInfo(NULL, fontdata, 0)
  return ta;
}

int GetTextDescent(Display *disp, GC agc, char *sometext)
{
  GContext gcx;
  XFontStruct *fontdata;
  int textcount;
  XCharStruct tdata;
  int dirhint, ta, td;
  
  gcx = XGContextFromGC(agc);
  fontdata = XQueryFont(disp, (XID) gcx);
  textcount = strlen(sometext);
  
  XTextExtents(fontdata, sometext, textcount, &dirhint, &ta, &td, &tdata);
  
  XFreeFontInfo(NULL, fontdata, 0)
  return td;
}


int GetTextHeight(Display *disp, GC agc, char *sometext)
{
  int ta, td;
  ta = GetTextAscent(disp, agc, sometext);
  td = GetTextDescent(disp, agc, sometext);
  return ta+td;
}

void DoRTrim(char *sometext)
{
  unsigned long i = strlen(sometext) - 1;
  while ((i>=0) && (sometext[i] == ' ')) i--;
  i++;
  sometext[i] = 0;
}


struct WinPropNode *FindWinProps(Window awin)
{
  winsptr = winsroot;
  while (winsptr != NULL && winsptr->win != awin) winsptr = winsptr->next;
  return winsptr;
}

struct WinPropNode *NewWindow(Display *disp, Window parent, char *caption, 
  char *iconcaption, Pixmap icon, char **argv, int argc, XSizeHints *hints, 
  int x, int y, unsigned int width, unsigned int height, 
  unsigned int borderwidth, unsigned long bordercol, unsigned long bgcol, 
  unsigned long fgcol)
{
  if (winsroot == NULL)
  {
    winsroot = (struct WinPropNode *) malloc(sizeof(struct WinPropNode));
    winsptr = winsroot;
  }
  else
  {
    winsptr = winsroot;
    while (winsptr->next != NULL) winsptr = winsptr->next;
    winsptr->next = (struct WinPropNode *) malloc(sizeof(struct WinPropNode));
    winsptr = winsptr->next;
  }
  
  if (winsptr == NULL) return NULL;
  
  winsptr->next = NULL;
  winsptr->selitem = 0;
  winsptr->disp = disp;
  winsptr->parent = parent;
  winsptr->win = XCreateSimpleWindow(disp, parent, x, y, width, height, borderwidth, bordercol, bgcol);
  XSetStandardProperties(disp, winsptr->win, caption, iconcaption, icon, argv, argc, hints;
  XSelectInput(disp, winsptr->win, ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | StructureNotifyMask | PointerMotionMask);
  
  Atom wmDelete = XInternAtom(disp, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(disp,winsptr->win, &wmdelete, 1);
  
  winsptr->gc = XCreateGC(disp, winsptr->win, 0, 0);
  
  XSetBackground(disp, winsptr->gc, bgcol);
  XSetForeground(disp, winsptr->gc, fgcol);
  
  XClearWindow(disp, winsptr->win);
  XMapRaised(disp, winsptr->win);
  XFlush(disp);
  
  return winsptr;
}

struct ItemPropNode *FindItemProps(unsigned long itemid)
{
  itemsptr = itemsroot;
  while (itemsptr != NULL && itemsptr->itemid != itemid) itemsptr = itemsptr->next;
  return itemsptr;
}

unsigned long CreateItem(Window awin, int x, int y, unsigned int width, 
  unsigned int height, unsigned long bgcolour, unsigned long selfgcolour, 
  unsigned long unselfgcolour, char *itemtext)
{
  if (FindWinProps(awin) == NULL) return 0;
  
  if (itemsroot == NULL)
  {
    itemsroot = (struct ItemPropNode *) malloc(sizeof(struct ItemPropNode));
    itemsptr = itemsroot;
  }
  else
  {
    itemsptr = itemsroot;
    while (itemsptr->next != NULL) itemsptr = itemsptr->next;
    itemsptr->next = (struct ItemPropNode *) malloc(sizeof(struct ItemPropNode));
    itemsptr = itemsptr->next;
  }
  
  if (itemsptr == NULL) return 0;
  
  itemsptr->next = NULL;
  itemidmax++;
  itemsptr->itemid = itemidmax;
  itemsptr->curx = x;
  itemsptr->cury = y;
  itemsptr->width = width;
  itemsptr->height = height;
  itemsptr->bgcolour = bgcolour;
  itemsptr->selfgcolour = selfgcolour;
  itemsptr->unselfgcolour = unselfgcolour;
  strcpy(itemsptr->itemtext, itemtext);
  itemsptr->win = awin;
  
  DrawItem(itemidmax);
  
  return itemidmax;
}

int DrawItem(unsigned long itemid)
{
  if (FindItemProps(itemid) == NULL) return 0;
  if (FindWinProps(itemsptr->win) == NULL) return 0;
  
  //d=win
  XSetForeground(winsptr->disp, winsptr->gc, itemsptr->bgcolour);
  XFillRectangle(winsptr->disp, (Drawable) winsptr->win, winsptr->gc, itemsptr->curx, itemsptr->cury, itemsptr->width, itemsptr->height);
  
  int ta = GetTextAscent(winsptr->disp, winsptr->gc, itemsptr->itemtext);
  int tbw = GetTextLength(winsptr->disp, winsptr->gc, itemsptr->itemtext);
  int ax, ay;
  ax = itemsptr->curx + (itemsptr->width/2) - (tbw/2);
  ay = itemsptr->cury +(itemsptr->height/2) + ta;
  
  if (winsptr->selitem == itemid)
  {
    XSetForeground(winsptr->disp, winsptr->gc, itemsptr->selfgcolour);
  }
  else
  {
    XSetForeground(winsptr->disp, winsptr->gc, itemsptr->unselfgcolour);
  }
  
  XDrawString(winsptr->disp, (Drawable) winsptr->win, winsptr->gc, ax, ay, itemsptr->itemtext, strlen(itemsptr->itemtext));
  
  return 1;
}

int FreeWindow(Window awin)
{
  /* Returns 0 if can't find item, -1 if gone really wrong */
  struct WinPropNode *twin;
  if (FindWinProps(awin) == NULL) return 0;
  
  /* Free items in awin */
  struct ItemPropNode *titem, *previtem = NULL;
  itemsptr = itemsroot;
  while (itemsptr != NULL)
  {
    titem = itemsptr->next;
    if (itemsptr->win == awin)
    {
      if (previtem == NULL)
      {
        itemsroot = titem;
        free(itemsptr);
      }
      else
      {
        previtem->next = titem;
        free(itemsptr);
      }
    }
    else
    {
      previtem = itemsptr;
    }
    itemsptr = titem;
  }
  
  FindWinProps(awin);
  XFreeGC(winsptr->gc);
  XDestroyWindow(winsptr->disp, awin);
  XFlush(winsptr->disp);
  if (winsptr == winsroot)
  {
    winsroot = winsptr->next;
    free(winsptr);
  }
  else
  {
    twin = winsroot;
    while (twin->next!= NULL && twin->next->win != awin) twin = twin->next;
    if (twin->next == NULL) return -1;
    twin->next = winsptr->next;
    free(winsptr);
  }
  return 1;
}

void DestroyItems()
{
  struct ItemPropNode *titem;
  itemsptr = itemsroot;
  while (itemsptr != NULL)
  {
    titem = itemsptr->next;
    itemsroot = titem;
    free(itemsptr);
    itemsptr = titem;
  }
  itemsroot = NULL;
}

void DestroyWins()
{
  struct WinPropNode *twin;
  winsptr = winsroot;
  while (winsptr != NULL)
  {
    twin = winsptr->next;
    winsroot = twin;
    FreeWindow(winsptr->win);
    winsptr = twin;
  }
  winsroot = NULL;
}

int DrawItems(Window awin)
{
  struct ItemPropNode *titem;
  
  if (FindWinProps(awin) == NULL) return 0;
  
  XClearWindow(winsptr->disp, awin);
  
  for (itemsptr = itemsroot; itemsptr != NULL; itemsptr = itemsptr->next)
  {
    if (itemsptr->win == awin)
    {
      FindWinProps(awin);
      if (itemsptr->curx <= winsptr->width && itemsptr->cury <= winsptr->height && (itemsptr->curx + itemsptr->width) >= 0 && (itemsptr->cury + itemsptr->height) >= 0)
      {
        /* Only draw if on screen */
        titem = itemsptr;
        DrawItem(itemsptr->itemid); /* Might clobber itemsptr! */
        itemsptr = titem;
      }
    }
  }
  
  FindWinProps(awin);
  XFlush(winsptr->disp);
  
  return 1;
}

unsigned long Getitemclicked()
{
  return itemclicked;
}

int MouseOver(Window awin, int x, int y, unsigned int btnstate)
{
  if (FindWinProps(awin) == NULL) return 0;
  
  for (itemsptr = itemsroot; itemsptr != NULL; itemsptr = itemsptr->next)
  {
    if (itemsptr->win == awin)
    {
      FindWinProps(awin);
      if (itemsptr->curx <= x && itemsptr->cury <= y && (itemsptr->curx + itemsptr->width) >= x && (itemsptr->cury + itemsptr->height) >= y)
      {
        winsptr->selitem = itemsptr->itemid;
      }
    }
  }
  DrawItems(awin);
  return 1;
}

int ClickItem(Window awin, int x, int y, unsigned int mousebtn, int btndown);
{
  if (FindWinProps(awin) == NULL) return 0;
  itemclicked = 0;
  if (mousebtn != 1) return 1; /* Ignore all but left click */
  if (btndown != 0)
  {
    mouseclickdown = 1;
    return 1;
  }
  if (mouseclickdown == 0)
  {
    /* Was clicked off-screen */
    return 1;
  }
  
  for (itemsptr = itemsroot; itemsptr != NULL; itemsptr = itemsptr->next)
  {
    if (itemsptr->win == awin)
    {
      FindWinProps(awin);
      if (itemsptr->curx <= x && itemsptr->cury <= y && (itemsptr->curx + itemsptr->width) >= x && (itemsptr->cury + itemsptr->height) >= y)
      {
        winsptr->selitem = itemsptr->itemid;
        itemclicked = itemsptr->itemid;
      }
    }
  }
  
  return 1;
}
