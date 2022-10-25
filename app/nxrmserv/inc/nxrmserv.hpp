

#ifndef __NXRM_SERV_HPP__
#define __NXRM_SERV_HPP__


#define NXRMS_SERVICE_NAME          L"nxrmserv"
#define NXRMS_SERVICE_DISPLAY_NAME  L"NextLabs Rights Management Service"
#define NXRMS_SERVICE_DESCRIPTION   L"NextLabs Rights Management Service"
#define NXRMS_SERVICE_KEY           L"SYSTEM\\CurrentControlSet\\services\\nxrmserv"
#define NXRMS_SERVICE_KEY_PARAMETER L"SYSTEM\\CurrentControlSet\\services\\nxrmserv\\Parameters"
#define NXRMS_SERVICE_APPID_STR     L"{EB898211-0949-4D3E-A9B7-172F1FA61F4C}"

extern const GUID NxServAppGuid;


#define RMS_CONF_DELAY_SECONDS      L"DelaySeconds"
#define RMS_CONF_LOG_LEVEL          L"LogLevel"
#define RMS_CONF_LOG_SIZE           L"LogSize"

#define RMS_DEFAULT_LOGLEVEL        13  // LL_DEBUG
#define RMS_DEFAULT_LOGSIZE         5


#endif