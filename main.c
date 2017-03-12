#include <stdio.h>
#include <stdlib.h>
#include <X11/Xft/Xft.h>

#include "xfuncs.h"
#include "sfuncs.h"


Display *thedisplay;
int thescreen;
Window mainwindow, subwindow;
struct MenuIDRel *winarray = NULL;
long wasize = 0;
/* XFontStruct *xfs = NULL; */
#ifdef HAVE_XFT
XftFont *keenfont = NULL, *sgafont = NULL;
struct FontIDRel *fontarray = NULL;
long fasize = 0;
#endif

struct ItemActionRel *iactarray = NULL;
long iaasize = 0;

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

int ParseMenuFile(FILE *mf)
{
  char mfline[1024] = "";
  char ctype = 0;
  while ( fgets(mfline,1023,mf) != NULL)
  {
    
  }
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
    keenfont = XftFontOpenName(thedisplay, thescreen, "Keen 4 Menu Font-12");
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
    ith = GetXftTextHeight(mwprop->disp, keenfont, "MENU ITEM 1")+10;
    	printf("#Text height is %d\n",ith);
    CreateItem(mainwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, keenfont, "MENU ITEM 1", 1);
    ity += ith + 5;
    ith = GetXftTextHeight(mwprop->disp, sgafont, "Menu Item 2")+10;
    CreateItem(mainwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, sgafont, "Menu Item 2", 2);
    ity += ith + 5;
    char mi3[] = "Menu Item 3 ____";
    mi3[12] = 0xB7;
    mi3[13] = 0xD7;
    ith = GetXftTextHeight(mwprop->disp, keenfont, mi3)+10;
    CreateItem(mainwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, keenfont, mi3, 3);
#else
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 1")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 1", 1);
    ity += ith + 5;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 2")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 2", 2);
    ity += ith + 5;
    ith = GetTextHeight(mwprop->disp, mwprop->gc, "Menu Item 3")+10;
    CreateItem(mainwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 3", 3);
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

int dosubwin()
{
  struct WinPropNode *swprop;
  int winx = 0, winy = 0, ans = 1;
  Window rootret;
  unsigned int winw = 300, winh = 200, winbord = 5, depthret;
  unsigned long winbordcol, winbgcol;
  
  swprop = NewWindow(thedisplay, DefaultRootWindow(thedisplay), "CompuWrist Sub Menu", "CompuWrist Sub Menu", None, NULL, 0, NULL, winx, winy, winw, winh, winbord, winbordcol, winbgcol, white);
  if (swprop == NULL)
  {
    printf("Error Making Window!");
    return 0;
  }
  subwindow = swprop->win;
  XGetGeometry(swprop->disp, (Drawable) swprop->win, &rootret, &winx, &winy, &winw, &winh, &winbord, &depthret);
  int ity = 5, itx = 5, itw = winw-10, swh = winh;
  int ith = 10;
    	printf("#Drawing the Items...\n");
#ifdef HAVE_XFT
  ith = GetXftTextHeight(swprop->disp, keenfont, "MENU ITEM 4")+10;
    	printf("#Text height is %d\n",ith);
  CreateItem(subwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, keenfont, "MENU ITEM 4", 4);
  ity += ith + 5;
  ith = GetXftTextHeight(swprop->disp, sgafont, "Menu Item 5")+10;
  CreateItem(subwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, sgafont, "Menu Item 5", 5);
  ity += ith + 5;
  char mi3[] = "Menu Item 6 ____";
  mi3[12] = 0xB7;
  mi3[13] = 0xD7;
  ith = GetXftTextHeight(swprop->disp, keenfont, mi3)+10;
  CreateItem(subwindow, itx, ity, itw, ith, &xblack, &xlime, &xgreen, keenfont, mi3, 6);
#else
  ith = GetTextHeight(swprop->disp, swprop->gc, "Menu Item 4")+10;
  CreateItem(subwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 4", 4);
  ity += ith + 5;
  ith = GetTextHeight(swprop->disp, swprop->gc, "Menu Item 5")+10;
  CreateItem(subwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 5", 5);
  ity += ith + 5;
  ith = GetTextHeight(swprop->disp, swprop->gc, "Menu Item 6")+10;
  CreateItem(subwindow,itx,ity,itw,ith,black,lime,green,"Menu Item 6", 6);
#endif
  DrawItems(subwindow);
  
  XFlush(thedisplay);
  return 1;
}

int undosubwin()
{
  int ans = FreeWindow(subwindow);
  XFlush(thedisplay);
  return ans;
}

int ItemEvent(Window awin, unsigned long wih)
{
  /* Item selection event handler */
  
  /* Subwindow Test */
  if (wih == 3 && awin == mainwindow)
  {
    if (subwindow != mainwindow && FindWinProps(subwindow) == NULL)
    {
      dosubwin();
      DrawItems(subwindow);
      XFlush(thedisplay);
    }
    else
    {
      undosubwin();
    }
  }
}

int main(int argc, char *argv[])
{
  	printf("#It Begins...\n");
  init_x();
  	printf("#Now into the message loop...\n");
  
  XEvent event;
  KeySym key;
  char text[256] = "";
  unsigned long lastitemclicked, lastwihclicked;
  Window lastwinclicked;
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
        Window rootret;
        int winx, winy;
        unsigned int winw, winh, winbord, depthret;
        
        XGetGeometry(mwprop->disp, (Drawable) mainwindow, &rootret, &winx, &winy, &winw, &winh, &winbord, &depthret);
        if (winw >= 200)
        {
          ResizeItems(mainwindow, winw, 0);
        }
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
      
      if (text[0] == 13)
      {
        /* Simulate Click */
        struct WinPropNode *awprop;
        awprop = FindWinProps(event.xkey.window);
        if (awprop != NULL)
        {
          struct ItemPropNode *aiprop;
          aiprop = FindItemProps(awprop->selitem);
          if (aiprop != NULL)
          {
            ItemEvent(event.xkey.window,aiprop->winitemhandle);
          }
        }
      }
    }
    else if (event.type == KeyPress && event.xkey.window == mainwindow)
    {
      unsigned long tiid;
      printf("Key press: %x\n",event.xkey.keycode);
      if (event.xkey.keycode == 0x6f || event.xkey.keycode == 0x71)
      {
        /* UP or LEFT */
        tiid = mwprop->selitem;
        tiid = PrevItemInWindow(mainwindow, tiid);
        if (tiid != 0)
        {
          /* Select item tiid */
          mwprop->selitem = tiid;
          DrawItems(mainwindow);
        }
      }
      else if (event.xkey.keycode == 0x72 || event.xkey.keycode == 0x74)
      {
        /* RIGHT or DOWN */
        tiid = mwprop->selitem;
        tiid = NextItemInWindow(mainwindow, tiid);
        if (tiid != 0)
        {
          /* Select item tiid */
          mwprop->selitem = tiid;
          DrawItems(mainwindow);
        }
      }
    }
    else if (event.type == KeyPress ) /*&& event.xkey.window == subwindow)*/
    {
      unsigned long tiid;
      struct WinPropNode *awprop;
      awprop = FindWinProps(event.xkey.window);
      if (awprop != NULL)
      {
        printf("Key press: %x\n",event.xkey.keycode);
        if (event.xkey.keycode == 0x6f || event.xkey.keycode == 0x71)
        {
          /* UP or LEFT */
          tiid = awprop->selitem;
          tiid = PrevItemInWindow(event.xkey.window, tiid);
          if (tiid != 0)
          {
            /* Select item tiid */
            awprop->selitem = tiid;
            DrawItems(event.xkey.window);
          }
        }
        else if (event.xkey.keycode == 0x72 || event.xkey.keycode == 0x74)
        {
          /* RIGHT or DOWN */
          tiid = awprop->selitem;
          tiid = NextItemInWindow(event.xkey.window, tiid);
          if (tiid != 0)
          {
            /* Select item tiid */
            awprop->selitem = tiid;
            DrawItems(event.xkey.window);
          }
        }
      }
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
      lastwinclicked = GetItemWin(lastitemclicked);
      lastwihclicked = GetItemWIH(lastitemclicked);
      printf("Clicked item %X:%lu.\n",lastwinclicked, lastwihclicked);
      
      ItemEvent(event.xbutton.window, lastwihclicked);
    }
    
    if (event.type == MotionNotify)
    {
      MouseOver(event.xmotion.window, event.xmotion.x, event.xmotion.y, event.xmotion.state);
      
    }
    
    if (event.type == DestroyNotify || event.type == ClientMessage)
    {
      /* Closed window ? */
      if (event.xany.window == mainwindow) close_x();
      if (event.xany.window == subwindow) undosubwin();
    }
  }
  return 1;
}
