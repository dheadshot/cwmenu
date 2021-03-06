#include <stdio.h>
#include <stdlib.h>
#include <X11/Xft/Xft.h>
#include <signal.h>

#include "xfuncs.h"
#include "sfuncs.h"
#include "execs.h"


char versionstr[] = "0.01.00";

Display *thedisplay;
int thescreen;
Window mainwindow = 0, subwindow = 0;
struct MenuIDRel *winarray = NULL;
long wasize = 0;
/* XFontStruct *xfs = NULL; */
#ifdef HAVE_XFT
XftFont *keenfont = NULL, *sgafont = NULL;
struct FontIDRel *fontarray = NULL;
long fasize = 0;
#endif

int interrupted = 0;

struct ItemActionRel *iactarray = NULL;
long iaasize = 0;

unsigned long black, white, green, lime, grey, silver, red, maroon, blue, navy,
              yellow, brown, cyan, teal, magenta, fuchsia;
unsigned long setbgcol, setselcol, setunselcol;
#ifdef HAVE_XFT
XftColor xblack, xwhite, xgreen, xlime, xgrey, xsilver, xred, xmaroon, xblue,
         xnavy, xyellow, xbrown, xcyan, xteal, xmagenta, xfuchsia;
XftColor *setxbgcol, *setxselcol, *setxunselcol;
#endif


void close_x(int returnval);


/* Don't need this anymore! * /
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
}*/

void handle_sigint(int sig)
{
  if (interrupted == 0)
  {
    	printf("# Interrupted!  Terminating...\n");
    interrupted = 5;
  }
  else
  {
  	printf("# Already terminating!  Patience please!\n");
  }
}

void handle_sigterm(int sig)
{
  if (interrupted == 0)
  {
    	printf("# Terminated!  Terminating...\n");
    interrupted = 6;
  }
  else
  {
  	printf("# Already terminating!  Patience please!\n");
  }
}

