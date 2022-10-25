

#include <Windows.h>

#include <string>
#include <list>
#include <iostream>
#include <fstream>

#include "pql_parser.hpp"
#include "pql_bundle.hpp"

using namespace pql;
using namespace BOOST_SPIRIT_CLASSIC_NS;


std::string  s1("user.GROUP has 26 AND ((application.path = \"**\\\\Excel.exe AND OR test\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\\\AcroRd32.exe\" AND application.publisher = \"Adobe Systems, Incorporated\") OR (application.path = \"**\\\\POWERPNT.EXE\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\\\WINWORD.EXE\" AND application.publisher = \"Microsoft Corporation\") OR (application.path = \"**\\\\VEViewer.exe\" AND application.publisher = \"sap ag\") OR application.name = \"RMS\")");
std::string  s2("TRUE AND (ENVIRONMENT.TIME_SINCE_LAST_HEARTBEAT > 2592000 AND TRUE)");


void main()
{
    pql::pql_parser parser;
    pql::pql_tree   tree;

    std::cout << "Original PQL:" << std::endl;
    std::cout << s1 << std::endl;
    std::cout << "" << std::endl;

    // Parse PQL
    parser.parse(s1, tree);

    std::cout << "Print PQL Tree" << std::endl;
    std::string s = tree.serialize();
    std::cout << s << std::endl;
    std::cout << "" << std::endl;
    std::cout << "" << std::endl;

    //pql_grammar parser;

    //tree_parse_info<> info = pt_parse(s1.c_str(), parser, space_p);

    //if (!info.full) {
    //    std::cout << "stopped at: \": " << info.stop << "\"\n";
    //}

    //pql::bundle::_Bundle bundle;

    //std::ifstream fs;
    //fs.open("D:\\dev\\testdata\\policybundle\\lam.out");
    //if (!fs.good()) {
    //    std::cout << "fail to open file" << std::endl;
    //    return;
    //}

    //bundle.parse(fs);

    system("pause");
}