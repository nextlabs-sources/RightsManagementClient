

#include <Windows.h>
#include <stdio.h>

#include <nudf\exception.hpp>
#include <nudf\cert.hpp>


int main(int argc, char** argv)
{
    if(argc < 2) {
        printf("CertViewer Usage:\n");
        printf("   %s <cert_path>\n", argv[0]);
        return -1;
    }

    std::string sCert(argv[1]);
    std::wstring wsCert(sCert.begin(), sCert.end());
    HRESULT hr = S_OK;

    nudf::crypto::CX509CertContext cert;
    hr = cert.CreateFromFile(wsCert.c_str());
    if(FAILED(hr)) {
        printf("Fail to load cert from file (0x%08X)\n", hr);
        return -1;
    }

    printf("[X.509 Cert]\n");
    printf("  - File: %s\n", sCert.c_str());

    std::wstring wsSubject;
    std::wstring wsIssuer;
    SYSTEMTIME dtValidFrom;
    SYSTEMTIME dtValidThru;
    std::wstring sign_alg;
    std::wstring hash_alg;

    cert.GetSubjectName(wsSubject);
    cert.GetIssuerName(wsIssuer);
    cert.GetValidFromDate(&dtValidFrom);
    cert.GetValidThruDate(&dtValidThru);
    cert.GetPropSignHashCngAlgorithm(sign_alg, hash_alg);

    printf("  - Subject: %S\n", wsSubject.c_str());
    printf("  - Issuer: %S\n", wsIssuer.c_str());
    printf("  - ValidFrom: %04d-%02d-%02d %02d:%02d:%02d\n", dtValidFrom.wYear, dtValidFrom.wMonth, dtValidFrom.wDay, dtValidFrom.wHour, dtValidFrom.wMinute, dtValidFrom.wSecond);
    printf("  - ValidThru: %04d-%02d-%02d %02d:%02d:%02d\n", dtValidThru.wYear, dtValidThru.wMonth, dtValidThru.wDay, dtValidThru.wHour, dtValidThru.wMinute, dtValidThru.wSecond);
    printf("  - Algorithm: %S/%S\n", sign_alg.c_str(), hash_alg.c_str());
    

    nudf::crypto::CRsaPubKeyBlob pubkey;
    hr = cert.GetPublicKeyBlob(pubkey);
    if(FAILED(hr)) {
        printf("Fail to get public key BLOB from cert (0x%08X)\n", hr);
        return -1;
    }

    printf("  - Public Key BLOB:\n");
    printf("    * KeyLength: %d\n", pubkey.GetKeyBitsLength());   
    printf("    * PubExp: ");
    for(int i=0; i<(int)pubkey.GetPubexpLen(); i++) {
        printf("%02X", *(pubkey.GetPubexp()+i));
    }
    printf("\n");  
    printf("    * Modulus: ");
    for(int i=0; i<(int)pubkey.GetModulusLen(); i++) {
        printf("%02X", *(pubkey.GetModulus()+i));
    }
    printf("\n");

    return 0;
}