#include <stdio.h>
#include <stdlib.h>
#include <X11/Xft/Xft.h>

#include "xfuncs.h"

Display *thedisplay;
int thescreen;
Window mainwindow;
/* XFontStruct *xfs = NULL; */
#ifdef HAVE_XFT
XftFont *keenfont = NULL, *sgafont = NULL;
#endif

unsigned long black, white, green, lime;
#ifdef HAVE_XFT
XftColor xblack, xwhite, xgreen, xlime;
#endif

int DoListFonts()
{
  char **flist = NULL;
  int fcount, i;
  flist = XListFonts(thedisplay, "*", 200, &fcount);
  printf("Fonts:\n");
  if (flist == NULL) return 0;
  for (i=0; i<fcount; i++)
  {
    printf("%d: '%s'\n",i+1,flist[i]);
  }
  XFreeFontNames(flist);
  return 1;
}

int init_x()
{
  /* Returns 0 if it can't make the Window. */
  int winx = 0, winy = 0, ans = 1;
  Window rootret;
  unsigned int winw = 300, winh = 200, winbord = 5, depthret;
  unsigned long winbordcol, winbgcol;
  struct WinPropNode *mwprop;
  
  	printf("#This bit should work fine...\n");
  thedisplay = XOpenDisplay(NULL);
  thescreen = DefaultScreen(thedisplay);
  black = BlackPixel(thedisplay, thescreen);
  white = WhitePixel(thedisplay, thescreen);
  lime = get_colour("green", thedisplay, thescreen);
  green = get_colour("dark green", thedisplay, thescreen);
  	printf("#Up to here\n#Then...\n");
#ifdef HAVE_XFT
  get_xft_colour(&xblack, 0,0,0,0xffff, thedisplay, thescreen);
  	printf("#Made xblack\n");
  get_xft_colour(&xwhite, 0xffff,0xffff,0xffff,0xffff, thedisplay, thescreen);
  	printf("#Made xwhite\n");
  get_xft_colour(&xgreen, 0,0x8888,0,0xffff, thedisplay, thescreen);
  	printf("#Made xgreen\n");
  get_xft_colour(&xlime, 0,0xffff,0,0xffff, thedisplay, thescreen);
  	printf("#Made xlime\n");
  /* Remember to free these at the end! */
#endif
  
  winbordcol = black;
  winbgcol = black;
  
  	printf("#Time to make the window!\n");
  mwprop = NewWindow(thedisplay, DefaultRootWindow(thedisplay), "CompuWrist Menu", "CompuWrist Menu", None, NULL, 0, NULL, winx, winy, winw, winh, winbord, winbordcol, winbgcol, white);
  if (mwprop == NULL)
  {
    printf("Error Making Window!");
    ans = 0;
  }
  else
  {
    	printf("#Done that\n#Now.  Fonts...\n");
/*    xfs = XLoadQueryFont(thedisplay,"*Keen*");
    if (xfs == NULL) printf("Cannot find font.\n");
    DoListFonts();*/
#ifdef HAVE_XFT
    keenfont = XftFontOpenName(thedisplay, thescreen, "Keen 1FON0000-12");
    if (keenfont == NULL) printf("Cannot find Keen font.\n");
    sgafont = XftFontOpenName(thedisplay, thescreen, "SGA K3 Direct-12");
    if (sgafont == NULL) printf("Cannot find SGA font.\n");
#endif
    	printf("#Fonts done!\n#Time to set up the Window...\n");
    
    mainwindow = mwprop->win;
    XGetGeometry(mwprop->disp, (Drawable) mwprop->win, &rootret, &winx, &winy, &winw, &winh, &winbord, &depthret);
    int ity = 5, itx = 5, itw = winw-10, mwh = winh;
    int ith = 10;
    	printf("#Drawing the Items...\n");
#ifdef HAVE_XFT
    ith = GetXftTextHeight(mwprop->disp, keenfont, "Menu Item 1")+10;
    	printf("#Text height is %d\n",ith);
    CreateItem(mainwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, keenfont, "Menu Item 1");
    ity += ith + 5;
    ith = GetXftTextHeight(mwprop->disp, keenfont, "Menu Item 2")+10;
    CreateItem(mainwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, sgafont, "Menu Item 2");
    ity += ith + 5;
    ith = GetXftTextHeight(mwprop->disp, keenfont, "Menu Item 3")+10;
    CreateItem(mainwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, keenfont, "Menu Item 3");
#else
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 1")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 1");
    ity += ith + 5;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 2")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 2");
    ity += ith + 5;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 3")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 3");
#endif
    DrawItems(mainwindow);
  }
  
  XFlush(thedisplay);
  return ans;
}

void close_x()
{
/*  if (xfs != NULL) XFreeFont(thedisplay, xfs);  */
  FreeWindow(mainwindow);
  DestroyItems();
  DestroyWins();
#ifdef HAVE_XFT
  if (keenfont != NULL) XftFontClose(thedisplay, keenfont);
  if (sgafont != NULL) XftFontClose(thedisplay, sgafont);
  free_xft_colour(thedisplay, thescreen, &xblack);
  free_xft_colour(thedisplay, thescreen, &xwhite);
  free_xft_colour(thedisplay, thescreen, &xgreen);
  free_xft_colour(thedisplay, thescreen, &xlime);
#endif
  XCloseDisplay(thedisplay);
  exit(0);
}

int main(int argc, char *argv[])
{
  	printf("#It Begins...\n");
  init_x();
  	printf("#Now into the message loop...\n");
  
  XEvent event;
  KeySym key;
  char text[256] = "";
  unsigned long lastitemclicked;
  struct WinPropNode *mwprop = FindWinProps(mainwindow);
  
  while (1)
  {
    /* Message Loop */
    XNextEvent(thedisplay, &event);
    
    if (event.type == Expose && event.xexpose.count == 0)
    {
      /* Redraw Window! */
      if (event.xany.window == mainwindow)
      {
        XSetBackground(mwprop->disp, mwprop->gc, black);
        XClearWindow(mwprop->disp, mainwindow);
        DrawItems(mainwindow);
      }
      XFlush(thedisplay);
    }
    
    if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1)
    {
      if (text[0] == 'q') close_x(); /* For DEBUGGING ONLY */
      else printf("The %c (%d) key was pressed\n", text[0], text[0]);
    }
    
    if (event.type == ButtonPress)
    {
      /* Mouse Clicked */
      ClickItem(event.xbutton.window, event.xbutton.x, event.xbutton.y, event.xbutton.button, 1);
      /* lastitemclicked = Getitemclicked();
      printf("Clicked item %lu.\n", lastitemclicked); */
    }
    
    if (event.type == ButtonRelease)
    {
      /* Mouse Clicked */
      ClickItem(event.xbutton.window, event.xbutton.x, event.xbutton.y, event.xbutton.button, 0);
      lastitemclicked = Getitemclicked();
      printf("Clicked item %lu.\n", lastitemclicked);
    }
    
    if (event.type == MotionNotify)
    {
      MouseOver(event.xmotion.window, event.xmotion.x, event.xmotion.y, event.xmotion.state);
      
    }
    
    if (event.type == DestroyNotify || event.type == ClientMessage)
    {
      /* Closed window ? */
      if (event.xany.window == mainwindow) close_x();
    }
  }
  return 1;
}
