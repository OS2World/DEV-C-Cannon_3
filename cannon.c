//----------------------------------------------------------------------------------------------------//
/*
   PHYSICS FOR GAME DEVELOPERS

   CHAPTER 4 EXAMPLE PROGRAM

   NAME:    Cannon 2
   PURPOSE: To demonstrate 3D particle kinetics
   BY:         David Bourg
   DATE:    04/28/00
   COPYRIGHT:  Copyright 2000 by David Bourg
*/
//----------------------------------------------------------------------------------------------------//
#ifdef WINDOZE
// Windows Header Files:
#include <windows.h>
#include <windef.h>
#include <commctrl.h>
#include <commdlg.h>
#include <wingdi.h>
#else
#define  INCL_BASE
#define  INCL_GPI
#define INCL_WINSWITCHLIST
#define INCL_WINPROGRAMLIST
#define INCL_WINWINDOWMGR
#include <os2.h>
#endif

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <math.h>

// Local Header Files:
#include "resource.h"
#include "cannon.h"

// Defines:
#define     MYSIDEVIEW  2000

void  InitializeVariables(void);
int      DoSimulation(void);

#ifdef WINDOZE
// Forward declarations for window related functions
LRESULT CALLBACK DemoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DefaultWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// Window related global variables
HINSTANCE   hinst;
HWND     hMainWindow;

// Forward declarations for non-window related functions
void  DrawSideView(HDC hdc, RECT *r);
void  DrawLine(HDC hdc, int h1, int v1, int h2, int v2, int thk, COLORREF clr);
void  DrawRectangle(HDC hdc, RECT *r, int thk, COLORREF clr);
void  DrawString(HDC hdc, int x, int y, LPCSTR lpszString, int size, int ptsz);

#else

