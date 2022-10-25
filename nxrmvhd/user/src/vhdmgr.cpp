
#include <Windows.h>
#include <assert.h>
#include <Shlobj.h>
#include <Dbt.h>

#include <boost\algorithm\string.hpp>

#ifndef ASSERT
#define ASSERT assert
#endif
#include <nudf\shared\vhddef.h>

#include "vhdmgr.hpp"



static const UCHAR DEFAULT_DISKKEY[32] = {
    0xB0, 0x00, 0x17, 0xCD, 0xFF, 0xC5, 0xA7, 0xB1,
    0xB0, 0x00, 0x17, 0xCD, 0xFF, 0xC5, 0xA7, 0xB1,
    0xB0, 0x00, 0x17, 0xCD, 0xFF, 0xC5, 0xA7, 0xB1,
    0xB0, 0x00, 0x17, 0xCD, 0xFF, 0xC5, 0xA7, 0xB1
};


static void VhdS2O(_In_ const NXRMVHDINFO* s, _Out_ CVhdInfo& o)
{
    WCHAR dos_name[128] = {0};
    o.SetFile(s->HostFileName);
    o.SetDiskId(s->DiskId);
    o.SetNtName(s->VolumeName);
    o.SetRemovable(s->Removable);
    o.SetVisible(s->Visible);
    if(o.IsVisible()) {
        o.SetDriveLetter(s->DriveLetter);
        swprintf_s(dos_name, 128, L"%s%C:", DOS_MOUNT_PREFIX, s->DriveLetter);
        o.SetDosName(dos_name);
    }
}

CVhdMgr::CVhdMgr() : _h(INVALID_HANDLE_VALUE)
{
}

CVhdMgr::~CVhdMgr()
{
}

BOOL CVhdMgr::Connect(_In_ BOOL bReadOnly)
{
    if(_h != INVALID_HANDLE_VALUE) {
        return TRUE;
    }

    _h = ::CreateFileW(NXRMVHD_WIN32_DEVICE_NAME_W,
                       bReadOnly ? GENERIC_READ : (GENERIC_READ|GENERIC_WRITE),
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_DEVICE,
                       NULL);
    return (INVALID_HANDLE_VALUE == _h) ? FALSE : TRUE;
}

void CVhdMgr::Disconnect()
{
    if(_h != INVALID_HANDLE_VALUE) {
        CloseHandle(_h);
        _h = INVALID_HANDLE_VALUE;
    }
}

BOOL CVhdMgr::CreateVhd(_In_ const std::wstring& file)
{
    if(!InterCreateVhd(file, 2046)) {
        return FALSE;
    }

    printf("Vhd has been created!\n");
    return TRUE;
}