int ParseMenuFile(FILE *mf)
{
  char mfline[1024] = "";
  char ctype = 0;
  long fnum = 0, mnum = 0, inum = 0, n;
  
  while ( fgets(mfline,1023,mf) != NULL)
  {
    if (mfline[strlen(mfline)-1] == '\n') mfline[strlen(mfline)-1] = 0;
    if (mfline[strlen(mfline)-1] == '\r') mfline[strlen(mfline)-1] = 0;
    if (streq_(mfline,"[COLOURS]") || streq_(mfline,"[COLORS]"))
    {
      ctype = 'C';
      	printf("# In Colours\n");
    }
    else if (streq_(mfline,"[FONTS]"))
    {
      ctype = 'F';
      	printf("# In Fonts\n");
    }
    else if (streq_(mfline,"[MENUS]"))
    {
      ctype = 'M';
      	printf("# In Menus\n");
    }
    else if (streq_(mfline,"[ITEMS]"))
    {
      ctype = 'I';
      	printf("# In Items\n");
    }
    else if (mfline[0] == '#' || mfline[0] == '\r' || mfline[0] == '\n' || mfline[0] == 0)
    {
      /* Ignore comments and blank lines! */
    }
    else if (startsame_(mfline,"NUM="))
    {
      switch (ctype)
      {
        case 'C':
          if (atol(mfline+4) >3) printf("Warning: Only 3 colours should be specified!");
        break;
        
        case 'F':
#ifdef HAVE_XFT
          if (fasize >0)
          {
            for (n=0;n<fasize;n++)
            {
              if (fontarray[n].font != NULL) XftFontClose(thedisplay, fontarray[n].font);
            } /* Free each font, then... */
            free(fontarray);
            fontarray = NULL;
            fasize = 0;
          }
          else fontarray = NULL;
          fasize=atol(mfline+4);
          if (fasize>0) fontarray = (struct FontIDRel *) malloc(sizeof(struct FontIDRel)*fasize);
          if (fontarray == NULL) fasize = 0;
          if (fasize > 0) memset(fontarray, 0, fasize*sizeof(struct FontIDRel));
          	printf("# %ld Fonts\n", fasize);
#endif
        break;
        
        case 'M':
          if (wasize >0)
          {
            for (n=0;n<wasize;n++)
            {
              if (winarray[n].win != 0) FreeWindow(winarray[n].win);
            } /* Free each menu, then... */
            free(winarray);
            winarray = NULL;
            wasize = 0;
          }
          else winarray = NULL;
          wasize=atol(mfline+4);
          if (wasize>0) winarray = (struct MenuIDRel *) malloc(sizeof(struct MenuIDRel)*wasize);
          if (winarray == NULL) wasize = 0;
          if (wasize > 0) memset(winarray, 0, wasize*sizeof(struct MenuIDRel));
          	printf("# %ld Menus\n", wasize);
        break;
        
        case 'I':
          if (iaasize >0)
          {
            for (n=0;n<iaasize;n++)
            {
              if (iactarray[n].type == 'C' || iactarray[n].type == 'c')
              {
                if (iactarray[n].action.command != NULL) free(iactarray[n].action.command);
              }
            } /* Free each Action, then... */
            free(iactarray);
            iactarray = NULL;
            iaasize = 0;
          }
          else iactarray = NULL;
          iaasize=atol(mfline+4);
          if (iaasize>0) iactarray = (struct ItemActionRel *) malloc(sizeof(struct ItemActionRel)*iaasize);
          if (iactarray == NULL) iaasize = 0;
          if (iaasize > 0) memset(iactarray, 0, iaasize*sizeof(struct ItemActionRel));
          	printf("# %ld Items\n", iaasize);
        break;
      }
    }
    else
    {
      switch (ctype)
      {
        case 0:
         /* Ignore anything written before the first section */
         	printf("# Ignoring '%s'\n",mfline);
        break;
        
        case 'C':
        {
          char coltype, col;
          unsigned long pickedcol = black;
#ifdef HAVE_XFT
          XftColor *pickedxcol = &xblack;
#endif
          sscanf(mfline, "%c, %c", &coltype, &col);
          
          switch (col)
          {
            case '0':
              pickedcol = black;
#ifdef HAVE_XFT
              pickedxcol = &xblack;
#endif
            break;
            
            case '1':
              pickedcol = navy;
#ifdef HAVE_XFT
              pickedxcol = &xnavy;
#endif
            break;
            
            case '2':
              pickedcol = green;
#ifdef HAVE_XFT
              pickedxcol = &xgreen;
#endif
            break;
            
            case '3':
              pickedcol = teal;
#ifdef HAVE_XFT
              pickedxcol = &xteal;
#endif
            break;
            
            case '4':
              pickedcol = maroon;
#ifdef HAVE_XFT
              pickedxcol = &xmaroon;
#endif
            break;
            
            case '5':
              pickedcol = magenta;
#ifdef HAVE_XFT
              pickedxcol = &xmagenta;
#endif
            break;
            
            case '6':
              pickedcol = brown;
#ifdef HAVE_XFT
              pickedxcol = &xbrown;
#endif
            break;
            
            case '7':
              pickedcol = silver;
#ifdef HAVE_XFT
              pickedxcol = &xsilver;
#endif
            break;
            
            case '8':
              pickedcol = grey;
#ifdef HAVE_XFT
              pickedxcol = &xgrey;
#endif
            break;
            
            case '9':
              pickedcol = blue;
#ifdef HAVE_XFT
              pickedxcol = &xblue;
#endif
            break;
            
            case 'A':
            case 'a':
              pickedcol = lime;
#ifdef HAVE_XFT
              pickedxcol = &xlime;
#endif
            break;
            
            case 'B':
            case 'b':
              pickedcol = cyan;
#ifdef HAVE_XFT
              pickedxcol = &xcyan;
#endif
            break;
            
            case 'C':
            case 'c':
              pickedcol = red;
#ifdef HAVE_XFT
              pickedxcol = &xred;
#endif
            break;
            
            case 'D':
            case 'd':
              pickedcol = fuchsia;
#ifdef HAVE_XFT
              pickedxcol = &xfuchsia;
#endif
            break;
            
            case 'E':
            case 'e':
              pickedcol = yellow;
#ifdef HAVE_XFT
              pickedxcol = &xyellow;
#endif
            break;
            
            case 'F':
            case 'f':
              pickedcol = white;
#ifdef HAVE_XFT
              pickedxcol = &xwhite;
#endif
            break;
            
            default:
              printf("Warning: Unknown colour \"%c\" - using black!\n",col);
            break;
          }
          
          switch (coltype)
          {
            case 'B':
            case 'b':
              setbgcol = pickedcol;
#ifdef HAVE_XFT
              setxbgcol = pickedxcol;
#endif
            break;
            
            case 'U':
            case 'u':
              setunselcol = pickedcol;
#ifdef HAVE_XFT
              setxunselcol = pickedxcol;
#endif
            break;
            
            case 'S':
            case 's':
              setselcol = pickedcol;
#ifdef HAVE_XFT
              setxselcol = pickedxcol;
#endif
            break;
            
            default:
              printf("Warning: Unknown colour type \"%s\".  Ignoring this line!\n",coltype);
            break;
          }
        }
        break;
        
        case 'F':
        {
#ifdef HAVE_XFT
          if (fnum < fasize)
          {
            unsigned long fid = 0;
            unsigned long fsize = 0;
            char fname[256] = "", fspec[300] = "";
            sscanf(mfline, "%lu, %lu, \"%[^\"]\"", &fid, &fsize, fname);
            sprintf(fspec, "%s-%lu", fname, fsize);
            	printf("# Font %ld = \"%s\"\n",fid,fspec);
            fontarray[fnum].id = fid;
            fontarray[fnum].font = XftFontOpenName(thedisplay, thescreen, fspec);
            if (fontarray[fnum].font == NULL) printf("Failed to open font \"%s\" (size %lu)!\n",fname, fsize);
          }
          fnum++;
#endif
        }
        break;
        
        case 'M':
        {
          if (mnum<wasize)
          {
            unsigned long wid;
            char wcapt[257] = "";
            int wx, wy;
            unsigned int wwidth, wheight, wbwidth;
            sscanf(mfline, "%lu, \"%[^\"]\", %d, %d, %u, %u, %u", &wid, wcapt, &wx, &wy, &wwidth, &wheight, &wbwidth);
            winarray[mnum].id = wid;
            winarray[mnum].curiy = 0;
            struct WinPropNode *awp;
#ifdef HAVE_XFT
            awp = NewWindow(thedisplay, DefaultRootWindow(thedisplay), wcapt, wcapt, None, NULL, 0, NULL, wx, wy, wwidth, wheight, wbwidth, white, setbgcol, setunselcol, setxbgcol, 0);
#else
            awp = NewWindow(thedisplay, DefaultRootWindow(thedisplay), wcapt, wcapt, None, NULL, 0, NULL, wx, wy, wwidth, wheight, wbwidth, white, setbgcol, setunselcol, 0);
#endif
            if (awp != NULL)
            {
              winarray[mnum].win = awp->win;
              	printf("# Made Window 0x%X (\"%s\")\n", awp->win, wcapt);
            }
            else winarray[mnum].win = 0;
          }
          mnum++;
        }
        break;
        
        case 'I':
        {
          if (inum>=iaasize) break;
          
          unsigned long wid, wih, fid, iamid = 0, iapt = 0;
          char icaption[257] = "", iacmd[1024] = "";
          char iatype;
          
          sscanf(mfline, "%lu, %lu, %lu, \"%[^\"]\", %c,%n", &wid, &wih, &fid, icaption, &iatype, &iapt);
          	printf("# Item %lu:%lu (\"%s\") type %c rp: %lu\n", wid, wih, icaption, iatype, iapt);
          switch (iatype)
          {
            case 'M':
            case 'm':
              iamid = atol(mfline+iapt);
              iactarray[inum].type = 'M';
              iactarray[inum].action.menuid = iamid;
            break;
            
            case 'C':
            case 'c':
              strltrim(iacmd, mfline+iapt);
              iactarray[inum].type = 'C';
              iactarray[inum].action.command = (char *) malloc(sizeof(char)*(1+strlen(iacmd)));
              if (iactarray[inum].action.command == NULL)
              {
                printf("Warning: Out of memory assigning action \"%s\"!", iacmd);
              }
              else
              {
                strcpy(iactarray[inum].action.command,iacmd+1);
                unsigned long nend = strlen(iactarray[inum].action.command);
                while (iactarray[inum].action.command[nend] != '"' && nend > 0) nend--;
                if (nend != 0) iactarray[inum].action.command[nend] = 0;
              }
            break;
            
            case 'B':
            case 'b':
              iactarray[inum].type = 'B';
            break;
            
            case 'X':
            case 'x':
              iactarray[inum].type = 'X';
            break;
            
            case 'N':
            case 'n':
              iactarray[inum].type = 'N';
            break;
            
            default:
              printf("Warning: Invalid Item Type '%c'!\n", iatype);
            break;
          }
          Window awin = 0;
#ifdef HAVE_XFT
          XftFont *afont = NULL;
          long m;
#endif
          for (n = 0; n < wasize; n++)
          {
            if (winarray[n].id == wid)
            {
              awin = winarray[n].win;
              break;
            }
          }
          if (awin == 0)
          {
            iactarray[inum].itemptr = NULL;
            printf("Warning: No MenuID '%lu', cannot create item with handle '%lu:%lu'\n", wid, wid, wih);
            inum++;
            break;
          }
          m = n;
#ifdef HAVE_XFT
          for (n = 0; n < fasize; n++)
          {
            if (fontarray[n].id == fid)
            {
              afont = fontarray[n].font;
              break;
            }
          }
          if (afont == NULL)
          {
            iactarray[inum].itemptr = NULL;
            printf("Warning: No FontID '%lu', cannot create item with handle '%lu:%lu'\n", fid, wid, wih);
            inum++;
            break;
          }
#endif
          Window wrr;
          int wxr, wyr;
          unsigned int wwr,whr,wbwr,wdr, ith, itw, ity;
          unsigned long iid;
          XGetGeometry(thedisplay, awin, &wrr, &wxr, &wyr, &wwr, &whr, &wbwr, &wdr);
          itw = wwr-10;
          ith = GetXftTextHeight(thedisplay, afont, icaption)+10;
          /* winarray[m] is window */
          ity = winarray[m].curiy;
          winarray[m].curiy += ith+5;
          
#ifdef HAVE_XFT
          iid = CreateItem(awin,5,ity,itw,ith,setxbgcol,setxselcol,setxunselcol,afont,icaption,wih);
#else
          iid = CreateItem(awin,5,ity,itw,ith,setbgcol,setselcol,setunselcol,icaption,wih);
#endif
          if (iid == 0) printf("Warning: Could not create item \"%s\"!\n", icaption);
          else iactarray[inum].itemptr = FindItemProps(iid);
          inum++;
        }
        break;
      }
    }
  }
  if (!feof(mf)) return 0;
  return 1;
}