static MRESULT EXPENTRY WinMain( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
static MRESULT EXPENTRY DemoDlgProc( HWND hDlg, ULONG message, MPARAM wParam, MPARAM lParam );
static void DisplayMessage( HAB, ULONG );
static void InitTitle( HAB hab, HWND hwnd, char *szTitle );
static MRESULT EXPENTRY DefaultWndProc(HWND hwnd, ULONG message, MPARAM wParam, MPARAM lParam);
static char   szAppName[ 255 ];    /* application name string        */
#define MSG_BOX_ID     256                  /* error message box id           */
void  DrawRectangle(HPS hdc, RECTL *r, int thk, RGB clr);
void  DrawSideView(HPS hdc, RECTL *r);
void  DrawLine(HPS hdc, int h1, int v1, int h2, int v2, int thk, RGB clr);
void DrawString(HPS hdc, int x1, int y1, PSZ lpszString, int size, int ptsz);

HAB   hab = NULLHANDLE;                  /* PM anchor block handle         */
HMQ   hmq = NULLHANDLE;                  /* message queue handle           */
HWND  hwndFrame = NULLHANDLE;            /* frame window handle            */
static HWND   hwndClient = NULLHANDLE;      /* client window handle           */

#endif

int main( void )
{
   ULONG flCreate = 0UL;                    /* window creation control flags  */
   QMSG  qmsg;                              /* message from message queue     */
   int   rc = 1;

   do
   {
      /* Initialize PM and create a message queue of default size.            */

      if ( ( hab = WinInitialize( 0UL ) ) == NULLHANDLE )
         break;

      if ( ( hmq = WinCreateMsgQueue( hab, 0UL ) ) == NULLHANDLE )
         break;

      /* Register client window class.                                        */

      if ( !WinRegisterClass( hab,          /* PM anchor block handle         */
                              WINDOW_CLASS, /* window class name              */
                              WinMain, /* address of window procedure    */
                              CS_SIZEREDRAW,/* size changes cause redrawing   */
                              0UL ) )       /* window data                    */
      {
         DisplayMessage( hab, IDS_NOREGISTER );
         break;
      }

      /* Create the standard windows but do not add the application to the    */
      /* task manager list.                                                   */

      flCreate = FCF_STANDARD & ~FCF_TASKLIST & ~FCF_ICON;

      hwndFrame =
         WinCreateStdWindow( HWND_DESKTOP,  /* make desktop window the parent */
                             0L,    /* frame window class style       */
                             &flCreate,     /* frame control flags            */
                             WINDOW_CLASS,  /* client window class name       */
                             "",            /* title bar text                 */
                             CS_SIZEREDRAW, /* client window class style      */
                             0UL,           /* resource file handle - in EXE  */
                             ID_WINDOW,     /* resources identifier           */
                             &hwndClient ); /* client window handle           */
      if ( hwndFrame == NULLHANDLE )
      {
         DisplayMessage( hab, IDS_NOSTDWINDOWS );
         break;
      }

      /* Initialize the window title and task switch list.                    */

      InitTitle( hab, hwndFrame, szAppName );

      /* Create the thread that will draw the lines.                          */
      /* NOTE: _beginthread MUST be used if the thread contains CRT calls     */


      /* While the WM_QUIT message is not received, dispatch the message.     */
      /* When the WM_QUIT message is received, WinGetMsg will return FALSE.   */

      while( WinGetMsg( hab, &qmsg, 0UL, 0UL, 0UL ) )
         WinDispatchMsg( hab,               /* PM anchor block handle         */
                         &qmsg );           /* pointer to message             */
      rc = 0;

   }  while ( FALSE );

   /* Destroy the standard windows if they were created.                      */

   if ( hwndFrame != NULLHANDLE )
      WinDestroyWindow( hwndFrame );        /* frame window handle            */

   /* Destroy the message queue and release the anchor block.                 */

   if ( hmq != NULLHANDLE )
      WinDestroyMsgQueue( hmq );

   if ( hab != NULLHANDLE )
      WinTerminate( hab );

   return rc;
}

/*+--------------------------------------------------------------------------+*/
/*| DisplayMessage - display an error message in a message box.              |*/
/*+--------------------------------------------------------------------------+*/

static void DisplayMessage( HAB hab, ULONG ulStringNum )
{
   char szTemp[ 255 ];

   WinLoadString( hab, 0UL, ulStringNum, 255, szTemp );

   WinAlarm( HWND_DESKTOP,                  /* desktop window handle          */
             WA_ERROR );                    /* type of alarm                  */

   WinMessageBox( HWND_DESKTOP,             /* parent window handle           */
                  HWND_DESKTOP,             /* owner window handle            */
                  szTemp,                   /* pointer to message text        */
                  szAppName,                /* pointer to title text          */
                  MSG_BOX_ID,               /* message box identifier         */
                  MB_OK | MB_ERROR |        /* message box style              */
                  MB_SYSTEMMODAL );

   return;
}

/*+--------------------------------------------------------------------------+*/
/*| InitTitle - Initializes window title and task switch list.               |*/
/*+--------------------------------------------------------------------------+*/

static void InitTitle( HAB hab, HWND hwnd, char *szTitle )
{
   SWCNTRL tSwcntrl;                        /* task switch structure          */

   /* Load the application name from the resources in the EXE.  Set the title */
   /* bar text to this name.  Then add this application to the task manager   */
   /* list with the loaded application name.                                  */

   WinLoadString( hab, 0UL, IDS_APPNAME, MAXNAMEL, szTitle );
   WinSetWindowText( hwnd, szTitle );

   tSwcntrl.hwnd = hwnd;
   tSwcntrl.hwndIcon = NULLHANDLE;
   tSwcntrl.hprog = NULLHANDLE;
   tSwcntrl.idProcess = 0;
   tSwcntrl.idSession = 0;
   tSwcntrl.uchVisibility = SWL_VISIBLE;
   tSwcntrl.fbJump = SWL_JUMPABLE;
   strcpy( tSwcntrl.szSwtitle, szTitle );
   tSwcntrl.bProgType = PROG_PM;
   WinAddSwitchEntry( &tSwcntrl );

   return;
}

//----------------------------------------------------------------------------------------------------//
// This is the applications "main" function. Note that I'm not using a message loop here
// since there is no main window. All I do is display a dialog box immediately upon startup
// and let the dialog handler take care of the messages.
//----------------------------------------------------------------------------------------------------//
static MRESULT EXPENTRY WinMain( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
//----------------------------------------------------------------------------------------------------//
{
   HPS   hps = NULLHANDLE;                  /* presentation space handle      */


   switch( msg )
   {
      case WM_CREATE:
         /* The client window has been created but is not visible yet.        */
         /* Initialize the window here.                                       */
         WinDlgBox(HWND_DESKTOP, hwnd, DemoDlgProc, 0UL, IDD_DEMODIALOG, NULL);
         WinPostMsg( hwnd, WM_QUIT, NULL, NULL );
         break;

      case WM_CLOSE:
         /* Tell the main procedure to terminate the message loop.            */

         WinPostMsg( hwnd, WM_QUIT, NULL, NULL );
         break;

      case WM_PAINT:
      {
         RECTL rClient;
         RECTL rcl;

         hps = WinBeginPaint( hwnd, NULLHANDLE, &rcl );

         WinFillRect(hps, &rcl, CLR_WHITE);

         WinEndPaint( hps );
         break;
      }

      default:
         /* For all other messages, let the default window procedure          */
         /* process them.                                                     */

         return ( WinDefWindowProc( hwnd, msg, mp1, mp2 ) );
   }
   return NULL;
}


//----------------------------------------------------------------------------------------------------//
// This is the message handler function for the main dialog box.  It is responsible for handling
// any interaction with it's controls and for updating the trajectory views that we'll be displaying.
//----------------------------------------------------------------------------------------------------//
static MRESULT EXPENTRY DemoDlgProc( HWND hDlg, ULONG message, MPARAM wParam, MPARAM lParam )
//----------------------------------------------------------------------------------------------------//
{
   static   HWND  hTopView;
   static   HWND  hSideView;
/* WNDCLASSEX     wclass;  */
   char        str[16];
   int            status;
   RECTL        r;
   APIRET   ret;
/* HDC            hdc;    */

   switch (message) {
      // Initialize the dialog box here:
      case WM_INITDLG:
         // setup a child window class so we can create a couple
         // of windows to draw the two trajectory views on
         WinRegisterClass( hab, "ViewClass", DefaultWndProc, CS_SIZEREDRAW, 16UL );
         // Now go ahead and create a child window for the side view
         hSideView = WinCreateWindow(hDlg, "ViewClass","Status", WS_VISIBLE,
                     150, 10+10, 500, 200, hDlg, HWND_TOP,150, 0, 0 );
         ret = WinSetWindowULong(hSideView, QWL_USER, MYSIDEVIEW);

         // Now initialize all of the edit controls on the dialog box with our
         // default values for each variable.

         // Set default values for all variables
         InitializeVariables();

         // Now convert each variable value to a string and
         // set the appropriate edit control
         sprintf( str, "%f", V1.i );
         WinSetDlgItemText(hDlg, IDC_V0, str);

         sprintf( str, "%f", spin );
         WinSetDlgItemText(hDlg, IDC_SPIN, str);

         sprintf( str, "%f", m );
         WinSetDlgItemText(hDlg, IDC_M, str);

         sprintf( str, "%f", radius );
         WinSetDlgItemText(hDlg, IDC_RADIUS, str);
         break;

      // handle the dialog controls here:
      case WM_COMMAND:
         switch( SHORT1FROMMP( wParam) )
         {
            case IDC_REFRESH:
            {
               HPS hdc;

               // update the variables with
               // the values shown in the edit controls
               WinQueryDlgItemText(hDlg, IDC_V0, 15, str);
               V1.i = atof(str);
               V1.j = 0;

               WinQueryDlgItemText(hDlg, IDC_SPIN, 15, str);
               spin = atof(str);

               WinQueryDlgItemText(hDlg, IDC_M, 15, str);
               m = atof(str);

               WinQueryDlgItemText(hDlg, IDC_RADIUS, 15, str);
               radius = atof(str);

               omega = (2*PI*spin)/60;


               // re-initialize the time and position of the shell
               time = 0;
               s1.i = 0;
               s1.j = 1.8288;

               hdc = WinGetPS(hSideView);
               WinQueryWindowRect(hSideView, &r);
               DrawSideView(hdc, &r);
               WinReleasePS(hdc);

               break;
            }

            case IDC_FIRE:
            {
               HPS hdc;

               // update the variables with
               // the values shown in the edit controls
               WinQueryDlgItemText(hDlg, IDC_V0, 15, str);
               V1.i = atof(str);
               V1.j = 0;

               WinQueryDlgItemText(hDlg, IDC_SPIN, 15, str);
               spin = atof(str);

               WinQueryDlgItemText(hDlg, IDC_M, 15, str);
               m = atof(str);

               WinQueryDlgItemText(hDlg, IDC_RADIUS, 15, str);
               radius = atof(str);

               omega = (2*PI*spin)/60;
               s1.i = 0;
               s1.j = 1.8288;

               // initialize the time and status variables
               status = 0;
               time = 0;
               // start stepping through time for the sim.
               // until the target is hit, the shell hits
               // the ground, or the sim. times out.
               while(status == 0)
               {
                  // do the next time step
                  status = DoSimulation();

                  hdc = WinGetPS(hSideView);
                  WinQueryWindowRect(hSideView, &r);
                  DrawSideView(hdc, &r);
                  WinReleasePS(hdc);

               }

               // Report results
               if (status == 3)
                  WinMessageBox(HWND_DESKTOP,hDlg,"Timed Out","Error",
                     (HMODULE)NULL,MB_OK | MB_INFORMATION );

               break;
            }

            case ID_CLOSE:
               // clean up the child windows and close the dialog box
               WinDestroyWindow( hSideView );
               WinDismissDlg( hDlg, FALSE );
               break;

            case DID_CANCEL:
               // clean up the child windows and close the dialog box
               WinDestroyWindow( hSideView );
               WinDismissDlg( hDlg, FALSE );
               break;
         }
         break;

      default:
         return WinDefDlgProc( hDlg, message, wParam, lParam );
   }
    return ( TRUE );
}


//----------------------------------------------------------------------------------------------------//
// This is the default message processing function for the two child windows that are created
// so to draw the trajectory views.
//----------------------------------------------------------------------------------------------------//
static MRESULT EXPENTRY DefaultWndProc(HWND hwnd, ULONG message, MPARAM wParam, MPARAM lParam)
//----------------------------------------------------------------------------------------------------//
{
/*   PAINTSTRUCT       ps;
   HDC               hdc; */
   RECTL           r;
   LONG           tag;
   HPS hps;

   switch (message) {
      case WM_PAINT:
        /* hdc = BeginPaint(hwnd, &ps);  */
         hps = WinBeginPaint( hwnd, NULLHANDLE, &r );

         WinFillRect(hps, &r, CLR_BLACK);
         // Get the tag that we set when we created the child windows
         // where we plan to draw the shell trajectory
         tag = WinQueryWindowULong(hwnd, QWL_USER);

         if(tag == MYSIDEVIEW)
         {
            WinQueryWindowRect( hwnd, &r );
            DrawSideView(hps, &r);
         }

         WinEndPaint(hps);
         break;
   }
    return WinDefWindowProc(hwnd, message, wParam, lParam);
}

//----------------------------------------------------------------------------------------------------//
// Initialize the global variables required for the simulation.
//----------------------------------------------------------------------------------------------------//
void  InitializeVariables(void)
//----------------------------------------------------------------------------------------------------//
{
   V1.i  = 40.0;     // m/s
   V1.j  = 0.0;      // m/s

   m     = 0.136; // kgs
   spin  = 0;     // rpm
   radius   = 0.0635;   // m

   time  =  0;    // seconds
   tInc  =  0.01; // seconds
   g     =  9.8;  // m/(s*s)

   s1.i  =  0.0;
   s1.j  =  1.8288;

}

//----------------------------------------------------------------------------------------------------//
// This function draws the side (profile) view of the simulation.  It shows the cannon,
// the target and the shell trajectory.  Scale is 1 pixel per meter.
//----------------------------------------------------------------------------------------------------//
void  DrawSideView(HPS hdc, RECTL *r)
//----------------------------------------------------------------------------------------------------//
{
   int               w = (r->xRight - r->xLeft);  // the window width
   int               h = (r->yTop - r->yBottom);  // the window height
   RGB       red = {0,0,255};
   RGB       black = {0,0,0};
   RGB       blue = {255,0, 0};
   RGB       white = {255, 255, 255};
   RGB       green = {0, 255, 0};
   int               x,y;

   // NOTE: the h factor that you see in the following
   // calculations is an adjustment to place the origin
   // at the bottom of the window.  The window origin is
   // the upper left corner by default. Note also that
   // since the vertical coordinate in the default window
   // coordinate system is positive down, we need to take
   // the negative of our calculated y-values in order to
   // plot the trajectory as though y is positive up.

   if(time == 0)
      DrawRectangle(hdc, r, 1, black);

   // Draw the shell in blue
   // Draw the shell in blue (only draw the shell if time is greater than zero, i.e.,
   // only after it leaves the barrel in our simulation
   if(time>0)
   {
      x = (int) (s2.i * 20);
      y = (int) ((-s2.j * 20) + h);
      y = (int) (s2.j * 20);
      DrawLine(hdc, x, y, x, y, 2, green);
   }

   // Draw label for this view
   DrawString(hdc, 5, h - 22, "Side View", 9, 14);

}

//----------------------------------------------------------------------------------------------------//
// This function simply draws a solid line to the given device context, given the line
// start and end point, its thickness and its color.
//----------------------------------------------------------------------------------------------------//
void DrawLine(HPS hdc, int h1, int v1, int h2, int v2, int thk, RGB clr)
//----------------------------------------------------------------------------------------------------//
{
   POINTL ptl;
   LINEBUNDLE lb;
   ULONG *ulp;
   ULONG lp;
   APIRET rc;
   ULONG *ulClr;
   ULONG lClr;

   GpiCreateLogColorTable ( hdc, LCOL_RESET, LCOLF_RGB, 0, 0, 0 ) ;
   ulClr = (ULONG *)&clr;
   lClr = *ulClr & 0xFFFFFF;
   GpiSetColor(hdc, lClr);
   GpiSetLineWidth(hdc, thk);
   GpiSetLineType(hdc, LINETYPE_SOLID);

   ptl.x = h1;
   ptl.y = v1;
   GpiMove(hdc, &ptl);

   ptl.x = h2;
   ptl.y = v2;
   GpiLine(hdc, &ptl);

}


//----------------------------------------------------------------------------------------------------//
// This function simply draws a filled rectangle to the given device context, given the
// rectangle dimensions, its border thickness and its border color (the rectangle is filled
// in black).
//----------------------------------------------------------------------------------------------------//
void DrawRectangle(HPS hps, RECTL *r, int thk, RGB clr)
{
   POINTL pt;
   ULONG *ulClr;
   ULONG lClr;
   RGB FColor = clr;
   RGB BColor = {0, 0, 0};

   pt.x = r->xLeft;
   pt.y = r->yBottom;
   GpiMove(hps, &pt);
   GpiSetColor(hps, CLR_BLACK);
   pt.x = r->xRight;
   pt.y = r->yTop;
   GpiBox(hps, DRO_OUTLINEFILL, &pt , 0 , 0);

   pt.x = r->xLeft;
   pt.y = r->yBottom;
   GpiMove(hps, &pt);
   GpiCreateLogColorTable ( hps, LCOL_RESET, LCOLF_RGB, 0, 0, 0 ) ;
   ulClr = (ULONG *)&clr;
   lClr = *ulClr & 0xFFFFFF;
   GpiSetColor(hps, lClr);
   pt.x = r->xRight;
   pt.y = r->yTop;
   GpiBox(hps, DRO_OUTLINE, &pt, 0, 0);

}


//----------------------------------------------------------------------------------------------------//
// This function simply draws text to the given device context, given the text string
// and the x,y coordinates of its lower left corner, the number of characters in the string,
// and the desired point size.
//----------------------------------------------------------------------------------------------------//
void DrawString(HPS hdc, int x1, int y1, PSZ lpszString, int size, int ptsz)
{

#define NONE 0
#define RAISED 1
#define INDENTED 2
#define GRAYEDOUT 3
#define USE_OVERPAINT 1
#define USE_DEFAULT     0
   RECTL rClient;
   SHORT sShift;
   SHORT x;
   SHORT sFeature = RAISED;
   SHORT sMix = USE_OVERPAINT;
   ULONG ulAttrs = DT_LEFT | DT_BOTTOM | DT_TEXTATTRS;

   rClient.xLeft = x1;
   rClient.yBottom = y1;
   rClient.xRight = rClient.xLeft + 100;
   rClient.yTop = rClient.yBottom + 50;

   GpiCreateLogColorTable ( hdc, LCOL_RESET, LCOLF_INDRGB, 0, 0, 0 ) ;
   if (sFeature != NONE)
   {

      switch (sFeature)
      {
         case RAISED:
            GpiSetColor(hdc,CLR_WHITE);
            sShift = 1;
            break;
         case INDENTED:
            GpiSetColor(hdc,CLR_DARKGRAY);
            sShift = 1;
            break;
         case GRAYEDOUT:
            GpiSetColor(hdc,CLR_PALEGRAY);
            sShift = 1;
            break;
      }
      GpiSetBackColor(hdc,CLR_BLACK);
      if (sMix == USE_OVERPAINT)
         GpiSetBackMix(hdc,BM_OVERPAINT);

   /*   rClient.yTop = 50; */
      rClient.xLeft -= sShift;
      rClient.yBottom += sShift;
      WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );

      if (sMix == USE_OVERPAINT)
         GpiSetBackMix(hdc,BM_DEFAULT);

      for (x = 0;x > sShift ; x++)
      {

         rClient.xLeft = x1;
         rClient.yBottom = y1;
         rClient.xLeft -= x;
         WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );

         rClient.yBottom += x;
         WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );

         rClient.xLeft += x;
         WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );
      }

      switch (sFeature)
      {
         case RAISED:
            GpiSetColor(hdc,CLR_DARKGRAY);
            break;
         case INDENTED:
            GpiSetColor(hdc,CLR_WHITE);
            break;
         case GRAYEDOUT:
            GpiSetColor(hdc,CLR_BLACK);
            break;
      }



      for (x = 1; x <= sShift; x++)
      {

         rClient.xLeft = x1;
         rClient.yBottom = y1;
         rClient.yBottom -= x;
         WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );

      }

      for (x = 1; x <= sShift; x++)
      {

         rClient.xLeft = x1;
         rClient.yBottom = y1;
         rClient.yBottom -= sShift;
         rClient.xLeft += x;
         WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );

      }

      for (x = 1; x <= sShift; x++)
      {

         rClient.xLeft = x1;
         rClient.yBottom = y1;
         rClient.yBottom -= sShift;
         rClient.yBottom += x;
         rClient.xLeft += sShift;
         WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );

      }


   }

   rClient.xLeft = x1;
   rClient.yBottom = y1;
   switch (sFeature)
   {
      case RAISED:
      case NONE:
         GpiSetColor(hdc,CLR_WHITE);
         break;
      case INDENTED:
         GpiSetColor(hdc,CLR_BLACK);
         break;
      case GRAYEDOUT:
         GpiSetColor(hdc,CLR_PALEGRAY);
         break;
   }
   WinDrawText(hdc,strlen(lpszString),lpszString,&rClient,0L,0L,ulAttrs );
}

