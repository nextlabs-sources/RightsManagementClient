

#pragma once
#ifndef __NXRMS_DAEMON_HPP__
#define __NXRMS_DAEMON_HPP__



#define NXRMS_SERVICE_NAME          L"nxkmsd"
#define NXRMS_SERVICE_DISPLAY_NAME  L"NextLabs Key Management Server"
#define NXRMS_SERVICE_DESCRIPTION   L"NextLabs Key Management Daemon"
#define NXRMS_SERVICE_KEY           L"SYSTEM\\CurrentControlSet\\services\\nxkmsd"
#define NXRMS_SERVICE_KEY_PARAMETER L"SYSTEM\\CurrentControlSet\\services\\nxkmsd\\Parameter"
#define NXRMS_SERVICE_APP_ID        L"{254744FF-5252-414A-A31E-2D7B0122E345}"


#define KMS_CONF_HTTPS_ENABLED      L"HttpsEnabled"
#define KMS_CONF_PORT               L"ServerPort"
#define KMS_CONF_CONTROL_PORT       L"ControlPort"
#define KMS_CONF_CONTROL_REMOTE     L"RemoteControl"
#define KMS_CONF_DELAY_SECONDS      L"DelaySeconds"
#define KMS_CONF_LOG_LEVEL          L"DefaultLogLevel"

extern const GUID KmsAppGuid;



#endif