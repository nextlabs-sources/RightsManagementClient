

#include <Windows.h>
#include <assert.h>
#include <gdiplus.h>

#include <nudf\bitmap.hpp>
#include <nudf\string.hpp>
#include <nudf\regex.hpp>

using namespace Gdiplus;


using namespace nudf::image;



const float nudf::image::CBitmap::PI = 3.14159265f;


CBitmap::CBitmap() : _hbmp(NULL), _clrBK(RGB(255,255,255))
{
	GdiplusStartupInput gdiplusStartupInput;
	
	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
}

CBitmap::CBitmap(HBITMAP hBitmap, COLORREF clrBK) : _hbmp(hBitmap), _clrBK(clrBK)
{
	GdiplusStartupInput gdiplusStartupInput;

	GdiplusStartup(&_gdiplusToken, &gdiplusStartupInput, NULL);
}

CBitmap::~CBitmap()
{
    Clear();
	GdiplusShutdown(_gdiplusToken);
}

bool CBitmap::Create(_In_ HDC hDC, _In_ int cx, _In_ int cy, _In_ COLORREF clrBK)
{
    bool        result = false;
    HDC         hdc = NULL;
    BITMAPINFO  bmpinfo;
    PVOID       bits = NULL;
    HDC         hTopDC = NULL;
    PBYTE       pbData = NULL;

    if(NULL == hDC) {
        hDC = hTopDC = GetDC(NULL);
        if(NULL == hDC) {
            return false;
        }
    }

    hdc = ::CreateCompatibleDC(hDC);
    if(NULL == hdc) {
        goto _exit;
    }

    ZeroMemory(&bmpinfo, sizeof(bmpinfo));
    bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpinfo.bmiHeader.biWidth = cx;
    bmpinfo.bmiHeader.biHeight = cy;
    bmpinfo.bmiHeader.biPlanes = 1;
    bmpinfo.bmiHeader.biBitCount = 32;         // four 8-bit components (R, G, B, Alpha)
    bmpinfo.bmiHeader.biCompression = BI_RGB;
    bmpinfo.bmiHeader.biSizeImage = cx * cy * 4;
    
    _hbmp = CreateDIBSection(hdc, &bmpinfo, DIB_RGB_COLORS, &bits, NULL, 0x0);    
    if(NULL == _hbmp) {
        goto _exit;
    }

    pbData = (PBYTE)bits;
    for(int i=0; i< cy; i++) {
        for(int j=0; j< cx; j++) {
            pbData[0] = GetRValue(clrBK);
            pbData[1] = GetGValue(clrBK);
            pbData[2] = GetBValue(clrBK);
            pbData[3] = 0;
            pbData += 4;
        }
    }

    result = true;

_exit:
    if(NULL != hdc) {
        ::DeleteDC(hdc);
    }
    if(NULL != hTopDC) {
        ::ReleaseDC(NULL, hTopDC);
    }
    return result;
}

void CBitmap::Clear()
{
    if(NULL != _hbmp) {
        ::DeleteObject(_hbmp);
    }
    _clrBK = RGB(255, 255, 255);
}

CBitmap& CBitmap::operator = (const CBitmap& bmp)
{
    if(this != &bmp) {
        Attach(bmp.Detach(), bmp.GetBkColor());
    }
    return *this;
}

