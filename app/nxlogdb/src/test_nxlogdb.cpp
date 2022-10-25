

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>

#include <nudf\shared\logdef.h>
#include <nudf\string.hpp>

#include "nxlogdb.hpp"
#include "nxlogdbfile.hpp"
#include "dbglog.hpp"


using namespace NXLOGDB;

static bool __stdcall TestdbgLogAccept(unsigned long level)
{
    return true;
}

static const wchar_t* LevelToName(unsigned long level)
{
    switch (level)
    {
    case LOGCRITICAL:
        return L"CRITICAL:";
    case LOGERROR:
        return L"ERROR:   ";
    case LOGWARNING:
        return L"WARNING: ";
    case LOGINFO:
        return L"INFO:    ";
    case LOGDEBUG:
        return L"DEBUG:   ";
    case LOGDETAIL:
        return L"DETAIL:  ";
    case LOGUSER:
        return L"USERDEF: ";
    default:
        return L"UNKNOWN: ";
    }
}

static void __stdcall TestdbgLogPrint(unsigned long level, const wchar_t* message)
{
    std::wcout << LevelToName(level) << message << std::endl;
}


void dump_conf(const db_conf& conf)
{
    DBGLOG(LOGDEBUG, L"<<<<<<<<<<<<<<<<<<<<<<<<<<");
    DBGLOG(LOGDEBUG, L"DB Header");
    DBGLOG(LOGDEBUG, L"Description: %S", conf.get_description().c_str());
    DBGLOG(LOGDEBUG, L"DBSize:          %d (0x%08X) Bytes", conf.get_db_size(), conf.get_db_size());
    DBGLOG(LOGDEBUG, L"BlockSize:       %d (0x%08X) Bytes", conf.get_block_size(), conf.get_block_size());
    DBGLOG(LOGDEBUG, L"SectorSize:      %d (0x%08X) Bytes", conf.get_sector_size(), conf.get_sector_size());
    DBGLOG(LOGDEBUG, L"SectorDataSize:  %d (0x%08X) Bytes", conf.get_sector_data_size(), conf.get_sector_data_size());
    DBGLOG(LOGDEBUG, L"RecordMapOffset: %d (0x%08X) Bytes", conf.get_record_map_offset(), conf.get_record_map_offset());
    DBGLOG(LOGDEBUG, L"RecordMapSize:   %d (0x%08X) Bytes", conf.get_record_map_size(), conf.get_record_map_size());
    DBGLOG(LOGDEBUG, L"RecordCount:     %d (0x%08X)", conf.get_record_count(), conf.get_record_count());
    DBGLOG(LOGDEBUG, L"SectorMapOffset: %d (0x%08X) Bytes", conf.get_sector_map_offset(), conf.get_sector_map_offset());
    DBGLOG(LOGDEBUG, L"SectorMapSize:   %d (0x%08X) Bytes", conf.get_sector_map_size(), conf.get_sector_map_size());
    DBGLOG(LOGDEBUG, L"SectorCount:     %d (0x%08X)", conf.get_sector_count(), conf.get_sector_count());
    DBGLOG(LOGDEBUG, L"Sector_0_Offset: %d (0x%08X) Bytes", conf.get_sector_start_offset(), conf.get_sector_start_offset());
    DBGLOG(LOGDEBUG, L"FieldsCount:     %d", conf.get_record_layout().get_fields_def().size());
    DBGLOG(LOGDEBUG, L"FieldsSize:      %d (0x%08X) Bytes", conf.get_fields_size(), conf.get_fields_size());
    for (int i = 0; i < (int)conf.get_record_layout().get_fields_def().size(); i++) {
        DBGLOG(LOGDEBUG, L"  > Fields #%d:", i);
        DBGLOG(LOGDEBUG, L"      Name:  %S", conf.get_record_layout().get_fields_def()[i].get_field_name().c_str());
        DBGLOG(LOGDEBUG, L"      Type:  %s", conf.get_record_layout().get_fields_def()[i].get_field_type_name());
        DBGLOG(LOGDEBUG, L"      Flags: 0x%08X", conf.get_record_layout().get_fields_def()[i].get_field_flags());
        DBGLOG(LOGDEBUG, L"      Size:  %d (0x%08X) Bytes", conf.get_record_layout().get_fields_def()[i].get_field_size(), conf.get_record_layout().get_fields_def()[i].get_field_size());
    }
    DBGLOG(LOGDEBUG, L"------------------------->");
}