//----------------------------------------------------------------------------------------------------//
// This function steps the simulation ahead in time. This is where the kinematic properties
// are calculated.  The function will return 1 when the target is hit, and 2 when the shell
// hits the ground (x-z plane) before hitting the target, otherwise the function returns 0.
//----------------------------------------------------------------------------------------------------//
int   DoSimulation(void)
//----------------------------------------------------------------------------------------------------//
{
   double   C = PI * RHO * RHO * radius * radius * radius * omega;
   double   t;

   // step to the next time in the simulation
   time+=tInc;
   t = time;

   // calc V2:
   V2.i = 1.0f/(1.0f-(t/m)*(t/m)*C*C) * (V1.i + C * V1.j * (t/m) - C * g * (t*t)/m);
   V2.j = V1.j + (t/m)*C*V2.i - g*t;

   // calc S2:
   s2.i = s1.i + V1.i * t + (1.0f/2.0f) * (C/m * V2.j) * (t*t);
   s2.j = s1.j + V1.j * t + (1.0f/2.0f) * ( ((C*V2.i) - m*g)/m ) * (t*t);


   // Check for collision with ground (x-z plane)
   if(s2.j <= 0)
      return 2;

   // Cutoff the simulation if it's taking too long
   // This is so the program does not get stuck in the while loop
   if(time>60)
      return 3;

   return 0;
}