bool CBitmap::Rotate(_In_ HDC hDC, _In_ int nAngle)
{
	// Create a memory DC compatible with the display
    bool result = false;
    HDC hdcSource = NULL, hdcTarget = NULL;    
	HBITMAP hbmOldSource = NULL, hbmOldDest = NULL; 
    BITMAP bm;
    HBITMAP hbmResult = NULL;
    HBRUSH hbrBack = NULL;
	HBRUSH hbrOld = NULL;
    int nOldGraphicMode = 0;
	XFORM xform;
    HDC hTopDC = NULL;

    if(NULL == hDC) {
        hDC = hTopDC = GetDC(NULL);
        if(NULL == hDC) {
            return false;
        }
    }

 
	// Get logical coordinates
	::GetObjectW(_hbmp, sizeof(bm), &bm);
 
    nAngle %= 180;
    float radian = (float)((nAngle * CBitmap::PI) / 180.0f);
	float cosine = (float)cos(radian);
	float sine = (float)sin(radian);

	// Compute dimensions of the resulting bitmap
	// First get the coordinates of the 3 corners other than origin
	int x1 = (int)(bm.bmHeight * sine);
	int y1 = (int)(bm.bmHeight * cosine);
    int x2 = (int)(bm.bmWidth * cosine + bm.bmHeight * sine);
	int y2 = (int)(bm.bmHeight * cosine - bm.bmWidth * sine);
	int x3 = (int)(bm.bmWidth * cosine);
	int y3 = (int)(-bm.bmWidth * sine);
 
    int minx = min(0,min(x1, min(x2,x3)));
	int miny = min(0,min(y1, min(y2,y3)));
	int maxx = max(0,max(x1, max(x2,x3)));
	int maxy = max(0,max(y1, max(y2,y3)));
 
    int w = maxx - minx;
	int h = maxy - miny;
 
	// Create a bitmap to hold the result
	hdcSource = ::CreateCompatibleDC(hDC);
    if(NULL == hdcSource) {
        goto _exit;
    }
    hbmResult = ::CreateCompatibleBitmap(hDC, w, h);
    if(NULL == hbmResult) {
        goto _exit;
    }
	hbmOldSource = (HBITMAP)::SelectObject(hdcSource, _hbmp);
    if(NULL == hbmOldSource) {
        goto _exit;
    }
	hdcTarget = ::CreateCompatibleDC(hDC);
    if(NULL == hdcTarget) {
        goto _exit;
    }
    hbmOldDest = (HBITMAP)::SelectObject(hdcTarget, hbmResult);
    if(NULL == hbmOldDest) {
        goto _exit;
    }
 
	// Draw the background color before we change mapping mode
    hbrBack = CreateSolidBrush(_clrBK);
	hbrOld = (HBRUSH)::SelectObject(hdcTarget, hbrBack);
	PatBlt(hdcTarget, 0, 0, w, h, PATCOPY);
	::DeleteObject(::SelectObject(hdcTarget, hbrOld));
    hbrBack = NULL; hbrOld = NULL;
 
    // We will use world transform to rotate the bitmap
    nOldGraphicMode = SetGraphicsMode(hdcTarget, GM_ADVANCED);
	if(0 == nOldGraphicMode) {
        goto _exit;
    }

	xform.eM11 = cosine;
	xform.eM12 = -sine;
    xform.eM21 = sine;
	xform.eM22 = cosine;
	xform.eDx = (float)-minx;
	xform.eDy = (float)-miny;
 
    if(!SetWorldTransform(hdcTarget, &xform)) {
        SetGraphicsMode(hdcTarget, nOldGraphicMode);
        ::SelectObject(hdcSource, hbmOldSource);
        ::SelectObject(hdcTarget, hbmOldDest);
        ::DeleteDC(hdcSource);
        ::DeleteDC(hdcTarget);
        return NULL;
    }
 
	// Now do the actual rotating - a pixel at a time
	BitBlt(hdcTarget, 0, 0, bm.bmWidth, bm.bmHeight, hdcSource, 0, 0, SRCCOPY);

    ::DeleteObject(_hbmp);
    _hbmp = hbmResult;
    hbmResult = NULL;
    result = true;
 
_exit:
    // Restore DCs
    if(0 != nOldGraphicMode) {
        SetGraphicsMode(hdcTarget, nOldGraphicMode);
    }
    if(NULL != hbmOldSource) {
	    ::SelectObject(hdcSource, hbmOldSource);
    }
    if(NULL != hbmOldDest) {
	    ::SelectObject(hdcTarget, hbmOldDest);
    }
    if(NULL != hdcSource) {
        ::DeleteDC(hdcSource);
    }
    if(NULL != hdcTarget) {
        ::DeleteDC(hdcTarget);
    }
    if(NULL != hTopDC) {
        ::ReleaseDC(NULL, hTopDC);
    }
    return result;
}

