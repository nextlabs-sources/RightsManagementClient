
#include <Windows.h>
#include <atlbase.h>
#include <atlcomcli.h>


#include <string>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\xmlparser.hpp>

#include "profile_classify.hpp"



using namespace nxrm::pkg;


CClassifyProfile::CClassifyProfile() : CProfile()
{
}

CClassifyProfile::~CClassifyProfile()
{
}

void CClassifyProfile::FromXml(IXMLDOMNode* node)
{
}

void CClassifyProfile::ToXml(IXMLDOMDocument* doc, IXMLDOMNode* parent, IXMLDOMNode** node) const
{
}