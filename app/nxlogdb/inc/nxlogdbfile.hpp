
#pragma once
#ifndef __NXLOGDB_FILE_HPP__
#define __NXLOGDB_FILE_HPP__


#include <string>
#include <boost\noncopyable.hpp>

//  1MB =    1048576 (  0x100000) Bytes
//  1GB = 1073741824 (0x40000000) Bytes

//
// Max file size is 256 MB (268435456 / 0x10000000 Bytes)
// Page size is 512 KB
// Sector size is 512 Bytes
//

//
//  4096 sectors per page
//  256 MB has 524288 (0x80000) sectors, 128 Pages
//  Map size for 0x200000 sectors is 262144 (0x40000) Bytes, 256 KB
//

namespace NXLOGDB {

#pragma pack(push)
#pragma pack(4)


typedef struct _FIELD_INFO {
    char            name[MAX_FIELD_NAME_SIZE];   // Offset: 0  Bytes
    unsigned long   type;       // Offset: 20 Bytes
    unsigned long   size;       // Offset: 24 Bytes
    unsigned long   flags;      // Offset: 28 Bytes
} FIELD_INFO, *PFIELD_INFO;     // Size:   32 Bytes


/*
  magic:        a predefined hard coded magic code
  max_size:     maximum size the log file could be
  flags:        log db flags
  record_start: the start position of the record list
  record_count: the number of valid records
  description:  the description of this log db
  fields:       list of fields, we support up to 128 fields
  reserved:     not use
*/
typedef struct _DBHEADER {
    unsigned __int64    magic;                              // Offset: 0    Bytes
    char                description[MAX_DESCRIPTION_SIZE];  // Offset: 8    Bytes: utf8 encoded
    unsigned long       db_flags;                           // Offset: 128  Bytes
    unsigned long       db_size;                            // Offset: 132  Bytes
    unsigned long       sector_size;                        // Offset: 136  Bytes
    unsigned long       block_size;                         // Offset: 140  Bytes
    unsigned long       record_map_offset;                  // Offset: 144  Bytes
    unsigned long       record_map_size;                    // Offset: 148  Bytes
    unsigned long       record_start_id;                    // Offset: 152  Bytes
    unsigned long       record_count;                       // Offset: 156  Bytes
    unsigned long       fields_size;                        // Offset: 160  Bytes
    unsigned long       sector_map_offset;                  // Offset: 164  Bytes
    unsigned long       sector_map_size;                    // Offset: 168  Bytes
    unsigned long       sector_count;                       // Offset: 172  Bytes
    unsigned long       sector_start_offset;                // Offset: 176  Bytes
    unsigned long       fields_count;                       // Offset: 180  Bytes
    unsigned char       reserved[72];                       // Offset: 184  Bytes
    FIELD_INFO          fields[MAX_FIELDS_NUMBER];          // Offset: 256  Bytes
} DBHEADER, *PDBHEADER;                                     // Size:   2048 Bytes


typedef struct _BITBLOCK {
    union  {
        struct {
            unsigned long bit0  : 1;
            unsigned long bit1  : 1;
            unsigned long bit2  : 1;
            unsigned long bit3  : 1;
            unsigned long bit4  : 1;
            unsigned long bit5  : 1;
            unsigned long bit6  : 1;
            unsigned long bit7  : 1;
            unsigned long bit8  : 1;
            unsigned long bit9  : 1;
            unsigned long bit10 : 1;
            unsigned long bit11 : 1;
            unsigned long bit12 : 1;
            unsigned long bit13 : 1;
            unsigned long bit14 : 1;
            unsigned long bit15 : 1;
            unsigned long bit16 : 1;
            unsigned long bit17 : 1;
            unsigned long bit18 : 1;
            unsigned long bit19 : 1;
            unsigned long bit20 : 1;
            unsigned long bit21 : 1;
            unsigned long bit22 : 1;
            unsigned long bit23 : 1;
            unsigned long bit24 : 1;
            unsigned long bit25 : 1;
            unsigned long bit26 : 1;
            unsigned long bit27 : 1;
            unsigned long bit28 : 1;
            unsigned long bit29 : 1;
            unsigned long bit30 : 1;
            unsigned long bit31 : 1;
        } DUMMYSTRUCTNAME;
        unsigned long     u;
    } DUMMYUNIONNAME;
} BITBLOCK, PBITBLOCK;

#pragma pack(pop)


class bits_map
{
public:
    bits_map();
    bits_map(size_t bits_size);
    virtual ~bits_map();

    void resize(size_t bits_size);
    void clear();
    void attach(size_t bits_size, const std::vector<unsigned long>& buffer);

    bool is_bit_on(_In_ unsigned long bit) const noexcept;
    bool operator [](_In_ unsigned long bit) const noexcept;
    void set_bit(_In_ unsigned long bit) noexcept;
    void clear_bit(_In_ unsigned long bit) noexcept;
    void clear_all();
    bits_map& operator = (_In_ unsigned long data);
    bits_map& operator = (_In_ int data);

    unsigned long reserve_first_free_bit(unsigned long start_pos);

    inline bool empty() const { return (0 == _size); }
    inline const unsigned long* data() const { return _map.empty() ? NULL : _map.data(); }
    inline size_t data_size() const noexcept { return (_map.size() * sizeof(unsigned long)); }
    inline size_t block_count() const noexcept { return _map.size(); }
    inline size_t size() const noexcept { return _size; }