bool CBitmap::SetTransparency(_In_ ULONG percentage)
{
    HDC     hdc = NULL;
    BITMAP  bm;
    BITMAPINFOHEADER bi;
    std::vector<unsigned char> bits;
    PBYTE   pbData = NULL; 
    UCHAR   tranRatio = (UCHAR)(((percentage*255) / 100));
    
    if(NULL == _hbmp) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

	// Get logical coordinates
	::GetObjectW(_hbmp, sizeof(bm), &bm);
    
    ZeroMemory(&bi, sizeof(bi));
    bi.biSize           = sizeof(BITMAPINFOHEADER);    
    bi.biWidth          = bm.bmWidth;
    bi.biHeight         = bm.bmHeight;
    bi.biPlanes         = 1;
    bi.biBitCount       = 32;
    bi.biCompression    = BI_RGB;
    bi.biSizeImage      = ((bm.bmWidth * bi.biBitCount + 31) / 32) * 4 * bm.bmHeight;
    bi.biXPelsPerMeter  = 0;
    bi.biYPelsPerMeter  = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant   = 0;

	hdc = CreateCompatibleDC(NULL);
    if(NULL == hdc) {
        return false;
    }

    bits.resize(bi.biSizeImage, 0);
    
    if(0 == GetDIBits(hdc, _hbmp, 0, (UINT)bm.bmHeight, (void*)bits.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS)) {
        ::DeleteDC(hdc);
        return false;
    }
    
    pbData = (PBYTE)bits.data();
    for(int i=0; i< bm.bmHeight; i++) {

        for(int j=0; j< bm.bmWidth; j++) {

            if (pbData[0] == GetRValue(_clrBK) && pbData[1] ==GetGValue(_clrBK) && pbData[2] == GetBValue(_clrBK)) {
                pbData[3] = 0;          // make it completely transparent
            }
            else {
                pbData[3] = tranRatio;  // make it transparent at ratio%
				pbData[0] = (BYTE)((DWORD)pbData[0] * pbData[3] / 255);
				pbData[1] = (BYTE)((DWORD)pbData[1] * pbData[3] / 255);
				pbData[2] = (BYTE)((DWORD)pbData[2] * pbData[3] / 255);
			}
            pbData += 4;

        }
    }

    SetDIBits(hdc, _hbmp, 0, (UINT)bm.bmHeight, bits.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    ::DeleteDC(hdc);
    return true;
}


bool CBitmap::SetAlphaChannel(_In_ ULONG percentage, COLORREF _clrForground)
{
    HDC     hdc = NULL;
    BITMAP  bm;
    BITMAPINFOHEADER bi;
    std::vector<unsigned char> bits;
    PBYTE   pbData = NULL; 
    UCHAR   tranRatio = (UCHAR)(((percentage *255) / 100));
    
    if(NULL == _hbmp) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

	// Get logical coordinates
	::GetObjectW(_hbmp, sizeof(bm), &bm);
    
    ZeroMemory(&bi, sizeof(bi));
    bi.biSize           = sizeof(BITMAPINFOHEADER);    
    bi.biWidth          = bm.bmWidth;
    bi.biHeight         = bm.bmHeight;
    bi.biPlanes         = 1;
    bi.biBitCount       = 32;
    bi.biCompression    = BI_RGB;
    bi.biSizeImage      = ((bm.bmWidth * bi.biBitCount + 31) / 32) * 4 * bm.bmHeight;
    bi.biXPelsPerMeter  = 0;
    bi.biYPelsPerMeter  = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant   = 0;

	//BITMAPV5HEADER bi;
	//ZeroMemory(&bi, sizeof(bi));
	//bi.bV5Size = sizeof(BITMAPV5HEADER);
	//bi.bV5Width = bm.bmWidth;
	//bi.bV5Height = bm.bmHeight;
	//bi.bV5Planes = 1;
	//bi.bV5BitCount = 32;
	//bi.bV5Compression = BI_BITFIELDS;
	//bi.bV5SizeImage = bm.bmWidth*bm.bmHeight * 4;
	//bi.bV5RedMask = 0x00FF0000;
	//bi.bV5GreenMask = 0x0000FF00;
	//bi.bV5BlueMask = 0x000000FF;
	//bi.bV5AlphaMask = 0xFF000000;
	//bi.bV5CSType = 0x57696e20; // LCS_WINDOWS_COLOR_SPACE
	//bi.bV5Intent = LCS_GM_BUSINESS;


	hdc = CreateCompatibleDC(NULL);
    if(NULL == hdc) {
        return false;
    }

    bits.resize(bi.biSizeImage, 0);
    
    if(0 == GetDIBits(hdc, _hbmp, 0, (UINT)bm.bmHeight, (void*)bits.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS)) {
        ::DeleteDC(hdc);
        return false;
    }
    
	pbData = (PBYTE)bits.data();
    for(int i=0; i< bm.bmHeight; i++) {

        for(int j=0; j< bm.bmWidth; j++) {

			if (pbData[0] == GetBValue(_clrBK) && pbData[1] == GetGValue(_clrBK) && pbData[2] == GetRValue(_clrBK)) {
				pbData[3] = tranRatio;          // make it completely transparent
			}
			else
			{ //Remove text boarder here

				pbData[3] = tranRatio;          // make it completely transparent
				pbData[0] = (BYTE)((DWORD)pbData[0] * pbData[3] / 255);
				pbData[1] = (BYTE)((DWORD)pbData[1] * pbData[3] / 255);
				pbData[2] = (BYTE)((DWORD)pbData[2] * pbData[3] / 255);
			}

            pbData += 4;

        }
    }

    SetDIBits(hdc, _hbmp, 0, (UINT)bm.bmHeight, bits.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    ::DeleteDC(hdc);
    return true;
}





int CBitmap::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) const throw()
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

bool CBitmap::ToPNGFile(_In_ const wchar_t* file) const throw()
{
    HANDLE              hFile = INVALID_HANDLE_VALUE;

	if(NULL == _hbmp) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }
 

	Bitmap bmp(_hbmp, NULL);
	//bmp.SetPixel()

	// Save the altered image.
	CLSID pngClsid;
	//GetEncoderClsid();
	GetEncoderClsid(L"image/png", &pngClsid);
	Status ret = bmp.Save(file, &pngClsid, NULL);


    return true;
}