BOOL CVhdMgr::LoadVhd(_In_ const std::wstring& file, _In_ WCHAR drive, _Out_ CVhdInfo& vhdi)
{
    if(_h == INVALID_HANDLE_VALUE) {
        printf("Error: Not connect to NextLabs VHD Service\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    NXRMVHDMOUNTDRIVE   mount = {0};
    NXRMVHDINFO         vhdinf = {0};
    ULONG               outsize = 0;

    memset(&vhdinf, 0, sizeof(vhdinf));
    memset(&mount, 0, sizeof(mount));
	mount.BytesPerSector = VHDFMT_SECTOR_SIZE;
	mount.Removable = FALSE;
	mount.Visible = (0 == drive) ? FALSE : TRUE;
	mount.PreferredDriveLetter = drive;
    memcpy(mount.Key, DEFAULT_DISKKEY, 32);
    if (!boost::algorithm::istarts_with(file, L"\\??\\")) {
        wcsncpy_s(mount.HostFileName, NXRMVHD_MAX_PATH, L"\\??\\", _TRUNCATE);
        wcsncat_s(mount.HostFileName, NXRMVHD_MAX_PATH, file.c_str(), _TRUNCATE);
    }
    else {
        wcsncpy_s(mount.HostFileName, NXRMVHD_MAX_PATH, file.c_str(), _TRUNCATE);
    }

    if(!DeviceIoControl(_h, IOCTL_NXRMVHD_MOUNT_DISK, &mount, sizeof(mount), &vhdinf, sizeof(vhdinf), &outsize, NULL)) {
        return FALSE;
    }

    if (mount.Visible) {
        DWORD dwResult = 0;
        DEV_BROADCAST_VOLUME dbv = { 0 };
        dbv.dbcv_size = sizeof(DEV_BROADCAST_VOLUME);
        dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
        dbv.dbcv_reserved = 0;
        dbv.dbcv_unitmask = (1 << (drive - L'A'));
        dbv.dbcv_flags = 0;
        SendMessageTimeoutW(HWND_BROADCAST, WM_DEVICECHANGE, DBT_DEVICEARRIVAL, (LPARAM)(&dbv), SMTO_ABORTIFHUNG, 200, &dwResult);
        SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, L"K:", NULL);
    }
    
    assert(outsize == sizeof(vhdinf));
    VhdS2O(&vhdinf, vhdi);
    assert(vhdi.IsValid());

    return TRUE;
}

BOOL CVhdMgr::UnloadVhd(_In_ ULONG id)
{
    ULONG   ret = 0;
    ULONG   outsize = 0;

    if(_h == INVALID_HANDLE_VALUE) {
        printf("Error: Not connect to NextLabs VHD Service\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CVhdInfo vhdi;
    if (!QueryVhd(id, vhdi)) {
        printf("Error: Fail to get specified VHD (id = %d) (err = %d)\n", id, GetLastError());
        return FALSE;
    }

    if(!DeviceIoControl(_h, IOCTL_NXRMVHD_UNMOUNT_DISK, &id, sizeof(ULONG), &ret, sizeof(ULONG), &outsize, NULL)) {
        printf("Error: Fail to send control code (unload vhd - %d) to NextLabs VHD Service (%d)\n", id, GetLastError());
        return FALSE;
    }

    if(0 != ret) {
        SetLastError(ret);
        return FALSE;
    }

    printf("VHD %d\n", id);

    if (vhdi.IsVisible()) {
        DWORD dwResult = 0;
        DEV_BROADCAST_VOLUME dbv = { 0 };
        const WCHAR cchDrive = vhdi.GetDriveLetter();
        dbv.dbcv_size = sizeof(DEV_BROADCAST_VOLUME);
        dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
        dbv.dbcv_reserved = 0;
        dbv.dbcv_unitmask = (1 << (cchDrive - L'A'));
        dbv.dbcv_flags = 0;
        SendMessageTimeoutW(HWND_BROADCAST, WM_DEVICECHANGE, DBT_DEVICEREMOVECOMPLETE, (LPARAM)(&dbv), SMTO_ABORTIFHUNG, 200, &dwResult);
        WCHAR wzDrive[3] = { cchDrive, L':', 0 };
        SHChangeNotify(SHCNE_DRIVEREMOVED, SHCNF_PATH, wzDrive, NULL);
        printf("VHD %d (%S) has been removed\n", id, wzDrive);
    }
    else {
        printf("VHD %d has been removed\n", id);
    }

    return TRUE;
}

BOOL CVhdMgr::UnloadAllVhds()
{
    if(_h == INVALID_HANDLE_VALUE) {
        printf("Error: Not connect to NextLabs VHD Service\n");
        return FALSE;
    }

    return UnloadVhd((ULONG)-1);
}

BOOL CVhdMgr::QueryVhd(_In_ ULONG id, _Out_ CVhdInfo& vhdi)
{
    NXRMVHDINFO         vhdinf = {0};
    ULONG               outsize = 0;


    if(_h == INVALID_HANDLE_VALUE) {
        printf("Error: Not connect to NextLabs VHD Service\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(id >= 9999) {
        SetLastError(ERROR_INVALID_PARAMETER);
        printf("Error: Id exceeds max number\n");
        return FALSE;
    }

    memset(&vhdinf, 0, sizeof(vhdinf));

    if(!DeviceIoControl(_h, IOCTL_NXRMVHD_QUERY_DISK, &id, sizeof(ULONG), &vhdinf, sizeof(vhdinf), &outsize, NULL)) {
        printf("Error: Fail to send control code (get vhd inf) to NextLabs VHD Service (%d)\n", GetLastError());
        return FALSE;
    }
    if(outsize == 0) {
        SetLastError(ERROR_NOT_FOUND);
        printf("Error: Vhd not found\n");
        return FALSE;
    }
    
    VhdS2O(&vhdinf, vhdi);
    if(!vhdi.IsValid()) {
        printf("Error: Vhd information is invalid\n");
        SetLastError(ERROR_NOT_FOUND);
        return FALSE;
    }

    return TRUE;
}

BOOL CVhdMgr::QueryAllVhds(_Out_ std::vector<CVhdInfo>& vhdis)
{
    ULONG   id = -1;
    ULONG   retsize = 0;
    ULONG   count = 0;
    std::vector<UCHAR>  buf;
    ULONG               outsize = 0;
    NXRMVHDINFOS*       vhdinfs = NULL;

    if(_h == INVALID_HANDLE_VALUE) {
        printf("Error: Not connect to NextLabs VHD Service\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(!DeviceIoControl(_h, IOCTL_NXRMVHD_QUERY_DISK, &id, sizeof(ULONG), &count, sizeof(ULONG), &retsize, NULL)) {
        printf("Error: Fail to send control code (get vhd count) to NextLabs VHD Service (%d)\n", GetLastError());
        return FALSE;
    }
    
    vhdis.clear();
    if(count == 0) {
        return TRUE;
    }

    outsize = sizeof(NXRMVHDINFOS) + sizeof(NXRMVHDINFO)*(count-1);
    buf.resize(outsize, 0);
    vhdinfs = (NXRMVHDINFOS*)(&buf[0]);
    if(!DeviceIoControl(_h, IOCTL_NXRMVHD_QUERY_DISK, &id, sizeof(ULONG), vhdinfs, outsize, &retsize, NULL)) {
        printf("Error: Fail to send control code (get vhds infs) to NextLabs VHD Service (%d)\n", GetLastError());
        return FALSE;
    }

    for(int i=0; i<(int)vhdinfs->Count; i++) {
        CVhdInfo    vhdi;
        VhdS2O(&vhdinfs->Infs[i], vhdi);
        if(vhdi.IsValid()) {
            vhdis.push_back(vhdi);
        }
    }

    return TRUE;
}

BOOL CVhdMgr::InterCreateVhd(_In_ const std::wstring& file, _In_ ULONG size_in_mb)
{
    HANDLE h = INVALID_HANDLE_VALUE;
    VHDFILEHEADER header = {0};
    DWORD dwWritten = 0;

    if(size_in_mb > 2046) {
        printf("Error: Disk space is too large\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    // set header
    memset(&header, 0, sizeof(header));
    header.Magic = VHDFMT_FILE_MAGIC;
    header.Version = VHDFMT_FILE_VERSION;
    memset(header.UniqueId, 0, 64);
    header.DiskSpace = ((LONGLONG)size_in_mb) * 0x100000;
    memset(header.KeyId, 0, 64);
    memcpy(header.KeyBlob, DEFAULT_DISKKEY, 32);

    // Create File
    h = ::CreateFileW(file.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == h) {
        printf("Error: Fail to create file\n");
        return FALSE;
    }

    // Allocate
    ::SetFilePointer(h, VHDFMT_BLOCK_SIZE, NULL, FILE_BEGIN);
    if(!::SetEndOfFile(h)) {
        printf("Error: Fail to allocate vhd file header\n");
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
        return FALSE;
    }
    ::SetFilePointer(h, 0, NULL, FILE_BEGIN);

    // Write File Header
    if(!::WriteFile(h, &header, sizeof(header), &dwWritten, NULL)) {
        printf("Error: Fail to initialize vhd file header\n");
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    CloseHandle(h);
    h = INVALID_HANDLE_VALUE;
    return TRUE;
}