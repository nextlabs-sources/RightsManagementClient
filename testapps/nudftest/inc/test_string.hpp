

#ifndef __TEST_STRING_HPP__
#define __TEST_STRING_HPP__

#include <gtest\gtest.h>

#include <nudf\string.hpp>

class StringSuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: String" << std::endl;
   }

   static void TearDownTestCase()
   {
   }
};


TEST_F(StringSuite, StrFormatCheck)
{
    EXPECT_TRUE(nudf::string::IsInteger<wchar_t>(L"127"));
    EXPECT_TRUE(nudf::string::IsInteger<wchar_t>(L"-127"));
    EXPECT_FALSE(nudf::string::IsInteger<wchar_t>(L"-127.0"));

    EXPECT_TRUE(nudf::string::IsHex<wchar_t>(L"C8176DC5B"));
    EXPECT_TRUE(nudf::string::IsHex<wchar_t>(L"0xC8176DC5B"));
    EXPECT_FALSE(nudf::string::IsHex<wchar_t>(L"0xC8176DC5G"));

    EXPECT_TRUE(nudf::string::IsDecimal<wchar_t>(L"189"));
    EXPECT_TRUE(nudf::string::IsDecimal<wchar_t>(L"189.0"));
    EXPECT_FALSE(nudf::string::IsDecimal<wchar_t>(L"B0"));

    EXPECT_TRUE(nudf::string::IsBytesString<wchar_t>(L"007B493D007B493D007B493D007B493D"));

    EXPECT_TRUE(nudf::string::IsDatetime<wchar_t>(L"2009-06-15T13:45:30"));
    EXPECT_TRUE(nudf::string::IsDatetime<wchar_t>(L"2009-06-15T13:45:30Z"));
    EXPECT_FALSE(nudf::string::IsUTCDatetime<wchar_t>(L"2009-06-15T13:45:30"));
    EXPECT_TRUE(nudf::string::IsUTCDatetime<wchar_t>(L"2009-06-15T13:45:30Z"));
    
    EXPECT_TRUE(nudf::string::IsDosPath<wchar_t>(L"C:\\Program Files\\Office\\15\\~Winword.exe"));
    EXPECT_TRUE(nudf::string::IsGlobalDosPath<wchar_t>(L"\\??\\C:\\Program Files\\Office\\15\\~Winword.exe"));
    EXPECT_FALSE(nudf::string::IsDosPath<wchar_t>(L"C:\\Program Files\\Office\\15\\Winword?.exe"));
    EXPECT_FALSE(nudf::string::IsGlobalDosPath<wchar_t>(L"\\??\\C:\\Program Files\\Office\\15\\Winword?.exe"));

    EXPECT_TRUE(nudf::string::IsUncPath<wchar_t>(L"\\\\nextlabs.com\\share\\data\\~design.docx"));
    EXPECT_TRUE(nudf::string::IsGlobalUncPath<wchar_t>(L"\\??\\UNC\\nextlabs.com\\share\\data\\~design.docx"));
    EXPECT_FALSE(nudf::string::IsUncPath<wchar_t>(L"\\\\nextlabs.com\\share\\data\\*design.docx"));
    EXPECT_FALSE(nudf::string::IsGlobalUncPath<wchar_t>(L"\\??\\UNC\\nextlabs.com\\share\\data\\*design.docx"));

    EXPECT_TRUE(nudf::string::IsFileName<wchar_t>(L"~design.docx"));
    EXPECT_FALSE(nudf::string::IsFileName<wchar_t>(L"C:\\Program Files\\Office\\15\\~design.docx"));
    EXPECT_FALSE(nudf::string::IsFileName<wchar_t>(L"|~design.docx"));

    EXPECT_TRUE(nudf::string::IsEmailAddress<wchar_t>(L"Gavin.Ye@nextlabs.com"));
    EXPECT_TRUE(nudf::string::IsEmailAddress<wchar_t>(L"Gavin-Ye@nextlabs.com"));
    EXPECT_TRUE(nudf::string::IsEmailAddress<wchar_t>(L"Gavin_Ye@nextlabs.com"));
    EXPECT_FALSE(nudf::string::IsEmailAddress<wchar_t>(L"Gavin*Ye@nextlabs.com"));
    EXPECT_FALSE(nudf::string::IsEmailAddress<wchar_t>(L"Gavin~Ye@nextlabs.com"));

    EXPECT_TRUE(nudf::string::IsIpv4Address<wchar_t>(L"192.168.0.14"));
    EXPECT_FALSE(nudf::string::IsIpv4Address<wchar_t>(L"192.168.0"));
    EXPECT_FALSE(nudf::string::IsIpv4Address<wchar_t>(L"192.168.0.3672"));
}

