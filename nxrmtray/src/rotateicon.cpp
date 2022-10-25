#include <Windows.h>
#include <math.h>
#include <crtdbg.h>
#include "rotateicon.h"

//RotateIcon
//Rotate the icon by using SetWorldTransform API
void RotateIcon(HDC hDc, POINT centerPt, int iAngle, HICON hIcon)
{
	int nGraphicsMode = SetGraphicsMode(hDc, GM_ADVANCED);
	XFORM xform;
	if (iAngle != 0)
	{
		double fangle = (double)iAngle / 180. * 3.1415926;
		xform.eM11 = (float)cos(fangle);
		xform.eM12 = (float)sin(fangle);
		xform.eM21 = (float)-sin(fangle);
		xform.eM22 = (float)cos(fangle);
		xform.eDx = (float)(centerPt.x - cos(fangle)*centerPt.x + sin(fangle)*centerPt.y);
		xform.eDy = (float)(centerPt.y - cos(fangle)*centerPt.y - sin(fangle)*centerPt.x);
		SetWorldTransform(hDc, &xform);
	}
	DrawIconEx(hDc, centerPt.x - ROTATE_ICON_WIDTH / 2, centerPt.y - ROTATE_ICON_WIDTH / 2,
		hIcon, ROTATE_ICON_WIDTH, ROTATE_ICON_WIDTH, 0, NULL, DI_NORMAL);

	// restore the DC
	if (iAngle != 0)
	{
		xform.eM11 = (float)1.0;
		xform.eM12 = (float)0;
		xform.eM21 = (float)0;
		xform.eM22 = (float)1.0;
		xform.eDx = (float)0;
		xform.eDy = (float)0;
		SetWorldTransform(hDc, &xform);
	}
	SetGraphicsMode(hDc, nGraphicsMode);
}


//DrawRatatedIconFromBmp
//Draw hIcon in the center of hDc window, with the iAngle, and background color in rgbBkColor
//As we use memory to draw it does not cause screen fliker.
void DrawRotatedIconFromBmp(HDC hDc, int iAngle, HICON hIcon, COLORREF rgbBkColor)
{
	RECT rect;
	POINT pt;
	HDC hMemDC = CreateCompatibleDC(hDc);
	if(hMemDC)
	{
		::GetClientRect(WindowFromDC(hDc), &rect);
		int nWidth = rect.right - rect.left;
		int nHeight = rect.bottom - rect.top;
		HBITMAP hDrawBmp = CreateCompatibleBitmap(hDc, nWidth, nHeight);
		if (hDrawBmp)
		{//Using memory to draw
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hDrawBmp);
			SetDCBrushColor(hMemDC, rgbBkColor);
			::GetClientRect(WindowFromDC(hDc), &rect);
			pt.x = nWidth / 2;
			pt.y = nHeight / 2;
			FillRect(hMemDC,
				&rect,
				(HBRUSH)GetStockObject(DC_BRUSH));
			RotateIcon(hMemDC, pt, iAngle, hIcon);
			BitBlt(hDc, 0, 0,
				nWidth, nHeight, hMemDC, 0, 0, SRCCOPY);
			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hDrawBmp); //clean up memroy allocation
		}
		DeleteDC(hMemDC);
	}
}