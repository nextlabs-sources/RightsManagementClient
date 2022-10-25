

#include <Windows.h>
#include <assert.h>
#include <iphlpapi.h>

#include <nudf\exception.hpp>
#include <nudf\registry.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>
#include <nudf\regex.hpp>
#include <nudf\host.hpp>
#include <nudf\convert.hpp>
#include <nudf\encoding.hpp>
#include <nudf\cert.hpp>


#include "nxrmeng.h"
#include "engine.hpp"
#include "diagnose.hpp"
#include "nxrminfo.hpp"
#include "rmsutil.hpp"


using namespace nxrm::engine;


#define AGENT_NAME      L"NXRMC/8.0"
#define HEADER_KEY      L"X-NXL-S-KEY"
#define HEADER_CERT     L"X-NXL-S-CERT"


#define TEST_REGISTER_RESPONSE  \
    L"<RegisterAgentResponse>"\
        L"<StartupConfiguration>"\
            L"<id>1000</id>"\
            L"<CommProfile>"\
                L"<id>1</id>"\
                L"<name>Default Comm Profile</name>"\
                L"<defaultProfile>true</defaultProfile>"\
                L"<createdDate>2015-04-26T16:48:51.324+08:00</createdDate>"\
                L"<modifiedDate>2015-04-26T16:48:51.324+08:00</modifiedDate>"\
                L"<DABSLocation>https://10.63.0.152:8443/RMS/service/HeartBeat</DABSLocation>"\
                L"<agentType>DESKTOP</agentType>"\
                L"<heartBeatFrequency>"\
                    L"<time>1</time>"\
                    L"<timeUnit>hours</timeUnit>"\
                L"</heartBeatFrequency>"\
                L"<logLimit >5</logLimit >"\
                L"<logFrequency>"\
                    L"<time>10</time>"\
                    L"<timeUnit>minutes</timeUnit>"\
                L"</logFrequency>"\
                L"<passwordHash>EEFCBAB37F049DA6514BC1B16422CF28</passwordHash>"\
                L"<currentActivityJournalingSettings>"\
                    L"<name>Default</name>"\
                    L"<loggedActivities>OPEN</loggedActivities>"\
                L"</currentActivityJournalingSettings>"\
                L"<customActivityJournalingSettings>"\
                    L"<name>Customized</name>"\
                    L"<loggedActivities>OPEN</loggedActivities>"\
                L"</customActivityJournalingSettings>"\
            L"</CommProfile>"\
            L"<AgentProfile>"\
                L"<id>1</id>"\
                L"<name>Default Agent Profile</name>"\
                L"<defaultProfile>true</defaultProfile>"\
                L"<createdDate>2015-04-26T16:48:51.324+08:00</createdDate>"\
                L"<modifiedDate>2015-04-26T16:48:51.324+08:00</modifiedDate>"\
                L"<hookAllProc>true</hookAllProc>"\
                L"<hookSystemProc>true</hookSystemProc>"\
                L"<logViewingEnabled>true</logViewingEnabled>"\
                L"<trayIconEnabled>true</trayIconEnabled>"\
            L"</AgentProfile>"\
            L"<ClassificationProfile timestamp=\"2015-04-26T16:48:51.324+08:00\">"\
                L"<Classify>"\
	                L"<TopLevel>0,1</TopLevel>"\
	                L"<LabelList>"\
		                L"<Label id=\"0\" name=\"Sensitivity\" display-name=\"Sensitivity\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
			                L"<VALUE priority=\"0\" value=\"Non Business\" />"\
			                L"<VALUE priority=\"1\" value=\"General Business\" />"\
			                L"<VALUE priority=\"2\" value=\"Proprietary\" />"\
			                L"<VALUE priority=\"3\" value=\"Confidential\" />"\
		                L"</Label>"\
		                L"<Label id=\"1\" name=\"Program\" display-name=\"Program\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
			                L"<VALUE value=\"PR-01\" sub-label=\"2\"/>"\
			                L"<VALUE value=\"PR-02\" sub-label=\"2\"/>"\
			                L"<VALUE value=\"PR-03\" sub-label=\"2\"/>"\
		                L"</Label>"\
		                L"<Label id=\"2\" name=\"Jurisdiction\" display-name=\"Jurisdiction\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
			                L"<VALUE value=\"ITAR\" sub-label=\"3\"/>"\
			                L"<VALUE value=\"EAR\" sub-label=\"4\"/>"\
			                L"<VALUE value=\"BAFA\" sub-label=\"5\"/>"\
		                L"</Label>"\
		                L"<Label id=\"3\" name=\"ITAR\" display-name=\"ITAR Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
			                L"<VALUE value=\"TAA-01\" />"\
			                L"<VALUE value=\"TAA-02\" />"\
			                L"<VALUE value=\"TAA-03\" />"\
			                L"<VALUE value=\"TAA-04\" />"\
		                L"</Label>"\
		                L"<Label id=\"4\" name=\"EAR\" display-name=\"EAR Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
			                L"<VALUE value=\"EAR-01\" />"\
			                L"<VALUE value=\"EAR-02\" />"\
			                L"<VALUE value=\"EAR-03\" />"\
			                L"<VALUE value=\"EAR-04\" />"\
		                L"</Label>"\
		                L"<Label id=\"5\" name=\"BAFA\" display-name=\"BAFA Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
			                L"<VALUE value=\"BAFA-01\" />"\
			                L"<VALUE value=\"BAFA-02\" />"\
			                L"<VALUE value=\"BAFA-03\" />"\
			                L"<VALUE value=\"BAFA-04\" />"\
		                L"</Label>"\
	                L"</LabelList>"\
                L"</Classify>"\
            L"</ClassificationProfile>"\
            L"<registrationId>2000</registrationId>"\
        L"</StartupConfiguration>"\
        L"<certificate>"\
            L"-----BEGIN CERTIFICATE-----\
            MIIFZTCCBE2gAwIBAgIQPeH7li+IB9t5LiFqywEA6zANBgkqhkiG9w0BAQUFADCB\
            tDELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\
            ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTswOQYDVQQLEzJUZXJtcyBvZiB1c2Ug\
            YXQgaHR0cHM6Ly93d3cudmVyaXNpZ24uY29tL3JwYSAoYykxMDEuMCwGA1UEAxMl\
            VmVyaVNpZ24gQ2xhc3MgMyBDb2RlIFNpZ25pbmcgMjAxMCBDQTAeFw0xMzAzMDUw\
            MDAwMDBaFw0xNjA0MDMyMzU5NTlaMIGoMQswCQYDVQQGEwJVUzETMBEGA1UECBMK\
            Q2FsaWZvcm5pYTESMBAGA1UEBxMJU2FuIE1hdGVvMRcwFQYDVQQKFA5OZXh0TGFi\
            cywgSW5jLjE+MDwGA1UECxM1RGlnaXRhbCBJRCBDbGFzcyAzIC0gTWljcm9zb2Z0\
            IFNvZnR3YXJlIFZhbGlkYXRpb24gdjIxFzAVBgNVBAMUDk5leHRMYWJzLCBJbmMu\
            MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzTmVSLGLcYZNT71XMkr6\
            LJYfFhDC9hlx98ySBigepxdwYKWfccgtV76WMvmmicnZof9g78PSydLJ4eoB9dWq\
            QwtpiTMadcr9CWy6a0CAEyNMlZcZ6o7cuyAguL75nnvITVslccEJ8yNWuber2x+3\
            lPnuiL3luGk70pz8TPAjHH1Ctw5f6fzgLVjnCqbbWzn+ELjuZXZOk6V/bOwX3sP3\
            e3h4iFpU3k6y0EjAkaKtrGJCJoTZFKL6dOA3S/theF82ZX7Fx5SXYIBfvt2NdiCv\
            hJdCc/pK1ya5LnoZwraoku6joZ8YWlmeaNCYKc3snrHaba1mr9HOd4r5VZ/qfdJo\
            QwIDAQABo4IBezCCAXcwCQYDVR0TBAIwADAOBgNVHQ8BAf8EBAMCB4AwQAYDVR0f\
            BDkwNzA1oDOgMYYvaHR0cDovL2NzYzMtMjAxMC1jcmwudmVyaXNpZ24uY29tL0NT\
            QzMtMjAxMC5jcmwwRAYDVR0gBD0wOzA5BgtghkgBhvhFAQcXAzAqMCgGCCsGAQUF\
            BwIBFhxodHRwczovL3d3dy52ZXJpc2lnbi5jb20vcnBhMBMGA1UdJQQMMAoGCCsG\
            AQUFBwMDMHEGCCsGAQUFBwEBBGUwYzAkBggrBgEFBQcwAYYYaHR0cDovL29jc3Au\
            dmVyaXNpZ24uY29tMDsGCCsGAQUFBzAChi9odHRwOi8vY3NjMy0yMDEwLWFpYS52\
            ZXJpc2lnbi5jb20vQ1NDMy0yMDEwLmNlcjAfBgNVHSMEGDAWgBTPmanqeyb0S8mO\
            j9fwBSbv49KnnTARBglghkgBhvhCAQEEBAMCBBAwFgYKKwYBBAGCNwIBGwQIMAYB\
            AQABAf8wDQYJKoZIhvcNAQEFBQADggEBAH6LVac9a6DzxaRZ2TobzXgIfpG6RDWy\
            gWsS66LzxHY5zV0G7VzYev1jHzw56iCMNj46L3n5FsVI/jEeDE1q0Ym5L/McFUBG\
            Ff3m4WKVzjckb8Y5UQpjPYYQALGnwTKqQUG8bCDeg9jSUe9kd5gtb3hrcMiqDSGV\
            G9mq7nK8nl8hoWYkah7xkojQ19IPV2PGWFZcT+CKOQ6WUVIzdsZuQ4BacpxQNPHU\
            DCPN8J0qHP6FXX8xPX43zg3rqGCMMKILzrSFn40IqUMxPBeE83YRxfbt+4pzr9Sw\
            KMs1IDewCgQNM9I6KTV3DONNaN9B4gMz71kYnVyZSTs2PMHSxe9lWrQ=\
            -----END CERTIFICATE-----"\
        L"</certificate>"\
    L"</RegisterAgentResponse>"

//
//  CRmsServer
//

CRmsServer& CRmsServer::operator = (const CRmsServer& server) throw()
{
    if(this != &server) {
        _server = server.GetServer();
        _path = server.GetServicePath();
        _port = server.GetPort();
    }
    return *this;
}

void CRmsServer::Load(_In_ const std::wstring& conf)
{
    try {

        nudf::util::CXmlDocument doc;
        CComPtr<IXMLDOMElement>  spRoot;
        doc.LoadFromFile(conf.c_str());
        if(!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR();
        }

        OnLoad(spRoot);
    }
    catch(const nudf::CException& e) {
        _server.clear();
        _port = 0;
        throw e;
    }
}

