

#include <Windows.h>

#include <string>
#include <vector>

#include <nudf\exception.hpp>
#include <nxrmrest\nxrmrest.hpp>

//    "<?xml version=\"1.0\" ?> \

#define REGISTER_XML    \
    "<RegisterAgentRequest> \
         <RegistrationData> \
             <host>KUNLUN.nextlabs.com</host> \
             <type>DESKTOP</type> \
             <version> \
                <major>8</major> \
                <minor>0</minor> \
                <maintenance>0</maintenance> \
                <patch>0</patch> \
                <build>9999</build> \
            </version> \
            <osInformation> \
               <osType>Windows</osType> \
               <osVersion>8.1</osVersion> \
            </osInformation> \
            <hardwareDetails> \
               <cpu>Intel(R) Core(TM) i7-4790 CPU @ 3.60 GHz</cpu> \
               <memoryAmount>16</memoryAmount> \
               <memoryUnits>GB</memoryUnits> \
               <macAddressList>EC-0E-C4-6A-23-99,34-17-EB-E0-45-DF</macAddressList> \
               <deviceId>0</deviceId> \
            </hardwareDetails> \
            <groupInformation> \
               <groupId>0</groupId> \
               <groupName>SMDC</groupName> \
            </groupInformation> \
            <tenantId>smdc</tenantId> \
        </RegistrationData> \
    </RegisterAgentRequest>"


void CltRegister()
{
    HRESULT hr = S_OK;
    nudf::http::CSession session(L"NXRMC/8.0");;
    std::vector<UCHAR> buf;
    DWORD dwBufSize = 4096;
    WINHTTP_CERTIFICATE_INFO* certinfo = NULL;

    buf.resize(4096, 0);

    hr = session.Open();
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }
    else {

        // https://10.63.0.152:8443/RMS/service/RegisterAgent
        nxrmrest::CRegisterRequest request(L"/RMS/service/RegisterAgent");
        //nxrmrest::CRegisterRequest request(L"/post.php");

        //request.SetPath(L"/RMS/service/RegisterAgent");
        request.SetSecure(true);

        hr = request.Initialize(session, L"10.63.0.152", 8443);
        //hr = request.Initialize(session, L"posttestserver.com", 443);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        
        try {

            nudf::http::CResponse response;

            hr = request.Send(REGISTER_XML, (ULONG)strlen(REGISTER_XML) + 1);
            if(FAILED(hr)) {
                throw WIN32ERROR2(hr);
            }

            hr = request.GetResponse(response);
            if(FAILED(hr)) {
                throw WIN32ERROR2(hr);
            }

            const std::vector<std::wstring>& headers = response.GetHeaders();
            printf("\n");
            printf("Headers:\n");
            for(std::vector<std::wstring>::const_iterator it=headers.begin(); it!=headers.end(); ++it) {
                printf("  %S\n", (*it).c_str());
            }
            printf("\n");
            printf("Content:\n%s\n", response.GetContent().c_str());
        }
        catch(const nudf::CException& e) {
            printf("Exception: err=%d, function=%s, line=%d, file=%s\n", e.GetCode(), e.GetFunction(), e.GetLine(), e.GetFile());
        }

        request.Close();
    }


    system("Pause");
}


