// nxlogdb.cpp : Defines the exported functions for the DLL application.
//

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>

#include <nudf\shared\logdef.h>
#include <nudf\string.hpp>


#include "dbglog.hpp"
#include "nxlogdb.hpp"
#include "nxlogdbfile.hpp"


using namespace NXLOGDB;

//
// HEADER FILE
//
//extern NXLOGDB_API int nnxlogdb;
//NXLOGDB_API int fnnxlogdb(void);

//
// SOURCE FILE
//
// This is an example of an exported variable
//NXLOGDB_API int nnxlogdb=0;
// This is an example of an exported function.
//NXLOGDB_API int fnnxlogdb(void)
//{
//    return 42;
//}

NXLOGDB_API const unsigned long DB_HEADER_SIZE = sizeof(DBHEADER);

static const unsigned char DB_MAGIC_CODE[8] = { 'N', 'X', 'L', 'O', 'G', 'D', 'B', 0xFB };

static DBG_LOG_ACCEPT  GlobalDbgLogAcceptFunc = nullptr;
static DBG_LOG  GlobalDbgLogFunc = nullptr;
bool DBGLOGACCEPT(unsigned long level)
{
    return (nullptr != GlobalDbgLogAcceptFunc) ? GlobalDbgLogAcceptFunc(level) : false;
}
void DBGLOG(unsigned long level, const wchar_t* message, ...)
{
    if (nullptr != GlobalDbgLogFunc && DBGLOGACCEPT(level)) {

        va_list args;
        size_t  len = 0;
        std::wstring s;

        va_start(args, message);
        len = _vscwprintf_l(message, 0, args) + 1;
        vswprintf_s(nudf::string::tempstr<wchar_t>(s, len), len, message, args); // C4996
        va_end(args);

        GlobalDbgLogFunc(level, s.c_str());
    }
}

NXLOGDB_API void __stdcall NXLOGDB::SetDbgLogRoutine(DBG_LOG_ACCEPT pAccept, DBG_LOG pLog)
{
    assert(NULL != pAccept);
    assert(NULL != pLog);
    GlobalDbgLogAcceptFunc = pAccept;
    GlobalDbgLogFunc = pLog;
}


NXLOGDB_API bool __stdcall NXLOGDB::db_conf_validate(const db_conf& conf)
{
    DBGLOG(LOGDEBUG, L"Validate logdb_ifno");

    if (conf.empty()) {
        DBGLOG(LOGDEBUG, L"  --> Empty");
        return false;
    }

    if (0 != (conf.get_block_size() % ONE_MEGA_BYTES)) {
        DBGLOG(LOGDEBUG, L"  --> Invalid block_size (%d)", conf.get_block_size());
        return false;
    }

    if (conf.get_db_size() > ONE_GIGA_BYTES) {
        DBGLOG(LOGDEBUG, L"  --> Invalid db_size (%d)", conf.get_db_size());
        return false;
    }

    if (conf.get_sector_size() != SECTOR_512 && conf.get_sector_size() != SECTOR_1024 && conf.get_sector_size() != SECTOR_2048 && conf.get_sector_size() != SECTOR_4096) {
        DBGLOG(LOGDEBUG, L"  --> Invalid sector_size (%d)", conf.get_sector_size());
        return false;
    }

    if (conf.get_record_layout().get_fields_def().size() > MAX_FIELDS_NUMBER) {
        DBGLOG(LOGDEBUG, L"  --> Invalid fields number (%d)", conf.get_record_layout().get_fields_def().size());
        return false;
    }

    const unsigned long expected_sector_count = conf.get_db_size() / conf.get_sector_size();
    const unsigned long expected_fields_size = conf.get_record_layout().get_fields_size();
    const unsigned long expected_record_map_size = expected_sector_count * sizeof(unsigned long);
    const unsigned long expected_sector_map_size = ((expected_sector_count + 31) / 32) * sizeof(unsigned long);
    const unsigned long expected_record_map_offset = DB_HEADER_SIZE;
    const unsigned long expected_sector_map_offset = expected_record_map_offset + expected_record_map_size;
    const unsigned long expected_sector_start_offset = ROUND_TO_SIZE((expected_sector_map_offset + expected_sector_map_size), conf.get_sector_size());

    if (expected_sector_count != conf.get_sector_count()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid sector_count (%d, expected %d)", conf.get_sector_count(), expected_sector_count);
        return false;
    }
    if (expected_fields_size != conf.get_fields_size()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid field_size (%d, expected %d)", conf.get_fields_size(), expected_fields_size);
        return false;
    }
    if (expected_record_map_size != conf.get_record_map_size()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid record_map_size (%d, expected %d)", conf.get_record_map_size(), expected_record_map_size);
        return false;
    }
    if (expected_sector_map_size != conf.get_sector_map_size()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid sector_map_size (%d, expected %d)", conf.get_sector_map_size(), expected_sector_map_size);
        return false;
    }
    if (expected_record_map_offset != conf.get_record_map_offset()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid record_map_offset (%d, expected %d)", conf.get_record_map_offset(), expected_record_map_offset);
        return false;
    }
    if (expected_sector_map_offset != conf.get_sector_map_offset()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid sector_map_offset (%d, expected %d)", conf.get_sector_map_offset(), expected_sector_map_offset);
        return false;
    }
    if (expected_sector_start_offset != conf.get_sector_start_offset()) {
        DBGLOG(LOGDEBUG, L"  --> Invalid sector_start_offset (%d, expected %d)", conf.get_sector_start_offset(), expected_sector_start_offset);
        return false;
    }

    DBGLOG(LOGDEBUG, L"  --> Succeed");
    return true;
}



//
//  exported class: nxlogdb
//

nxlogdb::nxlogdb() : _dbfile(nullptr)
{
    _dbfile = new logdb_file();
}

nxlogdb::~nxlogdb()
{
    if (nullptr != _dbfile) {
        delete _dbfile;
        _dbfile = nullptr;
    }
}


void nxlogdb::create(const std::wstring& file, const db_conf& conf)
{
    _dbfile->create(file, conf);
}

void nxlogdb::open(const std::wstring& file, bool read_only)
{
    _dbfile->open(file, read_only);
}

void nxlogdb::close()
{
    _dbfile->close();
}

const db_conf& nxlogdb::get_conf() const
{
    return _dbfile->get_db_conf();
}

std::vector<db_record> nxlogdb::query(std::shared_ptr<db_query_condition> query_condition,
    unsigned long start_pos,
    bool descending,
    unsigned long count_to_get)
{
    return std::vector<db_record>();
}

std::vector<db_record> nxlogdb::query(std::shared_ptr<db_query_condition> query_condition,
    OPAQUE_DATA_QUERY opaque_query,
    unsigned long start_pos,
    bool descending,
    unsigned long count_to_get)
{
    return std::vector<db_record>();
}

unsigned long nxlogdb::get_record_count() const
{
    return _dbfile->get_db_conf().get_record_count();
}

db_record nxlogdb::read_record(unsigned long record_id)
{
    return _dbfile->read_record(record_id);
}

void nxlogdb::push_record(const db_record& record)  
{
    _dbfile->push_record(record);
}

void nxlogdb::pop_record()
{
    _dbfile->pop_record();
}

void nxlogdb::export_log(const std::wstring& file) const
{
    _dbfile->export_log(file);
}