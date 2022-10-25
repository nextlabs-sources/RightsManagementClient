
#ifndef __NXRM_ENGINE_DRV_SERV_H__
#define __NXRM_ENGINE_DRV_SERV_H__


#include <nudf\thread.hpp>

namespace nxrm {
namespace engine {
    

class CDrvWorkerThread : public nudf::util::thread::CThreadEx
{
public:
    CDrvWorkerThread();
    virtual ~CDrvWorkerThread();
    
    virtual DWORD OnStart(_In_opt_ PVOID Context);
    virtual DWORD OnRunning(_In_opt_ PVOID Context);
};


typedef struct _DRVREQUEST{
	LIST_ENTRY      Link;
	PVOID           Request;
	ULONG           Type;
	PVOID           Context;
}DRVREQUEST, *PDRVREQUEST;

class CDrvProcessInfo
{
public:
    CDrvProcessInfo() :  _pid(0), _parent_pid(0), _session_id(0), _platform(0), _service(false), _hooked(false)
    {
    }
    ~CDrvProcessInfo()
    {
    }

    inline const std::wstring& GetImage() const throw() {return _image;}
    inline const std::wstring& GetPublisher() const throw() {return _publisher;}
    inline ULONG GetPid() const throw() {return _pid;}
    inline ULONG GetParentPid() const throw() {return _parent_pid;}
    inline ULONG GetSessionId() const throw() {return _session_id;}
    inline ULONG GetPlatform() const throw() {return _platform;}
    inline bool IsService() const throw() {return _service;}
    inline bool IsHooked() const throw() {return _hooked;}

    inline void SetImage(_In_ LPCWSTR image) throw() {_image = image;}
    inline void SetPublisher(_In_ LPCWSTR publisher) throw() {_publisher = publisher;}
    inline void SetPid(_In_ ULONG pid) throw() {_pid = pid;}
    inline void SetParentPid(_In_ ULONG pid) throw() {_parent_pid = pid;}
    inline void SetSessionId(_In_ ULONG id) throw() {_session_id = id;}
    inline void SetPlatform(_In_ ULONG platform) throw() {_platform = platform;}
    inline void SetService(_In_ bool service) throw() {_service = service;}
    inline void SetHooked(_In_ bool hooked) throw() {_hooked = hooked;}

    inline void Clear() throw()
    {
        _image      = L"";
        _publisher  = L"";
        _pid        = 0;
        _parent_pid = 0;
        _session_id = 0;
        _platform   = 0;
        _service    = false;
        _hooked     = false;
    }

private:
    std::wstring    _image;
    std::wstring    _publisher;
    ULONG           _pid;
    ULONG           _parent_pid;
    ULONG           _session_id;
    ULONG           _platform;
    bool            _service;
    bool            _hooked;
};

class CWmTextInfo
{
public:
    CWmTextInfo() : _text(L"$(User)\\n$(Time)"), _transratio(90), _font(L"Sitka Text"), _size(16), _color(RGB(0,0,0)), _rotation(-45)
    {
    }
    virtual ~CWmTextInfo(){}

    inline const std::wstring& GetText() const throw() {return _text;}
    inline int GetTransparency() const throw() {return _transratio;}
    inline const std::wstring& GetFontName() const throw() {return _font;}
    inline int GetFontSize() const throw() {return _size;}
    inline COLORREF GetFontColor() const throw() {return _color;}
    inline int GetRotationAngle() const throw() {return _rotation;}