TEST_F(StringSuite, StrFromString)
{
    int intVal;
    __int64 int64Val;
    unsigned int uintVal;
    unsigned __int64 uint64Val;
    bool boolVal = false;
    SYSTEMTIME st;
    bool utc = false;

    // 128 == 0x80
    EXPECT_TRUE(nudf::string::ToInt<char>("-128", &intVal) && -128==intVal);
    EXPECT_TRUE(nudf::string::ToInt<char>("+128", &intVal) && 128==intVal);
    EXPECT_FALSE(nudf::string::ToInt<char>("128.0", &intVal) && 128==intVal);
    EXPECT_TRUE(nudf::string::ToUint<char>("80", &uintVal) && 0x80==uintVal);
    EXPECT_TRUE(nudf::string::ToUint<char>("0x80", &uintVal) && 0x80==uintVal);

    // 0xC8176DC5B UUL == 53711658075 LL

    EXPECT_TRUE(nudf::string::ToInt64<char>("-53711658075", &int64Val) && -53711658075==int64Val);
    EXPECT_TRUE(nudf::string::ToInt64<char>("53711658075", &int64Val) && 53711658075==int64Val);
    EXPECT_TRUE(nudf::string::ToUint64<char>("C8176DC5B", &uint64Val) && 0xC8176DC5B==int64Val);
    EXPECT_TRUE(nudf::string::ToUint64<char>("0xC8176DC5B", &uint64Val) && 0xC8176DC5B==int64Val);

    EXPECT_TRUE(nudf::string::ToBoolean<char>("true", &boolVal) && boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("yes", &boolVal) && boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("on", &boolVal) && boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("1", &boolVal) && boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("false", &boolVal) && !boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("no", &boolVal) && !boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("off", &boolVal) && !boolVal);
    EXPECT_TRUE(nudf::string::ToBoolean<char>("0", &boolVal) && !boolVal);
    EXPECT_TRUE(!nudf::string::ToBoolean<char>("unknown", &boolVal));

    EXPECT_TRUE(nudf::string::ToSystemTime<char>("2009-06-15T13:45:30", &st, &utc) && !utc);
    EXPECT_TRUE(nudf::string::ToSystemTime<wchar_t>(L"2009-06-15T13:45:30Z", &st, &utc) && utc);
}

TEST_F(StringSuite, StrToString)
{
    std::wstring wsValue;
    SYSTEMTIME st;
    bool utc = false;
    
    wsValue = nudf::string::FromInt<wchar_t>(128, 4);
    EXPECT_TRUE(wsValue == L"0128");
    wsValue = nudf::string::FromInt<wchar_t>(-128);
    EXPECT_TRUE(wsValue == L"-128");
    wsValue = nudf::string::FromInt64<wchar_t>(9223372036854775807);
    EXPECT_TRUE(wsValue == L"9223372036854775807");
    wsValue = nudf::string::FromInt64<wchar_t>(-9223372036854775807);
    EXPECT_TRUE(wsValue == L"-9223372036854775807");
    wsValue = nudf::string::FromUint<wchar_t>(0x008B49C2);
    EXPECT_TRUE(wsValue == L"0x008B49C2");
    wsValue = nudf::string::FromUint64<wchar_t>(0x000007BF008B49C2);
    EXPECT_TRUE(wsValue == L"0x000007BF008B49C2");
    wsValue = nudf::string::FromBoolean<wchar_t>(true);
    EXPECT_TRUE(wsValue == L"true");
    wsValue = nudf::string::FromBoolean<wchar_t>(false);
    EXPECT_TRUE(wsValue == L"false");
    
    EXPECT_TRUE(nudf::string::ToSystemTime<char>("2009-06-15T13:45:30", &st, &utc) && !utc);
    wsValue = nudf::string::FromSystemTime<wchar_t>(&st, false);
    EXPECT_TRUE(wsValue == L"2009-06-15T13:45:30");
    wsValue = nudf::string::FromSystemTime<wchar_t>(&st, true);
    EXPECT_TRUE(wsValue == L"2009-06-15T13:45:30Z");
}

TEST_F(StringSuite, StrUtil)
{
    std::wstring wsTrim(L"  This is a Test  ");
    wsTrim = nudf::string::TrimLeft(wsTrim);
    EXPECT_TRUE(0 == wcscmp(wsTrim.c_str(), L"This is a Test  "));
    wsTrim = nudf::string::TrimRight(wsTrim);
    EXPECT_TRUE(0 == wcscmp(wsTrim.c_str(), L"This is a Test"));
    wsTrim = nudf::string::Trim(wsTrim);
    EXPECT_TRUE(0 == wcscmp(wsTrim.c_str(), L"ThisisaTest"));
}



#endif