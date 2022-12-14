

#ifndef __NUDF_SHARE_ENGINE_CTL_H__
#define __NUDF_SHARE_ENGINE_CTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NXRM_ENGINE_IPC_NAME    L"NXRM_ENGINE_IPC_0"


#define NXRM_ENGINE_ID          0x0001
#define ENGINE_CTL_QUERYSTATUS  ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_STOP         ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x001, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_START        ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x002, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_PAUSE        ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x003, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_CONTINUE     ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x004, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_PROTECT      ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x005, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_UPDATE       ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x006, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_DBGCTL       ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x007, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define ENGINE_CTL_DBGCOLLECT   ((ULONG)CTL_CODE(NXRM_ENGINE_ID, 0x008, METHOD_BUFFERED, FILE_ANY_ACCESS))


#define NXRM_AUTHN_ENGINE_ID    0x0002
#define AUTHNENGINE_CTL_LOGON   ((ULONG)CTL_CODE(NXRM_AUTHN_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define AUTHNENGINE_CTL_LOGOFF  ((ULONG)CTL_CODE(NXRM_AUTHN_ENGINE_ID, 0x001, METHOD_BUFFERED, FILE_ANY_ACCESS))


#define NXRM_AUTHZ_ENGINE_ID    0x0003
#define AUTHZENGINE_CTL_EVAL    ((ULONG)CTL_CODE(NXRM_AUTHZ_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_READ_ACCESS))
#define AUTHZENGINE_CTL_UPDATE  ((ULONG)CTL_CODE(NXRM_AUTHZ_ENGINE_ID, 0x001, METHOD_BUFFERED, FILE_ANY_ACCESS))


#define NXRM_KEY_ENGINE_ID      0x0004
#define KEYENGINE_CTL_GETKEY    ((ULONG)CTL_CODE(NXRM_KEY_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_READ_ACCESS))
#define KEYENGINE_CTL_REVOKEKEY ((ULONG)CTL_CODE(NXRM_KEY_ENGINE_ID, 0x001, METHOD_BUFFERED, FILE_WRITE_ACCESS))
#define KEYENGINE_CTL_DISPKEY   ((ULONG)CTL_CODE(NXRM_KEY_ENGINE_ID, 0x003, METHOD_BUFFERED, FILE_WRITE_ACCESS))


#define NXRM_AUDIT_ENGINE_ID    0x0005
#define AUDITENGINE_CTL_AUDIT   ((ULONG)CTL_CODE(NXRM_AUDIT_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_WRITE_ACCESS))


#define NXRM_HOOK_ENGINE_ID     0x0006
#define HOOKENGINE_CTL_STOP     ((ULONG)CTL_CODE(NXRM_HOOK_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_ANY_ACCESS))
#define HOOKENGINE_CTL_START    ((ULONG)CTL_CODE(NXRM_HOOK_ENGINE_ID, 0x001, METHOD_BUFFERED, FILE_ANY_ACCESS))


#define NXRM_VHD_ENGINE_ID      0x0007
#define VHDENGINE_CTL_SHOW      ((ULONG)CTL_CODE(NXRM_VHD_ENGINE_ID, 0x000, METHOD_BUFFERED, FILE_READ_ACCESS))
#define VHDENGINE_CTL_HIDE      ((ULONG)CTL_CODE(NXRM_VHD_ENGINE_ID, 0x001, METHOD_BUFFERED, FILE_READ_ACCESS))
#define VHDENGINE_CTL_LIST      ((ULONG)CTL_CODE(NXRM_VHD_ENGINE_ID, 0x002, METHOD_BUFFERED, FILE_READ_ACCESS))


#define NXRM_TRAYAPP_ID                 0x0008
#define TRAYAPP_CTL_NOTIFY              ((ULONG)CTL_CODE(NXRM_TRAYAPP_ID, 0x001, METHOD_BUFFERED, FILE_WRITE_ACCESS))
#define TRAYAPP_CTL_QUIT                ((ULONG)CTL_CODE(NXRM_TRAYAPP_ID, 0x002, METHOD_BUFFERED, FILE_WRITE_ACCESS))
#define TRAYAPP_CTL_SET_TIMESTAMP       ((ULONG)CTL_CODE(NXRM_TRAYAPP_ID, 0x003, METHOD_BUFFERED, FILE_WRITE_ACCESS))
#define TRAYAPP_CTL_SET_SERVER_STATUS   ((ULONG)CTL_CODE(NXRM_TRAYAPP_ID, 0x004, METHOD_BUFFERED, FILE_WRITE_ACCESS))




typedef struct _RM_REQUEST_HEADER {
    ULONG   Size;
    ULONG   CtlCode;
    ULONG   RequestorPid;
} RM_REQUEST_HEADER, *PRM_REQUEST_HEADER;
typedef const RM_REQUEST_HEADER* PCRM_REQUEST_HEADER;