void CRmsServer::OnLoad(_In_ IXMLDOMElement* pRoot)
{
    CComPtr<IXMLDOMNode>     spServer;
    CComPtr<IXMLDOMNode>     spPort;
    int                      nPort = 8443;
    std::wstring             sUrl;
    std::wstring             sServerPort;


    if(!nudf::util::XmlUtil::FindChildElement(pRoot, L"SERVER", &spServer)) {
        throw WIN32ERROR();
    }
    sUrl = nudf::util::XmlUtil::GetNodeText(spServer);
    if(sUrl.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    std::wstring::size_type pos = sUrl.find_first_of(L"://");
    if(std::wstring::npos != pos) {
        sUrl = sUrl.substr(pos+3);
    }

    pos = sUrl.find_first_of(L"/");
    if(std::wstring::npos != pos) {
        sServerPort = sUrl.substr(0, pos);
        sUrl = sUrl.substr(pos);
    }
    else {
        sServerPort = sUrl;
        sUrl = L"/";
    }

    pos = sServerPort.find_last_of(L":");
    if(std::wstring::npos != pos) {
        _server = sServerPort.substr(0, pos);
        std::wstring wsPort = sServerPort.substr(pos+1);
        int nPort = 8443;
        if(!wsPort.empty() && nudf::string::ToInt<wchar_t>(wsPort, &nPort) && nPort > 0) {
            _port = (USHORT)nPort;
        }
        else {
            _port = 8443;
        }
    }
    else {
        _server = sServerPort;
        _port = 8443;
    }

    if(L'/' != sUrl.c_str()[sUrl.length()-1]) {
        sUrl += L"/";
    }
    _path = sUrl;
}


CRmsRegisterServer& CRmsRegisterServer::operator = (const CRmsRegisterServer& server) throw()
{
    if(this != &server) {
        CRmsServer::operator=(server);
        _groupId = server.GetGroupId();
        _groupName = server.GetGroupName();
        _tenantId = server.GetTenantId();
        _cert = server.GetCert();
    }
    return *this;
}

void CRmsRegisterServer::OnLoad(_In_ IXMLDOMElement* pRoot)
{
    CComPtr<IXMLDOMNode>     spGroupId;
    CComPtr<IXMLDOMNode>     spGroupName;
    CComPtr<IXMLDOMNode>     spTenantId;
    CComPtr<IXMLDOMNode>     spCert;

    CRmsServer::OnLoad(pRoot);

    if(nudf::util::XmlUtil::FindChildElement(pRoot, L"GROUPID", &spGroupId)) {
        _groupId = nudf::util::XmlUtil::GetNodeText(spGroupId);
    }

    if(nudf::util::XmlUtil::FindChildElement(pRoot, L"GROUPNAME", &spGroupName)) {
        _groupName = nudf::util::XmlUtil::GetNodeText(spGroupName);
    }

    if(nudf::util::XmlUtil::FindChildElement(pRoot, L"TENANTID", &spTenantId)) {
        _tenantId = nudf::util::XmlUtil::GetNodeText(spTenantId);
    }

    if(nudf::util::XmlUtil::FindChildElement(pRoot, L"CERTIFICATE", &spCert)) {
        _cert = nudf::util::XmlUtil::GetNodeText(spCert);
    }
    else {
        _cert = L"MIIDmzCCAoOgAwIBAgIEI8+FCTANBgkqhkiG9w0BAQsFADB+MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MR8wHQYDVQQDExZSaWdodHMgTWFuYWdlbWVudCBUZW1wMB4XDTE1MDUyNzE0MDI0MVoXDTI1MDUyNDE0MDI0MVowfjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIwEAYDVQQHEwlTYW4gTWF0ZW8xETAPBgNVBAoTCE5leHRMYWJzMRowGAYDVQQLExFSaWdodHMgTWFuYWdlbWVudDEfMB0GA1UEAxMWUmlnaHRzIE1hbmFnZW1lbnQgVGVtcDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPUFAKjEmwL/oWQpafK5vOz8/dOEf9mmcjKawhxNhJJ5R+4olKHF9ZaKMve542ehSKngA53buaPsvP96ix7j8O8E2DHnYqbR5I9jNfZarIWjAwfO94TDvQovuIFBCWMnJXsRUtVm36cF6WpQdqctIbgvSbjGbgYacqGZ6QaaWrySGxupjyB8lfD6dYBG5lXFRcFA7QQVbNAGM7Xis2S3sPZOch4VJK7faX2xRyW6sIKL0FU8W9HCbm2PjG+XBr+dmsP3lk6HOqlSEy55HRYldMI/KCSlTGIcUHjH0qpiBxceSHILgY+YOqJ3l6/d8k9ui3MK2XGUhNFgwwLYFruk1l8CAwEAAaMhMB8wHQYDVR0OBBYEFGKcZJB9ZVJ6q/T2DNJIUoQEqx6sMA0GCSqGSIb3DQEBCwUAA4IBAQA9oFoR9GYVvba1WTdq2sl7kqTxqTPkUtD5LGi5A7q1yxMkAwsR2kW00L5dbRmADT7PjE3x42V2ZHHuYhDjGg/zm+2xHVrUWl2ZxHodmHz6+qDbdAZ3+9U4Zz7nt2oxDFghp/eE1adXa2kfAIZzn8VVamD6TS9O0R/KyXToYgpjLmz6QD9GFsz5wGbVsnJGWTxfiNjX3LnFIkqJU8rHn1DcMyB3/xd3ytUJzKrAnD8f46JpfR1amJOQAxiDy5+kW1OnclGBImS9iisvCmwU3+UNixbFAAxymBA9VvAO90sw0tHcLN7M1NSpenVlAnJTHhGuLSepk8gv4jAEsa9+DPKR";
    }
}


//
//  CRmsRegisterService
//
void CRmsRegisterService::Register(_In_ const CRmsRegisterServer& server, _Out_ std::wstring& regdata)
{
    HRESULT hr = S_OK;
    nudf::http::CSession session(_nxInfo.GetProductInitial());
    
    //UNREFERENCED_PARAMETER(server);
    //regdata = TEST_REGISTER_RESPONSE;

    nudf::crypto::CAesKeyBlob reqsessionkey;
    nudf::crypto::CX509CertContext reqexchcert;
    nudf::crypto::CRsaPubKeyBlob reqexchpubkey;
    std::vector<UCHAR> cipher_sessionkey;
    std::wstring wsBase64SessionKey;

    hr = reqsessionkey.Generate(256);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.Create(server.GetCert().c_str());
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.GetPublicKeyBlob(reqexchpubkey);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = nudf::crypto::RsaEncrypt(reqexchpubkey, reqsessionkey.GetKey(), reqsessionkey.GetKeySize(), cipher_sessionkey);
    if(FAILED(hr) || cipher_sessionkey.empty()) {
        throw WIN32ERROR2(hr);
    }

    wsBase64SessionKey = nudf::util::encoding::Base64Encode<wchar_t>(&cipher_sessionkey[0], (ULONG)cipher_sessionkey.size());
    if(wsBase64SessionKey.empty())  {
        throw WIN32ERROR();
    }
    
    hr = session.Open();
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    try {

        nudf::http::CResponse response;
        std::string reginfo;
        std::wstring wsPath = server.GetServicePath() + L"RegisterAgent";
        
        DWORD dwSecureProto = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        hr = session.SetOption(WINHTTP_OPTION_SECURE_PROTOCOLS, &dwSecureProto, sizeof(DWORD));
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        this->SetPath(wsPath);
        this->SetSecure(true);
        hr = Initialize(session, server.GetServer(), server.GetPort());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        // Get Meta Data
        PrepareRegisterInfo(server, reginfo);

        // Add Header
        std::wstring wsHeaders;
        wsHeaders = HEADER_KEY;
        wsHeaders += L": ";
        wsHeaders += wsBase64SessionKey;
        wsHeaders += L"\r\n";
        wsHeaders += HEADER_CERT;
        wsHeaders += L": ";
        wsHeaders += server.GetCert();
        wsHeaders += L"\r\n";
        hr = this->AddHeaders(wsHeaders.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = Send((PVOID)reginfo.c_str(), (ULONG)reginfo.length());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = GetResponse(response);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        regdata = nudf::util::convert::Utf8ToUtf16(response.GetContent());
        if(regdata.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        this->Close();
    }
    catch(const nudf::CException& e) {
        this->Close();
        throw e;
    }
}

void CRmsRegisterService::PrepareRegisterInfo(_In_ const CRmsRegisterServer& regserver, _Out_ std::string& reginfo)
{
    nudf::util::CXmlDocument doc;
    std::wstring wsInfo;
    std::string  sInfo;
    
    try {

        CComPtr<IXMLDOMNode> spRegRequest;
        CComPtr<IXMLDOMNode> spRegData;
        CComPtr<IXMLDOMNode> spRegElem;
        doc.Create();
        doc.AppendChildElement(L"RegisterAgentRequest", &spRegRequest);
        doc.AppendChildElement(spRegRequest, L"RegistrationData", &spRegData);
        // Insert Host
        nudf::win::CHost host;
        doc.AppendChildElement(spRegData, L"host", &spRegElem);
        std::wstring wsHostName = host.GetHostName();
        std::transform(wsHostName.begin(), wsHostName.end(), wsHostName.begin(), tolower);
        doc.SetNodeText(spRegElem, wsHostName);
        spRegElem.Release();
        // Insert Type
        doc.AppendChildElement(spRegData, L"type", &spRegElem);
        doc.SetNodeText(spRegElem, L"DESKTOP");
        spRegElem.Release();
        // Insert Type
        doc.AppendChildElement(spRegData, L"version", &spRegElem);
        {
            CComPtr<IXMLDOMNode> spRegSubElem;
            CNxProdInfo nxinfo;
            nxinfo.Load();
            std::vector<std::wstring> verlist;
            nudf::string::Split<wchar_t>(nxinfo.GetProductVersion(), L'.', verlist);
            doc.AppendChildElement(spRegElem, L"major", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, (verlist.size() > 0) ? verlist[0] : L"8");
            spRegSubElem.Release();
            doc.AppendChildElement(spRegElem, L"minor", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, (verlist.size() > 1) ? verlist[1] : L"0");
            spRegSubElem.Release();
            doc.AppendChildElement(spRegElem, L"maintenance", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, L"0");
            spRegSubElem.Release();
            doc.AppendChildElement(spRegElem, L"patch", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, (verlist.size() > 3) ? verlist[3] : L"0");
            spRegSubElem.Release();
            doc.AppendChildElement(spRegElem, L"build", &spRegSubElem);
            doc.SetNodeText(spRegSubElem,  (verlist.size() > 2) ? verlist[2] : L"0");
            spRegSubElem.Release();
        }
        spRegElem.Release();
        // Insert OS Info
        doc.AppendChildElement(spRegData, L"osInformation", &spRegElem);
        {
            diagnose::COsInfo osinfo;
            CComPtr<IXMLDOMNode> spRegSubElem;
            osinfo.Load();
            doc.AppendChildElement(spRegElem, L"osType", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, L"Windows");
            spRegSubElem.Release();
            doc.AppendChildElement(spRegElem, L"osVersion", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, osinfo.GetOsName());
            spRegSubElem.Release();
        }
        spRegElem.Release();
        // Insert Hardware Info
        doc.AppendChildElement(spRegData, L"hardwareDetails", &spRegElem);
        {
            diagnose::CHwInfo hwinfo;
            CComPtr<IXMLDOMNode> spRegSubElem;
            hwinfo.Load();
            // CPU
            doc.AppendChildElement(spRegElem, L"cpu", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, hwinfo.GetProcessorsInfo().GetCpuInfo().Brand());
            spRegSubElem.Release();
            // Memory
            int nMem = (int)(hwinfo.GetMemStatus()->ullTotalPhys / 0x100000);
            std::wstring wsMem = nudf::string::FromInt<wchar_t>(nMem);
            doc.AppendChildElement(spRegElem, L"memoryAmount", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, wsMem);
            spRegSubElem.Release();
            doc.AppendChildElement(spRegElem, L"memoryUnits", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, L"MB");
            spRegSubElem.Release();
            // MAC Address List
            std::wstring wsMacList;
            for(int i=0; i<(int)hwinfo.GetAdaptersInfo().size(); i++) {
                if(IF_TYPE_ETHERNET_CSMACD != hwinfo.GetAdaptersInfo()[i].GetIfType() && IF_TYPE_IEEE80211 != hwinfo.GetAdaptersInfo()[i].GetIfType()) {
                    continue;
                }
                if(0 == hwinfo.GetAdaptersInfo()[i].GetMacAddress().size()) {
                    continue;
                }
                if(0 == _wcsnicmp(hwinfo.GetAdaptersInfo()[i].GetDescription().c_str(), L"VMware", 6)) {
                    continue;
                }
                std::wstring wsMac = nudf::string::FromBytes<wchar_t>(&hwinfo.GetAdaptersInfo()[i].GetMacAddress()[0], (ULONG)hwinfo.GetAdaptersInfo()[i].GetMacAddress().size());
                if(wsMacList.length() != 0) {
                    wsMacList += L",";
                }
                wsMacList += wsMac;
            }
            doc.AppendChildElement(spRegElem, L"macAddressList", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, wsMacList);
            spRegSubElem.Release();
            // Device Id
            doc.AppendChildElement(spRegElem, L"deviceId", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, L"0");
            spRegSubElem.Release();
        }
        spRegElem.Release();
        // Insert Group Info
        doc.AppendChildElement(spRegData, L"groupInformation", &spRegElem);
        {
            CComPtr<IXMLDOMNode> spRegSubElem;
            // Group Id
            doc.AppendChildElement(spRegElem, L"groupId", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, regserver.GetGroupId().empty() ? L"0" : regserver.GetGroupId());
            spRegSubElem.Release();
            // Group Id
            doc.AppendChildElement(spRegElem, L"groupName", &spRegSubElem);
            doc.SetNodeText(spRegSubElem, regserver.GetGroupName().empty() ? L"Unknown" : regserver.GetGroupName());
            spRegSubElem.Release();
        }
        spRegElem.Release();
        // Insert Tenant Id
        doc.AppendChildElement(spRegData, L"tenantId", &spRegElem);
        doc.SetNodeText(spRegElem, regserver.GetTenantId().empty() ? L"Unknown" : regserver.GetTenantId());
        spRegElem.Release();

        // Convert to string
        wsInfo = doc.GetNodeXml(spRegRequest);
        reginfo = std::string(wsInfo.begin(), wsInfo.end());

        spRegRequest.Release();
        doc.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        reginfo.clear();
    }
}


//
//  class CRmsLogService
//
void CRmsLogService::LogAudit(_In_ const CRmsAgentProfile& profile, _In_ const std::wstring& file)
{
    HRESULT hr = S_OK;
    std::string sLogs;
    nudf::http::CSession session(_nxInfo.GetProductInitial());
        
    nudf::crypto::CAesKeyBlob reqsessionkey;
    nudf::crypto::CX509CertContext reqexchcert;
    nudf::crypto::CRsaPubKeyBlob reqexchpubkey;
    std::vector<UCHAR> cipher_sessionkey;
    std::wstring wsBase64SessionKey;
    
    // Get Log Data

    try {

        std::wstring wsLogs;
        nudf::util::CXmlDocument docLog;
        CComPtr<IXMLDOMElement>  spLogRoot;

        docLog.LoadFromFile(file.c_str());
        if(!docLog.GetDocRoot(&spLogRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        wsLogs = docLog.GetNodeXml(spLogRoot);
        sLogs = nudf::util::convert::Utf16ToUtf8(wsLogs);
        spLogRoot.Release();
        docLog.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        sLogs.clear(); //
    }

    // The file is not a valid audit log file
    if(sLogs.empty()) {
        return;
    }

    hr = reqsessionkey.Generate(256);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.Create(profile.GetAgentCert().c_str());
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.GetPublicKeyBlob(reqexchpubkey);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = nudf::crypto::RsaEncrypt(reqexchpubkey, reqsessionkey.GetKey(), reqsessionkey.GetKeySize(), cipher_sessionkey);
    if(FAILED(hr) || cipher_sessionkey.empty()) {
        throw WIN32ERROR2(hr);
    }

    wsBase64SessionKey = nudf::util::encoding::Base64Encode<wchar_t>(&cipher_sessionkey[0], (ULONG)cipher_sessionkey.size());
    if(wsBase64SessionKey.empty())  {
        throw WIN32ERROR();
    }
    
    hr = session.Open();
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    try {

        nudf::http::CResponse response;
        std::wstring wsPath = profile.GetRmsPath() + L"SendLog";
        

        DWORD dwSecureProto = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        hr = session.SetOption(WINHTTP_OPTION_SECURE_PROTOCOLS, &dwSecureProto, sizeof(DWORD));
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        this->SetPath(wsPath);
        this->SetSecure(true);
        hr = Initialize(session, profile.GetRmsServer(), profile.GetRmsPort());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }


        // Add Header
        std::wstring wsHeaders;
        wsHeaders = HEADER_KEY;
        wsHeaders += L": ";
        wsHeaders += wsBase64SessionKey;
        wsHeaders += L"\r\n";
        wsHeaders += HEADER_CERT;
        wsHeaders += L": ";
        wsHeaders += profile.GetAgentCert();
        wsHeaders += L"\r\n";
        hr = this->AddHeaders(wsHeaders.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = Send((PVOID)sLogs.c_str(), (ULONG)sLogs.length());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = GetResponse(response);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        std::wstring  wsResponse = nudf::util::convert::Utf8ToUtf16(response.GetContent());
        if(wsResponse.empty()) {
            throw WIN32ERROR2(ERROR_UNKNOWN_FEATURE);
        }
        
        nudf::util::CXmlDocument docResponse;
        CComPtr<IXMLDOMElement>  spRoot;
        CComPtr<IXMLDOMNode> spResponseRoot;
        CComPtr<IXMLDOMNode> spResponse;
        docResponse.LoadFromXml(wsResponse.c_str());
        if(!docResponse.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_UNKNOWN_FEATURE);
        }
        if(!docResponse.FindChildElement(spRoot, L"logResponse", &spResponseRoot)) {
            throw WIN32ERROR2(ERROR_UNKNOWN_FEATURE);
        }
        if(!docResponse.FindChildElement(spResponseRoot, L"response", &spResponse)) {
            throw WIN32ERROR2(ERROR_UNKNOWN_FEATURE);
        }
        std::wstring wsResult = docResponse.GetNodeText(spResponse);
        if(0 != _wcsicmp(wsResult.c_str(), L"Success")) {
            throw WIN32ERROR2(ERROR_UNKNOWN_FEATURE);
        }

        // Succeed
        this->Close();
    }
    catch(const nudf::CException& e) {
        this->Close();
        throw e;
    }
}



//
//  class CRmsHeartBeatService
//

#define TEST_HEARTBEAT_RESPONSE  \
    L"<HeartBeatResponse>"\
        L"<AgentUpdates>"\
            L"<id>1000</id>"\
            L"<CommProfile>"\
                L"<id>1</id>"\
                L"<name>Default Comm Profile</name>"\
                L"<defaultProfile>true</defaultProfile>"\
                L"<createdDate>2015-04-26T16:48:51.324+08:00</createdDate>"\
                L"<modifiedDate>2015-04-26T16:48:51.324+08:00</modifiedDate>"\
                L"<DABSLocation>https://10.63.0.152:8443/RMS/service/HeartBeat</DABSLocation>"\
                L"<agentType>DESKTOP</agentType>"\
                L"<heartBeatFrequency>"\
                    L"<time>1</time>"\
                    L"<time-unit>minutes</time-unit>"\
                L"</heartBeatFrequency>"\
                L"<logLimit>5</logLimit >"\
                L"<logFrequency>"\
                    L"<time>5</time>"\
                    L"<time-unit>minutes</time-unit>"\
                L"</logFrequency>"\
                L"<passwordHash>EEFCBAB37F049DA6514BC1B16422CF28</passwordHash>"\
                L"<currentActivityJournalingSettings>"\
                    L"<name>Default</name>"\
                    L"<loggedActivities>OPEN</loggedActivities>"\
                L"</currentActivityJournalingSettings>"\
                L"<customActivityJournalingSettings>"\
                    L"<name>Customized</name>"\
                    L"<loggedActivities>OPEN</loggedActivities>"\
                L"</customActivityJournalingSettings>"\
            L"</CommProfile>"\
            L"<AgentProfile>"\
                L"<id>1</id>"\
                L"<name>Default Agent Profile</name>"\
                L"<defaultProfile>true</defaultProfile>"\
                L"<createdDate>2015-04-26T16:48:51.324+08:00</createdDate>"\
                L"<modifiedDate>2015-04-26T16:48:51.324+08:00</modifiedDate>"\
                L"<hookAllProc>true</hookAllProc>"\
                L"<hookSystemProc>true</hookSystemProc>"\
                L"<logViewingEnabled>true</logViewingEnabled>"\
                L"<trayIconEnabled>true</trayIconEnabled>"\
            L"</AgentProfile>"\
            L"<ClassificationProfile timestamp=\"2015-04-26T16:48:51\">"\
                L"<Classify>"\
	                L"<Entry>"\
		                L"<Item>0</Item>"\
		                L"<Item>1</Item>"\
	                L"</Entry>"\
	                L"<LabelList>"\
		                L"<Label id=\"0\" name=\"Sensitivity\" display-name=\"Sensitivity\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
			                L"<VALUE priority=\"0\" value=\"Non Business\" />"\
			                L"<VALUE priority=\"1\" value=\"General Business\" />"\
			                L"<VALUE priority=\"2\" value=\"Proprietary\" />"\
			                L"<VALUE priority=\"3\" value=\"Confidential\" />"\
		                L"</Label>"\
		                L"<Label id=\"1\" name=\"Program\" display-name=\"Program\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
			                L"<VALUE value=\"PR-01\" sub-label=\"2\"/>"\
			                L"<VALUE value=\"PR-02\" sub-label=\"2\"/>"\
			                L"<VALUE value=\"PR-03\" sub-label=\"2\"/>"\
		                L"</Label>"\
		                L"<Label id=\"2\" name=\"Jurisdiction\" display-name=\"Jurisdiction\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
			                L"<VALUE value=\"ITAR\"  sub-label=\"3\"/>"\
			                L"<VALUE value=\"EAR\" sub-label=\"4\"/>"\
			                L"<VALUE value=\"BAFA\" sub-label=\"5\"/>"\
		                L"</Label>"\
		                L"<Label id=\"3\" name=\"ITAR\" display-name=\"ITAR Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
			                L"<VALUE value=\"TAA-01\" />"\
			                L"<VALUE value=\"TAA-02\" />"\
			                L"<VALUE value=\"TAA-03\" />"\
			                L"<VALUE value=\"TAA-04\" />"\
		                L"</Label>"\
		                L"<Label id=\"4\" name=\"EAR\" display-name=\"EAR Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
			                L"<VALUE value=\"EAR-01\" />"\
			                L"<VALUE value=\"EAR-02\" />"\
			                L"<VALUE value=\"EAR-03\" />"\
			                L"<VALUE value=\"EAR-04\" />"\
		                L"</Label>"\
		                L"<Label id=\"5\" name=\"BAFA\" display-name=\"BAFA Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
			                L"<VALUE value=\"BAFA-01\" />"\
			                L"<VALUE value=\"BAFA-02\" />"\
			                L"<VALUE value=\"BAFA-03\" />"\
			                L"<VALUE value=\"BAFA-04\" />"\
		                L"</Label>"\
	                L"</LabelList>"\
                L"</Classify>"\
            L"</ClassificationProfile>"\
            L"<policyDeploymentBundle>"\
                L"<POLICYBUNDLE timestamp=\"2015-04-28T13:31:40.324+08:00\">"\
                    L"<POLICYSET>"\
                        L"<!--Rights Collection-->"\
                        L"<POLICY id=\"0\" rights=\"RIGHT_VIEW\" name=\"Give VIEW right to everyone\" >"\
                        L"</POLICY>"\
                        L"<POLICY id=\"1\" rights=\"RIGHT_VIEW,RIGHT_EDIT\" name=\"Give VIEW/EDIT rights to everyone except GUEST on project-01 file\" >"\
                            L"<CONDITION exclude=\"true\" type=\"usr\">4</CONDITION>"\
                            L"<CONDITION exclude=\"false\" type=\"res\">7</CONDITION>"\
                        L"</POLICY>"\
                        L"<POLICY id=\"2\" rights=\"RIGHT_VIEW,RIGHT_EDIT,RIGHT_PRINT,RIGHT_CLASSIFY\" name=\"Give VIEW/EDIT/PRINT/CLASSIFY rights to everyone except GUEST on project-02 file\" >"\
                            L"<CONDITION exclude=\"true\" type=\"usr\">4</CONDITION>"\
                            L"<CONDITION exclude=\"false\" type=\"res\">8</CONDITION>"\
                            L"<OBLIGATION name=\"OB_OVERLAY\">"\
                                L"<PARAM name=\"Text\" value=\"$(User) $(GmtTime)\"/>"\
                            L"</OBLIGATION>"\
                        L"</POLICY>"\
                        L"<POLICY id=\"3\" rights=\"RIGHT_VIEW,RIGHT_EDIT,RIGHT_PRINT,RIGHT_CLIPBOARD,RIGHT_SAVEAS,RIGHT_SEND,RIGHT_CLASSIFY,RIGHT_SCREENCAP\" name=\"Give VIEW/EDIT/PRINT/CLIPBOARD/SAVEAS/SEND/CLASSIFY/SCNCAP rights to everyone except GUEST on project-03 file\" >"\
                            L"<CONDITION exclude=\"true\" type=\"usr\">4</CONDITION>"\
                            L"<CONDITION exclude=\"false\" type=\"res\">9</CONDITION>"\
                            L"<OBLIGATION name=\"OB_OVERLAY\">"\
                                L"<PARAM name=\"Text\" value=\"$(User) $(GmtTime)\"/>"\
                            L"</OBLIGATION>"\
                        L"</POLICY>"\
                        L"<POLICY id=\"4\" rights=\"RIGHT_VIEW,RIGHT_EDIT,RIGHT_PRINT,RIGHT_CLIPBOARD,RIGHT_SAVEAS,RIGHT_SEND,RIGHT_CLASSIFY,RIGHT_SCREENCAP\" name=\"Give VIEW/EDIT/PRINT/CLIPBOARD/SAVEAS/SEND/CLASSIFY/SCNCAP rights to everyone except GUEST on general-business file file\" >"\
                            L"<CONDITION exclude=\"true\" type=\"usr\">4</CONDITION>"\
                            L"<CONDITION exclude=\"false\" type=\"res\">1</CONDITION>"\
                            L"<OBLIGATION name=\"OB_OVERLAY\">"\
                                L"<PARAM name=\"Text\" value=\"$(User) $(GmtTime)\"/>"\
                            L"</OBLIGATION>"\
                        L"</POLICY>"\
                        L"<POLICY id=\"5\" rights=\"RIGHT_VIEW,RIGHT_EDIT,RIGHT_PRINT,RIGHT_CLIPBOARD,RIGHT_SAVEAS,RIGHT_SEND,RIGHT_CLASSIFY,RIGHT_SCREENCAP,RIGHT_DECRYPT\" name=\"Give VIEW/EDIT/PRINT/CLIPBOARD/SAVEAS/SEND/CLASSIFY/SCNCAP/DECRYPT rights to everyone except GUEST on non-business file file\" >"\
                            L"<CONDITION exclude=\"true\" type=\"usr\">4</CONDITION>"\
                            L"<CONDITION exclude=\"false\" type=\"res\">0</CONDITION>"\
                            L"<OBLIGATION name=\"OB_OVERLAY\">"\
                                L"<PARAM name=\"Text\" value=\"$(User) $(GmtTime)\"/>"\
                            L"</OBLIGATION>"\
                        L"</POLICY>"\
                        L"<POLICY id=\"6\" rights=\"RIGHT_VIEW,RIGHT_EDIT,RIGHT_PRINT,RIGHT_CLIPBOARD,RIGHT_SAVEAS,RIGHT_SEND,RIGHT_CLASSIFY\" name=\"Give VIEW/EDIT/PRINT/CLIPBOARD/SAVEAS/SEND/CLASSIFY rights to NextLabs Domain Users\" >"\
                            L"<CONDITION exclude=\"true\" type=\"usr\">4</CONDITION>"\
                            L"<CONDITION exclude=\"false\" type=\"usr\">5</CONDITION>"\
                        L"</POLICY>"\
                    L"</POLICYSET>"\
                    L"<USERGROUPMAP>"\
                    L"</USERGROUPMAP>"\
                    L"<!--Rule Components-->"\
                    L"<COMPONENTS>"\
                        L"<!--User Rules-->"\
                        L"<USERS>"\
                            L"<!--User: BAE Authorized Business User-->"\
                            L"<USER id=\"0\">"\
                                L"<PROPERTY name=\"baesystemsintellectualpropertyaccessrights\" type=\"string\" method=\"EQ\" value=\"intellectual property - business authorization category - identifier\"/>"\
                            L"</USER>"\
                            L"<!--User: BAE Authorized Internal User-->"\
                            L"<USER id=\"1\">"\
                                L"<PROPERTY name=\"baesystemsintellectualpropertyaccessrights\" type=\"string\" method=\"EQ\" value=\"intellectual property - internal authorization category - identifier\"/>"\
                            L"</USER>"\
                            L"<!--User: BAE Authorized Contractor-->"\
                            L"<USER id=\"2\">"\
                                L"<PROPERTY name=\"baesystemsintellectualpropertyaccessrights\" type=\"string\" method=\"EQ\" value=\"intellectual property - contract authorization category - identifier\"/>"\
                            L"</USER>"\
                            L"<!--User: Administrator (Local or Domain)-->"\
                            L"<USER id=\"3\">"\
                                L"<PROPERTY name=\"name\" type=\"string\" method=\"EQ\" value=\"Administrator*\"/>"\
                            L"</USER>"\
                            L"<!--User: Guest-->"\
                            L"<USER id=\"4\">"\
                                L"<PROPERTY name=\"name\" type=\"string\" method=\"EQ\" value=\"Guest*\"/>"\
                            L"</USER>"\
                            L"<!--User: Nextlabs Domain User-->"\
                            L"<USER id=\"5\">"\
                                L"<PROPERTY name=\"name\" type=\"string\" method=\"EQ\" value=\"*@*nextlabs.com\"/>"\
                            L"</USER>"\
                        L"</USERS>"\
                        L"<!--Application Rules-->"\
                        L"<APPLICATIONS>"\
                            L"<!--Application: Microsoft Office Word-->"\
                            L"<APPLICATION id=\"0\">"\
                                L"<PROPERTY type=\"string\" name=\"path\" method=\"EQ\" value=\"**\\winword.exe\"/>"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"microsoft corporation\"/>"\
                            L"</APPLICATION>"\
                            L"<!--Application: Microsoft Office Excel-->"\
                            L"<APPLICATION id=\"1\">"\
                                L"<PROPERTY type=\"string\" name=\"path\" method=\"EQ\" value=\"**\\excel.exe\"/>"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"microsoft corporation\"/>"\
                            L"</APPLICATION>"\
                            L"<!--Application: Microsoft Office PowerPoint-->"\
                            L"<APPLICATION id=\"2\">"\
                                L"<PROPERTY type=\"string\" name=\"path\" method=\"EQ\" value=\"**\\powerpnt.exe\"/>"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"microsoft corporation\"/>"\
                            L"</APPLICATION>"\
                            L"<!--Application: Adobe Acrobat Reader-->"\
                            L"<APPLICATION id=\"3\">"\
                                L"<PROPERTY type=\"string\" name=\"path\" method=\"EQ\" value=\"**\\acrord32.exe\"/>"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"adobe systems, incorporated\"/>"\
                            L"</APPLICATION>"\
                            L"<!--Application: Microsoft Notepad-->"\
                            L"<APPLICATION id=\"4\">"\
                                L"<PROPERTY type=\"string\" name=\"path\" method=\"EQ\" value=\"**\\notepad.exe\"/>"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"microsoft corporation\"/>"\
                            L"</APPLICATION>"\
                            L"<!--Application: Microsoft Wordpad-->"\
                            L"<APPLICATION id=\"5\">"\
                                L"<PROPERTY type=\"string\" name=\"path\" method=\"EQ\" value=\"**\\wordpad.exe\"/>"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"microsoft corporation\"/>"\
                            L"</APPLICATION>"\
                            L"<!--Application: Nextlabs Software-->"\
                            L"<APPLICATION id=\"6\">"\
                                L"<PROPERTY type=\"string\" name=\"publisher\" method=\"EQ\" value=\"nextlabs, inc\"/>"\
                            L"</APPLICATION>"\
                        L"</APPLICATIONS>"\
                        L"<!--Location Rules-->"\
                        L"<LOCATIONS>"\
                            L"<!--Location: In Nextlabs Domain-->"\
                            L"<LOCATION id=\"0\">"\
                                L"<PROPERTY type=\"string\" name=\"host\" method=\"EQ\" value=\"*.nextlabs.com\"/>"\
                            L"</LOCATION>"\
                        L"</LOCATIONS>"\
                        L"<!--Resource Rules-->"\
                        L"<RESOURCES>"\
                            L"<!--Resource: Non-business data-->"\
                            L"<RESOURCE id=\"0\">"\
                                L"<PROPERTY name=\"sensitivity\" type=\"string\" method=\"EQ\" value=\"Non Business\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: General-business data-->"\
                            L"<RESOURCE id=\"1\">"\
                                L"<PROPERTY name=\"sensitivity\" type=\"string\" method=\"EQ\" value=\"General Business\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: Proprietary data-->"\
                            L"<RESOURCE id=\"2\">"\
                                L"<PROPERTY name=\"sensitivity\" type=\"string\" method=\"EQ\" value=\"Proprietary\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: Confidential data-->"\
                            L"<RESOURCE id=\"3\">"\
                                L"<PROPERTY name=\"sensitivity\" type=\"string\" method=\"EQ\" value=\"Confidential\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: ITAR data-->"\
                            L"<RESOURCE id=\"4\">"\
                                L"<PROPERTY name=\"Jurisdiction\" type=\"string\" method=\"EQ\" value=\"itar\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: EAR data-->"\
                            L"<RESOURCE id=\"5\">"\
                                L"<PROPERTY name=\"Jurisdiction\" type=\"string\" method=\"EQ\" value=\"ear\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: BAFA data-->"\
                            L"<RESOURCE id=\"6\">"\
                                L"<PROPERTY name=\"Jurisdiction\" type=\"string\" method=\"EQ\" value=\"BAFA\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: Program PR-01 Data-->"\
                            L"<RESOURCE id=\"7\">"\
                                L"<PROPERTY name=\"program\" type=\"string\" method=\"EQ\" value=\"pr-01\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: Program PR-02 Data-->"\
                            L"<RESOURCE id=\"8\">"\
                                L"<PROPERTY name=\"program\" type=\"string\" method=\"EQ\" value=\"pr-02\"/>"\
                            L"</RESOURCE>"\
                            L"<!--Resource: Program PR-03 Data-->"\
                            L"<RESOURCE id=\"9\">"\
                                L"<PROPERTY name=\"program\" type=\"string\" method=\"EQ\" value=\"pr-03\"/>"\
                            L"</RESOURCE>"\
                        L"</RESOURCES>"\
                    L"</COMPONENTS>"\
                L"</POLICYBUNDLE>"\
            L"</policyDeploymentBundle>"\
            L"<keyRings>"\
                L"<keyRing KeyRingName=\"NL_SHARE\" LastModifiedDate=\"2015-04-12T12:43:03.324+08:00\">"\
		            L"<Key>"\
			            L"<KeyId>2E4823829738FF6E0FA3EC2B500EBB68198AF80E</KeyId>"\
			            L"<KeyData>5i/jdpHbim/u38x3PF8WqSMHJeFd1/SX2WIEhqnMeXc=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:49.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>E32A3EE3C6D9DB8C8FB2FEA0945E3B93BA5F2E82</KeyId>"\
			            L"<KeyData>+bKw75DvuubhzOAPqoNOEMAOT9cEDo5wJOFm74pqibE=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:48.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>E8839F1EA9362B7274E3D0DB37EF3AB86E59DA10</KeyId>"\
			            L"<KeyData>1Diu4v8OqPoJueI6c6blvK8oURD0ALWaC+CShIYTwh4=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:47.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>8EA8C7963E8CA026B0A58C5D06803CE6D69B4F43</KeyId>"\
			            L"<KeyData>SSzKIpqZYHlpdDJyemkAtB0/jx2LdPuTp161v4JJVKk=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:46.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>5AAA0CFE188544210EF8C2F4016A5A0701B756F0</KeyId>"\
			            L"<KeyData>Wat7LFLfwTtXPCY7qUqZuWlFMDzHe4u4/z3C4JM7E3A=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:45.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>CE91ADA0D5C92F5243511F0B23087EE4B50FE42D</KeyId>"\
			            L"<KeyData>XcqC1tATvChIlKQARVcc4HZeNp7fxjThuvj2UP9OxCc=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:44.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>6C039D046966AFB34EA8C96F5107B8246EEA4412</KeyId>"\
			            L"<KeyData>FV4GAh5sx6UU9ySvzvw7/h0NJKScMWQ/uRy7NFqr+dE=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:43.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>C77F02A82E6E8A46FE182F8B39B4576D66D17F08</KeyId>"\
			            L"<KeyData>paI6WvmhFfVRHQnvsKfd8iHnaG9UVxD/r3HM/PVIm7Q=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:42.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>729EDF94781049F3613EAE44738E40151EDE6EB6</KeyId>"\
			            L"<KeyData>RmIIWO0IosYHOPvbWpLAZn5nt9FXlrRnp7eUy11ugQI=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:41.324+08:00</TimeStamp>"\
		            L"</Key>"\
		            L"<Key>"\
			            L"<KeyId>EEFCBAB37F049DA6514BC1B16422CF28C72F2B69</KeyId>"\
			            L"<KeyData>vmDRpJ+RPLxZFlcS6mv+rDuxqebndp2NwCTM0Au324I=</KeyData>"\
			            L"<KeyVersion>0</KeyVersion>"\
			            L"<TimeStamp>2015-04-01T10:18:40.324+08:00</TimeStamp>"\
		            L"</Key>"\
                L"</keyRing>"\
            L"</keyRings>"\
        L"</AgentUpdates>"\
    L"</HeartBeatResponse>"

/*
    Key: E62FE37691DB8A6FEEDFCC773C5F16A9230725E15DD7F497D9620486A9CC7977
    Base64: 5i/jdpHbim/u38x3PF8WqSMHJeFd1/SX2WIEhqnMeXc=

    Key: F9B2B0EF90EFBAE6E1CCE00FAA834E10C00E4FD7040E8E7024E166EF8A6A89B1
    Base64: +bKw75DvuubhzOAPqoNOEMAOT9cEDo5wJOFm74pqibE=

    Key: D438AEE2FF0EA8FA09B9E23A73A6E5BCAF285110F400B59A0BE092848613C21E
    Base64: 1Diu4v8OqPoJueI6c6blvK8oURD0ALWaC+CShIYTwh4=

    Key: 492CCA229A996079697432727A6900B41D3F8F1D8B74FB93A75EB5BF824954A9
    Base64: SSzKIpqZYHlpdDJyemkAtB0/jx2LdPuTp161v4JJVKk=

    Key: 59AB7B2C52DFC13B573C263BA94A99B96945303CC77B8BB8FF3DC2E0933B1370
    Base64: Wat7LFLfwTtXPCY7qUqZuWlFMDzHe4u4/z3C4JM7E3A=

    Key: 5DCA82D6D013BC284894A40045571CE0765E369EDFC634E1BAF8F650FF4EC427
    Base64: XcqC1tATvChIlKQARVcc4HZeNp7fxjThuvj2UP9OxCc=

    Key: 155E06021E6CC7A514F724AFCEFC3BFE1D0D24A49C31643FB91CBB345AABF9D1
    Base64: FV4GAh5sx6UU9ySvzvw7/h0NJKScMWQ/uRy7NFqr+dE=

    Key: A5A23A5AF9A115F5511D09EFB0A7DDF221E7686F545710FFAF71CCFCF5489BB4
    Base64: paI6WvmhFfVRHQnvsKfd8iHnaG9UVxD/r3HM/PVIm7Q=

    Key: 46620858ED08A2C60738FBDB5A92C0667E67B7D15796B467A7B794CB5D6E8102
    Base64: RmIIWO0IosYHOPvbWpLAZn5nt9FXlrRnp7eUy11ugQI=

    Key: BE60D1A49F913CBC59165712EA6BFEAC3BB1A9E6E7769D8DC024CCD00BB7DB82
    Base64: vmDRpJ+RPLxZFlcS6mv+rDuxqebndp2NwCTM0Au324I=
*/
    
CRmsHeartBeatService::CRmsHeartBeatService() : CRmsRequest()
{
    if(NULL != gpEngine) {
        gpEngine->GetSessionServ()->GetLoggedOnUsers(_vLoggedonUsers);
    }
}

CRmsHeartBeatService::~CRmsHeartBeatService()
{
    Close();
}

void CRmsHeartBeatService::Beat(_In_ const CRmsAgentProfile& profile, _Out_ std::wstring& update)
{
    HRESULT hr = S_OK;
    
    nudf::crypto::CAesKeyBlob reqsessionkey;
    nudf::crypto::CX509CertContext reqexchcert;
    nudf::crypto::CRsaPubKeyBlob reqexchpubkey;
    std::vector<UCHAR> cipher_sessionkey;
    std::wstring wsBase64SessionKey;

    hr = reqsessionkey.Generate(256);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.Create(profile.GetAgentCert().c_str());
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.GetPublicKeyBlob(reqexchpubkey);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = nudf::crypto::RsaEncrypt(reqexchpubkey, reqsessionkey.GetKey(), reqsessionkey.GetKeySize(), cipher_sessionkey);
    if(FAILED(hr) || cipher_sessionkey.empty()) {
        throw WIN32ERROR2(hr);
    }

    wsBase64SessionKey = nudf::util::encoding::Base64Encode<wchar_t>(&cipher_sessionkey[0], (ULONG)cipher_sessionkey.size());
    if(wsBase64SessionKey.empty())  {
        throw WIN32ERROR();
    }
    
    // update = TEST_HEARTBEAT_RESPONSE;
    
    _session.SetAgentName(_nxInfo.GetProductInitial());
    hr = _session.Open();
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    try {

        nudf::http::CResponse response;
        std::string reqinfo;
        std::wstring wsPath = profile.GetRmsPath() + L"HeartBeat";

        this->SetPath(wsPath);
        this->SetSecure(true);
        hr = Initialize(_session, profile.GetRmsServer(), profile.GetRmsPort());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        
        // Add Header
        std::wstring wsHeaders;
        wsHeaders = HEADER_KEY;
        wsHeaders += L": ";
        wsHeaders += wsBase64SessionKey;
        wsHeaders += L"\r\n";
        wsHeaders += HEADER_CERT;
        wsHeaders += L": ";
        wsHeaders += profile.GetAgentCert();
        wsHeaders += L"\r\n";
        hr = this->AddHeaders(wsHeaders.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        // Get Meta Data
        PrepareHeartBeatInfo(reqinfo);

#ifdef _DEBUG
        LOGDBG(L"###HeartBeat###");
#endif
        hr = Send((PVOID)reqinfo.c_str(), (ULONG)reqinfo.length());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = GetResponse(response);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        update = nudf::util::convert::Utf8ToUtf16(response.GetContent());
        if(update.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
    }
    catch(const nudf::CException& e) {
        update.clear();
        throw e;
    }
}

void CRmsHeartBeatService::Acknowledge(_In_ const CRmsAgentProfile& profile)
{
    HRESULT hr = S_OK;
    
    nudf::crypto::CAesKeyBlob reqsessionkey;
    nudf::crypto::CX509CertContext reqexchcert;
    nudf::crypto::CRsaPubKeyBlob reqexchpubkey;
    std::vector<UCHAR> cipher_sessionkey;
    std::wstring wsBase64SessionKey;

    hr = reqsessionkey.Generate(256);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.Create(profile.GetAgentCert().c_str());
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.GetPublicKeyBlob(reqexchpubkey);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = nudf::crypto::RsaEncrypt(reqexchpubkey, reqsessionkey.GetKey(), reqsessionkey.GetKeySize(), cipher_sessionkey);
    if(FAILED(hr) || cipher_sessionkey.empty()) {
        throw WIN32ERROR2(hr);
    }

    wsBase64SessionKey = nudf::util::encoding::Base64Encode<wchar_t>(&cipher_sessionkey[0], (ULONG)cipher_sessionkey.size());
    if(wsBase64SessionKey.empty())  {
        throw WIN32ERROR();
    }

    if(NULL == _session.GetHandle()) {
        throw WIN32ERROR2(ERROR_INVALID_HANDLE);
    }

    try {


        nudf::http::CResponse response;
        std::string ackinfo;
        std::wstring wsPath = profile.GetRmsPath() + L"AckHeartBeat";

        this->SetPath(wsPath);
        this->SetSecure(true);
        hr = Reset();
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        
        // Add Header
        std::wstring wsHeaders;
        wsHeaders = HEADER_KEY;
        wsHeaders += L": ";
        wsHeaders += wsBase64SessionKey;
        wsHeaders += L"\r\n";
        wsHeaders += HEADER_CERT;
        wsHeaders += L": ";
        wsHeaders += profile.GetAgentCert();
        wsHeaders += L"\r\n";
        hr = this->AddHeaders(wsHeaders.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        // Get Meta Data
        PrepareAcknowledgeInfo(ackinfo);

        hr = Send((PVOID)ackinfo.c_str(), (ULONG)ackinfo.length());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        this->Close();
    }
    catch(const nudf::CException& e) {
        this->Close();
        throw e;
    }
}

void CRmsHeartBeatService::PrepareAcknowledgeInfo(_Out_ std::string& ackinfo)
{
    nudf::util::CXmlDocument doc;
    std::wstring wsInfo;
    std::string  sInfo;
    
    try {

        CComPtr<IXMLDOMNode> spRequest;
        CComPtr<IXMLDOMNode> spAgentId;
        CComPtr<IXMLDOMNode> spAckData;
        doc.Create();
        doc.AppendChildElement(L"AcknowledgeHeartBeatRequest", &spRequest);
        // Insert Agent Id
        doc.AppendChildElement(spRequest, L"id", &spAgentId);
        doc.SetNodeText(spAgentId, gpEngine->GetRmsAgentProfile().GetAgentId());
        spAgentId.Release();
        // Insert HeartBeat Data
        doc.AppendChildElement(spRequest, L"acknowledgementData", &spAckData);
        {
            CComPtr<IXMLDOMNode> spProfileStatus;
            doc.AppendChildElement(spAckData, L"profileStatus", &spProfileStatus);
            {
                CComPtr<IXMLDOMNode> spRegSubElem;
                doc.AppendChildElement(spProfileStatus, L"lastCommittedAgentProfileName", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsAgentProfileName());
                spRegSubElem.Release();
                doc.AppendChildElement(spProfileStatus, L"lastCommittedAgentProfileTimestamp", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsAgentProfileTimestamp());
                spRegSubElem.Release();
                doc.AppendChildElement(spProfileStatus, L"lastCommittedCommProfileName", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsCommProfileName());
                spRegSubElem.Release();
                doc.AppendChildElement(spProfileStatus, L"lastCommittedCommProfileTimestamp", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsCommProfileTimestamp());
                spRegSubElem.Release();
            }
            spProfileStatus.Release();
            CComPtr<IXMLDOMNode> spPolicyAssemblyStatus;
            doc.AppendChildElement(spAckData, L"policyAssemblyStatus", &spPolicyAssemblyStatus);
            {
                nudf::win::CHost host;
                std::wstring wsHostName;

                wsHostName = host.GetHostName();
                std::transform(wsHostName.begin(), wsHostName.end(), wsHostName.begin(), tolower);
                doc.SetNodeAttribute(spPolicyAssemblyStatus, L"agentHost", wsHostName);
                doc.SetNodeAttribute(spPolicyAssemblyStatus, L"agentType", L"DESKTOP");
                doc.SetNodeAttribute(spPolicyAssemblyStatus, L"timestamp", gpEngine->GetPolicyServ()->GetStrPolicyTimestamp().empty() ? L"1970-01-01T00:00:00.000+00:00" : gpEngine->GetPolicyServ()->GetStrPolicyTimestamp());
                
                for(std::vector<std::pair<std::wstring,std::wstring>>::const_iterator it=_vLoggedonUsers.begin(); it!=_vLoggedonUsers.end(); ++it) {

                    CComPtr<IXMLDOMNode> spPolicyUsers;
                    CComPtr<IXMLDOMNode> spSubjectType;
                    CComPtr<IXMLDOMNode> spWinSid;

                    // Add users
                    doc.AppendChildElement(spPolicyAssemblyStatus, L"policyUsers", &spPolicyUsers);

                    // Add type/sid
                    doc.AppendChildElement(spPolicyUsers, L"userSubjectType", &spSubjectType);
                    doc.SetNodeText(spSubjectType, L"windowsSid");
                    doc.AppendChildElement(spPolicyUsers, L"systemId", &spWinSid);
                    doc.SetNodeText(spWinSid, (*it).first);
                }
            }
            spPolicyAssemblyStatus.Release();
            spProfileStatus.Release();
        }
        spAgentId.Release();

        // Convert to string
        wsInfo = doc.GetNodeXml(spRequest);
        ackinfo = std::string(wsInfo.begin(), wsInfo.end());

        spRequest.Release();
        doc.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        ackinfo.clear();
    }
}

void CRmsHeartBeatService::PrepareHeartBeatInfo(_Out_ std::string& reginfo)
{
    nudf::util::CXmlDocument doc;
    std::wstring wsInfo;
    std::string  sInfo;
    
    try {

        CComPtr<IXMLDOMNode> spRequest;
        CComPtr<IXMLDOMNode> spAgentId;
        CComPtr<IXMLDOMNode> spHeartBeat;
        doc.Create();
        doc.AppendChildElement(L"HeartBeatRequest", &spRequest);
        // Insert Agent Id
        doc.AppendChildElement(spRequest, L"agentId", &spAgentId);
        doc.SetNodeText(spAgentId, gpEngine->GetRmsAgentProfile().GetAgentId());
        spAgentId.Release();
        // Insert HeartBeat Data
        doc.AppendChildElement(spRequest, L"heartbeat", &spHeartBeat);
        {
            CComPtr<IXMLDOMNode> spProfileStatus;
            doc.AppendChildElement(spHeartBeat, L"profileStatus", &spProfileStatus);
            {
                CComPtr<IXMLDOMNode> spRegSubElem;
                doc.AppendChildElement(spProfileStatus, L"lastCommittedAgentProfileName", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsAgentProfileName());
                spRegSubElem.Release();
                doc.AppendChildElement(spProfileStatus, L"lastCommittedAgentProfileTimestamp", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsAgentProfileTimestamp());
                spRegSubElem.Release();
                doc.AppendChildElement(spProfileStatus, L"lastCommittedCommProfileName", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsCommProfileName());
                spRegSubElem.Release();
                doc.AppendChildElement(spProfileStatus, L"lastCommittedCommProfileTimestamp", &spRegSubElem);
                doc.SetNodeText(spRegSubElem, gpEngine->GetRmsAgentProfile().GetRmsCommProfileTimestamp());
                spRegSubElem.Release();
            }
            spProfileStatus.Release();
            CComPtr<IXMLDOMNode> spPolicyAssemblyStatus;
            doc.AppendChildElement(spHeartBeat, L"policyAssemblyStatus", &spPolicyAssemblyStatus);
            {
                nudf::win::CHost host;
                std::wstring wsHostName;

                wsHostName = host.GetHostName();
                std::transform(wsHostName.begin(), wsHostName.end(), wsHostName.begin(), tolower);
                doc.SetNodeAttribute(spPolicyAssemblyStatus, L"agentHost", wsHostName);
                doc.SetNodeAttribute(spPolicyAssemblyStatus, L"agentType", L"DESKTOP");

                if(!gpEngine->GetPolicyServ()->InvalidateLoggedOnUserList(_vLoggedonUsers)) {
                    gpEngine->GetPolicyServ()->ResetTimestamp();
                }
                doc.SetNodeAttribute(spPolicyAssemblyStatus, L"timestamp", gpEngine->GetPolicyServ()->GetStrPolicyTimestamp().empty() ? L"1970-01-01T00:00:00.000+00:00" : gpEngine->GetPolicyServ()->GetStrPolicyTimestamp());
               

                for(std::vector<std::pair<std::wstring,std::wstring>>::const_iterator it=_vLoggedonUsers.begin(); it!=_vLoggedonUsers.end(); ++it) {

                    CComPtr<IXMLDOMNode> spPolicyUsers;
                    CComPtr<IXMLDOMNode> spSubjectType;
                    CComPtr<IXMLDOMNode> spWinSid;

                    // Add users
                    doc.AppendChildElement(spPolicyAssemblyStatus, L"policyUsers", &spPolicyUsers);

                    // Add type/sid
                    doc.AppendChildElement(spPolicyUsers, L"userSubjectType", &spSubjectType);
                    doc.SetNodeText(spSubjectType, L"windowsSid");
                    doc.AppendChildElement(spPolicyUsers, L"systemId", &spWinSid);
                    doc.SetNodeText(spWinSid, (*it).first);
                }
            }
            spPolicyAssemblyStatus.Release();
            doc.AppendChildElement(spHeartBeat, L"keyRingsMetaData", &spProfileStatus);
            spProfileStatus.Release();
            doc.AppendChildElement(spHeartBeat, L"sharedFolderDataCookie", &spProfileStatus);
            {
                CComPtr<IXMLDOMNode> spSubElem;
                doc.AppendChildElement(spProfileStatus, L"timestamp", &spSubElem);
                doc.SetNodeText(spSubElem, L"2014-12-28T02:32:43.585+00:00");
                spSubElem.Release();
            }
            spProfileStatus.Release();
            doc.AppendChildElement(spHeartBeat, L"pluginData", &spProfileStatus);
            spProfileStatus.Release();
        }
        spAgentId.Release();

        // Convert to string
        wsInfo = doc.GetNodeXml(spRequest);
        reginfo = std::string(wsInfo.begin(), wsInfo.end());

        spRequest.Release();
        doc.Close();
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        reginfo.clear();
    }
}

void CRmsUpdateService::Update(_In_ const CRmsAgentProfile& profile, _Out_ std::wstring& new_version, _Out_ std::wstring& download_url, _Out_ std::wstring& checksum)
{
    HRESULT hr = S_OK;
    nudf::http::CSession session(_nxInfo.GetProductInitial());
    
    
    nudf::crypto::CAesKeyBlob reqsessionkey;
    nudf::crypto::CX509CertContext reqexchcert;
    nudf::crypto::CRsaPubKeyBlob reqexchpubkey;
    std::vector<UCHAR> cipher_sessionkey;
    std::wstring wsBase64SessionKey;

    hr = reqsessionkey.Generate(256);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.Create(profile.GetAgentCert().c_str());
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = reqexchcert.GetPublicKeyBlob(reqexchpubkey);
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    hr = nudf::crypto::RsaEncrypt(reqexchpubkey, reqsessionkey.GetKey(), reqsessionkey.GetKeySize(), cipher_sessionkey);
    if(FAILED(hr) || cipher_sessionkey.empty()) {
        throw WIN32ERROR2(hr);
    }

    wsBase64SessionKey = nudf::util::encoding::Base64Encode<wchar_t>(&cipher_sessionkey[0], (ULONG)cipher_sessionkey.size());
    if(wsBase64SessionKey.empty())  {
        throw WIN32ERROR();
    }
	
    // update = TEST_HEARTBEAT_RESPONSE;
    
    hr = session.Open();
    if(FAILED(hr)) {
        throw WIN32ERROR2(hr);
    }

    try {

        nudf::http::CResponse response;
        std::string reqinfo;
        std::wstring wsPath = profile.GetRmsPath() + L"CheckUpdates";
        

        DWORD dwSecureProto = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        hr = session.SetOption(WINHTTP_OPTION_SECURE_PROTOCOLS, &dwSecureProto, sizeof(DWORD));
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        this->SetPath(wsPath);
        this->SetSecure(true);
        hr = Initialize(session, profile.GetRmsServer(), profile.GetRmsPort());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        
        // Add Header
        std::wstring wsHeaders;
        wsHeaders = HEADER_KEY;
        wsHeaders += L": ";
        wsHeaders += wsBase64SessionKey;
        wsHeaders += L"\r\n";
        wsHeaders += HEADER_CERT;
        wsHeaders += L": ";
        wsHeaders += profile.GetAgentCert();
        wsHeaders += L"\r\n";
        hr = this->AddHeaders(wsHeaders.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
                
        std::string sVersion(_nxInfo.GetProductVersion().begin(), _nxInfo.GetProductVersion().end());
#pragma warning(push)
#pragma warning(disable: 4996)
        DWORD dwWinVer = GetVersion();
#pragma warning(pop)
        char szWinVer[64] = {0};
        sprintf_s(szWinVer, "%d.%d", ((DWORD)(LOBYTE(LOWORD(dwWinVer)))),  ((DWORD)(HIBYTE(LOWORD(dwWinVer)))));
        // Set Request         
        reqinfo =  "<CheckUpdates><CheckUpdatesRequest><CurrentVersion>";
        //   - CurrentVersion
        reqinfo += sVersion;
        reqinfo += "</CurrentVersion><LastUpdatedDate></LastUpdatedDate><groupInformation><groupId>";
        //   - groupId
        reqinfo += "NextLabs";
        reqinfo += "</groupId><groupName>";
        //   - groupName
        reqinfo += "NextLabs";
        reqinfo += "</groupName></groupInformation><tenantId>";
        //   - tenantId
        reqinfo += "NextLabs01";
        reqinfo += "</tenantId><agentId>";
        reqinfo += "</agentId><osType>Windows</osType><osVersion>";
        //   - osVersion
        reqinfo += szWinVer;
#ifdef _WIN64
        reqinfo += "</osVersion><architecture>64bit</architecture></CheckUpdatesRequest></CheckUpdates>";
#else
        reqinfo += "</osVersion><architecture>32bit</architecture></CheckUpdatesRequest></CheckUpdates>";
#endif

        hr = Send((PVOID)reqinfo.c_str(), (ULONG)reqinfo.length());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = GetResponse(response);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        std::wstring wsUpdate = nudf::util::convert::Utf8ToUtf16(response.GetContent());
        this->Close();
        if(wsUpdate.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        nudf::util::CXmlDocument doc;
        CComPtr<IXMLDOMElement>  spRoot;
        CComPtr<IXMLDOMNode>     spResponse;
        doc.LoadFromXml(wsUpdate.c_str());
        if(!doc.GetDocRoot(&spRoot) || NULL==spRoot.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spRoot, L"CheckUpdatesResponse", &spResponse)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        CComPtr<IXMLDOMNode>     spNewVersion;
        CComPtr<IXMLDOMNode>     spChecksum;
        CComPtr<IXMLDOMNode>     spDownloadUrl;
        if(!doc.FindChildElement(spResponse, L"NewVersion", &spNewVersion)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spResponse, L"CheckSum", &spChecksum)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spResponse, L"DownloadURL", &spDownloadUrl)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        new_version = doc.GetNodeText(spNewVersion);
        checksum = doc.GetNodeText(spChecksum);
        download_url = doc.GetNodeText(spDownloadUrl);
        if(new_version.empty() || checksum.empty() || download_url.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
    }
    catch(const nudf::CException& e) {
        this->Close();
        new_version.clear();
        checksum.clear();
        download_url.clear();
        throw e;
    }
}


//
//  class CRmsAgentProfile
//

void CRmsAgentProfile::LoadFromRegisterData(_In_ const std::wstring& regdata)
{
    nudf::util::CXmlDocument doc;

    try {

        CComPtr<IXMLDOMElement> spRoot;
        CComPtr<IXMLDOMNode>    spCert;
        CComPtr<IXMLDOMNode>    spStartConf;
        CComPtr<IXMLDOMNode>    spAgentId;
        CComPtr<IXMLDOMNode>    spRmsAgentProfile;
        CComPtr<IXMLDOMNode>    spRmsAgentProfileName;
        CComPtr<IXMLDOMNode>    spRmsAgentProfileTime;
        CComPtr<IXMLDOMNode>    spRmsCommProfile;
        CComPtr<IXMLDOMNode>    spRmsCommProfileName;
        CComPtr<IXMLDOMNode>    spRmsCommProfileTime;
        CComPtr<IXMLDOMNode>    spRmsCommProfileHbFrequency;
        CComPtr<IXMLDOMNode>    spRmsCommProfileLogFrequency;

        std::wstring            name;
        HRESULT hr = S_OK;

        doc.LoadFromXml(regdata.c_str());

        // Get Root <RegisterAgentResponse>
        if(!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        name = doc.GetNodeName(spRoot);
        if(name != L"RegisterAgentResponse") {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get StartupConf <StartupConfiguration>
        if(!doc.FindChildElement(spRoot, L"StartupConfiguration", &spStartConf) || NULL==spStartConf.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // Agent Id
        if(!doc.FindChildElement(spStartConf, L"id", &spAgentId) || NULL==spAgentId.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agentId = doc.GetNodeText(spAgentId);
        if(_agentId.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // RMS Agent Profile
        if(!doc.FindChildElement(spStartConf, L"AgentProfile", &spRmsAgentProfile) || NULL==spRmsAgentProfile.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        if(!doc.FindChildElement(spRmsAgentProfile, L"name", &spRmsAgentProfileName) || NULL==spRmsAgentProfileName.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsAgentProfileName = doc.GetNodeText(spRmsAgentProfileName);
        if(_rmsAgentProfileName.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Timestamp
        if(!doc.FindChildElement(spRmsAgentProfile, L"modifiedDate", &spRmsAgentProfileTime) || NULL==spRmsAgentProfileTime.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsAgentProfileTimestamp = doc.GetNodeText(spRmsAgentProfileTime);
        if(_rmsAgentProfileTimestamp.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // RMS Comm Profile
        if(!doc.FindChildElement(spStartConf, L"commProfile", &spRmsCommProfile) || NULL==spRmsCommProfile.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        if(!doc.FindChildElement(spRmsCommProfile, L"name", &spRmsCommProfileName) || NULL==spRmsCommProfileName.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsCommProfileName = doc.GetNodeText(spRmsCommProfileName);
        if(_rmsCommProfileName.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Timestamp
        if(!doc.FindChildElement(spRmsCommProfile, L"modifiedDate", &spRmsCommProfileTime) || NULL==spRmsCommProfileTime.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsCommProfileTimestamp = doc.GetNodeText(spRmsCommProfileTime);
        if(_rmsCommProfileTimestamp.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> HeartBeat Frequency
        if(doc.FindChildElement(spRmsCommProfile, L"heartBeatFrequency", &spRmsCommProfileHbFrequency) && NULL!=spRmsCommProfileHbFrequency.p) {
            CComPtr<IXMLDOMNode>    spFrquency;
            _hbFrequency = 86400; // 1 day
            if(doc.FindChildElement(spRmsCommProfileHbFrequency, L"time", &spFrquency) && NULL!=spFrquency.p) {
                std::wstring wsFrequency = doc.GetNodeText(spFrquency);
                if(!wsFrequency.empty()) {
                    int nFrequency = 0;
                    if(nudf::string::ToInt<wchar_t>(wsFrequency, &nFrequency) && nFrequency > 0) {
                        CComPtr<IXMLDOMNode>    spTimeUnit;
                        _hbFrequency = nFrequency;
                        if(doc.FindChildElement(spRmsCommProfileHbFrequency, L"time-Unit", &spTimeUnit) && NULL!=spTimeUnit.p) {
                            std::wstring wsTimeUnit = doc.GetNodeText(spTimeUnit);
                            if(0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
                                _hbFrequency *= 86400;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
                                _hbFrequency *= 3600;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
                                _hbFrequency *= 60;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"milliseconds")) {
                                _hbFrequency /= 1000;
                                if(0 == _hbFrequency) {
                                    _hbFrequency = 1;
                                }
                            }
                            else {
                                ; // Nothing
                            }
                        }
                    }
                }
            }
        }
        //  -> Log Frequency
        if(doc.FindChildElement(spRmsCommProfile, L"logFrequency", &spRmsCommProfileLogFrequency) && NULL!=spRmsCommProfileLogFrequency.p) {
            CComPtr<IXMLDOMNode>    spFrquency;
            _logFrequency = 300; // 5 minutes
            if(doc.FindChildElement(spRmsCommProfileLogFrequency, L"time", &spFrquency) && NULL!=spFrquency.p) {
                std::wstring wsFrequency = doc.GetNodeText(spFrquency);
                if(!wsFrequency.empty()) {
                    int nFrequency = 0;
                    if(nudf::string::ToInt<wchar_t>(wsFrequency, &nFrequency) && nFrequency > 0) {
                        CComPtr<IXMLDOMNode>    spTimeUnit;
                        _logFrequency = nFrequency;
                        if(doc.FindChildElement(spRmsCommProfileLogFrequency, L"time-Unit", &spTimeUnit) && NULL!=spTimeUnit.p) {
                            std::wstring wsTimeUnit = doc.GetNodeText(spTimeUnit);
                            if(0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
                                _logFrequency *= 86400;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
                                _logFrequency *= 3600;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
                                _logFrequency *= 60;
                            }
                            else if(0 == _wcsicmp(wsTimeUnit.c_str(), L"milliseconds")) {
                                _logFrequency /= 1000;
                                if(0 == _logFrequency) {
                                    _logFrequency = 1;
                                }
                            }
                            else {
                                ; // Nothing
                            }
                        }
                    }
                }
            }
        }

        // Get Cert <certificate>
        if(!doc.FindChildElement(spRoot, L"certificate", &spCert) || NULL==spCert.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agentCert = doc.GetNodeText(spCert);
        if(_agentCert.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Generate all the keys
        GenerateKeys(_agentCert);

        // Save
        Save();
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CRmsAgentProfile::Load()
{
    try {

        HRESULT hr = S_OK;

        nudf::util::CXmlDocument doc;
        CComPtr<IXMLDOMElement>  spBundle;
        CComPtr<IXMLDOMNode>     spSignature;
        CComPtr<IXMLDOMNode>     spProfile;
        
        doc.LoadFromFile(_nxPaths.GetProfile().c_str());
        if(!doc.GetDocRoot(&spBundle)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spBundle, L"Signature", &spSignature)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spBundle, L"Profile", &spProfile)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Set Elements
        CComPtr<IXMLDOMNode>     spNode;
        // Agent Id
        if(!doc.FindChildElement(spProfile, L"AgentId", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agentId = doc.GetNodeText(spNode);
        spNode.Release();
        if(_agentId.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // Agent Certificate
        if(!doc.FindChildElement(spProfile, L"AgentCertificate", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agentCert = doc.GetNodeText(spNode);
        spNode.Release();
        if(_agentCert.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // RMS Server
        if(!doc.FindChildElement(spProfile, L"RmsServer", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rms_server = doc.GetNodeText(spNode);
        spNode.Release();
        if(_rms_server.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // RMS Port
        _rms_port = 8443;
        if(doc.FindChildElement(spProfile, L"RmsPort", &spNode)) {
            std::wstring wsPort = doc.GetNodeText(spNode);
            int nPort = 8443;
            if(!wsPort.empty() && nudf::string::ToInt<wchar_t>(wsPort, &nPort) && nPort > 0) {
                _rms_port = nPort;
            }
            spNode.Release();
        }
        // RMS Service Path
        if(!doc.FindChildElement(spProfile, L"RmsPath", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rms_path = doc.GetNodeText(spNode);
        spNode.Release();
        if(_rms_path.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // heartBeatFrequency
        if(!doc.FindChildElement(spProfile, L"heartBeatFrequency", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }        
        std::wstring wsHeartBeatFrequency = doc.GetNodeText(spNode);
        spNode.Release();
        if(wsHeartBeatFrequency.empty() || !nudf::string::ToInt<wchar_t>(wsHeartBeatFrequency, (int*)&_hbFrequency)) {
            _hbFrequency = 60 * 60 * 24;    // 1 day
        }
        // logFrequency
        if(!doc.FindChildElement(spProfile, L"logFrequency", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }        
        std::wstring wsLogFrequency = doc.GetNodeText(spNode);
        spNode.Release();
        if(wsLogFrequency.empty() || !nudf::string::ToInt<wchar_t>(wsLogFrequency, (int*)&_logFrequency)) {
            _logFrequency = 5 * 60;    // 5 minutes
        }
        // AgentProfile Name
        if(!doc.FindChildElement(spProfile, L"AgentProfileName", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsAgentProfileName = doc.GetNodeText(spNode);
        spNode.Release();
        if(_rmsAgentProfileName.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // AgentProfile Timestamp
        if(!doc.FindChildElement(spProfile, L"AgentProfileTimestamp", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsAgentProfileTimestamp = doc.GetNodeText(spNode);
        spNode.Release();
        if(_rmsAgentProfileTimestamp.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // CommProfile Name
        if(!doc.FindChildElement(spProfile, L"CommProfileName", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsCommProfileName = doc.GetNodeText(spNode);
        spNode.Release();
        if(_rmsCommProfileName.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // CommProfile Timestamp
        if(!doc.FindChildElement(spProfile, L"CommProfileTimestamp", &spNode)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _rmsCommProfileTimestamp = doc.GetNodeText(spNode);
        spNode.Release();
        if(_rmsCommProfileTimestamp.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Load Keys
        LoadKeys();

        // Verify Signatrue
        std::vector<UCHAR> currentSignature;
        std::wstring wsSignature = doc.GetNodeText(spSignature);
        std::wstring wsProfileXml = doc.GetNodeXml(spProfile);

        if(0 == _wcsicmp(wsSignature.c_str(), L"N/A")) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!nudf::string::ToBytes<wchar_t>(wsSignature, currentSignature)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        nudf::crypto::CRsaPubKeyBlob pubkey;
        _signKey.GetPublicKeyBlob(pubkey);
        hr = nudf::crypto::RsaVerifySignature(pubkey, wsProfileXml.c_str(), (ULONG)(wsProfileXml.length() * sizeof(WCHAR)), &currentSignature[0], (ULONG)currentSignature.size());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CRmsAgentProfile::Save()
{
    try {

        HRESULT hr = S_OK;

        nudf::util::CXmlDocument doc;
        CComPtr<IXMLDOMNode>     spBundle;
        CComPtr<IXMLDOMNode>     spSignature;
        CComPtr<IXMLDOMNode>     spProfile;
        SYSTEMTIME               timestamp;
        
        doc.Create();
        doc.AppendChildElement(L"Bundle", &spBundle);
        doc.AppendChildElement(spBundle, L"Signature", &spSignature);
        doc.AppendChildElement(spBundle, L"Profile", &spProfile);

        // Set timestamp
        GetSystemTime(&timestamp);
        doc.SetNodeAttribute(spProfile, L"timestamp", &timestamp, false);

        // Set Elements
        CComPtr<IXMLDOMNode>     spNode;
        doc.AppendChildElement(spProfile, L"AgentId", &spNode);
        doc.SetNodeText(spNode, _agentId);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"AgentCertificate", &spNode);
        doc.SetNodeText(spNode, _agentCert);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"RmsServer", &spNode);
        doc.SetNodeText(spNode, _rms_server);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"RmsPort", &spNode);
        std::wstring wsPort = nudf::string::FromInt<wchar_t>((int)_rms_port);
        doc.SetNodeText(spNode, wsPort);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"RmsPath", &spNode);
        doc.SetNodeText(spNode, _rms_path);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"heartBeatFrequency", &spNode);
        std::wstring wsHeartBeatFrequency = nudf::string::FromInt<wchar_t>((int)_hbFrequency);
        doc.SetNodeText(spNode, wsHeartBeatFrequency);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"logFrequency", &spNode);
        std::wstring wsLogFrequency = nudf::string::FromInt<wchar_t>((int)_logFrequency);
        doc.SetNodeText(spNode, wsLogFrequency);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"AgentProfileName", &spNode);
        doc.SetNodeText(spNode, _rmsAgentProfileName);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"AgentProfileTimestamp", &spNode);
        doc.SetNodeText(spNode, _rmsAgentProfileTimestamp);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"CommProfileName", &spNode);
        doc.SetNodeText(spNode, _rmsCommProfileName);
        spNode.Release();
        doc.AppendChildElement(spProfile, L"CommProfileTimestamp", &spNode);
        doc.SetNodeText(spNode, _rmsCommProfileTimestamp);
        spNode.Release();

        // Sign
        std::wstring wsSignature(L"N/A");
        std::vector<UCHAR> signature;
        std::wstring wsProfileXml = doc.GetNodeXml(spProfile);
        if(wsProfileXml.length()) {
            hr = nudf::crypto::RsaSign(_signKey, wsProfileXml.c_str(), (ULONG)(wsProfileXml.length() * sizeof(WCHAR)), signature);
            if(FAILED(hr)) {
                throw WIN32ERROR2(hr);
            }
            wsSignature = nudf::string::FromBytes<wchar_t>(&signature[0], (ULONG)signature.size());
        }
        doc.SetNodeText(spSignature, wsSignature);

        // Save to file
        doc.SaveToFile(_nxPaths.GetProfile().c_str());
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CRmsAgentProfile::Clear()
{
    _agentId.clear();
    _rms_server.clear();
    _rms_port = 0;
    _hbFrequency = 0;
    _logFrequency = 0;
    _agentKey.Clear();
    _sessionKey.Clear();

    _rmsAgentProfileName.clear();
    _rmsAgentProfileTimestamp.clear();
    _rmsCommProfileName.clear();
    _rmsCommProfileTimestamp.clear();
}

void CRmsAgentProfile::GenerateKeys(const std::wstring& cert)
{
    try {

        HRESULT hr = S_OK;
        
        // Load AgentKey from Certificate
        nudf::crypto::CX509CertContext context;
        hr = context.Create(cert.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        hr = context.GetPublicKeyBlob(_agentKey);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        // Generate AES session key
        GenerateSessionKey();

        // Generate RSA Sign Key
        GenerateSignKey();
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CRmsAgentProfile::SaveCertFile(const std::wstring& cert)
{
    std::vector<UCHAR> vCert;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD  dwWritten = 0;
    std::wstring wsTmp = _nxPaths.GetConfDir() + L"\\agent.new.cer";

    try {

        if(!nudf::util::encoding::Base64Decode<wchar_t>(cert, vCert)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        hFile = ::CreateFileW(wsTmp.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hFile) {
            throw WIN32ERROR();
        }

        if(!::WriteFile(hFile, &vCert[0], (ULONG)vCert.size(), &dwWritten, NULL)) {
            CloseHandle(hFile);
            throw WIN32ERROR();
        }

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;

        if(INVALID_FILE_ATTRIBUTES != GetFileAttributesW(_nxPaths.GetAgentCert().c_str())) {
            if(!::ReplaceFileW(_nxPaths.GetAgentCert().c_str(), wsTmp.c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL)) {
                throw WIN32ERROR();
            }
        }
        else {
            if(!::MoveFileExW(wsTmp.c_str(), _nxPaths.GetAgentCert().c_str(), MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
                throw WIN32ERROR();
            }
        }
    }
    catch(const nudf::CException& e) {
        if(INVALID_HANDLE_VALUE != hFile) {
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }
        ::DeleteFileW(wsTmp.c_str());
        throw e;
    }
}

void CRmsAgentProfile::GenerateSessionPhrase(_Out_ std::wstring& phrase)
{
    static const ULONGLONG magic_ivec = 0xEA8FA09B9E23A73AULL;
    std::vector<UCHAR> cipher;
    UCHAR data[32] = {0};

    if(NULL == _sessionKey.GetBlob()) {
        throw WIN32ERROR2(ERROR_LOCAL_USER_SESSION_KEY);
    }
    
    memcpy(data, _sessionKey.GetKey(), 32);
    nudf::crypto::AesEncrypt(_sessionKey, data, 32, magic_ivec, cipher);
    phrase = nudf::string::FromBytes<wchar_t>(&cipher[0], 16);
#ifdef _DEBUG
    LOGSYSINF(L"Generate Session Phrase: %s", phrase.c_str());
#endif
}

void CRmsAgentProfile::GenerateSignKey()
{
    HRESULT hr = S_OK;   
    
    try {

        nudf::crypto::CCertContext cert;

        hr = cert.Create(L"CN=NextLabs Endpoint Authority; O=NextLabs; OU=Endpoint; C=US; L=San Mateo; ST=California", true, false, 20);
        if(S_OK != hr) {
            throw WIN32ERROR2(hr);
        }

        assert(cert.HasPrivateKey());

        nudf::crypto::CMemCertStore store;
        hr = store.Open();
        if(S_OK != hr) {
            cert.Close();
            throw WIN32ERROR2(hr);
        }

        hr = store.AddCert(cert);
        if(S_OK != hr) {
            cert.Close();
            throw WIN32ERROR2(hr);
        }

        std::wstring    phrase;
        GenerateSessionPhrase(phrase);
        hr = store.ExportToPkcs12Package(_nxPaths.GetSecureBin().c_str(), phrase.c_str(), true);
        if(S_OK != hr) {
            throw WIN32ERROR2(hr);
        }

        hr = cert.GetPrivateKeyBlob(_signKey);
        if(S_OK != hr) {
            throw WIN32ERROR2(hr);
        }
    }
    catch(const nudf::CException& e) {
        ::DeleteFileW(_nxPaths.GetSecureBin().c_str());
        _signKey.Clear();
        throw e;
    }
}

void CRmsAgentProfile::GenerateSessionKey()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR>  signature;
    std::vector<UCHAR>  key;

    if(_agentId.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(NULL == _agentKey.GetBlob()) {
        throw WIN32ERROR2(ERROR_LOCAL_USER_SESSION_KEY);
    }
        
    if(!nudf::crypto::ToSha256(_agentKey.GetBlob(), _agentKey.GetBlobSize(), key) || key.size()!=32) {
        throw WIN32ERROR();
    }

    _sessionKey.Clear();
    _sessionKey.SetKey(&key[0], 32);
    
#ifdef _DEBUG
    std::wstring sessionKeyStr = nudf::string::FromBytes<wchar_t>(&key[0], (ULONG)key.size());
    LOGSYSINF(L"Generate Session Key: %s", sessionKeyStr.c_str());
#endif
}

void CRmsAgentProfile::LoadKeys()
{
    try {

        HRESULT hr = S_OK;
        
        // Load AgentKey from Certificate
        nudf::crypto::CX509CertContext context;
        hr = context.Create(_agentCert.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        hr = context.GetPublicKeyBlob(_agentKey);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        // Generate AES session key
        GenerateSessionKey();

        // Load RSA Sign Key
        std::wstring    phrase;
        GenerateSessionPhrase(phrase);
        nudf::crypto::CPkcs12CertContext pk12context;
        hr = pk12context.CreateFromFile(_nxPaths.GetSecureBin().c_str(), phrase.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = pk12context.GetPrivateKeyBlob(_signKey);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

//#define CLASSIFY_CONF_TEMPLATE  \
//    L"<Bundle>"\
//        L"<Signature>##SIGNATURE##</Signature>"\
//        L"<Data timestamp=\"##TIMESTAMP##\">"\
//            L"<Classify>##AP_AGENTID##</Classify>"\
//        L"</Data>"\
//    L"</Bundle>"

void CRmsAgentClassify::LoadFromRegisterData(_In_ const std::wstring& regdata, const nudf::crypto::CLegacyRsaPriKeyBlob& key)
{
    nudf::util::CXmlDocument doc;

    try {

        HRESULT                 hr = S_OK;
        std::wstring            name;
        CComPtr<IXMLDOMElement> spRoot;
        CComPtr<IXMLDOMNode>    spStartConf;
        CComPtr<IXMLDOMNode>    spRmsClassifyProfile;

        doc.LoadFromXml(regdata.c_str());

        // Get Root <RegisterAgentResponse>
        if(!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        name = doc.GetNodeName(spRoot);
        if(name != L"RegisterAgentResponse") {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get StartupConf <StartupConfiguration>
        if(!doc.FindChildElement(spRoot, L"StartupConfiguration", &spStartConf) || NULL==spStartConf.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        // ClassificationProfile
        if(!doc.FindChildElement(spStartConf, L"ClassificationProfile", &spRmsClassifyProfile) || NULL==spRmsClassifyProfile.p) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        LoadFromNode(&doc, spRmsClassifyProfile, key);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CRmsAgentClassify::LoadFromNode(_In_ nudf::util::CXmlDocument* doc, _In_ IXMLDOMNode* pNode, const nudf::crypto::CLegacyRsaPriKeyBlob& key)
{
    SYSTEMTIME  timestamp = {0};
    std::wstring wsTimestamp;
    nudf::time::CTimeZone zone;
    std::wstring wsZone;
    CComPtr<IXMLDOMNode>     spClassify;


    GetLocalTime(&timestamp);
    wsTimestamp = nudf::string::FromSystemTime<wchar_t>(&timestamp, false);
    zone.BiasToString(wsZone);
    wsTimestamp += wsZone;
    doc->SetNodeAttribute(pNode, L"timestamp", wsTimestamp);
    // Sign ClassificationProfile Data
    if(!doc->FindChildElement(pNode, L"Classify", &spClassify)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    std::wstring wsDataXml = doc->GetNodeXml(spClassify);
    _signature = Sign(wsDataXml, key);

    // Load White App List
    LoadWhiteAppListNode(doc, spClassify);

    // Get XML
    _xml = doc->GetNodeXml(pNode);
    if(_xml.empty()) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
}

void CRmsAgentClassify::LoadWhiteAppListNode(_In_ nudf::util::CXmlDocument* doc, _In_ IXMLDOMNode* pNode)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNode>     spWhiteListNode;

    _whiteapps.clear();
    if(!doc->FindChildElement(pNode, L"WhiteList", &spWhiteListNode)) {
        return;
    }

    CComPtr<IXMLDOMNode>     spWhiteListAppNode;
    if(!doc->FindChildElement(spWhiteListNode, L"Applications", &spWhiteListAppNode)) {
        return;
    }

    CComPtr<IXMLDOMNodeList>    spNodeList;
    hr = spWhiteListAppNode->get_childNodes(&spNodeList);
    if(FAILED(hr)) {
        return;
    }

    long lCount = 0;
    hr = spNodeList->get_length(&lCount);
    if(FAILED(hr) || lCount <= 0) {
        return;
    }

    for(long i=0; i<lCount; i++) {
        CComPtr<IXMLDOMNode>    spAppNode;
        std::wstring            wsAppName;
        std::wstring            wsAppPublisher;
        std::wstring            wsAppRights;
        ULONGLONG               ullAppRights = 0;
        hr = spNodeList->get_item(i, &spAppNode);
        if(!SUCCEEDED(hr)) {
            continue;
        }
        if(NODE_ELEMENT != doc->GetNodeType(spAppNode)) {
            continue;
        }
        std::wstring nodename = doc->GetNodeName(spAppNode);
        if(0 != _wcsicmp(nodename.c_str(), L"Application")) {
            continue;
        }

        doc->GetNodeAttribute(spAppNode, L"name", wsAppName);
        doc->GetNodeAttribute(spAppNode, L"publisher", wsAppPublisher);
        doc->GetNodeAttribute(spAppNode, L"rights", wsAppRights);
        if(wsAppName.empty() && wsAppPublisher.empty()) {
            continue;
        }
        if(wsAppRights.empty()) {
            ullAppRights = BUILTIN_RIGHT_VIEW;
        }
        else {
            std::vector<std::wstring> vwsRights;
            nudf::string::Split<wchar_t>(wsAppRights, L',', vwsRights);
            for(std::vector<std::wstring>::const_iterator it=vwsRights.begin(); it!=vwsRights.end(); ++it) {
                ullAppRights |= ActionToRights((*it).c_str());
            }
            if(0 == (ullAppRights & BUILTIN_RIGHT_VIEW)) {
                ullAppRights |= BUILTIN_RIGHT_VIEW;
            }
        }
        // Good, insert to list
        _whiteapps.push_back(CAppInfo(wsAppName, wsAppPublisher, ullAppRights));
    }
}

CRmsAgentClassify& CRmsAgentClassify::operator = (const CRmsAgentClassify& cls) throw()
{
    if(this != &cls) {
        _xml = cls.GetXml();
        _signature = cls.GetSignature();
        _whiteapps = cls.GetAppWhiteList();
    }
    return *this;
}

void CRmsAgentClassify::Load(const std::wstring& file, const nudf::crypto::CRsaPubKeyBlob& key)
{
    nudf::util::CXmlDocument doc;

    try {

        HRESULT hr = S_OK;

        CComPtr<IXMLDOMElement>  spBundle;
        CComPtr<IXMLDOMNode>     spSignature;
        CComPtr<IXMLDOMNode>     spClassifyProfile;
        CComPtr<IXMLDOMNode>     spClassify;
        
        doc.LoadFromFile(_nxPaths.GetClassifyConf().c_str());
        if(!doc.GetDocRoot(&spBundle)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spBundle, L"Signature", &spSignature)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.FindChildElement(spBundle, L"ClassificationProfile", &spClassifyProfile)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        
        _xml = doc.GetNodeXml(spClassifyProfile);
        _signature = doc.GetNodeText(spSignature);

        // Verify Signatrue
        if(!doc.FindChildElement(spClassifyProfile, L"Classify", &spClassify)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        std::wstring wsDataXml = doc.GetNodeXml(spClassify);
        std::vector<UCHAR> currentSignature;

        if(0 == _wcsicmp(_signature.c_str(), L"N/A")) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!nudf::string::ToBytes<wchar_t>(_signature, currentSignature)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        hr = nudf::crypto::RsaVerifySignature(key, wsDataXml.c_str(), (ULONG)(wsDataXml.length()*sizeof(WCHAR)), &currentSignature[0], (ULONG)currentSignature.size());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        // Load White App List
        LoadWhiteAppListNode(&doc, spClassify);
    }
    catch(const nudf::CException& e) {
        _xml.clear();
        _signature.clear();
        _whiteapps.clear();
        throw e;
    }
}

void CRmsAgentClassify::Save(const std::wstring& file)
{
    nudf::util::CXmlDocument doc;
    nudf::util::CXmlDocument clsdoc;

    try {

        HRESULT hr = S_OK;

        CComPtr<IXMLDOMNode>     spBundle;
        CComPtr<IXMLDOMNode>     spSignature;
        CComPtr<IXMLDOMNode>     spClassifyProfile;
        
        doc.Create();
        doc.AppendChildElement(L"Bundle", &spBundle);
        doc.AppendChildElement(spBundle, L"Signature", &spSignature);
        
        // Set Elements
        clsdoc.LoadFromXml(_xml.c_str());
        CComPtr<IXMLDOMElement>  sTmppClassifyProfile;
        if(!clsdoc.GetDocRoot(&sTmppClassifyProfile)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        hr = spBundle->appendChild(sTmppClassifyProfile, &spClassifyProfile);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        clsdoc.Close();

        // Put Signatrue
        doc.SetNodeText(spSignature, _signature);

        // Save to file
        doc.SaveToFile(file.c_str());
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

std::wstring CRmsAgentClassify::Sign(_In_ const std::wstring& xml, _In_ const nudf::crypto::CLegacyRsaPriKeyBlob& key)
{
    std::wstring wsSignature(L"N/A");
    std::vector<UCHAR> signature;
    if(xml.length()) {
        HRESULT hr = nudf::crypto::RsaSign(key, xml.c_str(), (ULONG)(xml.length() * sizeof(WCHAR)), signature);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        wsSignature = nudf::string::FromBytes<wchar_t>(&signature[0], (ULONG)signature.size());
    }
    return wsSignature;
}

ULONGLONG CRmsAgentClassify::CheckAppInWhiteList(_In_ const std::wstring& appName, _In_ const std::wstring& appPublisher) const throw()
{
    ULONGLONG rights = 0;
    ::EnterCriticalSection(&_whiteappslock);
    for(std::vector<CAppInfo>::const_iterator it=_whiteapps.begin(); it!=_whiteapps.end(); ++it) {
        if((*it).MatchName(appName) && (*it).MatchPublisher(appPublisher)) {
            rights = (*it).GetRights();
            break;
        }
    }
    ::LeaveCriticalSection(&_whiteappslock);
    return rights;
}