unsigned long FindMainMenu()
{
  if (winarray == NULL || wasize <= 0) return 0;
  long n;
  unsigned long curminid = 0;
  curminid--; /* Set to max unsigned long... */
  for (n=0;n<wasize;n++)
  {
    if (winarray[n].id < curminid) curminid = winarray[n].id;
  }
  return curminid;
}

void FreeGeneratedArrays()
{
  long n;
  
  if (iactarray != NULL)
  {
    for (n=0;n<iaasize;n++)
    {
      if (iactarray[n].type == 'C' || iactarray[n].type == 'c')
      {
        if (iactarray[n].action.command != NULL) free(iactarray[n].action.command);
      }
    }
    free(iactarray);
    iactarray = NULL;
    iaasize = 0;
  }
  
  if (winarray != NULL)
  {
    for (n=0;n<wasize;n++)
    {
      if (winarray[n].win != 0) FreeWindow(winarray[n].win);
    }
    free(winarray);
    winarray = NULL;
    wasize = 0;
  }
  
  if (fontarray == NULL) return;
  for (n=0;n<fasize;n++)
  {
#ifdef HAVE_XFT
    if (fontarray[n].font != NULL) XftFontClose(thedisplay, fontarray[n].font);
#endif
  }
  free(fontarray);
  fontarray = NULL;
  fasize = 0;
  
  return;
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
  grey = get_colour("grey40", thedisplay, thescreen);
  silver = get_colour("grey75", thedisplay, thescreen);
  red = get_colour("red", thedisplay, thescreen);
  maroon = get_colour("dark red", thedisplay, thescreen);
  blue = get_colour("blue", thedisplay, thescreen);
  navy = get_colour("dark blue", thedisplay, thescreen);
  yellow = get_colour("yellow", thedisplay, thescreen);
  brown = get_colour("yellow4", thedisplay, thescreen);
  cyan = get_colour("cyan", thedisplay, thescreen);
  teal = get_colour("dark cyan", thedisplay, thescreen);
  fuchsia = get_colour("magenta", thedisplay, thescreen);
  magenta = get_colour("dark magenta", thedisplay, thescreen);
  	printf("#Up to here\n#Then...\n");
#ifdef HAVE_XFT
  get_xft_colour(&xblack, 0,0,0,0xffff, thedisplay, thescreen);
  	printf("#Made xblack\n");
  get_xft_colour(&xwhite, 0xffff,0xffff,0xffff,0xffff, thedisplay, thescreen);
  	printf("#Made xwhite\n");
  get_xft_colour(&xgreen, 0,0xAAAA,0,0xffff, thedisplay, thescreen);
  	printf("#Made xgreen\n");
  get_xft_colour(&xlime, 0x5555,0xffff,0x5555,0xffff, thedisplay, thescreen);
  	printf("#Made xlime\n");
  get_xft_colour(&xgrey, 0x5555,0x5555,0x5555,0xffff, thedisplay, thescreen);
  	printf("#Made xgrey\n");
  get_xft_colour(&xsilver, 0xAAAA,0xAAAA,0xAAAA,0xffff, thedisplay, thescreen);
  	printf("#Made xsilver\n");
  get_xft_colour(&xred, 0xFFFF,0x5555,0x5555,0xffff, thedisplay, thescreen);
  	printf("#Made xred\n");
  get_xft_colour(&xmaroon, 0xAAAA,0x0000,0x0000,0xffff, thedisplay, thescreen);
  	printf("#Made xmaroon\n");
  get_xft_colour(&xbrown, 0xAAAA,0x5555,0x0000,0xffff, thedisplay, thescreen);
  	printf("#Made xbrown\n");
  get_xft_colour(&xyellow, 0xFFFF,0xFFFF,0x5555,0xffff, thedisplay, thescreen);
  	printf("#Made xyellow\n");
  get_xft_colour(&xteal, 0x0000,0xAAAA,0xAAAA,0xffff, thedisplay, thescreen);
  	printf("#Made xteal\n");
  get_xft_colour(&xcyan, 0x5555,0xFFFF,0xFFFF,0xffff, thedisplay, thescreen);
  	printf("#Made xcyan\n");
  get_xft_colour(&xnavy, 0x0000,0x0000,0xAAAA,0xffff, thedisplay, thescreen);
  	printf("#Made xnavy\n");
  get_xft_colour(&xblue, 0x5555,0x5555,0xFFFF,0xffff, thedisplay, thescreen);
  	printf("#Made xblue\n");
  get_xft_colour(&xmagenta, 0xAAAA,0x0000,0xAAAA,0xffff, thedisplay, thescreen);
  	printf("#Made xmagenta\n");
  get_xft_colour(&xfuchsia, 0xFFFF,0x5555,0xFFFF,0xffff, thedisplay, thescreen);
  	printf("#Made xfuchsia\n");
  /* Remember to free these at the end! */
#endif
  
  setbgcol = black;
  setselcol = lime;
  setunselcol = green;
#ifdef HAVE_XFT
  setxbgcol = &xblack;
  setxselcol = &xlime;
  setxunselcol = &xgreen;
#endif
  winbordcol = black;
  winbgcol = black;
  
  /*	printf("#Time to make the window!\n"); /* This won't work anymore... * /
  mwprop = NewWindow(thedisplay, DefaultRootWindow(thedisplay), "CompuWrist Menu", "CompuWrist Menu", None, NULL, 0, NULL, winx, winy, winw, winh, winbord, winbordcol, winbgcol, white, 1);
  if (mwprop == NULL)
  {
    printf("Error Making Window!");
    ans = 0;
  }
  else
  {
    	printf("#Done that\n#Now.  Fonts...\n");
/ *    xfs = XLoadQueryFont(thedisplay,"*Keen*");
    if (xfs == NULL) printf("Cannot find font.\n");
    DoListFonts();* /
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
  }*/
  
  XFlush(thedisplay);
  return ans;
}