bool CBitmap::ToFile(_In_ const wchar_t* file) const throw()
{
    HDC                 hdc = NULL;
    BITMAPFILEHEADER    bmfHeader;
    BITMAPINFOHEADER    bi;
    BITMAP              bm;
    std::vector<unsigned char> bits;
    HANDLE              hFile = INVALID_HANDLE_VALUE;

    
    if(NULL == _hbmp) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }
 
	// Get logical coordinates
	::GetObjectW(_hbmp, sizeof(bm), &bm);
     
    ZeroMemory(&bi, sizeof(bi));
    bi.biSize           = sizeof(BITMAPINFOHEADER);    
    bi.biWidth          = bm.bmWidth;
    bi.biHeight         = bm.bmHeight;
    bi.biPlanes         = 1;
    bi.biBitCount       = 32;
    bi.biCompression    = BI_RGB;
    bi.biSizeImage      = ((bm.bmWidth * bi.biBitCount + 31) / 32) * 4 * bm.bmHeight;
    bi.biXPelsPerMeter  = 0;
    bi.biYPelsPerMeter  = 0;
    bi.biClrUsed        = 0;
    bi.biClrImportant   = 0;

	hdc = CreateCompatibleDC(NULL);
    if(NULL == hdc) {
        return false;
    }

    bits.resize(bi.biSizeImage, 0);
    
    if(0 == GetDIBits(hdc, _hbmp, 0, (UINT)bm.bmHeight, (LPVOID)bits.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS)) {
        ::DeleteDC(hdc);
        return false;
    }

    hFile = ::CreateFileW(file, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        ::DeleteDC(hdc);
        return false;
    }

    // Add the size of the headers to the size of the bitmap to get the total file size
    DWORD dwSizeofDIB = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
 
    //Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER); 
    
    //Size of the file
    bmfHeader.bfSize = dwSizeofDIB; 
    
    //bfType must always be BM for Bitmaps
    bmfHeader.bfType = 0x4D42; //BM   
 
    DWORD dwBytesWritten = 0;
    WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, bits.data(), bi.biSizeImage, &dwBytesWritten, NULL);
    CloseHandle(hFile);
    ::DeleteDC(hdc);

    return true;
}


CTextBitmap::CTextBitmap()
{
}

CTextBitmap::~CTextBitmap()
{
}

