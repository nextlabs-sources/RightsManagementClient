

#ifndef __TEST_LRUCACHE_HPP__
#define __TEST_LRUCACHE_HPP__

#include <gtest\gtest.h>
#include <nudf\lrucache.hpp>
#include <nudf\listentry.hpp>

class LruSuite : public testing::Test
{
protected:
   static void SetUpTestCase()
   {
       std::cout << std::endl;
       std::cout << "TestSuite: LRU Cache" << std::endl;
   }

   static void TearDownTestCase()
   {
   }
   
   static nudf::util::CLRUList<std::wstring, std::pair<std::wstring,std::wstring>, 10> _cache;
};

nudf::util::CLRUList<std::wstring, std::pair<std::wstring,std::wstring>, 10> LruSuite::_cache;


TEST_F(LruSuite, LruInsert)
{
    const std::pair<std::wstring,std::wstring> item00(L"C:\\test\\00.docx", L"doc 0");
    const std::pair<std::wstring,std::wstring> item01(L"C:\\test\\01.docx", L"doc 1");
    const std::pair<std::wstring,std::wstring> item02(L"C:\\test\\02.docx", L"doc 2");
    const std::pair<std::wstring,std::wstring> item03(L"C:\\test\\03.docx", L"doc 3");
    const std::pair<std::wstring,std::wstring> item04(L"C:\\test\\04.docx", L"doc 4");
    const std::pair<std::wstring,std::wstring> item05(L"C:\\test\\05.docx", L"doc 5");
    const std::pair<std::wstring,std::wstring> item06(L"C:\\test\\06.docx", L"doc 6");
    const std::pair<std::wstring,std::wstring> item07(L"C:\\test\\07.docx", L"doc 7");
    const std::pair<std::wstring,std::wstring> item08(L"C:\\test\\08.docx", L"doc 8");
    const std::pair<std::wstring,std::wstring> item09(L"C:\\test\\09.docx", L"doc 9");


    EXPECT_TRUE(_cache.Insert(item00.first, item00));
    EXPECT_EQ(1, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item01.first, item01));
    EXPECT_EQ(2, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item02.first, item02));
    EXPECT_EQ(3, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item03.first, item03));
    EXPECT_EQ(4, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item04.first, item04));
    EXPECT_EQ(5, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item05.first, item05));
    EXPECT_EQ(6, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item06.first, item06));
    EXPECT_EQ(7, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item07.first, item07));
    EXPECT_EQ(8, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item08.first, item08));
    EXPECT_EQ(9, _cache.Size());

    EXPECT_TRUE(_cache.Insert(item09.first, item09));
    EXPECT_EQ(10, _cache.Size());
}

TEST_F(LruSuite, LruExceedSize)
{
    const std::pair<std::wstring,std::wstring> item00(L"C:\\test\\00.docx", L"doc 0");
    const std::pair<std::wstring,std::wstring> item01(L"C:\\test\\01.docx", L"doc 1");
    const std::pair<std::wstring,std::wstring> item02(L"C:\\test\\02.docx", L"doc 2");
    const std::pair<std::wstring,std::wstring> item03(L"C:\\test\\03.docx", L"doc 3");
    const std::pair<std::wstring,std::wstring> item04(L"C:\\test\\04.docx", L"doc 4");
    const std::pair<std::wstring,std::wstring> item05(L"C:\\test\\05.docx", L"doc 5");
    const std::pair<std::wstring,std::wstring> item06(L"C:\\test\\06.docx", L"doc 6");
    const std::pair<std::wstring,std::wstring> item07(L"C:\\test\\07.docx", L"doc 7");
    const std::pair<std::wstring,std::wstring> item08(L"C:\\test\\08.docx", L"doc 8");
    const std::pair<std::wstring,std::wstring> item09(L"C:\\test\\09.docx", L"doc 9");
    const std::pair<std::wstring,std::wstring> item10(L"C:\\test\\10.docx", L"doc 10");
    const std::pair<std::wstring,std::wstring> item11(L"C:\\test\\11.docx", L"doc 11");
    const std::pair<std::wstring,std::wstring> item12(L"C:\\test\\12.docx", L"doc 12");
    const std::pair<std::wstring,std::wstring> item13(L"C:\\test\\13.docx", L"doc 13");

    std::pair<std::wstring,std::wstring> item;    

    EXPECT_TRUE(_cache.Insert(item10.first, item10));
    EXPECT_EQ(10, _cache.Size());
    EXPECT_FALSE(_cache.Find(item00.first, item));

    EXPECT_TRUE(_cache.Insert(item11.first, item11));
    EXPECT_EQ(10, _cache.Size());
    EXPECT_FALSE(_cache.Find(item00.first, item));
    EXPECT_FALSE(_cache.Find(item01.first, item));

    EXPECT_TRUE(_cache.Insert(item12.first, item12));
    EXPECT_EQ(10, _cache.Size());
    EXPECT_FALSE(_cache.Find(item00.first, item));
    EXPECT_FALSE(_cache.Find(item01.first, item));
    EXPECT_FALSE(_cache.Find(item02.first, item));

    EXPECT_TRUE(_cache.Insert(item13.first, item13));
    EXPECT_EQ(10, _cache.Size());
    EXPECT_FALSE(_cache.Find(item00.first, item));
    EXPECT_FALSE(_cache.Find(item01.first, item));
    EXPECT_FALSE(_cache.Find(item02.first, item));
    EXPECT_FALSE(_cache.Find(item03.first, item));
}

TEST_F(LruSuite, LruFind)
{
    const std::pair<std::wstring,std::wstring> item04(L"C:\\test\\04.docx", L"doc 4");
    const std::pair<std::wstring,std::wstring> item05(L"C:\\test\\05.docx", L"doc 5");
    const std::pair<std::wstring,std::wstring> item06(L"C:\\test\\06.docx", L"doc 6");
    const std::pair<std::wstring,std::wstring> item07(L"C:\\test\\07.docx", L"doc 7");
    const std::pair<std::wstring,std::wstring> item08(L"C:\\test\\08.docx", L"doc 8");
    const std::pair<std::wstring,std::wstring> item09(L"C:\\test\\09.docx", L"doc 9");
    const std::pair<std::wstring,std::wstring> item10(L"C:\\test\\10.docx", L"doc 10");
    const std::pair<std::wstring,std::wstring> item11(L"C:\\test\\11.docx", L"doc 11");
    const std::pair<std::wstring,std::wstring> item12(L"C:\\test\\12.docx", L"doc 12");
    const std::pair<std::wstring,std::wstring> item13(L"C:\\test\\13.docx", L"doc 13");

    std::wstring key;
    std::pair<std::wstring,std::wstring> item;    

    EXPECT_TRUE(_cache.Find(item09.first, item));
    EXPECT_EQ(item09, item);
    EXPECT_TRUE(_cache.RemoveHead(key, item));
    EXPECT_EQ(item09, item);

    EXPECT_TRUE(_cache.Find(item05.first, item));
    EXPECT_EQ(item05, item);
    EXPECT_TRUE(_cache.RemoveHead(key, item));
    EXPECT_EQ(item05, item);

    EXPECT_TRUE(_cache.Find(item13.first, item));
    EXPECT_EQ(item13, item);
    EXPECT_TRUE(_cache.RemoveHead(key, item));
    EXPECT_EQ(item13, item);
}



#endif