void close_x( int returnval)
{
/*  if (xfs != NULL) XFreeFont(thedisplay, xfs);  */
  if (mainwindow != 0) FreeWindow(mainwindow); /* Don't think I need this line either... */
  	printf("# Freed mw\n");
  FreeGeneratedArrays();
  	printf("# Freed Parsed Items/Menus/Fonts\n");
  DestroyItems();
  	printf("# Freed Items\n");
  DestroyWins();
  	printf("# Freed Windows\n");
#ifdef HAVE_XFT
  if (keenfont != NULL) XftFontClose(thedisplay, keenfont);
  if (sgafont != NULL) XftFontClose(thedisplay, sgafont);
  	printf("# Freed fonts\n");
  free_xft_colour(thedisplay, thescreen, &xblack);
  free_xft_colour(thedisplay, thescreen, &xwhite);
  free_xft_colour(thedisplay, thescreen, &xgreen);
  free_xft_colour(thedisplay, thescreen, &xlime);
  free_xft_colour(thedisplay, thescreen, &xgrey);
  free_xft_colour(thedisplay, thescreen, &xsilver);
  free_xft_colour(thedisplay, thescreen, &xred);
  free_xft_colour(thedisplay, thescreen, &xmaroon);
  free_xft_colour(thedisplay, thescreen, &xbrown);
  free_xft_colour(thedisplay, thescreen, &xyellow);
  free_xft_colour(thedisplay, thescreen, &xteal);
  free_xft_colour(thedisplay, thescreen, &xcyan);
  free_xft_colour(thedisplay, thescreen, &xnavy);
  free_xft_colour(thedisplay, thescreen, &xblue);
  free_xft_colour(thedisplay, thescreen, &xmagenta);
  free_xft_colour(thedisplay, thescreen, &xfuchsia);
  	printf("# Freed colours\n");
#endif
  clean_pwnlist();
  	printf("# Freed children list\n");
  	printf("# The Display = 0x%X\n",(unsigned long) thedisplay);
  XCloseDisplay(thedisplay);
  	printf("# Closed the display\n");
  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  	printf("# Reset Signals\n");
  exit(returnval);
  	printf("# Why are you still here?\n");
  
}