typedef struct _RM_REPLY_HEADER {
    ULONG   Size;
    ULONG   CtlCode;
    LONG    Result;
} RM_REPLY_HEADER, *PRM_REPLY_HEADER;
typedef const RM_REPLY_HEADER* PCRM_REPLY_HEADER;


//
//  Engine Request/Reply Structs
//

typedef struct _RM_ENGINE_REQUEST {
    RM_REQUEST_HEADER Header;
} RM_ENGINE_REQUEST, *PRM_ENGINE_REQUEST;

typedef struct _RM_ENGINE_REPLY {
    RM_REPLY_HEADER Header;
} RM_ENGINE_REPLY, *PRM_ENGINE_REPLY;


//
//  Query/Set Debug Level Request/Reply Structs
//

typedef struct _RM_SETDEBUG_REQUEST {
    RM_REQUEST_HEADER Header;
    ULONG             LogLevel;
} RM_SETDEBUG_REQUEST, *PRM_SETDEBUG_REQUEST;

typedef struct _RM_SETDEBUG_REPLY {
    RM_REPLY_HEADER Header;
    ULONG           LogLevel;
} RM_SETDEBUG_REPLY, *PRM_SETDEBUG_REPLY;

//
//  Query/Set Status Request/Reply Structs
//

typedef struct _RM_QUERYSTATUS_REQUEST {
    RM_REQUEST_HEADER Header;
} RM_QUERYSTATUS_REQUEST, *PRM_QUERYSTATUS_REQUEST;

typedef struct _RM_ENGINE_STATUS {
    ULONG       State;
    ULONG       LogLevel;
    ULONG       Connected;
    FILETIME    PolicyTimestamp;
    FILETIME    LastUpdateTime;
    WCHAR       ProductVersion[64];
    WCHAR       CurrentServer[64];
    WCHAR       CurrentUser[64];
} RM_ENGINE_STATUS, *PRM_ENGINE_STATUS;

typedef struct _RM_QUERYSTATUS_REPLY {
    RM_REPLY_HEADER  Header;
    RM_ENGINE_STATUS Status;
} RM_QUERYSTATUS_REPLY, *PRM_QUERYSTATUS_REPLY;



//
//  Authentication Request/Reply Structs
//

typedef struct _RM_AUTHN_REQUEST {
    RM_REQUEST_HEADER Header;
    union {
        struct {
            WCHAR   Name[64];
            WCHAR   Domain[64];
            WCHAR   Password[64];
        } Logon;
        struct {
            ULONG   Reserved;
        } Logoff;
    } DUMMYUNIONNAME;
} RM_AUTHN_REQUEST, *PRM_AUTHN_REQUEST;

typedef struct _RM_AUTHN_REPLY {
    RM_REPLY_HEADER Header;
} RM_AUTHN_REPLY, *PRM_AUTHN_REPLY;


//
//  Authorization Request/Reply Structs
//

typedef struct _RM_AUTHZ_REQUEST {
    RM_REQUEST_HEADER Header;
    union {
        struct {
            GUID    TokenId;
            ULONG   AppId;
            WCHAR   AppPath[MAX_PATH];
            WCHAR   Resource[MAX_PATH];
        } QueryInternalRights;
    } DUMMYUNIONNAME;
} RM_AUTHZ_REQUEST, *PRM_AUTHZ_REQUEST;

typedef struct _RM_AUTHZ_REPLY {
    RM_REPLY_HEADER Header;
    union {
        struct {
            LONGLONG    ExpireTime;
            LONGLONG    IssueTime;
            WCHAR       Issuer[64];
            ULONGLONG   InternalRights;
            ULONGLONG   Rights;
        } Apply;
    } DUMMYUNIONNAME;
} RM_AUTHZ_REPLY, *PRM_AUTHZ_REPLY;




//
//  TrayApp Request/Reply Structs
//

typedef struct _RM_TRAYAPP_REQUEST {
    RM_REQUEST_HEADER Header;
    union {
        struct {
            WCHAR   InfoTitle[64];
            WCHAR   Info[256];
        } Notify;
        struct {
            ULONG   Reserved;
        } Exit;
        struct {
            FILETIME    PolicyTimestamp;
            FILETIME    UpdateTimestamp;
        } SetTimeStamp;
        struct {
            BOOL        Connected;
            WCHAR       Server[128];
        } SetServerStatus;
    } DUMMYUNIONNAME;
} RM_TRAYAPP_REQUEST, *PRM_TRAYAPP_REQUEST;

typedef struct _RM_TRAYAPP_REPLY {
    RM_REPLY_HEADER Header;
} RM_TRAYAPP_REPLY, *PRM_TRAYAPP_REPLY;



#ifdef __cplusplus
}
#endif


#endif  // #ifndef __NUDF_SHARE_ENGINE_CTL_H__