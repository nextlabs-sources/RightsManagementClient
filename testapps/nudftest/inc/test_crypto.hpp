

#ifndef __TEST_CRYPTO_HPP__
#define __TEST_CRYPTO_HPP__

#include <gtest\gtest.h>
#include <nudf\crypto.hpp>
#include <nudf\encoding.hpp>
#include <nudf\stopwatch.hpp>

class CryptoSuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: Crypto" << std::endl;
   }

   static void TearDownTestCase()
   {
   }

public:
   static std::wstring _crc32_string;
   static std::wstring _crc64_string;
   static std::wstring _source_string;
   static unsigned long _crc32;
   static unsigned __int64 _crc64;   
   static nudf::crypto::CAesKeyBlob _aes_key;
   static unsigned char* data;
   
   const static std::wstring _sign_pub_cert;
   const static std::wstring _sign_pri_cert;
   const static std::wstring _exch_pub_cert;
   const static std::wstring _exch_pri_cert;
};

std::wstring CryptoSuite::_crc32_string = L"CRC 32 Test Data";
std::wstring CryptoSuite::_crc64_string = L"CRC 64 Test Data";
std::wstring CryptoSuite::_source_string = L"This is a string for Crypto test";
unsigned long CryptoSuite::_crc32 = nudf::crypto::ToCrc32(0, CryptoSuite::_crc32_string.c_str(), (unsigned long)CryptoSuite::_crc32_string.length()*sizeof(wchar_t));
unsigned __int64 CryptoSuite::_crc64 = nudf::crypto::ToCrc64(0, CryptoSuite::_crc64_string.c_str(), (unsigned long)CryptoSuite::_crc64_string.length()*sizeof(wchar_t));
unsigned char* CryptoSuite::data = NULL;

const std::wstring CryptoSuite::_sign_pub_cert(L"d:\\certs\\sign.cer");
const std::wstring CryptoSuite::_sign_pri_cert(L"d:\\certs\\sign.pfx");
const std::wstring CryptoSuite::_exch_pub_cert(L"d:\\certs\\NextLabsExch.cer");
const std::wstring CryptoSuite::_exch_pri_cert(L"d:\\certs\\NextLabsExch.pfx");

TEST_F(CryptoSuite, Crc32Test)
{
    EXPECT_EQ(CryptoSuite::_crc32, nudf::crypto::ToCrc32(0, CryptoSuite::_crc32_string.c_str(), (unsigned long)CryptoSuite::_crc32_string.length()*sizeof(wchar_t)));
}

TEST_F(CryptoSuite, Crc64Test)
{
    EXPECT_EQ(CryptoSuite::_crc64, nudf::crypto::ToCrc64(0, CryptoSuite::_crc64_string.c_str(), (unsigned long)CryptoSuite::_crc64_string.length()*sizeof(wchar_t)));
}