#ifdef WINDOZE
//----------------------------------------------------------------------------------------------------//
// This is the applications "main" function. Note that I'm not using a message loop here
// since there is no main window. All I do is display a dialog box immediately upon startup
// and let the dialog handler take care of the messages.
//----------------------------------------------------------------------------------------------------//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//----------------------------------------------------------------------------------------------------//
{
   DLGPROC     dlgprc;
   int         retval = 0;

    // Display the dialog box and check to make sure it was created:
    dlgprc = (DLGPROC) MakeProcInstance(DemoDlgProc, hInstance);
    retval = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DEMODIALOG), NULL, dlgprc);
   if(retval == -1)
   {
      MessageBox(NULL, "Error", "Can't create dialog box.", MB_OK);
      return FALSE;
   }
   FreeProcInstance((FARPROC) dlgprc);


   // Return false since we never got to a main message loop
   return (FALSE);
}

//----------------------------------------------------------------------------------------------------//
// This is the message handler function for the main dialog box.  It is responsible for handling
// any interaction with it's controls and for updating the trajectory views that we'll be displaying.
//----------------------------------------------------------------------------------------------------//
LRESULT CALLBACK DemoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//----------------------------------------------------------------------------------------------------//
{
   static   HWND  hTopView;
   static   HWND  hSideView;
   WNDCLASSEX     wclass;
   char        str[16];
   int            status;
   RECT        r;
   HDC            hdc;

   switch (message) {
      // Initialize the dialog box here:
      case WM_INITDIALOG:
         // setup a child window class so we can create a couple
         // of windows to draw the two trajectory views on
         wclass.cbSize           =  sizeof(wclass);
         wclass.style            =  CS_HREDRAW | CS_VREDRAW;
         wclass.lpfnWndProc         =  DefaultWndProc;
         wclass.cbClsExtra       =  0;
         wclass.cbWndExtra       =  0;
         wclass.hInstance        =  hinst;
         wclass.hIcon            =  NULL;
         wclass.hCursor          =   NULL;
         wclass.hbrBackground    =  (HBRUSH) CreateSolidBrush(RGB(0,0,0));
         wclass.lpszMenuName        =  NULL;
         wclass.lpszClassName    =  "ViewClass";
         wclass.hIconSm          =  NULL;

         RegisterClassEx(&wclass);

         // Now go ahead and create a child window for the side view
         hSideView = CreateWindowEx(   0L,
                           "ViewClass",
                           NULL,
                           WS_CHILD | WS_VISIBLE | WS_BORDER,
                           150,
                           10+10,
                           500,
                           200,
                           hDlg,
                           NULL,
                           hinst,
                           NULL);

         // Tag the window with our identifier, MYSIDEVIEW, so that we
         // can distinguish it from the top view window when it's time
         // to draw in the window.
         SetWindowLong(hSideView, GWL_USERDATA, MYSIDEVIEW);

         // Show the window
         ShowWindow(hSideView, SW_SHOW);
         UpdateWindow(hSideView);

         // Now initialize all of the edit controls on the dialog box with our
         // default values for each variable.

         // Set default values for all variables
         InitializeVariables();

         // Now convert each variable value to a string and
         // set the appropriate edit control
         sprintf( str, "%f", V1.i );
         SetDlgItemText(hDlg, IDC_V0, str);

         sprintf( str, "%f", spin );
         SetDlgItemText(hDlg, IDC_SPIN, str);

         sprintf( str, "%f", m );
         SetDlgItemText(hDlg, IDC_M, str);

         sprintf( str, "%f", radius );
         SetDlgItemText(hDlg, IDC_RADIUS, str);
         break;

      // handle the dialog controls here:
      case WM_COMMAND:
         switch( LOWORD( wParam) )
         {
            case IDC_REFRESH:
               // update the variables with
               // the values shown in the edit controls
               GetDlgItemText(hDlg, IDC_V0, str, 15);
               V1.i = atof(str);
               V1.j = 0;

               GetDlgItemText(hDlg, IDC_SPIN, str, 15);
               spin = atof(str);

               GetDlgItemText(hDlg, IDC_M, str, 15);
               m = atof(str);

               GetDlgItemText(hDlg, IDC_RADIUS, str, 15);
               radius = atof(str);

               omega = (2*PI*spin)/60;


               // re-initialize the time and position of the shell
               time = 0;
               s1.i = 0;
               s1.j = 1.8288;

               hdc = GetDC(hSideView);
               GetClientRect(hSideView, &r);
               DrawSideView(hdc, &r);
               ReleaseDC(hSideView, hdc);
               break;

            case IDC_FIRE:
               // update the variables with
               // the values shown in the edit controls
               GetDlgItemText(hDlg, IDC_V0, str, 15);
               V1.i = atof(str);
               V1.j = 0;

               GetDlgItemText(hDlg, IDC_SPIN, str, 15);
               spin = atof(str);

               GetDlgItemText(hDlg, IDC_M, str, 15);
               m = atof(str);

               GetDlgItemText(hDlg, IDC_RADIUS, str, 15);
               radius = atof(str);

               omega = (2*PI*spin)/60;
               s1.i = 0;
               s1.j = 1.8288;

               // initialize the time and status variables
               status = 0;
               time = 0;
               // start stepping through time for the sim.
               // until the target is hit, the shell hits
               // the ground, or the sim. times out.
               while(status == 0)
               {
                  // do the next time step
                  status = DoSimulation();

                  hdc = GetDC(hSideView);
                  GetClientRect(hSideView, &r);
                  DrawSideView(hdc, &r);
                  ReleaseDC(hSideView, hdc);
               }

               // Report results
               if (status == 3)
                  MessageBox(NULL, "Timed Out", "Error", MB_OK);

               break;

            case ID_CLOSE:
               // clean up the child windows and close the dialog box
               DestroyWindow(hSideView);
               EndDialog(hDlg, 1);
               break;

            case IDCANCEL:
               // clean up the child windows and close the dialog box
               DestroyWindow(hSideView);
               EndDialog(hDlg, 0);
               break;
         }
         break;

      default:
         return( FALSE );
   }
    return ( TRUE );
}

