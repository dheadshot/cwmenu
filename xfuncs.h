#ifndef __INC_XFUNCS_H__
#define __INC_XFUNCS_H__ 1

#define ITEMMAXLEN 256

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif


struct WinPropNode {
  Display *disp;
  Window win;
  Window parent;
  GC gc;
#ifdef HAVE_XFT
  XftDraw *xftdc; /* Xft version of GC */
  XftColor *xbgcol; /* Background Colour for xftdc */
#endif
  int visible;
  unsigned long selitem;
  struct WinPropNode *next;
};

struct ItemPropNode {
  unsigned long itemid;        /* This changes if the window is destroyed and then recreated. */
  unsigned long winitemhandle; /* Within a Window, this is always static. */
  int curx;
  int cury;
  unsigned int width;
  unsigned int height;
  unsigned long bgcolour;
  unsigned long selfgcolour;
  unsigned long unselfgcolour;
#ifdef HAVE_XFT
  XftColor *xftbgcolour;
  XftColor *xftselcolour;
  XftColor *xftunselcolour;
  XftFont *font;
#endif
  char itemtext[ITEMMAXLEN+1];
  Window win;
  struct ItemPropNode *next;
};

#ifdef HAVE_XFT
struct FontIDRel {
  unsigned long id;
  XftFont *font;
};
#endif

struct MenuIDRel {
  unsigned long id;
  Window win;
  int curiy;
};

union ItemAction {
  char *command;
  unsigned long menuid;
};

struct ItemActionRel {
  struct ItemPropNode *itemptr;
  char type;
  union ItemAction action;
};


unsigned long get_colour(char *acolour, Display *dis, int screen);
#ifdef HAVE_XFT
int get_xft_colour(XftColor *result, unsigned short redv, 
  unsigned short greenv, unsigned short bluev, unsigned short alphav, 
  Display *disp, int screen);
void free_xft_colour(Display *disp, int screen, XftColor *xftcolour);
int GetXftTextAscent(Display *disp, XftFont *afont, char *sometext);
unsigned short GetXftTextHeight(Display *disp, XftFont *afont, char *sometext);
int GetXftTextDescent(Display *disp, XftFont *afont, char *sometext);
int GetXftTextLength(Display *disp, XftFont *afont, char *sometext);

#endif
int GetTextLength(Display *disp, GC agc, char *sometext);
int GetTextAscent(Display *disp, GC agc, char *sometext);
int GetTextDescent(Display *disp, GC agc, char *sometext);
int GetTextHeight(Display *disp, GC agc, char *sometext);
void DoRTrim(char *sometext);
struct WinPropNode *FindWinProps(Window awin);
#ifdef HAVE_XFT
struct WinPropNode *NewWindow(Display *disp, Window parent, char *caption, 
  char *iconcaption, Pixmap icon, char **argv, int argc, XSizeHints *hints, 
  int x, int y, unsigned int width, unsigned int height, 
  unsigned int borderwidth, unsigned long bordercol, unsigned long bgcol, 
  unsigned long fgcol, XftColor *xbgcol, int visible);
#else
struct WinPropNode *NewWindow(Display *disp, Window parent, char *caption, 
  char *iconcaption, Pixmap icon, char **argv, int argc, XSizeHints *hints, 
  int x, int y, unsigned int width, unsigned int height, 
  unsigned int borderwidth, unsigned long bordercol, unsigned long bgcol, 
  unsigned long fgcol, int visible);
#endif
int ChangeWindowVisibility(Window awin, int visible);
Window GetItemWin(unsigned long itemid);
unsigned long GetItemWIH(unsigned long itemid);
struct ItemPropNode *FindItemProps(unsigned long itemid);
struct ItemPropNode *FindItemWIH(Window awin, unsigned long wih);
#ifdef HAVE_XFT
unsigned long CreateItem(Window awin, int x, int y, unsigned int width, 
  unsigned int height, XftColor *bgcolour, XftColor *selcolour, 
  XftColor *unselcolour, XftFont *afont, char *itemtext, 
  unsigned long winitemhandle);
#else
unsigned long CreateItem(Window awin, int x, int y, unsigned int width, 
  unsigned int height, unsigned long bgcolour, unsigned long selfgcolour, 
  unsigned long unselfgcolour, char *itemtext, unsigned long winitemhandle);
#endif
int DrawItem(unsigned long itemid);
int FreeWindow(Window awin);
void DestroyItems();
void DestroyWins();
int DrawItems(Window awin);
unsigned long Getitemclicked();
int MouseOver(Window awin, int x, int y, unsigned int btnstate);
int ClickItem(Window awin, int x, int y, unsigned int mousebtn, int btndown);
unsigned long PrevItemInWindow(Window awin, unsigned long itemid);
unsigned long NextItemInWindow(Window awin, unsigned long itemid);
int ResizeItems(Window awin, unsigned int newwidth, unsigned int newheight);



#endif