    inline void SetText(const std::wstring& text) throw() {_text = text;}
    inline void SetTransparency(int ratio) throw()
    {
        _transratio = (100 - ((abs(ratio) <= 100) ? abs(ratio) : 100));
    }
    inline void SetFontName(const std::wstring& name) throw() {_font = name;}
    inline void SetFontSize(int size) throw() {_size = abs(size);}
    inline void SetTextColor(COLORREF color) throw() {_color = color;}
    inline void SetTextColor(const std::wstring& name) throw()
    {
        //
        // See RGB color table:
        //    http://www.rapidtables.com/web/color/RGB_Color.htm
        //
        if(0 == _wcsicmp(name.c_str(), L"Red")) {
            _color = RGB(255,0,0);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Lime")) {
            _color = RGB(0,255,0);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Blue")) {
            _color = RGB(0,0,255);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Yellow")) {
            _color = RGB(255,255,0);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Cyan / Aqua")) {
            _color = RGB(0,255,255);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Magenta / Fuchsia")) {
            _color = RGB(255,0,255);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Gray")) {
            _color = RGB(128,128,128);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Dim Gray")) {
            _color = RGB(105,105,105);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Maroon")) {
            _color = RGB(128,0,0);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Olive")) {
            _color = RGB(128,128,0);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Green")) {
            _color = RGB(0,128,0);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Purple")) {
            _color = RGB(128,0,128);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Teal")) {
            _color = RGB(0,128,128);
        }
        else if(0 == _wcsicmp(name.c_str(), L"Navy")) {
            _color = RGB(0,0,128);
        }
        else {
            // Black
            _color = RGB(0,0,0);
        }
    }
    inline void SetRotationAngle(int angle) throw() {_rotation = (angle%180);}
    inline void SetRotationAngle(const std::wstring& angle) throw()
    {
        if(0 == _wcsicmp(angle.c_str(), L"None")) {
            _rotation = 0;
        }
        else if(0 == _wcsicmp(angle.c_str(), L"Clockwise")) {
            _rotation = -45;
        }
        else {
            _rotation = 45;
        }
    }

    CWmTextInfo& operator = (const CWmTextInfo& ti)
    {
        if(this != (&ti)) {
            _text = ti.GetText();
            _transratio = ti.GetTransparency();
            _font = ti.GetFontName();
            _size = ti.GetFontSize();
            _color = ti.GetFontColor();
            _rotation = ti.GetRotationAngle();
        }
        return *this;
    }

private:
    std::wstring    _text;
    int             _transratio;
    std::wstring    _font;
    int             _size;
    COLORREF        _color;
    int             _rotation;
};

class CDrvServ : public nudf::util::thread::CThreadExPool<CDrvWorkerThread>
{
public:
    CDrvServ();
    virtual ~CDrvServ();

    void Start();
    virtual void Stop() throw();
    
    PDRVREQUEST PopRequest();
    VOID PushRequest(PDRVREQUEST Request);
    VOID CleanRequestQueue();

    bool QueryProcessInfo(_In_ ULONG pid, _Out_ CDrvProcessInfo& info);
    void IncreasePolicySerianNo();

public:
    // Make request
    PDRVREQUEST MakeGetCustomUIRequest(_In_ PVOID Msg, _In_ ULONG Length);
    PDRVREQUEST MakeCheckObligationsRequest(_In_ PVOID Msg, _In_ ULONG Length);
    PDRVREQUEST MakeSaveAsForecastRequest(_In_ PVOID Msg, _In_ ULONG Length);
    PDRVREQUEST MakeGetClassifyUIRequest(_In_ PVOID Msg, _In_ ULONG Length);
    PDRVREQUEST MakeNotificationRequest(_In_ PVOID Msg, _In_ ULONG Length);
    PDRVREQUEST MakeCheckProtectMenuRequest(_In_ PVOID Msg, _In_ ULONG Length);
    PDRVREQUEST MakeGetContextMenuRegexRequest(_In_ PVOID Msg, _In_ ULONG Length);
    // Handle request
    VOID OnGetCustomUI(_In_ PDRVREQUEST ReqItem);
    VOID OnCheckObligations(_In_ PDRVREQUEST ReqItem);
    VOID OnSaveAsForecast(_In_ PDRVREQUEST ReqItem);
    VOID OnGetClassifyUI(_In_ PDRVREQUEST ReqItem);
    VOID OnNotification(_In_ PDRVREQUEST ReqItem);
    VOID OnCheckProtectMenu(_In_ PDRVREQUEST ReqItem);
    VOID OnGetContextMenuRegex(_In_ PDRVREQUEST ReqItem);

    std::wstring GenerateWaterMarkText(_In_ ULONG pid, _In_ LPCWSTR file, _In_ LPCWSTR wzTempFolder, _In_ CWmTextInfo& ti);
    std::wstring GenerateWaterMarkImage(_In_ const CWmTextInfo& ti, _In_ const std::wstring& folder);

private:
    LIST_ENTRY       _list;
    CRITICAL_SECTION _listlock;
    HANDLE           _drvman;
    HFONT            _hFont;
};

       
}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_DRV_SERV_H__