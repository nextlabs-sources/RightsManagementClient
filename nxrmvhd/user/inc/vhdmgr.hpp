

#ifndef __VHD_MGR_HPP__
#define __VHD_MGR_HPP__


#include <string>
#include <vector>


class CVhdInfo
{
public:
    CVhdInfo() : _diskid(-1), _drive(0), _visible(FALSE), _removable(FALSE)
    {
    }
    virtual ~CVhdInfo()
    {
    }

    inline const std::wstring& GetFile() const throw() {return _file;}
    inline const std::wstring& GetNtName() const throw() {return _nt_name;}
    inline const std::wstring& GetDosName() const throw() {return _dos_name;}
    inline ULONG GetDiskId() const throw() {return _diskid;}
    inline WCHAR GetDriveLetter() const throw() {return _drive;}
    inline BOOL  IsVisible() const throw() {return _visible;}
    inline BOOL  IsRemovable() const throw() {return _removable;}
    inline BOOL  IsValid() const throw() {return ((-1!=_diskid) && !_nt_name.empty());}

    inline void SetFile(const std::wstring& file) throw() {_file = file;}
    inline void SetNtName(const std::wstring& nt_name) throw() {_nt_name = nt_name;}
    inline void SetDosName(const std::wstring& dos_name) throw() {_dos_name = dos_name;}
    inline void SetDiskId(ULONG disk_id) throw() {_diskid = disk_id;}
    inline void SetDriveLetter(WCHAR drive) throw() {_drive = drive;}
    inline void SetVisible(BOOL visible) throw() {_visible = visible;}
    inline void SetRemovable(BOOL removable) throw() {_removable = removable;}

    CVhdInfo& operator = (const CVhdInfo& info) throw()
    {
        if(this != (&info)) {
            _file       = info.GetFile();;
            _nt_name    = info.GetNtName();
            _dos_name   = info.GetDosName();
            _diskid     = info.GetDiskId();
            _drive      = info.GetDriveLetter();
            _visible    = info.IsVisible();
            _removable  = info.IsRemovable();
        }

        return *this;
    }


private:
    std::wstring    _file;
    std::wstring    _nt_name;
    std::wstring    _dos_name;
    ULONG           _diskid;
    WCHAR           _drive;
    BOOL            _visible;
    BOOL            _removable;
};

class CVhdMgr
{
public:
    CVhdMgr();
    virtual ~CVhdMgr();

    BOOL Connect(_In_ BOOL bReadOnly);
    void Disconnect();

    BOOL CreateVhd(_In_ const std::wstring& file);
    BOOL LoadVhd(_In_ const std::wstring& file, _In_ WCHAR drive, _Out_ CVhdInfo& vhdi);
    BOOL UnloadVhd(_In_ ULONG id);
    BOOL UnloadAllVhds();
    BOOL QueryVhd(_In_ ULONG id, _Out_ CVhdInfo& vhdi);
    BOOL QueryAllVhds(_Out_ std::vector<CVhdInfo>& vhdis);


private:
    BOOL InterCreateVhd(_In_ const std::wstring& file, _In_ ULONG size_in_mb);
        
private:
    HANDLE  _h;
};


#endif