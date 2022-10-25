

#ifndef __TEST_KEYBUNDLE_HPP__
#define __TEST_KEYBUNDLE_HPP__

#include <gtest\gtest.h>
#include <nudf\exception.hpp>
#include <nudf\crypto.hpp>
#include <nudf\path.hpp>
#include <nudf\encoding.hpp>
#include <nxrmpkg.hpp>



extern const std::wstring sign_pub_cert;
extern const std::wstring sign_pri_cert;
extern const std::wstring exch_pub_cert;
extern const std::wstring exch_pri_cert;

class KeyBundleSuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: Key Bundle" << std::endl;
       EXPECT_TRUE(nudf::crypto::Initialize());

       nudf::win::CModulePath mod(NULL);
       _bundlefile = mod.GetParentDir();
       _bundlefile+= L"\\keybundle.xml";
   }

   static void TearDownTestCase()
   {
   }
   
   static std::wstring _bundlefile;
};

std::wstring KeyBundleSuite::_bundlefile;


TEST_F(KeyBundleSuite, Create)
{
    HRESULT hr = 0;
    NXRM_KEY_BLOB KeyChain[10];

    memset(KeyChain, 0, sizeof(KeyChain));
    for(int i=0; i<10; i++) {
        nudf::crypto::CAesKeyBlob key;
        std::vector<unsigned char> keyid;
        ASSERT_TRUE(S_OK == key.Generate(256));
        ASSERT_TRUE(32 == key.GetKeySize());
        ASSERT_TRUE(nudf::crypto::ToSha256(key.GetKey(), key.GetKeySize(), keyid));
        ASSERT_TRUE(!keyid.empty());
        KeyChain[i].KeKeyId.Algorithm = NXRM_ALGORITHM_AES256;
        KeyChain[i].KeKeyId.IdSize = keyid.size();
        memcpy(KeyChain[i].KeKeyId.Id, &keyid[0], keyid.size());
        memcpy(KeyChain[i].CeKey, key.GetKey(), key.GetKeySize());
    }

    nxrm::engine::pkg::key::CKeyBundle keybundle(KeyChain, (ULONG)sizeof(KeyChain));
    nudf::crypto::CPkcs12CertContext pfxcert;
    nudf::crypto::CLegacyRsaPriKeyBlob prikey;

    hr = pfxcert.CreateFromFile(sign_pri_cert.c_str(), L"123blue!");
    EXPECT_TRUE(SUCCEEDED(hr));
    if(FAILED(hr)) {
        return;
    }

    hr = pfxcert.GetPrivateKeyBlob(prikey);
    EXPECT_TRUE(SUCCEEDED(hr));
    if(FAILED(hr)) {
        return;
    }

    try {
        keybundle.ToFile(_bundlefile.c_str(), L"Test", prikey);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        ASSERT_TRUE(FALSE);
    }
}

TEST_F(KeyBundleSuite, Load)
{
    nxrm::engine::pkg::key::CKeyBundle keybundle;
    
    try {
        keybundle.LoadFromFile(_bundlefile.c_str());
        PCNXRM_KEY_BLOB KeyChain = keybundle.GetKeyChain();
        int count = (int)keybundle.GetKeyCount();
        ASSERT_TRUE(count == 10);
        for(int i=0; i<count; i++) {
            std::vector<unsigned char> keyid;
            ASSERT_TRUE(nudf::crypto::ToSha256(KeyChain[i].CeKey, 32, keyid));
            ASSERT_TRUE(KeyChain[i].KeKeyId.IdSize == keyid.size());
            ASSERT_TRUE(0 == memcmp(KeyChain[i].KeKeyId.Id, &keyid[0], keyid.size()));
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        ASSERT_TRUE(FALSE);
    }
}


#endif