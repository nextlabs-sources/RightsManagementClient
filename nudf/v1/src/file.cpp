
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\file.hpp>

using namespace nudf::win::file;


CFile::CFile() : _Handle(INVALID_HANDLE_VALUE)
{
}

CFile::~CFile()
{
}