    inline unsigned long bit_id_to_block_id(unsigned long bit_id) const { return (bit_id < _size) ? (bit_id / 32) : MAX_ULONG; }
    inline unsigned long get_block(unsigned long block_id) const { return (block_id < _map.size()) ? _map[block_id] : 0; }
    inline unsigned long get_bit_block(unsigned long bit_id) const { return get_block(bit_id_to_block_id(bit_id)); }

private:
    unsigned long find_free_bit(unsigned long value) const noexcept;
    bool check_bit(unsigned long value, unsigned long bit) const noexcept;
    void set_bit(unsigned long& value, unsigned long bit) noexcept;
    void clear_bit(unsigned long& value, unsigned long bit) noexcept;

private:
    std::vector<unsigned long> _map;
    size_t  _size;
};

class readwrite_lock
{
public:
    readwrite_lock() { InitializeSRWLock(&_lock); }
    virtual ~readwrite_lock() {}

    inline PSRWLOCK get_lock() { return &_lock; }

private:
    SRWLOCK _lock;
};

class exclusive_locker
{
public:
    exclusive_locker(_In_ readwrite_lock* plk) : _lock(plk), _locked(false)
    {
        lock();
    }
    ~exclusive_locker()
    {
        unlock();
    }

    inline bool is_locked() const { return _locked; }

    inline void lock()
    {
        if (NULL != _lock) {
            AcquireSRWLockExclusive(_lock->get_lock());
            _locked = true;
        }
    }

    inline void unlock()
    {
        if (_locked) {
            ReleaseSRWLockExclusive(_lock->get_lock());
            _locked = false;
        }
    }

private:
    readwrite_lock* _lock;
    bool _locked;
};

class shared_locker
{
public:
    shared_locker(_In_ readwrite_lock* plk) : _lock(plk), _locked(false)
    {
        lock();
    }
    ~shared_locker()
    {
        unlock();
    }

    inline bool is_locked() const { return _locked; }

    inline void lock()
    {
        if (NULL != _lock) {
            AcquireSRWLockShared(_lock->get_lock());
            _locked = true;
        }
    }

    inline void unlock()
    {
        if (_locked) {
            ReleaseSRWLockShared(_lock->get_lock());
            _locked = false;
        }
    }


private:
    readwrite_lock* _lock;
    bool _locked;
};

class logdb_record_map
{
public:
    logdb_record_map();
    ~logdb_record_map();

    void resize(size_t size);
    void attach(const std::vector<unsigned long>& buffer);
    unsigned long get_record_sector_id(size_t record_id) const;

    inline const std::vector<unsigned long>& get_records() const { return _records; }
    inline std::vector<unsigned long>& get_records() { return _records; }
    inline bool empty() const { return _records.empty(); }
    inline void clear() { _records.clear(); }

private:
    std::vector<unsigned long>  _records;
};

class logdb_file : boost::noncopyable
{
public:
    logdb_file();
    virtual ~logdb_file();

    void create(const std::wstring& file, const db_conf& conf);
    void open(const std::wstring& file, bool read_only = false);
    void close();
    void export_log(const std::wstring& file) const;
    
    // return bytes freed
    NXLOGDB::db_record read_record(unsigned long record_id) const;
    NXLOGDB::db_record read_record_safe(unsigned long record_id) const;
    unsigned long pop_record();
    bool push_record(const NXLOGDB::db_record& record);

    inline bool opened() const { return (INVALID_HANDLE_VALUE != _h); }
    inline const db_conf& get_db_conf() const { return _db_conf; }

protected:
    void read_conf(const DBHEADER* header);
    void write_conf(DBHEADER* header) const;


    // return bytes freed
    unsigned long free_record(unsigned long record_count);
    unsigned long inter_pop_record();

    // allocation
    unsigned long get_required_sector_number(unsigned long size);
    unsigned long allocate_sector_list(unsigned long record_size);
    bool allocate_block() noexcept;


private:
    inline unsigned long get_sector_offset(unsigned long sector_id) const { return (get_db_conf().get_sector_start_offset() + sector_id * get_db_conf().get_sector_size()); }
    unsigned long get_next_sector(unsigned long sector_id) const;

    // return bytes freed
    unsigned long inter_free_sector(unsigned long sector_id);
    std::vector<unsigned char> inter_read_record(unsigned long sector_id) const;
    bool inter_write_record(unsigned long sector_id, const std::vector<unsigned char>& data);
    

private:
    bool read_file(unsigned long offset, void* buf, unsigned long size)  const noexcept;
    bool write_file(unsigned long offset, const void* buf, const unsigned long size) noexcept;
    bool set_file_size(unsigned long size) noexcept;

    bool write_record_count(unsigned long count);
    bool write_record_start_id(unsigned long record_id);
    bool write_record_sector_id(unsigned long record_id, unsigned long sector_id);
    bool write_sector_next_id(unsigned long sector_id, unsigned long next_sector_id);
    bool write_sector_map_block(unsigned long block_id, unsigned long block_value);
    unsigned long read_sector_next_id(unsigned long sector_id);

private:
    HANDLE                  _h;
    db_conf                 _db_conf;
    bits_map                _sector_map;
    logdb_record_map        _record_map;
    mutable readwrite_lock  _db_lock;
};


}   // namespace NXLOGDB


#endif