TEST_F(CryptoSuite, AesTest)
{
    unsigned char* data0 = NULL;
    unsigned char* data1 = NULL;
    bool result = false;

    // 64 MB = 64 * (1024 * 10240) = 67108864 Bytes
    // 16 MB = 16 * (1024 * 10240) = 16777216 Bytes
    // 80 MB = 16 MB + 64 MB = 83886080 Bytes
    result = SetProcessWorkingSetSize(GetCurrentProcess(), 83886080*2, 83886080*2) ? true : false;
    EXPECT_TRUE(result);
    if(!result) {
        return;
    }

    data0 = (unsigned char*)VirtualAlloc(NULL, 67108864, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    EXPECT_TRUE(NULL != data0);
    if(NULL == data0) {
        SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
    }
    result = VirtualLock(data0, 67108864) ? true : false;
    EXPECT_TRUE(result);
    if(!result) {
        VirtualFree(data0, 67108864, 0);
        SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
        return;
    }

    data1 = (unsigned char*)VirtualAlloc(NULL, 67108864, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    EXPECT_TRUE(NULL != data1);
    if(NULL == data1) {
        VirtualFree(data0, 67108864, 0);
        SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
    }    
    result = VirtualLock(data1, 67108864) ? true : false;
    EXPECT_TRUE(result);
    if(!result) {
        VirtualFree(data0, 67108864, 0);
        VirtualFree(data1, 67108864, 0);
        SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
        return;
    }

    memset(data0, 'Z', 67108864);
    memset(data1, 'Z', 67108864);

    nudf::crypto::CAesKeyBlob key;
    result = (S_OK == key.Generate(256));
    EXPECT_TRUE(result);
    if(!result) {
        VirtualFree(data0, 67108864, 0);
        VirtualFree(data1, 67108864, 0);
        SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
        return;
    }

    nudf::util::CHighResolStopWatch sw;
    std::wstring sperf;
    __int64 total_time = 0;

    for(int i=0; i<10; i++) {
        memset(data1, 'Z', 67108864);
        sw.Start();
        result = nudf::crypto::AesEncrypt(key, data1, 67108864UL, 0ULL);
        sw.Stop();
        total_time += sw.GetElapse();
        sperf = nudf::string::FromInt64<wchar_t>(sw.GetElapse());
        std::wcout << L"Aes Encrypt 64MB Data. Time used: "<< sperf << std::endl;
        EXPECT_TRUE(result);
        if(!result) {
            VirtualFree(data0, 67108864, 0);
            VirtualFree(data1, 67108864, 0);
            SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
            return;
        }
    }
    sperf = nudf::string::FromInt64<wchar_t>(total_time/10);
    std::wcout << L"Aes Encrypt 64MB Data. Avg. Time: "<< sperf << std::endl;
    sperf = nudf::string::FromInt64<wchar_t>((__int64)(64.0 / (total_time/10000000.0)));
    std::wcout << L"Aes Encrypt Avg. Speed: "<< sperf << L" MB/s" << std::endl;

    result = nudf::crypto::AesDecrypt(key, data1, 67108864UL, 0ULL);
    EXPECT_TRUE(result);
    EXPECT_TRUE(0 == memcmp(data0, data1, 67108864));

    VirtualFree(data0, 67108864, 0);
    VirtualFree(data1, 67108864, 0);
    SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}

TEST_F(CryptoSuite, CertTest)
{
    HRESULT result = S_OK;

    nudf::crypto::CX509CertContext   signpubcert;
    nudf::crypto::CPkcs12CertContext signpricert;
    nudf::crypto::CLegacyRsaPubKeyBlob pubkey;
    nudf::crypto::CLegacyRsaPriKeyBlob prikey;

    result = signpubcert.CreateFromFile(CryptoSuite::_sign_pub_cert.c_str());
    EXPECT_TRUE(result == S_OK);

    result = signpubcert.GetPublicKeyBlob(pubkey);
    EXPECT_TRUE(result == S_OK);

    result = signpricert.CreateFromFile(CryptoSuite::_sign_pri_cert.c_str(), L"123blue!");
    EXPECT_TRUE(result == S_OK);
    
    result = signpricert.GetPrivateKeyBlob(prikey);
    EXPECT_TRUE(result == S_OK);


    std::vector<unsigned char> signature;
    result = nudf::crypto::RsaSign(prikey, CryptoSuite::_source_string.c_str(), 2*((unsigned long)CryptoSuite::_source_string.length()+1), signature);
    EXPECT_TRUE(result == S_OK);

    if(result == S_OK) {
        //nudf::crypto::CRsaPubKeyBlob blob;
        //blob.SetBlob(&pubkey[0], (ULONG)pubkey.size());
        result = nudf::crypto::RsaVerifySignature(pubkey, CryptoSuite::_source_string.c_str(), 2*((ULONG)CryptoSuite::_source_string.length()+1), &signature[0], (unsigned long)signature.size());
        EXPECT_TRUE(result == S_OK);
    }

    // Create a new cert
    nudf::crypto::CCertContext   newcert;
    result = newcert.Create(L"CN=NextLabs Engineer Root Authority; O=NextLabs; OU=Engineer; C=US; L=San Mateo; ST=California", true, true, 10);
    EXPECT_TRUE(result == S_OK);

    nudf::crypto::CMemCertStore   memstore;
    result = memstore.Open();
    EXPECT_TRUE(result == S_OK);

    result = memstore.AddCert(newcert);
    EXPECT_TRUE(result == S_OK);

    result = memstore.ExportToPkcs12Package(L"test.pfx", L"123blue!", true);
    EXPECT_TRUE(result == S_OK);

    nudf::crypto::CSysCertStore   sysstore(L"MY");

    result = sysstore.Open();
    EXPECT_TRUE(result == S_OK);

    result = sysstore.AddCert(newcert);
    EXPECT_TRUE(result == S_OK);
}

TEST_F(CryptoSuite, RsaTest)
{
}

TEST_F(CryptoSuite, Md5Test)
{
    std::vector<unsigned char> md5_00;
    std::vector<unsigned char> md5_01;
    EXPECT_TRUE(nudf::crypto::ToMd5(_source_string.c_str(), (unsigned long)_source_string.length()*sizeof(wchar_t), md5_00));
    EXPECT_EQ(16, md5_00.size());
    EXPECT_TRUE(nudf::crypto::ToMd5(_source_string.c_str(), (unsigned long)_source_string.length()*sizeof(wchar_t), md5_01));
    EXPECT_EQ(16, md5_01.size());
    EXPECT_EQ(md5_00, md5_01);
}

TEST_F(CryptoSuite, Sha1Test)
{
    std::vector<unsigned char> sha1_00;
    std::vector<unsigned char> sha1_01;
    EXPECT_TRUE(nudf::crypto::ToSha1(_source_string.c_str(), (unsigned long)_source_string.length()*sizeof(wchar_t), sha1_00));
    EXPECT_EQ(20, sha1_00.size());
    EXPECT_TRUE(nudf::crypto::ToSha1(_source_string.c_str(), (unsigned long)_source_string.length()*sizeof(wchar_t), sha1_01));
    EXPECT_EQ(20, sha1_01.size());
    EXPECT_EQ(sha1_00, sha1_01);
}

TEST_F(CryptoSuite, Sha256Test)
{
    std::vector<unsigned char> sha256_00;
    std::vector<unsigned char> sha256_01;
    EXPECT_TRUE(nudf::crypto::ToSha256(_source_string.c_str(), (unsigned long)_source_string.length()*sizeof(wchar_t), sha256_00));
    EXPECT_EQ(32, sha256_00.size());
    EXPECT_TRUE(nudf::crypto::ToSha256(_source_string.c_str(), (unsigned long)_source_string.length()*sizeof(wchar_t), sha256_01));
    EXPECT_EQ(32, sha256_01.size());
    EXPECT_EQ(sha256_00, sha256_01);
}

TEST_F(CryptoSuite, Base64Test)
{
    std::wstring s = nudf::util::encoding::Base64Encode<wchar_t>(_source_string.c_str(), (unsigned long)(_source_string.length()+1)*sizeof(wchar_t));
    std::vector<unsigned char> data;

    EXPECT_TRUE(nudf::util::encoding::Base64Decode<wchar_t>(s, data, false));
    std::wstring ws((const wchar_t*)(&data[0]));
    EXPECT_EQ(_source_string, ws);
}


#endif