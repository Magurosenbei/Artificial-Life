#include "stdafx.h"
#include "./WinMain/WinMain.h"

static bool bStopRenderThread = false;
static bool bResizeFrame = false;
static bool bPaused = false;

// local function prototypes
static double GetCPUTicks	(void);
static void   OnPreprocess	(const PRENDERARGS pArgList);
static void   OnResizeFrame	(HWND hWnd, unsigned int nWidth, unsigned int nHeight);
static void   RenderFrame	(const double dElapsed, const unsigned int nWidth, const unsigned int nHeight);
static void   ThreadProc	(UINT uMsg, WPARAM wParam, LPARAM lParam);

static double GetCPUTicks(void)
{
	static LARGE_INTEGER nFreq = {0}, nCount = {0};
	static double dReturn = 0.0, dCheckTime = 5.0;

	// check for a new frequency once every 5 seconds
	// note: this is in case ACPI, etc. alters it
	if((nFreq.QuadPart == 0) || (dCheckTime < dReturn))
	{
		dCheckTime = dReturn + 5.0;
		// avoid a division by zero by returning zero on error
		if(!QueryPerformanceFrequency(&nFreq)) return 0.0;
	}

	// use the ratio of tick amount divided by frequency to find the hertz
	QueryPerformanceCounter(&nCount);
	dReturn = ((double)nCount.QuadPart / (double)nFreq.QuadPart);
	return dReturn;
}

unsigned int __stdcall InitScene (void * pParam)
{
	PRENDERARGS pArgList = static_cast<PRENDERARGS>(pParam);
	//#ifdef _DEBUG
		DWORD dwFPSCurrent = 0, dwFPSLast = 0, dwFPSInterval = 0;	// used to help calc the fps
		unsigned short nFPS = 0;									// current framerate/FPS for the main loop
	//#endif

	double dLastTime = 0, dCurTime = 0;	// used to calc CPU cycles during a render
	double dElapsed = 0;					// used to calc CPU cycles during a render
	long EndTime = 0;
	HGLRC hRC = NULL;							// handle to the GLs render context
	RECT rcClient = {0};						// coords of the area safe to draw on
	MSG msg	= {0};								// message struct for the queue

	// create and activate (in OGL) the render context
	hRC = wglCreateContext(pArgList->hDC);
	if(wglMakeCurrent(pArgList->hDC, hRC))
	{
		#ifdef _DEBUG
			ENTER_GL
		#endif
		// call the preprocess event handler to take care of initialization routines specific to OGL
		OnResizeFrame(pArgList->hWnd, APP_DEF_WIDTH, APP_DEF_HEIGHT);
		OnPreprocess(pArgList);
		#ifdef _DEBUG
			// test to see if we got an initialization error, if so then stop the render thread
			bStopRenderThread = LEAVE_GL(_T("OnPreprocess()"))
		#endif
		// if no previous error exists, let the main thread know it's ok to display the main window
		if(!bStopRenderThread) bStopRenderThread = (bool)!SendMessage(pArgList->hWnd, UWM_SHOW, pArgList->bZoomed, 0);
	}

	////////////////////// START OF OGL DRAWING/PROCESSING ////////////////////

	// since we are in another thread, start a separate endless loop for rendering
	while(!bStopRenderThread)
	{
		// NOTE: it is imparative that PeekMessage() is used rather than GetMessage()
		// listen to the queue in case this thread receives a message
		if(PeekMessage(&msg, NULL, UWM_PAUSE, UWM_STOP, PM_REMOVE))
			ThreadProc(msg.message, msg.wParam, msg.lParam);

		// do not waste processing time if the window is minimized
		// note: even in fullscreen a window can end up minimized
		if(!IsIconic(pArgList->hWnd))
		{
			// if we need to resize the window then do so, but only once per request
			// this will be called on startup
			if(bResizeFrame)
			{
				// set-up the perspective screen to be the size of the client area
				// NOTE: to avoid clipping, use the client area size, not the window size
				GetClientRect(pArgList->hWnd, &rcClient);

				// call handler and set flag that it's been processed
				OnResizeFrame(pArgList->hWnd, rcClient.right, rcClient.bottom);
				bResizeFrame = false;
			}

			// only perform this action if the rendering isn't paused
			if(!bPaused)
			{
				#ifdef _DEBUG
					// use a low resolution for the FPS count
					dwFPSCurrent = GetTickCount();

					ENTER_GL
				#endif

				// call the main drawing routine, if it's to be seen, this routine must show it
				// note: we use time-based rendering, so the time arg should be used as a factor
				//if(bStopRenderThread)
				//	break;
				dCurTime = GetCPUTicks();
				dElapsed = dCurTime - dLastTime;
				EndTime = long((1000 / APP_UPDATERATE) - dwFPSInterval);
				(EndTime > 0)? Sleep(EndTime) : NULL;
				RenderFrame(dwFPSInterval * 0.001, rcClient.right, rcClient.bottom);
				g_Engine->UpdateObjects();
				g_Engine->ProcessInput();
				dLastTime = dCurTime;

				#ifdef _DEBUG
					// if we had OGL errors during the render, let's find out about them
					bPaused = LEAVE_GL(_T("RenderFrame()"))
				#endif

				// swap the buffers (double buffering)
				SwapBuffers(pArgList->hDC);
				dwFPSInterval = GetTickCount() - dwFPSCurrent;
				#ifdef _DEBUG

					if(!pArgList->bFullscreen)
					{
						// display FPS on the titlebar of the main window if not in fullscreen mode
						if(nFPS == 0)
						{
							dwFPSLast = dwFPSCurrent;
							nFPS = 1;
						}
						else
						{
							// check for overflow (as a precaution)
							if(nFPS < 0xFFFF) nFPS++;

							// reset the FPS counter if we reach a new second
							if((dwFPSCurrent - dwFPSLast) >= 1000)
							{
								TCHAR szBuff[MAX_LOADSTRING] = {0};

								// to save performance, only update the window title once a second
								// also, place the OGL version information in the titlebar
								wsprintf(szBuff, _T("%hu FPS Interval : %i"), nFPS, dwFPSInterval);
								SetWindowText(pArgList->hWnd, szBuff);

								nFPS = 0;
							}
						}
					}
				#endif
			}
			else
			{
				// in order to keep the timed-based counter current, call this if we're paused
				dLastTime = GetCPUTicks();
			}
		}
	}
	////////////////////// END OF OGL DRAWING/PROCESSING //////////////////////
	// clean-up (OGL and wiggle specific items)
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);

	// kill this thread and its resources (CRT allocates them)
	_endthreadex(0);
	return 0;
}

