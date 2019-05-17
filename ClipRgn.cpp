/* ClipRgn.cpp ****************************************************************
Author:		solgits
Date:		5/19/2019
Purpose:	Implementation file for a ClipRgn demo.
******************************************************************************/
#include "stdafx.h"
#include "resource.h"

#include <list>

#define MAX_LOADSTRING 100

/* Global Variables / typedefs ***********************************************/
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];	

using std::list;
typedef list<POINT>			listPoint;
typedef listPoint::iterator iterPoint;
			//C: The list of points that are stored for the user path that 
			//   outlines the user Rgn.
listPoint g_lstPoints;
			//C: The region that is used if the user wants to define their 
			//   own custom path for the system region.
HRGN g_hUserRgn		= NULL;
bool g_isDragging	= false;
bool g_isNT			= false;

/* Forward Declarations ******************************************************/
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	Main entry point for this application.
******************************************************************************/
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
			//C: Initialize the global strings.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CLIP_RGN, szWindowClass, MAX_LOADSTRING);
			//C: Register the class for the main window of this application.
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_CLIP_RGN;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);
			//C: Perform application initialization.
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

			//C: Main message pump.
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	Initializes global variables and creates the MainWindow.
Parameters:	hInstance[in]: The HINSTANCE of this application.
			nCmdShow[in]: The display mode to set for the main window when it 
				is created.
Return:		If this function succeeds then true will be returned otherwise false.
******************************************************************************/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
			//C: Store the instance handle in the global varaible.
	hInst = hInstance;
			//C: Determine if the current operating system is NT.  This
			//   will affect calculations that are made during paint.
	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&osInfo);
	if (VER_PLATFORM_WIN32_NT == osInfo.dwPlatformId)
	{
		g_isNT = true;
	}
			//C: Create the mainwindow.
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
			//C: The main window creation failed.
		return FALSE;
	}
			//C: Display the main window.
	ShowWindow(hWnd, nCmdShow);
			//C: Force the main window to repaint itself.
	UpdateWindow(hWnd);

	return TRUE;
}


LRESULT OnCommand (HWND hWnd, int iID, int iEvent, HWND hWndControl, bool &isHandled);
LRESULT OnLButtonDown (HWND hWnd, UINT nCtrl, UINT x, UINT y);
LRESULT OnMouseMove   (HWND hWnd, UINT nCtrl, UINT x, UINT y);
LRESULT OnLButtonUp   (HWND hWnd, UINT nCtrl, UINT x, UINT y);
LRESULT OnPaint       (HWND hWnd);


/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	Window procedure for the main window.
Parameters:	hWnd[in]: Handle to the window that the message is intended for.
			message[in]: The message being sent.
			wParam[in]: Data
			lParam[in]: Data
