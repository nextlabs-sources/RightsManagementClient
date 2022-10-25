

#include <Windows.h>

#include <iostream>
#include <string>


#include <nudf\crypto.hpp>
#include <nudf\encoding.hpp>
#include <nudf\cert.hpp>
#include <nudf\regex.hpp>
#include <nudf\lrucache.hpp>
#include <nudf\string.hpp>

#include <gtest\gtest.h>

#include "test_string.hpp"
#include "test_regex.hpp"
#include "test_lrucache.hpp"
#include "test_profile.hpp"
#include "test_crypto.hpp"


const std::wstring sign_pub_cert(L"d:\\certs\\sign.cer");
const std::wstring sign_pri_cert(L"d:\\certs\\sign.pfx");
const std::wstring exch_pub_cert(L"d:\\certs\\exch.cer");
const std::wstring exch_pri_cert(L"d:\\certs\\exch.pfx");

std::wstring    gDir;
std::wstring    gProfile;

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    
    nudf::crypto::Initialize();

    nudf::win::CModulePath mod(NULL);
    gDir     = mod.GetParentDir();
    gProfile += gDir + L"\\profile.xml";

    return RUN_ALL_TESTS();
}