// use all the enable stuff here
static void OnPreprocess (const PRENDERARGS pArgList)
{
	if(pArgList == NULL)	// if there is no thread 
		return;

	// do we modify vsync?
	SetVSync(pArgList->Status);

	glShadeModel(GL_SMOOTH);					// enable smooth shading
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);		// black background/clear color

	// set-up the depth buffer
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

#ifndef _2D_MODE
	glDepthFunc(GL_LESS);
#else
	glDepthFunc(GL_LEQUAL);
#endif
	// set up one-byte alignment for pixel storage (saves memory)
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// use backface culling (this is 2D, so we'll never see the back faces anyway)
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	glEnable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	// perspective calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_BLEND);									//blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//type of blending
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0);

#ifdef OUTLINED
	glLineWidth(OUTLINE_W);
#endif
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glHint(GL_FOG_HINT, GL_NICEST);
	float Color[] = {0.6f, 0.4f, 0.4f, 1.0f};
	glFogfv(GL_FOG_COLOR, Color);
	glFogf(GL_FOG_START, 50.0f);
	glFogf(GL_FOG_END,	600.0f);

	glEnable(GL_TEXTURE_2D);

	g_Engine->ContextPreProcess();
}

static void OnResizeFrame (HWND hWnd, unsigned int nWidth, unsigned int nHeight)
{
	// prevent division by zero
	if(nHeight <= 0) nHeight = 1;
	// (re)size the viewport to consume the entire client area
	glViewport(0, 0, nWidth, nHeight);
	// reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// (re)calculate the aspect ratio of the viewport (0,0 is bottom left)
	//glOrtho(0.0f, nWidth, 0.0f, nHeight, 0.0f, 1.0f);
	gluPerspective(45.0f, (float)nWidth / (float)nHeight, 0.01f, 1000.0f);
	// lastly, reset the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	#ifdef _2D_MODE
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-APP_DEF_WIDTH / 2, APP_DEF_WIDTH / 2, -APP_DEF_HEIGHT / 2, APP_DEF_HEIGHT / 2, -1, 1);	// Create Ortho Resoultion View (0,0 At Top Left)
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	#endif
}

static void RenderFrame (const double dElapsed, const unsigned int nWidth, const unsigned int nHeight)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear screen and depth buffers
	glLoadIdentity();	// reset modelview matrix
	glPushMatrix();

	g_Engine->RenderObjects();

	glPopMatrix();
	glFlush();
}

static void ThreadProc (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case UWM_PAUSE:
			// notification that we should either pause or resume rendering
			bPaused = (bool)wParam;
			break;

		case UWM_RESIZE:
			// notification that viewport(s) need(s) to be resized
			bResizeFrame = true;
			break;

		case UWM_STOP:
			// notification that we should stop the render thread from executing
			bStopRenderThread = true;
			break;
	}
}