class audit_logdb_conf : public NXLOGDB::db_conf
{
public:
    audit_logdb_conf()
        : NXLOGDB::db_conf("test data base", 15, NXLOGDB::BLOCK_1MB, NXLOGDB::SECTOR_4096, NXLOGDB::BLOCK_1MB, NXLOGDB::record_layout(std::vector<NXLOGDB::field_definition>({
                    field_definition("timestamp", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, FIELD_FLAG_SEQUENTIAL),
                    field_definition("archived", FIELD_BOOL, FIELD_BOOL_SIZE, 0),
                    field_definition("rights", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0),
                    field_definition("flags", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0),
                    field_definition("user_name", FIELD_CHAR, 64, 0),
                    field_definition("user_id", FIELD_CHAR, 64, 0),
                    field_definition("app_name", FIELD_CHAR, 64, 0),
                    field_definition("file_type", FIELD_CHAR, 32, 0)
                }
            )))
    {
        int a;
        a = 0;
    }
    virtual ~audit_logdb_conf()
    {
    }
};

class audit_logdb_record : public NXLOGDB::db_record
{
public:
    audit_logdb_record() : NXLOGDB::db_record()
    {
    }

    audit_logdb_record(unsigned __int64 timestamp,
        unsigned __int64 rights,
        unsigned long flags,
        const std::wstring& user_name,
        const std::wstring& user_id,
        const std::wstring& app_name,
        const std::wstring& file_type,
        const std::string& json_data) : NXLOGDB::db_record(0xFFFFFFFF, std::vector<NXLOGDB::field_value>({
                NXLOGDB::field_value(timestamp),
                NXLOGDB::field_value(false),
                NXLOGDB::field_value(rights),
                NXLOGDB::field_value(flags),
                NXLOGDB::field_value(user_name),
                NXLOGDB::field_value(user_id),
                NXLOGDB::field_value(app_name),
                NXLOGDB::field_value(file_type)
            }
            ), std::vector<unsigned char>(json_data.begin(), json_data.end()))
    {
    }

    virtual ~audit_logdb_record()
    {
    }
};

