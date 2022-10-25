

#ifndef __TEST_REGEX_HPP__
#define __TEST_REGEX_HPP__

#include <gtest\gtest.h>
#include <nudf\regex.hpp>

class RegexSuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: Regex" << std::endl;
   }

   static void TearDownTestCase()
   {
   }
};



TEST_F(RegexSuite, RgxTrim)
{
    std::wstring trim_str1(L",,1,3,5,7,,,");
    std::wstring trim_str2(L"this is a cat");

    trim_str1 = nudf::util::regex::TrimLeft<wchar_t>(trim_str1, L',', false);
    EXPECT_TRUE(std::wstring(L"1,3,5,7,,,") == trim_str1);
    trim_str1 = nudf::util::regex::TrimRight<wchar_t>(trim_str1, L',', false);
    EXPECT_TRUE(std::wstring(L"1,3,5,7") == trim_str1);
    trim_str1 = nudf::util::regex::TrimAll<wchar_t>(trim_str1, L',', false);
    EXPECT_TRUE(std::wstring(L"1357") == trim_str1);
    trim_str2 = nudf::util::regex::TrimAll<wchar_t>(trim_str2, L' ', false);
    EXPECT_TRUE(std::wstring(L"thisisacat") == trim_str2);
}

TEST_F(RegexSuite, RgxReplace)
{
    std::wstring wsDate(L"1/30/2015 15.3.46");
    wsDate = nudf::util::regex::SimpleReplaceAll<wchar_t>(wsDate, L"/", L"-", false);
    EXPECT_TRUE(std::wstring(L"1-30-2015 15.3.46") == wsDate);
    wsDate = nudf::util::regex::SimpleReplaceAll<wchar_t>(wsDate, L".", L":", false);
    EXPECT_TRUE(std::wstring(L"1-30-2015 15:3:46") == wsDate);
    wsDate = nudf::util::regex::SimpleReplaceAll<wchar_t>(wsDate, L" ", L"T", false);
    EXPECT_TRUE(std::wstring(L"1-30-2015T15:3:46") == wsDate);
}

TEST_F(RegexSuite, RgxMatch)
{
    std::wstring wsPathPattern(L"C:\\Program Files*\\Adobe\\Reader\\*.exe");
    std::wstring wsFilePattern(L"C:\\Users\\*\\Desktop\\?c?c?c?c?c?c?c?c.tmp");
    std::wstring wsFilePattern2(L"C:\\Users\\*\\Desktop\\?d?d?d*.doc*");
    std::wstring wsAppPattern(L"**\\notepad.exe");


    wsPathPattern = nudf::util::regex::WildcardsToRegex<wchar_t>(wsPathPattern);
    wsFilePattern = nudf::util::regex::WildcardsToRegexEx<wchar_t>(wsFilePattern);
    wsFilePattern2 = nudf::util::regex::WildcardsToRegexEx<wchar_t>(wsFilePattern2);
    wsAppPattern = nudf::util::regex::WildcardsToRegexEx<wchar_t>(wsAppPattern);

    EXPECT_TRUE(nudf::util::regex::Match<wchar_t>(L"C:\\Program Files (x86)\\Adobe\\Reader\\AcroRd32.exe", wsPathPattern, true));
    EXPECT_TRUE(nudf::util::regex::Match<wchar_t>(L"C:\\Users\\gye\\Desktop\\2F3E2782.tmp", wsFilePattern, true));
    EXPECT_FALSE(nudf::util::regex::Match<wchar_t>(L"C:\\Users\\gye\\Desktop\\2F3E2782.tmp", wsFilePattern2, true));
    EXPECT_TRUE(nudf::util::regex::Match<wchar_t>(L"C:\\Users\\john.tyler\\Desktop\\333New.docx", wsFilePattern2, true));
    EXPECT_FALSE(nudf::util::regex::Match<wchar_t>(L"C:\\Users\\john.tyler\\Desktop\\~6FNew.docx", wsFilePattern2, true));
    EXPECT_TRUE(nudf::util::regex::Match<wchar_t>(L"C:\\Windows\\system32\\notepad.exe", wsAppPattern, true));
}


#endif