//----------------------------------------------------------------------------------------------------//
// This is the default message processing function for the two child windows that are created
// so to draw the trajectory views.
//----------------------------------------------------------------------------------------------------//
LRESULT CALLBACK DefaultWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//----------------------------------------------------------------------------------------------------//
{
   PAINTSTRUCT       ps;
   HDC               hdc;
   RECT           r;
   LONG           tag;

   switch (message) {
      case WM_PAINT:
         hdc = BeginPaint(hwnd, &ps);

         // Get the tag that we set when we created the child windows
         // where we plan to draw the shell trajectory
         tag = GetWindowLong(hwnd, GWL_USERDATA);

         if(tag == MYSIDEVIEW)  // We need to draw the side view
         {
            GetClientRect(hwnd, &r);
            DrawSideView(hdc, &r);
         }

         EndPaint(hwnd, &ps);
         break;
   }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

//----------------------------------------------------------------------------------------------------//
// Initialize the global variables required for the simulation.
//----------------------------------------------------------------------------------------------------//
void  InitializeVariables(void)
//----------------------------------------------------------------------------------------------------//
{
   V1.i  = 40.0;     // m/s
   V1.j  = 0.0;      // m/s

   m     = 0.136; // kgs
   spin  = 0;     // rpm
   radius   = 0.0635;   // m

   time  =  0;    // seconds
   tInc  =  0.01; // seconds
   g     =  9.8;  // m/(s*s)

   s1.i  =  0.0;
   s1.j  =  1.8288;

}

//----------------------------------------------------------------------------------------------------//
// This function draws the side (profile) view of the simulation.  It shows the cannon,
// the target and the shell trajectory.  Scale is 1 pixel per meter.
//----------------------------------------------------------------------------------------------------//
void  DrawSideView(HDC hdc, RECT *r)
//----------------------------------------------------------------------------------------------------//
{
   int               w = (r->right - r->left);  // the window width
   int               h = (r->bottom - r->top);  // the window height
   COLORREF       red = RGB(255,0,0);
   COLORREF       black = RGB(0,0,0);
   COLORREF       blue = RGB(0,0,255);
   COLORREF       white = RGB(255,255,255);
   COLORREF       green = RGB(0,255,0);
   int               x,y;

   // NOTE: the h factor that you see in the following
   // calculations is an adjustment to place the origin
   // at the bottom of the window.  The window origin is
   // the upper left corner by default. Note also that
   // since the vertical coordinate in the default window
   // coordinate system is positive down, we need to take
   // the negative of our calculated y-values in order to
   // plot the trajectory as though y is positive up.

   if(time == 0)
      DrawRectangle(hdc, r, 1, black);

   // Draw the shell in blue
   // Draw the shell in blue (only draw the shell if time is greater than zero, i.e.,
   // only after it leaves the barrel in our simulation
   if(time>0)
   {
      x = (int) (s2.i * 20);
      y = (int) ((-s2.j * 20) + h);
      DrawLine(hdc, x, y, x, y, 2, green);
   }

   // Draw label for this view
   DrawString(hdc, 5, 20, "Side View", 9, 14);

}

//----------------------------------------------------------------------------------------------------//
// This function simply draws a solid line to the given device context, given the line
// start and end point, its thickness and its color.
//----------------------------------------------------------------------------------------------------//
void DrawLine(HDC hdc, int h1, int v1, int h2, int v2, int thk, COLORREF clr)
//----------------------------------------------------------------------------------------------------//
{
   HBRUSH      CurrentBrush;
   HBRUSH      OldBrush;
   HPEN     CurrentPen;
   HPEN     OldPen;
   COLORREF FColor = clr;
   COLORREF BColor = RGB(0, 0, 0);

   CurrentBrush = CreateSolidBrush(FColor);
   OldBrush = (HBRUSH) SelectObject( hdc, CurrentBrush);
   CurrentPen = CreatePen(PS_SOLID, thk, FColor);
   OldPen = (HPEN) SelectObject(hdc, CurrentPen);

   MoveToEx(hdc, h1, v1, NULL);
   LineTo(hdc, h2, v2);

   SelectObject(hdc, OldBrush);
   SelectObject(hdc, OldPen);
   DeleteObject(CurrentBrush);
   DeleteObject(CurrentPen);
}

//----------------------------------------------------------------------------------------------------//
// This function simply draws a filled rectangle to the given device context, given the
// rectangle dimensions, its border thickness and its border color (the rectangle is filled
// in black).
//----------------------------------------------------------------------------------------------------//
void DrawRectangle(HDC hdc, RECT *r, int thk, COLORREF clr)
{
   HBRUSH      CurrentBrush;
   HBRUSH      OldBrush;
   HPEN     CurrentPen;
   HPEN     OldPen;
   COLORREF FColor = clr;
   COLORREF BColor = RGB(0, 0, 0);

   CurrentBrush = CreateSolidBrush(BColor);
   OldBrush = (HBRUSH) SelectObject( hdc, CurrentBrush);
   CurrentPen = CreatePen(PS_SOLID, thk, FColor);
   OldPen = (HPEN) SelectObject(hdc, CurrentPen);

   Rectangle(hdc, r->left, r->top, r->right, r->bottom);

   SelectObject(hdc, OldBrush);
   SelectObject(hdc, OldPen);
   DeleteObject(CurrentBrush);
   DeleteObject(CurrentPen);
}

//----------------------------------------------------------------------------------------------------//
// This function simply draws text to the given device context, given the text string
// and the x,y coordinates of its lower left corner, the number of characters in the string,
// and the desired point size.
//----------------------------------------------------------------------------------------------------//
void DrawString(HDC hdc, int x, int y, LPCSTR lpszString, int size, int ptsz)
{
   COLORREF FColor = RGB(255, 255, 255);
   COLORREF BColor = RGB(0, 0, 0);
   HFONT    hFont, hOldFont;

   SetTextColor(hdc, FColor);
   SetBkColor(hdc, BColor);
   SetBkMode(hdc, TRANSPARENT);
   SetTextAlign(hdc, TA_BOTTOM|TA_LEFT);

   hFont = CreateFont(-ptsz, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, "MS Serif");
   hOldFont = SelectObject(hdc, hFont);

   TextOut(hdc, x, y, lpszString, size);

   SelectObject(hdc, hOldFont);
   DeleteObject(hFont);
}


//----------------------------------------------------------------------------------------------------//
// This function steps the simulation ahead in time. This is where the kinematic properties
// are calculated.  The function will return 1 when the target is hit, and 2 when the shell
// hits the ground (x-z plane) before hitting the target, otherwise the function returns 0.
//----------------------------------------------------------------------------------------------------//
int   DoSimulation(void)
//----------------------------------------------------------------------------------------------------//
{
   double   C = PI * RHO * RHO * radius * radius * radius * omega;
   double   t;

   // step to the next time in the simulation
   time+=tInc;
   t = time;

   // calc V2:
   V2.i = 1.0f/(1.0f-(t/m)*(t/m)*C*C) * (V1.i + C * V1.j * (t/m) - C * g * (t*t)/m);
   V2.j = V1.j + (t/m)*C*V2.i - g*t;

   // calc S2:
   s2.i = s1.i + V1.i * t + (1.0f/2.0f) * (C/m * V2.j) * (t*t);
   s2.j = s1.j + V1.j * t + (1.0f/2.0f) * ( ((C*V2.i) - m*g)/m ) * (t*t);


   // Check for collision with ground (x-z plane)
   if(s2.j <= 0)
      return 2;

   // Cutoff the simulation if it's taking too long
   // This is so the program does not get stuck in the while loop
   if(time>60)
      return 3;

   return 0;
}

#endif