std::wstring serialize_time(unsigned __int64 tm)
{
    wchar_t wzTime[256] = { 0 };
    ULARGE_INTEGER uft = { 0, 0 };
    FILETIME ft = { 0, 0 };
    SYSTEMTIME st = { 0 };
    uft.QuadPart = tm;
    FileTimeToLocalFileTime((const FILETIME*)&uft, &ft);
    FileTimeToSystemTime(&ft, &st);
    swprintf_s(wzTime, L"%04d-%02d-%02dT%02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return wzTime;
}

int wmain(int argc, wchar_t** argv)
{
    // set routine
    NXLOGDB::SetDbgLogRoutine(TestdbgLogAccept, TestdbgLogPrint);

    nxlogdb audit_db;

    //std::vector<NXLOGDB::field_definition> fields_def;
    //fields_def.push_back(field_definition("timestamp", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, FIELD_FLAG_SEQUENTIAL));
    //fields_def.push_back(field_definition("archived", FIELD_BOOL, FIELD_BOOL_SIZE, 0));
    //fields_def.push_back(field_definition("rights", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0));
    //fields_def.push_back(field_definition("flags", FIELD_UNSIGNED_INTEGER, FIELD_INTEGER_SIZE, 0));
    //fields_def.push_back(field_definition("user_name", FIELD_CHAR, 64, 0));
    //fields_def.push_back(field_definition("user_id", FIELD_CHAR, 64, 0));
    //fields_def.push_back(field_definition("app_name", FIELD_CHAR, 64, 0));
    //fields_def.push_back(field_definition("file_type", FIELD_CHAR, 32, 0));


    static const wchar_t* app_names[] = { L"WinWord.exe", L"Excel.exe", L"Powerpnt.exe", L"Acrord32.exe" };
    static const wchar_t* file_types[] = { L".docx", L".xlsx", L".pptx", L".pdf" };

    int max_record_count = 4096;

    if (argc > 1) {
        max_record_count = _wtoi(argv[1]);
    }
    if (0 == max_record_count) {
        max_record_count = 4096;
    }

    try {

        audit_logdb_conf default_conf;

        ::DeleteFileW(L"test.db");

        std::cout << "Creating audit database ..." << std::endl;
        audit_db.create(L"test.db", default_conf);
        std::cout << "    Succeeded" << std::endl;

        std::cout << std::endl;
        dump_conf(audit_db.get_conf());
        std::cout << std::endl;

        std::cout << "Writing audit record ..." << std::endl;
        for (int i = 0; i < max_record_count; i++) {
            wchar_t user_name[64] = { 0 };
            wchar_t user_id[64] = { 0 };
            FILETIME ft = { 0, 0 };
            GetSystemTimeAsFileTime(&ft);
            ULARGE_INTEGER uft;
            uft.HighPart = ft.dwHighDateTime;
            uft.LowPart = ft.dwLowDateTime;
            swprintf_s(user_name, 64, L"user_%d@nextlabs.com", i);
            swprintf_s(user_id, 64, L"S-1-5-21-%d", i);
            if (i == 3838) {
                std::cout << "i == 3838" << std::endl;
            }
            audit_db.push_record(audit_logdb_record(uft.QuadPart, 0, 0, user_name, user_id, app_names[i % 4], file_types[i % 4], ""));
        }
        std::cout << "    Succeeded" << std::endl;

        std::cout << "Close audit database" << std::endl;
        audit_db.close();

        std::cout << "Opening audit database ..." << std::endl;
        audit_db.open(L"test.db", true);
        std::cout << "    Succeeded" << std::endl;

        std::cout << std::endl;
        dump_conf(audit_db.get_conf());
        std::cout << std::endl;

        for (int i = 0; i < (int)audit_db.get_conf().get_record_count(); i++) {
            const NXLOGDB::db_record& record = audit_db.read_record(i);
            const std::vector<NXLOGDB::field_definition>& defs = audit_db.get_conf().get_record_layout().get_fields_def();
            const std::vector<NXLOGDB::field_value>& values = record.get_fields();
            std::cout << std::endl;
            std::wstring stime = serialize_time(values[0].as_uint64());
            DBGLOG(LOGINFO, L"Record #%d", i);
            DBGLOG(LOGINFO, L"  %S: %s", defs[0].get_field_name().c_str(), stime.c_str());
            DBGLOG(LOGINFO, L"  %S: %s", defs[1].get_field_name().c_str(), values[1].as_boolean() ? L"true" : L"false");
            DBGLOG(LOGINFO, L"  %S: %08X", defs[2].get_field_name().c_str(), values[2].as_uint64());
            DBGLOG(LOGINFO, L"  %S: %08X", defs[3].get_field_name().c_str(), values[3].as_uint64());
            DBGLOG(LOGINFO, L"  %S: %s", defs[4].get_field_name().c_str(), values[4].as_string().c_str());
            DBGLOG(LOGINFO, L"  %S: %s", defs[5].get_field_name().c_str(), values[5].as_string().c_str());
            DBGLOG(LOGINFO, L"  %S: %s", defs[6].get_field_name().c_str(), values[6].as_string().c_str());
            DBGLOG(LOGINFO, L"  %S: %s", defs[7].get_field_name().c_str(), values[7].as_string().c_str());
        }

        std::cout << "Close audit database" << std::endl;
        audit_db.close();

    }
    catch (std::exception& e) {
        std::cout << "  Failed: " << e.what() << std::endl;
    }

    return 0;
}