bool CTextBitmap::Create(_In_ LPCWSTR wzText, _In_ HFONT hFont, _In_ COLORREF clrBK, _In_ COLORREF clrFont, _In_ Alignment alignment)
{
    bool    result = false;
    HDC     hScreenDC = NULL;
    HDC     hDC = NULL;
    HFONT   hOldFont = NULL;
    HBITMAP hOldBitmap = NULL;
    SIZE    bmpSize = {0, 0};
    std::wstring wsText = (NULL!=wzText) ? wzText : L"";
    std::vector<std::wstring> vText;
    std::vector<std::pair<std::wstring, SIZE>> vTextInfo;

    static const int nLineSpace = 6;

    if(wsText.length() == 0) {
        return false;
    }

    nudf::string::Split<wchar_t>(wsText, L'\n', vText);
    if(vText.empty()) {
        return false;
    }

    hScreenDC = GetDC(NULL);
    hDC = ::CreateCompatibleDC(hScreenDC);
    if(NULL == hDC) {
        ::ReleaseDC(NULL, hScreenDC);
        return false;
    }
    
    hOldFont = (HFONT)::SelectObject(hDC, hFont);

    // Calculate Image size
    for(std::vector<std::wstring>::iterator it=vText.begin(); it!=vText.end(); ++it) {

        SIZE    lineSize = {0, 0};

        if((*it).length() == 0) {
            // Avoid empty line
            (*it) = L" ";
        }

        if(!GetTextExtentPoint32W(hDC, (*it).c_str(), (int)(*it).length(), &lineSize)) {
            goto _exit;
        }

        bmpSize.cx  = max(bmpSize.cx, lineSize.cx);
        bmpSize.cy += (nLineSpace + lineSize.cy);

        // Add to vector
        vTextInfo.push_back(std::pair<std::wstring, SIZE>(*it, lineSize));
    }

    if(!CBitmap::Create(hDC, bmpSize.cx, bmpSize.cy, clrBK)) {
        goto _exit;
    }

    // Write text
    ::SelectObject(hDC, GetHandle());
    ::SetBkColor(hDC, clrBK);
    ::SetBkMode(hDC, TRANSPARENT);
    ::SetTextColor(hDC, clrFont);
    int cy = 0;
    for(std::vector<std::pair<std::wstring, SIZE>>::const_iterator it=vTextInfo.begin(); it!=vTextInfo.end(); ++it) {
        int cx = (alignment == Left) ? 0 : ((alignment == Right) ? (bmpSize.cx-(*it).second.cx) : ((bmpSize.cx-(*it).second.cx)/2));
        TextOutW(hDC, cx, cy, (*it).first.c_str(), (int)(*it).first.length());
        cy += (nLineSpace + (*it).second.cy);
    }

    result = true;

_exit:
    // Clean
    if(NULL != hOldBitmap) {
        ::SelectObject(hDC, hOldBitmap);
    }
    if(NULL != hOldFont) {
        ::SelectObject(hDC, hOldFont);
    }
    if(NULL != hDC) {
        ::DeleteDC(hDC);
    }
    if(NULL != hScreenDC) {
        ::ReleaseDC(NULL, hScreenDC);
    }
    return true;
}

bool CTextBitmap::Create(_In_ LPCWSTR wzText, _In_ LPCWSTR wzFont, _In_ int nFontSize, _In_ COLORREF clrBK, _In_ COLORREF clrFont, _In_ Alignment alignment)
{
    bool  result = false;
    HFONT hFont;
    int   nHeight = -nFontSize;

    HDC hDC = GetDC(NULL);
    nHeight = 0 - MulDiv(nFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ::ReleaseDC(NULL, hDC);
    hDC = NULL;

    hFont = CreateFontW( nHeight, 0, 0, 0, FW_BOLD,
                         0, FALSE, FALSE,
                         ANSI_CHARSET,
                         OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE,
                         wzFont);
    if(NULL == hFont) {
        return false;
    }

    result = Create(wzText, hFont, clrBK, clrFont, alignment);
    ::DeleteObject(hFont);
    hFont = NULL;

    return result;
}

bool CTextBitmap::Create(_In_opt_ HDC hDC, _In_ int cx, _In_ int cy, _In_ COLORREF clrBK)
{
    assert(false);
    return false;
}