Return:		
******************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_CREATE:
		{
			//C: Set the initial drawing mode.
			HMENU hMenu = ::GetMenu(hWnd);
			HMENU hMenuView = ::GetSubMenu(hMenu, 1);

			::CheckMenuRadioItem(hMenuView, ID_DEFAULT, ID_MANUAL, ID_DEFAULT, MF_BYCOMMAND | MF_CHECKED);
			::CheckMenuItem(hMenuView, ID_CLIP, MF_BYCOMMAND | MF_CHECKED);
			::CheckMenuItem(hMenuView, ID_META, MF_BYCOMMAND | MF_CHECKED);
		}
	case WM_COMMAND:
		{
			int wmId    = LOWORD(wParam); 
			int wmEvent = HIWORD(wParam); 

			bool isHandled = true;
			LRESULT lResult = OnCommand(hWnd, wmId, wmEvent, (HWND)lParam, isHandled);
			if (!isHandled)
			{
				lResult = DefWindowProc(hWnd, message, wParam, lParam);
			}

			return lResult;
		}
		break;
	case WM_ERASEBKGND:
		{
			//C: Handle the ERASEBKGND message ourselves in order to completely
			//   erase the background each time the display is painted.
			//   Normally the WPARAM parameter contains the hdc that should be used
			//   to erase the background, but this hdc usually has some portion of 
			//   the DC that is clipped.
			HDC hdc = ::GetDC(hWnd);

			HBRUSH hbr = (HBRUSH)::GetClassLong(hWnd, GCL_HBRBACKGROUND);
			RECT rClient;
			::GetClientRect(hWnd, &rClient);

			::FillRect(hdc, &rClient, hbr);
			::ReleaseDC(hWnd, hdc);
			return 0;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			return OnLButtonDown(hWnd, (UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_MOUSEMOVE:
		{
			return OnMouseMove(hWnd, (UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_LBUTTONUP:
		{
			return OnLButtonUp(hWnd, (UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_PAINT:
		{
			return OnPaint(hWnd);
		}
		break;
	case WM_DESTROY:
		{
			//C: Send a shutdown message to the message pump.
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
			//C: Exit.
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	WM_COMMAND message handler.  Basically parses the menu commands
			and either sets the current drawing shape, or exits the window.
Parameters:	iID[in]:
			iEvent[in]:
			hWndControl[in]:
			isHandled[out]: This value can be set to indicate if this function 
				handled the message or not.
Return:		The LRESULT of this handled message will be returned.
******************************************************************************/
LRESULT OnCommand (HWND hWnd, int iID, int iEvent, HWND hWndControl, bool &isHandled)
{
			//C: Parse the menu selections.
	switch (iID)
	{			
	case ID_DEFAULT:
	case ID_MANUAL:
		{
			//C: Set the drawing mode.
			HMENU hMenu = ::GetMenu(hWnd);
			HMENU hMenuView = ::GetSubMenu(hMenu, 1);
			::CheckMenuRadioItem(hMenuView, ID_DEFAULT, ID_MANUAL, iID, MF_BYCOMMAND);

			//C: If the new mode is Default, make sure that the user path is erased.
			if (ID_DEFAULT == iID)
			{
				if (g_hUserRgn)
				{
					::DeleteObject(g_hUserRgn);
					g_hUserRgn = NULL;
				}

				::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			}
			else
			{
			//C: If the mew mode is manual, set the current path the client rect.
				RECT rClient;
				::GetClientRect(hWnd, &rClient);
				g_hUserRgn = ::CreateRectRgnIndirect(&rClient);
			}
		}
		break;
	case ID_CLIP:
	case ID_META:
		{
			//C: Set the drawing mode.
			HMENU hMenu = ::GetMenu(hWnd);
			HMENU hMenuView = ::GetSubMenu(hMenu, 1);

			DWORD dwState = ::GetMenuState(hMenuView, iID, MF_BYCOMMAND);
			if (dwState & MF_CHECKED)
			{
				::CheckMenuItem(hMenuView, iID, MF_BYCOMMAND);
			}
			else
			{
				::CheckMenuItem(hMenuView, iID, MF_BYCOMMAND | MF_CHECKED);
			}

			::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		}
		break;
	case IDM_EXIT: 
		{
			//C: Destroy the window in order to exit the program.
			DestroyWindow(hWnd);
		}
		break;
	default:
		{
			//C: Flag this message as unhandled.
			isHandled = false;
		}
	}

	return 0;
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	Handles the WM_LBUTTONDOWN message.  This function will start the
			creation of a new custom region if the current mode is manual.
Parameters:	hWnd[in]: The handle of the window where the mouse button was pressed.
			nCtrl[in]: A set of flags that indicates the control keys that 
				are currently pressed.
			x[in]: The X coordinate where the left button was pressed.
			y[in]: The Y coordinate where the left button was pressed.
Return:		0 will be returned.
******************************************************************************/
LRESULT OnLButtonDown (HWND hWnd, UINT nCtrl, UINT x, UINT y)
{
			//C: If the current mode is not manual mode, then exit this function.
	HMENU hMenu = ::GetMenu(hWnd);
	HMENU hMenuView = ::GetSubMenu(hMenu, 1);
	if (!(::GetMenuState(hMenuView, ID_MANUAL, MF_BYCOMMAND) & MF_CHECKED))
	{
		return 0;
	}
			//C: Redraw the display.
	::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			//C: Clear the current list of points.
	g_lstPoints.clear();
			//C: Set the very first point as the current cursor position.
	POINT pt = {x, y};
	g_lstPoints.push_back(pt);
			//C: Set dragging to true.
	g_isDragging = true;
			//C: Success.
	return 0;	
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	Handles the WM_MOUSEMOVE message.  This function will continue the
			creation of a new custom region.
Parameters:	hWnd[in]: The handle of the window where the mouse button was pressed.
			nCtrl[in]: A set of flags that indicates the control keys that 
				are currently pressed.
			x[in]: The X coordinate where the left button was pressed.
			y[in]: The Y coordinate where the left button was pressed.
Return:		0 will be returned.
******************************************************************************/
LRESULT OnMouseMove   (HWND hWnd, UINT nCtrl, UINT x, UINT y)
{
			//C: If the current mode is not manual mode, then exit this function.
	HMENU hMenu = ::GetMenu(hWnd);
	HMENU hMenuView = ::GetSubMenu(hMenu, 1);
	if (!(::GetMenuState(hMenuView, ID_MANUAL, MF_BYCOMMAND) & MF_CHECKED))
	{
		return 0;
	}
			//C: If dragging is not true, then exit.
	if (!g_isDragging)
	{
		return 0;
	}
			//C: Get the current point at the end of the list.
	POINT ptLast = *(g_lstPoints.rbegin());
			//C: Add a new point to the list.
	POINT pt = {x, y};
	g_lstPoints.push_back(pt);
			//C: Draw teh latest segment to the screen.
	HDC hdc = ::GetDC(hWnd);
	::MoveToEx(hdc, ptLast.x, ptLast.y, NULL);
	::LineTo(hdc, pt.x, pt.y);
	::ReleaseDC(hWnd, hdc);
			//C: Success.
	return 0;		
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		2/28/2002
Purpose:	Handles the WM_LBUTTONDOWN message.  This function will end the
			creation of a custom system region by the user.
Parameters:	hWnd[in]: The handle of the window where the mouse button was pressed.
			nCtrl[in]: A set of flags that indicates the control keys that 
				are currently pressed.
			x[in]: The X coordinate where the left button was pressed.
			y[in]: The Y coordinate where the left button was pressed.
Return:		0 will be returned.
******************************************************************************/
LRESULT OnLButtonUp   (HWND hWnd, UINT nCtrl, UINT x, UINT y)
{
			//C: If the current mode is not manual mode, then exit this function.
	HMENU hMenu = ::GetMenu(hWnd);
	HMENU hMenuView = ::GetSubMenu(hMenu, 1);
	if (!(::GetMenuState(hMenuView, ID_MANUAL, MF_BYCOMMAND) & MF_CHECKED))
	{
		return 0;
	}
			//C: If dragging is not true, then exit.
	if (!g_isDragging)
	{
		return 0;
	}
			//C: Get the current point at the end of the list.
	POINT ptLast = *(g_lstPoints.rbegin());
			//C: Add a new point to the list.
	POINT pt = {x, y};
	g_lstPoints.push_back(pt);
			//C: Draw teh latest segment to the screen.
	HDC hdc = ::GetDC(hWnd);
	::MoveToEx(hdc, ptLast.x, ptLast.y, NULL);
	::LineTo(hdc, pt.x, pt.y);
			//C: Draw a line to the beginning of the list.
	POINT ptFirst = *(g_lstPoints.begin());
	::LineTo(hdc, ptFirst.x, ptFirst.y);
			//C: Create a path from this list of points.
	::BeginPath(hdc);
	::MoveToEx(hdc, ptFirst.x, ptFirst.y, NULL);

	iterPoint iter		= g_lstPoints.begin();
	iterPoint iterEnd	= g_lstPoints.end();
	if (iter != iterEnd)
	{
		iter++;
	}

	while (iter != iterEnd)
	{
		POINT ptCurrent = *iter;
		::LineTo(hdc, ptCurrent.x, ptCurrent.y);

		iter++;
	}

	::CloseFigure(hdc);
	::EndPath(hdc);
			//C: Create a region from that path;
	if (g_hUserRgn)
	{
		::DeleteObject(g_hUserRgn);
		g_hUserRgn = NULL;
	}

	g_hUserRgn = ::PathToRegion(hdc);
	::ReleaseDC(hWnd, hdc);
			//C: Repaint the entire display.
	::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
			//C: Turn dragging off.
	g_isDragging = false;
			//C: Success.
	return 0;		
}


/* Global *********************************************************************
Author:		Paul Watt
Date:		3/13/2002
Purpose:	Paints three distinct regions that are contained in the DC.
			SystemRgn:	Outlined by a red frame.
			ClipRgn:	Vertical Blue lines.
			MetaRgn:	Horzontal Dark Purple lines.
			Intersection:  The intersection of all three regions will be painted
				in a pale pink color.  This represents the area that would 
				actually be visible and painting could occur in a regular 
				window application.
Parameters:	hWnd[in]:
Return:		
******************************************************************************/
LRESULT OnPaint       (HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	BOOL isDefault;
	BOOL isClip;
	BOOL isMeta;

	HRGN hClipRgn = NULL;
	HRGN hMetaRgn = NULL;
			//C: Get the dimensions of the client rectangle.
	RECT rClient;
	::GetClientRect(hWnd, &rClient);
			//C: Get the current system region mode.  This will determine how the
			//   DC is initialized.
	HMENU hMenu = ::GetMenu(hWnd);
	HMENU hMenuView = ::GetSubMenu(hMenu, 1);
	if (::GetMenuState(hMenuView, ID_DEFAULT, MF_BYCOMMAND) & MF_CHECKED)
	{
		isDefault = TRUE;
		hdc = ::BeginPaint(hWnd, &ps);
	}
	else
	{
		isDefault = FALSE;
			//C: Create a region that is the inverse of the User region.
		HRGN hRgnExclude = ::CreateRectRgnIndirect(&rClient);
		::CombineRgn(hRgnExclude, hRgnExclude, g_hUserRgn, RGN_DIFF);
			//C: This region needs to be placed into the proper coordinate
			//   space depending on the operating system.
		if (g_isNT)
		{
			POINT pt = {0,0};
			::MapWindowPoints(hWnd, NULL, &pt, 1);

			::OffsetRgn(hRgnExclude, pt.x, pt.y);
		}
			//C: There is no need to delete the exclude region because the
			//   call to GetDCEx takes ownership of this region.
		hdc = ::GetDCEx(hWnd, hRgnExclude, DCX_CACHE | DCX_EXCLUDERGN);
			//C: Since BeginPaint was not used to create the DC, WM_ERASEBKGND
			//   has to be generated manually.
		::SendMessage(hWnd, WM_ERASEBKGND, (WPARAM)hdc, NULL);
			//C: Validate the update region for this window to prevent 
			//   other calls to WM_PAINT.
		::ValidateRect(hWnd, NULL);
	}

	int nContext = ::SaveDC(hdc);
			//C: Determine of the Meta Region is activated.
	isMeta = ::GetMenuState(hMenuView, ID_META, MF_BYCOMMAND) & MF_CHECKED;
			//C: If isMeta is set, then create the clipping region.
	if (isMeta)
	{
		RECT rMeta = rClient;

		rMeta.top = (LONG)(rMeta.bottom * 0.1);
		rMeta.bottom = (LONG)(rMeta.bottom * 0.85);
		hMetaRgn = ::CreateEllipticRgnIndirect(&rMeta);
			//C: Select the Meta region into the DC.
		::SelectClipRgn(hdc, hMetaRgn);
		::SetMetaRgn(hdc);
	}
			//C: Determine of the Clip Region is activated.
	isClip = ::GetMenuState(hMenuView, ID_CLIP, MF_BYCOMMAND) & MF_CHECKED;
			//C: If isClip is set, then create the clipping region.
	if (isClip)
	{
		RECT rClip = rClient;

		rClip.left = (LONG)(rClip.right * 0.1);
		rClip.right = (LONG)(rClip.right * 0.75);
		hClipRgn = ::CreateEllipticRgnIndirect(&rClip);
			//C: Select the clip region into the DC.
		::SelectClipRgn(hdc, hClipRgn);
	}
			//C: Draw the combined clipping region.
	HBRUSH hPinkBrush = ::CreateSolidBrush(RGB(0xFF, 0x80, 0x80));
	::FillRect(hdc, &rClient, hPinkBrush);
	::DeleteObject(hPinkBrush);
			//C: Restore the C to its original state with no clipping regions.
	::RestoreDC(hdc, nContext);
			//C: Extract the System Rgn.
	HRGN hSystemRgn = ::CreateRectRgn(0,0,0,0);
	HBRUSH hRedBrush = ::CreateSolidBrush(RGB(0xFF, 0x00, 0x00));
	::GetRandomRgn(hdc, hSystemRgn, SYSRGN);
			//C: The current operating system will determine the coordinate space
			//   that the system region is returned in.
			//C: NT OS return the region in screen coordinates, therefore the 
			//   region needs to be translated.
	if (g_isNT)
	{
			POINT pt = {0,0};
			::MapWindowPoints(NULL, hWnd, &pt, 1);

			::OffsetRgn(hSystemRgn, pt.x, pt.y);
	}

			//C: Frame the System Rgn.
	::FrameRgn(hdc, hSystemRgn, hRedBrush, 3, 3);
	::DeleteObject(hRedBrush);
	::DeleteObject(hSystemRgn);
			//C: Properly Paint then destroy the Clip and Meta Regions if
			//   they exist.
			//C: Create a new DC that is not clipped by the current system region.
	HDC hdcFull = ::GetDC(hWnd);

	if (isMeta)
	{
		nContext = ::SaveDC(hdcFull);
			//C: Reselect the meta region into the DC.
		::SelectClipRgn(hdcFull, hMetaRgn);
		::SetMetaRgn(hdcFull);
			//C: Paint the meta region.
		HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(0x80, 0x00, 0x80));
		::SelectObject(hdcFull, hPen);

		int index;
		for (index = rClient.top; index < rClient.bottom; index += 10)
		{
			::MoveToEx(hdcFull, 0, index, NULL);
			::LineTo(hdcFull, rClient.right, index);
		}
			//C: Unselect the meta region.
		::RestoreDC(hdcFull, nContext);
		::DeleteObject(hPen);
			//C: Delete the meta region.
		::DeleteObject(hMetaRgn);
	}

	if (isClip)
	{
			//C: Reselect the clip region into the DC.
		::SelectClipRgn(hdcFull, hClipRgn);
			//C: Paint the clip region.
		HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0xFF));
		HPEN hPenOld = (HPEN)::SelectObject(hdcFull, hPen);

		int index;
		for (index = rClient.left; index < rClient.right; index += 10)
		{
			::MoveToEx(hdcFull, index, 0, NULL);
			::LineTo(hdcFull, index, rClient.bottom);
		}

		::SelectObject(hdcFull, hPenOld);
		::DeleteObject(hPen);
			//C: Unselect the clip region.
		::SelectClipRgn(hdcFull, NULL);
			//C: Delete the clip region.
		::DeleteObject(hClipRgn);
	}
			//C: Cleanup the extra DC.
	::ReleaseDC(hWnd, hdcFull);
			//C: Cleanup the DC appropriately.
	if (isDefault)
	{
		::EndPaint(hWnd, &ps);
	}
	else
	{
		::ReleaseDC(hWnd, hdc);
	}

	return 0;	
}