int dosubwin()
{
  struct WinPropNode *swprop;
  int winx = 0, winy = 0, ans = 1;
  Window rootret;
  unsigned int winw = 300, winh = 200, winbord = 5, depthret;
  unsigned long winbordcol = black, winbgcol = black;
  
#ifdef HAVE_XFT
  swprop = NewWindow(thedisplay, DefaultRootWindow(thedisplay), "CompuWrist Sub Menu", "CompuWrist Sub Menu", None, NULL, 0, NULL, winx, winy, winw, winh, winbord, winbordcol, winbgcol, white, &xblack, 1);
#else
  swprop = NewWindow(thedisplay, DefaultRootWindow(thedisplay), "CompuWrist Sub Menu", "CompuWrist Sub Menu", None, NULL, 0, NULL, winx, winy, winw, winh, winbord, winbordcol, winbgcol, white, 1);
#endif
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
    return 1;
  }
  
  struct ItemPropNode *ipn;
  long n, i;
  
  ipn = FindItemWIH(awin,wih);
  if (ipn == NULL) return 0;
  
  /* This shouldn't be done this way, but I've written myself into a corner... */
  for (n=0;n<iaasize;n++)
  {
    if (iactarray[n].itemptr == ipn) break;
  }
  if (n >= iaasize) return 0;
  /* If it's got this far, it has an iactarray entry (and thus the menu file has been parsed).  Hurray! */
  if (iactarray[n].type == 'N' || iactarray[n].type == 'n') return 1; /* If it's not meant to do anything, don't do anything! */
  if (iactarray[n].type == 'M' || iactarray[n].type == 'm')
  {
    /* It's a menu! */
    for (i=0;i<wasize;i++)
    {
      if (winarray[i].id == iactarray[n].action.menuid) break;
    }
    if (i>=wasize) return 0; /* No matches.  Boo! */
    ChangeWindowVisibility(winarray[i].win, 1); /* Make it visible */
    return 1;
  }
  if (iactarray[n].type == 'C' || iactarray[n].type == 'c')
  {
    /* It's a Command! */
    /* Should this run asynchronously?  If so, change the 1 to a 0. */
    if (iactarray[n].action.command != NULL) runcommand(iactarray[n].action.command, 1);
    return 1;
  }
  if (iactarray[n].type == 'B' || iactarray[n].type == 'b')
  {
    /* It means "Go Back" (Unless in Main Menu). */
    unsigned long mmid = FindMainMenu();
    if (mmid == 0) return 0;
    for (i = 0; i < wasize; i++)
    {
      if (winarray[i].id == mmid) break;
    }
    if (i>=wasize) return 0;
    if (winarray[i].win == awin) return 1; /* Don't hide the main menu! */
    ChangeWindowVisibility(awin, 0); /* Hide this menu */
    return 1;
  }
  if (iactarray[n].type == 'X' || iactarray[n].type == 'x')
  {
    /* Exit the Program! */
    	printf("# Closing...");
    close_x(0);
    return 1;
  }
  /* Unknown Action! */
  return 0;
}

