#include <stdio.h>
#include <stdlib.h>

#include "xfuncs.h"

Display thedisplay;
int thescreen;
Window mainwindow;

unsigned long black, white, green, lime;

int init_x()
{
  /* Returns 0 if it can't make the Window. */
  int winx = 0, winy = 0, ans = 1;;
  unsigned int winw = 300, winh = 200, winbord = 5;
  unsigned long winbordcol, winbgcol;
  struct WinPropNode *mwprop;
  
  thedisplay = XOpenDisplay(NULL);
  thescreen = DefaultScreen(thedisplay);
  black = BlackPixel(thedisplay, thescreen);
  white = WhitePixel(thedisplay, thescreen);
  lime = get_colour("green");
  green = get_colour("dark green");
  winbordcol = black;
  winbgcol = black;
  
  mwprop = NewWindow(thedisplay, DefaultRootWindow(thedisplay), "CompuWrist Menu", "CompuWrist Menu", None, NULL, 0, NULL, winx, winy, winw, winh, winbord, winbordcol, winbgcol, white);
  if (mwprop == NULL)
  {
    printf("Error Making Window!");
    ans = 0;
  }
  else
  {
    mainwindow = mwprop->win;
    int ity = 5, itx = 5, itw = mwprop->width-10, mwh = mwprop->height;
    int ith = 10;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 1")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 1");
    ity += ith + 5;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 2")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 2");
    ity += ith + 5;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 3")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 3");
    DrawItems(mainwindow);
  }
  
  XFlush(thedisplay);
  return ans;
}

void close_x()
{
  FreeWindow(mainwindow);
  DestroyItems();
  DestroyWins();
  XCloseDisplay(thedisplay);
  exit(0);
}

int main(int argc, char *argv[])
{
  init_x();
  
  XEvent event;
  KeySym key;
  unsigned long lastitemclicked;
  
  while (1)
  {
    /* Message Loop */
  }
}
