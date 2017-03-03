#ifndef __INC_XFUNCS_H__
#define __INC_XFUNCS_H__ 1

#define ITEMMAXLEN 256

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


struct WinPropNode {
  Display *disp;
  Window win;
  Window parent;
  GC gc;
  unsigned long selitem;
  struct WinPropNode *next;
};

struct ItemPropNode {
  unsigned long itemid;
  int curx;
  int cury;
  unsigned int width;
  unsigned int height;
  unsigned long bgcolour;
  unsigned long selfgcolour;
  unsigned long unselfgcolour;
  char itemtext[ITEMMAXLEN+1];
  Window win;
  struct ItemPropNode *next;
};


unsigned long get_colour(char *acolour, Display *dis, int screen);
int GetTextLength(Display *disp, GC agc, char *sometext);
int GetTextAscent(Display *disp, GC agc, char *sometext);
int GetTextDescent(Display *disp, GC agc, char *sometext);
int GetTextHeight(Display *disp, GC agc, char *sometext);
void DoRTrim(char *sometext);
struct WinPropNode *FindWinProps(Window awin);
struct WinPropNode *NewWindow(Display *disp, Window parent, char *caption, 
  char *iconcaption, Pixmap icon, char **argv, int argc, XSizeHints *hints, 
  int x, int y, unsigned int width, unsigned int height, 
  unsigned int borderwidth, unsigned long bordercol, unsigned long bgcol, 
  unsigned long fgcol);
struct ItemPropNode *FindItemProps(unsigned long itemid);
unsigned long CreateItem(Window awin, int x, int y, unsigned int width, 
  unsigned int height, unsigned long bgcolour, unsigned long selfgcolour, 
  unsigned long unselfgcolour, char *itemtext);



#endif