int main(int argc, char *argv[])
{
  char mfname[1025] = "menu.txt";
  FILE *mf = NULL;
  
  	printf("#It Begins...\n");
  if ((argc==2 && (streq_(argv[1],"--help") || streq_(argv[1],"-?") || streq_(argv[1],"--version"))) || argc > 2)
  {
    printf("CompuWrist Menu System\nVersion %s\nUsage:\n\t%s [{ --help | -m=<menufile>}]\n", versionstr, argv[0]);
    printf("Where:\n--help\tShows this help message\n-m=<menufile>\t(Where <menufile> is the menu file to load) runs with the following menu file.\n");
    printf("Note that if you run %s without options, %s will look for a menu file called \"menu.txt\" in the current directory.\n",argv[0], argv[0]);
    return 0;
  }
  if (argc==2 && startsame_(argv[1],"-m="))
  {
    if (strlen(argv[1])>1027)
    {
      printf("Menu file path \"%s\" too long!\n",argv[1]+3);
      return 1;
    }
    strcpy(mfname,argv[1]+3);
  }
  
  mf = fopen(mfname,"r");
  if (mf == NULL)
  {
    printf("Error Opening File!\n");
    return 1;
  }
  
  init_x();
  
  if (signal(SIGINT, handle_sigint) == SIG_ERR)
  {
    perror(0);
    fprintf(stderr, "Warning: Failed to register SIGINT handler!\n");
  }
  	else printf("# Registered SIGINT handler.\n");
  if (signal(SIGTERM, handle_sigterm) == SIG_ERR)
  {
    perror(0);
    fprintf(stderr, "Warning: Failed to register SIGTERM handler!\n");
  }
  	else printf("# Registered SIGTERM handler.\n");
  
  if (ParseMenuFile(mf) == 0)
  {
    printf("Error Parsing Menu File!\n");
    close_x(1);
  }
  	printf("# Parsed Menu File\n");
  fclose(mf);
  
  unsigned long mmid = 0;
  long n;
  Window mfmain;
  mmid = FindMainMenu();
  if (mmid == 0 || (mmid+1)==0)
  {
    printf("No Main Menu Found!\n");
    close_x(1);
  }
  for (n=0;n<wasize;n++) if (winarray[n].id==mmid) break;
  if (n>=wasize)
  {
    printf("No Main Menu Found!\n");
    close_x(1);
  }
  mfmain = winarray[n].win;
  if (mfmain == 0)
  {
    printf("Error loading main window!\n");
    close_x(1);
  }
  ChangeWindowVisibility(mfmain,1);
  
  	printf("#Now into the message loop...\n");
  
  XEvent event;
  KeySym key;
  char text[256] = "";
  unsigned long lastitemclicked, lastwihclicked;
  Window lastwinclicked;
  struct WinPropNode *mwprop = FindWinProps(mainwindow);
  
  while (interrupted == 0)
  {
    /* Message Loop */
    XNextEvent(thedisplay, &event);
    
    if (event.type == Expose && event.xexpose.count == 0)
    {
      /* Redraw Window! */
      struct WinPropNode *awpn = FindWinProps(event.xany.window);
      
      /*if (event.xany.window == mainwindow)*/
      if (awpn != NULL)
      {
        Window rootret;
        int winx, winy;
        unsigned int winw, winh, winbord, depthret;
        
        XGetGeometry(awpn->disp, (Drawable) awpn->win, &rootret, &winx, &winy, &winw, &winh, &winbord, &depthret);
        if (winw >= 200)
        {
          ResizeItems(awpn->win, winw, 0);
        }
        XSetBackground(awpn->disp, awpn->gc, black);
        XClearWindow(awpn->disp, awpn->win);
        DrawItems(awpn->win);
      }
      XFlush(thedisplay);
    }
    
    if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1)
    {
      if (text[0] == 'q') close_x(0); /* For DEBUGGING ONLY */
      else printf("# The %c (%d) key was pressed\n", text[0], text[0]);
      
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
      else if (text[0] == 27)
      {
        /* Back out */
        unsigned long mmuid = FindMainMenu();
        long wai;
        if (mmuid != 0)
        {
          for (wai = 0; wai < wasize; wai++)
          {
            if (winarray[wai].id == mmuid) break;
          }
          if (wai<wasize && winarray[wai].win != event.xkey.window)
          {
            /* Don't hide the main menu! */
            ChangeWindowVisibility(event.xkey.window, 0); /* Hide this menu */
          }
        }
      }
      
    }
    /*else if (event.type == KeyPress && event.xkey.window == mainwindow)
    {
      unsigned long tiid;
      printf("# Key press: %x\n",event.xkey.keycode);
      if (event.xkey.keycode == 0x6f || event.xkey.keycode == 0x71)
      {
        / * UP or LEFT * /
        tiid = mwprop->selitem;
        tiid = PrevItemInWindow(mainwindow, tiid);
        if (tiid != 0)
        {
          / * Select item tiid * /
          mwprop->selitem = tiid;
          DrawItems(mainwindow);
        }
      }
      else if (event.xkey.keycode == 0x72 || event.xkey.keycode == 0x74)
      {
        / * RIGHT or DOWN * /
        tiid = mwprop->selitem;
        tiid = NextItemInWindow(mainwindow, tiid);
        if (tiid != 0)
        {
          / * Select item tiid * /
          mwprop->selitem = tiid;
          DrawItems(mainwindow);
        }
      }
    }
    */
    else if (event.type == KeyPress ) /*&& event.xkey.window == subwindow)*/
    {
      unsigned long tiid;
      struct WinPropNode *awprop;
      awprop = FindWinProps(event.xkey.window);
      if (awprop != NULL)
      {
        printf("# Key press: %x\n",event.xkey.keycode);
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
      printf("# Clicked item %lu.\n", lastitemclicked);
      lastwinclicked = GetItemWin(lastitemclicked);
      lastwihclicked = GetItemWIH(lastitemclicked);
      printf("# Clicked item %X:%lu.\n",lastwinclicked, lastwihclicked);
      
      ItemEvent(event.xbutton.window, lastwihclicked);
    }
    
    if (event.type == MotionNotify)
    {
      MouseOver(event.xmotion.window, event.xmotion.x, event.xmotion.y, event.xmotion.state);
      
    }
    
    if (event.type == DestroyNotify || event.type == ClientMessage)
    {
      /* Closed window ? */
      /*if (event.xany.window == mainwindow) close_x(0);
      if (event.xany.window == subwindow) undosubwin();*/
      if (event.xany.window == mfmain) close_x(0);
      else ChangeWindowVisibility(event.xany.window, 0);
      /* Hide other windows on close! */
    }
    
    int pidret;
    
    if ((pidret = CleanPIDs()) != 1)
    {
      printf("A process returned %d.\n",pidret);
    }
    
  }
  close_x(interrupted);
  return 1;
}
