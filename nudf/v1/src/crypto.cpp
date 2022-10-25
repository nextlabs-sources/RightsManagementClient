

#include <Windows.h>
#include <assert.h>

#include <nudf\crypto.hpp>
#include <nudf\convert.hpp>


using namespace nudf::crypto;


static const unsigned long crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


static const unsigned __int64 crc64_tab[256] = {
    0x0000000000000000ULL, 0x42F0E1EBA9EA3693ULL,
    0x85E1C3D753D46D26ULL, 0xC711223CFA3E5BB5ULL,
    0x493366450E42ECDFULL, 0x0BC387AEA7A8DA4CULL,
    0xCCD2A5925D9681F9ULL, 0x8E224479F47CB76AULL,
    0x9266CC8A1C85D9BEULL, 0xD0962D61B56FEF2DULL,
    0x17870F5D4F51B498ULL, 0x5577EEB6E6BB820BULL,
    0xDB55AACF12C73561ULL, 0x99A54B24BB2D03F2ULL,
    0x5EB4691841135847ULL, 0x1C4488F3E8F96ED4ULL,
    0x663D78FF90E185EFULL, 0x24CD9914390BB37CULL,
    0xE3DCBB28C335E8C9ULL, 0xA12C5AC36ADFDE5AULL,
    0x2F0E1EBA9EA36930ULL, 0x6DFEFF5137495FA3ULL,
    0xAAEFDD6DCD770416ULL, 0xE81F3C86649D3285ULL,
    0xF45BB4758C645C51ULL, 0xB6AB559E258E6AC2ULL,
    0x71BA77A2DFB03177ULL, 0x334A9649765A07E4ULL,
    0xBD68D2308226B08EULL, 0xFF9833DB2BCC861DULL,
    0x388911E7D1F2DDA8ULL, 0x7A79F00C7818EB3BULL,
    0xCC7AF1FF21C30BDEULL, 0x8E8A101488293D4DULL,
    0x499B3228721766F8ULL, 0x0B6BD3C3DBFD506BULL,
    0x854997BA2F81E701ULL, 0xC7B97651866BD192ULL,
    0x00A8546D7C558A27ULL, 0x4258B586D5BFBCB4ULL,
    0x5E1C3D753D46D260ULL, 0x1CECDC9E94ACE4F3ULL,
    0xDBFDFEA26E92BF46ULL, 0x990D1F49C77889D5ULL,
    0x172F5B3033043EBFULL, 0x55DFBADB9AEE082CULL,
    0x92CE98E760D05399ULL, 0xD03E790CC93A650AULL,
    0xAA478900B1228E31ULL, 0xE8B768EB18C8B8A2ULL,
    0x2FA64AD7E2F6E317ULL, 0x6D56AB3C4B1CD584ULL,
    0xE374EF45BF6062EEULL, 0xA1840EAE168A547DULL,
    0x66952C92ECB40FC8ULL, 0x2465CD79455E395BULL,
    0x3821458AADA7578FULL, 0x7AD1A461044D611CULL,
    0xBDC0865DFE733AA9ULL, 0xFF3067B657990C3AULL,
    0x711223CFA3E5BB50ULL, 0x33E2C2240A0F8DC3ULL,
    0xF4F3E018F031D676ULL, 0xB60301F359DBE0E5ULL,
    0xDA050215EA6C212FULL, 0x98F5E3FE438617BCULL,
    0x5FE4C1C2B9B84C09ULL, 0x1D14202910527A9AULL,
    0x93366450E42ECDF0ULL, 0xD1C685BB4DC4FB63ULL,
    0x16D7A787B7FAA0D6ULL, 0x5427466C1E109645ULL,
    0x4863CE9FF6E9F891ULL, 0x0A932F745F03CE02ULL,
    0xCD820D48A53D95B7ULL, 0x8F72ECA30CD7A324ULL,
    0x0150A8DAF8AB144EULL, 0x43A04931514122DDULL,
    0x84B16B0DAB7F7968ULL, 0xC6418AE602954FFBULL,
    0xBC387AEA7A8DA4C0ULL, 0xFEC89B01D3679253ULL,
    0x39D9B93D2959C9E6ULL, 0x7B2958D680B3FF75ULL,
    0xF50B1CAF74CF481FULL, 0xB7FBFD44DD257E8CULL,
    0x70EADF78271B2539ULL, 0x321A3E938EF113AAULL,
    0x2E5EB66066087D7EULL, 0x6CAE578BCFE24BEDULL,
    0xABBF75B735DC1058ULL, 0xE94F945C9C3626CBULL,
    0x676DD025684A91A1ULL, 0x259D31CEC1A0A732ULL,
    0xE28C13F23B9EFC87ULL, 0xA07CF2199274CA14ULL,
    0x167FF3EACBAF2AF1ULL, 0x548F120162451C62ULL,
    0x939E303D987B47D7ULL, 0xD16ED1D631917144ULL,
    0x5F4C95AFC5EDC62EULL, 0x1DBC74446C07F0BDULL,
    0xDAAD56789639AB08ULL, 0x985DB7933FD39D9BULL,
    0x84193F60D72AF34FULL, 0xC6E9DE8B7EC0C5DCULL,
    0x01F8FCB784FE9E69ULL, 0x43081D5C2D14A8FAULL,
    0xCD2A5925D9681F90ULL, 0x8FDAB8CE70822903ULL,
    0x48CB9AF28ABC72B6ULL, 0x0A3B7B1923564425ULL,
    0x70428B155B4EAF1EULL, 0x32B26AFEF2A4998DULL,
    0xF5A348C2089AC238ULL, 0xB753A929A170F4ABULL,
    0x3971ED50550C43C1ULL, 0x7B810CBBFCE67552ULL,
    0xBC902E8706D82EE7ULL, 0xFE60CF6CAF321874ULL,
    0xE224479F47CB76A0ULL, 0xA0D4A674EE214033ULL,
    0x67C58448141F1B86ULL, 0x253565A3BDF52D15ULL,
    0xAB1721DA49899A7FULL, 0xE9E7C031E063ACECULL,
    0x2EF6E20D1A5DF759ULL, 0x6C0603E6B3B7C1CAULL,
    0xF6FAE5C07D3274CDULL, 0xB40A042BD4D8425EULL,
    0x731B26172EE619EBULL, 0x31EBC7FC870C2F78ULL,
    0xBFC9838573709812ULL, 0xFD39626EDA9AAE81ULL,
    0x3A28405220A4F534ULL, 0x78D8A1B9894EC3A7ULL,
    0x649C294A61B7AD73ULL, 0x266CC8A1C85D9BE0ULL,
    0xE17DEA9D3263C055ULL, 0xA38D0B769B89F6C6ULL,
    0x2DAF4F0F6FF541ACULL, 0x6F5FAEE4C61F773FULL,
    0xA84E8CD83C212C8AULL, 0xEABE6D3395CB1A19ULL,
    0x90C79D3FEDD3F122ULL, 0xD2377CD44439C7B1ULL,
    0x15265EE8BE079C04ULL, 0x57D6BF0317EDAA97ULL,
    0xD9F4FB7AE3911DFDULL, 0x9B041A914A7B2B6EULL,
    0x5C1538ADB04570DBULL, 0x1EE5D94619AF4648ULL,
    0x02A151B5F156289CULL, 0x4051B05E58BC1E0FULL,
    0x87409262A28245BAULL, 0xC5B073890B687329ULL,
    0x4B9237F0FF14C443ULL, 0x0962D61B56FEF2D0ULL,
    0xCE73F427ACC0A965ULL, 0x8C8315CC052A9FF6ULL,
    0x3A80143F5CF17F13ULL, 0x7870F5D4F51B4980ULL,
    0xBF61D7E80F251235ULL, 0xFD913603A6CF24A6ULL,
    0x73B3727A52B393CCULL, 0x31439391FB59A55FULL,
    0xF652B1AD0167FEEAULL, 0xB4A25046A88DC879ULL,
    0xA8E6D8B54074A6ADULL, 0xEA16395EE99E903EULL,
    0x2D071B6213A0CB8BULL, 0x6FF7FA89BA4AFD18ULL,
    0xE1D5BEF04E364A72ULL, 0xA3255F1BE7DC7CE1ULL,
    0x64347D271DE22754ULL, 0x26C49CCCB40811C7ULL,
    0x5CBD6CC0CC10FAFCULL, 0x1E4D8D2B65FACC6FULL,
    0xD95CAF179FC497DAULL, 0x9BAC4EFC362EA149ULL,
    0x158E0A85C2521623ULL, 0x577EEB6E6BB820B0ULL,
    0x906FC95291867B05ULL, 0xD29F28B9386C4D96ULL,
    0xCEDBA04AD0952342ULL, 0x8C2B41A1797F15D1ULL,
    0x4B3A639D83414E64ULL, 0x09CA82762AAB78F7ULL,
    0x87E8C60FDED7CF9DULL, 0xC51827E4773DF90EULL,
    0x020905D88D03A2BBULL, 0x40F9E43324E99428ULL,
    0x2CFFE7D5975E55E2ULL, 0x6E0F063E3EB46371ULL,
    0xA91E2402C48A38C4ULL, 0xEBEEC5E96D600E57ULL,
    0x65CC8190991CB93DULL, 0x273C607B30F68FAEULL,
    0xE02D4247CAC8D41BULL, 0xA2DDA3AC6322E288ULL,
    0xBE992B5F8BDB8C5CULL, 0xFC69CAB42231BACFULL,
    0x3B78E888D80FE17AULL, 0x7988096371E5D7E9ULL,
    0xF7AA4D1A85996083ULL, 0xB55AACF12C735610ULL,
    0x724B8ECDD64D0DA5ULL, 0x30BB6F267FA73B36ULL,
    0x4AC29F2A07BFD00DULL, 0x08327EC1AE55E69EULL,
    0xCF235CFD546BBD2BULL, 0x8DD3BD16FD818BB8ULL,
    0x03F1F96F09FD3CD2ULL, 0x41011884A0170A41ULL,
    0x86103AB85A2951F4ULL, 0xC4E0DB53F3C36767ULL,
    0xD8A453A01B3A09B3ULL, 0x9A54B24BB2D03F20ULL,
    0x5D45907748EE6495ULL, 0x1FB5719CE1045206ULL,
    0x919735E51578E56CULL, 0xD367D40EBC92D3FFULL,
    0x1476F63246AC884AULL, 0x568617D9EF46BED9ULL,
    0xE085162AB69D5E3CULL, 0xA275F7C11F7768AFULL,
    0x6564D5FDE549331AULL, 0x279434164CA30589ULL,
    0xA9B6706FB8DFB2E3ULL, 0xEB46918411358470ULL,
    0x2C57B3B8EB0BDFC5ULL, 0x6EA7525342E1E956ULL,
    0x72E3DAA0AA188782ULL, 0x30133B4B03F2B111ULL,
    0xF7021977F9CCEAA4ULL, 0xB5F2F89C5026DC37ULL,
    0x3BD0BCE5A45A6B5DULL, 0x79205D0E0DB05DCEULL,
    0xBE317F32F78E067BULL, 0xFCC19ED95E6430E8ULL,
    0x86B86ED5267CDBD3ULL, 0xC4488F3E8F96ED40ULL,
    0x0359AD0275A8B6F5ULL, 0x41A94CE9DC428066ULL,
    0xCF8B0890283E370CULL, 0x8D7BE97B81D4019FULL,
    0x4A6ACB477BEA5A2AULL, 0x089A2AACD2006CB9ULL,
    0x14DEA25F3AF9026DULL, 0x562E43B4931334FEULL,
    0x913F6188692D6F4BULL, 0xD3CF8063C0C759D8ULL,
    0x5DEDC41A34BBEEB2ULL, 0x1F1D25F19D51D821ULL,
    0xD80C07CD676F8394ULL, 0x9AFCE626CE85B507ULL
};


class CCryptProv
{
public:
    CCryptProv() throw() : _inited(false),_provaes(NULL),_provrsa(NULL),_provrc4(NULL),_provmd5(NULL),_provsha1(NULL),_provsha256(NULL),
        _md5_hashlen(0),_md5_objlen(0),_sha1_hashlen(0),_sha1_objlen(0),_sha256_hashlen(0),_sha256_objlen(0)
    {
        
    }

    ~CCryptProv() throw()
    {
    }

    bool Initialize() throw()
    {
        NTSTATUS Status;
        ULONG    cbResult = 0;

        if(_inited) {
            return true;
        }

        Status = BCryptOpenAlgorithmProvider(&_provaes, BCRYPT_AES_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if(0 != Status) {
            Clear();
            return false;
        }           
        Status = BCryptSetProperty(_provaes, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptOpenAlgorithmProvider(&_provrsa, BCRYPT_RSA_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptOpenAlgorithmProvider(&_provrc4, BCRYPT_RC4_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptOpenAlgorithmProvider(&_provmd5, BCRYPT_MD5_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptGetProperty(_provmd5, BCRYPT_HASH_LENGTH, (PUCHAR)&_md5_hashlen, sizeof(ULONG), &cbResult, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptGetProperty(_provmd5, BCRYPT_OBJECT_LENGTH, (PUCHAR)&_md5_objlen, sizeof(ULONG), &cbResult, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptOpenAlgorithmProvider(&_provsha1, BCRYPT_SHA1_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptGetProperty(_provsha1, BCRYPT_HASH_LENGTH, (PUCHAR)&_sha1_hashlen, sizeof(ULONG), &cbResult, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptGetProperty(_provsha1, BCRYPT_OBJECT_LENGTH, (PUCHAR)&_sha1_objlen, sizeof(ULONG), &cbResult, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptOpenAlgorithmProvider(&_provsha256, BCRYPT_SHA256_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptGetProperty(_provsha256, BCRYPT_HASH_LENGTH, (PUCHAR)&_sha256_hashlen, sizeof(ULONG), &cbResult, 0);
        if(0 != Status) {
            Clear();
            return false;
        }
        Status = BCryptGetProperty(_provsha256, BCRYPT_OBJECT_LENGTH, (PUCHAR)&_sha256_objlen, sizeof(ULONG), &cbResult, 0);
        if(0 != Status) {
            Clear();
            return false;
        }

        _inited = true;
        return true;
    }

    void Clear() throw()
    {
        _md5_hashlen = 0;
        _md5_objlen = 0;
        _sha1_hashlen = 0;
        _sha1_objlen = 0;
        _sha256_hashlen = 0;
        _sha256_objlen = 0;
        if(NULL != _provaes) { BCryptCloseAlgorithmProvider(_provaes, 0); _provaes = NULL; }
        if(NULL != _provrsa) { BCryptCloseAlgorithmProvider(_provrsa, 0); _provrsa = NULL; }
        if(NULL != _provrc4) { BCryptCloseAlgorithmProvider(_provrc4, 0); _provrc4 = NULL; }
        if(NULL != _provmd5) { BCryptCloseAlgorithmProvider(_provmd5, 0); _provmd5 = NULL; }
        if(NULL != _provsha1) { BCryptCloseAlgorithmProvider(_provsha1, 0); _provsha1 = NULL; }
        if(NULL != _provsha256) { BCryptCloseAlgorithmProvider(_provsha256, 0); _provsha256 = NULL; }
        _inited = false;
    }

    bool initialized() const throw() {return _inited;}

    BCRYPT_ALG_HANDLE AlgProvAes() const throw() {return _provaes;}
    BCRYPT_ALG_HANDLE AlgProvRsa() const throw() {return _provrsa;}
    BCRYPT_ALG_HANDLE AlgProvRc4() const throw() {return _provrc4;}
    BCRYPT_ALG_HANDLE AlgProvMd5() const throw() {return _provmd5;}
    BCRYPT_ALG_HANDLE AlgProvSha1() const throw() {return _provsha1;}
    BCRYPT_ALG_HANDLE AlgProvSha256() const throw() {return _provsha256;}

    unsigned long GetMd5HashLength() const throw() {return _md5_hashlen;}
    unsigned long GetSha1HashLength() const throw() {return _sha1_hashlen;}
    unsigned long GetSha256HashLength() const throw() {return _sha256_hashlen;}
    unsigned long GetMd5ObjectLength() const throw() {return _md5_objlen;}
    unsigned long GetSha1ObjectLength() const throw() {return _sha1_objlen;}
    unsigned long GetSha256ObjectLength() const throw() {return _sha256_objlen;}


private:
    bool _inited;
    BCRYPT_ALG_HANDLE _provaes;
    BCRYPT_ALG_HANDLE _provrsa;
    BCRYPT_ALG_HANDLE _provrc4;
    BCRYPT_ALG_HANDLE _provmd5;
    BCRYPT_ALG_HANDLE _provsha1;
    BCRYPT_ALG_HANDLE _provsha256;
    ULONG             _md5_hashlen;
    ULONG             _md5_objlen;
    ULONG             _sha1_hashlen;
    ULONG             _sha1_objlen;
    ULONG             _sha256_hashlen;
    ULONG             _sha256_objlen;
};


static CCryptProv _algprov;
static const ULONG _cbcsize = 512;


bool nudf::crypto::Initialize() throw()
{
    return _algprov.Initialize();
}

bool nudf::crypto::AesEncrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    DWORD cbBlock = 0;
    DWORD cbResult = 0;
    UCHAR IV[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    if(!_algprov.initialized()) {return false;}

    Status = BCryptImportKey(_algprov.AlgProvAes(), NULL, BCRYPT_KEY_DATA_BLOB, &hKey, NULL, 0, (PUCHAR)key.GetBlob(), key.GetBlobSize(), 0);
    if(0 != Status) {
        return false;
    }
    
    Status = BCryptGetProperty(hKey, BCRYPT_BLOCK_LENGTH, (PUCHAR)&cbBlock, sizeof(ULONG), &cbResult, 0);
    if(0 != Status) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        return false;
    }

    if(0 != (size%cbBlock)) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }
        
    while (size != 0) {

        const ULONG cbToEncrypt = min(size, _cbcsize);
        ULONG   cbEncrypted = 0;

        memset(IV, 0, 16);
        memcpy(IV, &ivec, sizeof(unsigned __int64));
        Status = BCryptEncrypt(hKey, (PUCHAR)data, cbToEncrypt, NULL, IV, 16, (PUCHAR)data, cbToEncrypt, &cbEncrypted, 0);
        if (0 !=Status) {
            BCryptDestroyKey(hKey);
            hKey = NULL;
            return false;
        }

        assert(cbToEncrypt == cbEncrypted);
        size -= cbToEncrypt;
        ivec += cbToEncrypt;
        data = (PVOID)(((PUCHAR)data) + cbToEncrypt);
    }

    BCryptDestroyKey(hKey);
    hKey = NULL;
    return true;
}

bool nudf::crypto::AesEncrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec, _Out_ std::vector<unsigned char>& cipher) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    DWORD cbBlock = 0;
    DWORD cbResult = 0;
    UCHAR IV[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    if(!_algprov.initialized()) {return false;}

    Status = BCryptImportKey(_algprov.AlgProvAes(), NULL, BCRYPT_KEY_DATA_BLOB, &hKey, NULL, 0, (PUCHAR)key.GetBlob(), key.GetBlobSize(), 0);
    if(0 != Status) {
        return false;
    }
    
    Status = BCryptGetProperty(hKey, BCRYPT_BLOCK_LENGTH, (PUCHAR)&cbBlock, sizeof(ULONG), &cbResult, 0);
    if(0 != Status) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        return false;
    }

    if(0 != (size%cbBlock)) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }
    
    cipher.reserve(size);
    
    while (size != 0) {

        const ULONG cbToEncrypt = min(size, _cbcsize);
        ULONG   cbEncrypted = 0;
        std::vector<unsigned char> vCipher;

        memset(IV, 0, 16);
        memcpy(IV, &ivec, sizeof(unsigned __int64));
        vCipher.resize(cbToEncrypt, 0);
        Status = BCryptEncrypt(hKey, (PUCHAR)data,cbToEncrypt, NULL, IV, 16, &vCipher[0], cbToEncrypt, &cbEncrypted, 0);
        if (0 !=Status) {
            BCryptDestroyKey(hKey);
            hKey = NULL;
            return false;
        }

        assert(cbToEncrypt == cbEncrypted);
        size -= cbToEncrypt;
        ivec += cbToEncrypt;
        data = (PVOID)(((PUCHAR)data) + cbToEncrypt);
        for(int i=0; i<(int)vCipher.size(); i++) {
            cipher.push_back(vCipher[i]);
        }
    }

    BCryptDestroyKey(hKey);
    hKey = NULL;
    return true;
}

bool nudf::crypto::AesDecrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    DWORD cbBlock = 0;
    DWORD cbResult = 0;
    UCHAR IV[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    if(!_algprov.initialized()) {return false;}

    Status = BCryptImportKey(_algprov.AlgProvAes(), NULL, BCRYPT_KEY_DATA_BLOB, &hKey, NULL, 0, (PUCHAR)key.GetBlob(), key.GetBlobSize(), 0);
    if(0 != Status) {
        return false;
    }
    
    Status = BCryptGetProperty(hKey, BCRYPT_BLOCK_LENGTH, (PUCHAR)&cbBlock, sizeof(ULONG), &cbResult, 0);
    if(0 != Status) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        return false;
    }

    if(0 != (size%cbBlock)) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }
    
    while (size != 0) {

        const ULONG cbToDecrypt = min(size, _cbcsize);
        ULONG   cbDecrypted = 0;

        memset(IV, 0, 16);
        memcpy(IV, &ivec, sizeof(unsigned __int64));
        Status = BCryptDecrypt(hKey, (PUCHAR)data, cbToDecrypt, NULL, IV, 16, (PUCHAR)data, cbToDecrypt, &cbDecrypted, 0);
        if (0 !=Status) {
            BCryptDestroyKey(hKey);
            hKey = NULL;
            return false;
        }

        assert(cbToDecrypt == cbDecrypted);
        size -= cbToDecrypt;
        ivec += cbToDecrypt;
        data = (PVOID)(((PUCHAR)data) + cbToDecrypt);
    }

    BCryptDestroyKey(hKey);
    hKey = NULL;
    return true;
}

bool nudf::crypto::AesDecrypt(_In_ const CAesKeyBlob& key, _In_ const void* data, _In_ unsigned long size, _In_ unsigned __int64 ivec, _Out_ std::vector<unsigned char>& plain) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    DWORD cbBlock = 0;
    DWORD cbResult = 0;
    UCHAR IV[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    if(!_algprov.initialized()) {return false;}

    Status = BCryptImportKey(_algprov.AlgProvAes(), NULL, BCRYPT_KEY_DATA_BLOB, &hKey, NULL, 0, (PUCHAR)key.GetBlob(), key.GetBlobSize(), 0);
    if(0 != Status) {
        return false;
    }
    
    Status = BCryptGetProperty(hKey, BCRYPT_BLOCK_LENGTH, (PUCHAR)&cbBlock, sizeof(ULONG), &cbResult, 0);
    if(0 != Status) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        return false;
    }

    if(0 != (size%cbBlock)) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        SetLastError(ERROR_INVALID_PARAMETER);
        return false;
    }

    plain.reserve(size);
    
    while (size != 0) {

        const ULONG cbToDecrypt = min(size, _cbcsize);
        ULONG   cbDecrypted = 0;
        std::vector<unsigned char> vPlain;

        memset(IV, 0, 16);
        memcpy(IV, &ivec, sizeof(unsigned __int64));
        vPlain.resize(cbToDecrypt, 0);
        Status = BCryptDecrypt(hKey, (PUCHAR)data, cbToDecrypt, NULL, IV, 16, &vPlain[0], cbToDecrypt, &cbDecrypted, 0);
        if (0 !=Status) {
            BCryptDestroyKey(hKey);
            hKey = NULL;
            return false;
        }

        assert(cbToDecrypt == cbDecrypted);
        size -= cbToDecrypt;
        ivec += cbToDecrypt;
        data = (PVOID)(((PUCHAR)data) + cbToDecrypt);
        for(int i=0; i<(int)vPlain.size(); i++) {
            plain.push_back(vPlain[i]);
        }
    }

    BCryptDestroyKey(hKey);
    hKey = NULL;
    return true;
}

HRESULT nudf::crypto::RsaSign(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& sig) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    std::vector<unsigned char> vHash;
    BCRYPT_PKCS1_PADDING_INFO Pkcs1PaddingInfo = {BCRYPT_SHA1_ALGORITHM};
    ULONG cbSig = 0;

    RSAKEYBLOBTYPE blobtype = CRsaPubKeyBlob::GetBlobType(keyblob, blobsize);
    if(RSAFULLPRIKEYBLOBTYPE != blobtype && LEGACYRSAPRIKEYBLOBTYPE != blobtype) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if(!_algprov.initialized()) {return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);}
        
    Status = BCryptImportKeyPair(_algprov.AlgProvRsa(), NULL, (LEGACYRSAPRIKEYBLOBTYPE == blobtype) ? LEGACY_RSAPRIVATE_BLOB : BCRYPT_RSAPRIVATE_BLOB, &hKey, (PUCHAR)keyblob, blobsize, 0); 
    if(0 != Status) {
        return HRESULT_FROM_NT(Status);
    }

    if(!nudf::crypto::ToSha1(data, size, vHash)) {
        BCryptDestroyKey(hKey); 
        hKey = NULL; 
        return E_FAIL;
    }
    
    // Get signature size
    Status = BCryptSignHash(hKey, &Pkcs1PaddingInfo, &vHash[0], (ULONG)vHash.size(), NULL, 0, &cbSig, BCRYPT_PAD_PKCS1);
    if(0 == cbSig) {
        return HRESULT_FROM_NT(Status);
    }

    sig.resize(cbSig, 0);
    Status = BCryptSignHash(hKey, &Pkcs1PaddingInfo, &vHash[0], (ULONG)vHash.size(), &sig[0], cbSig, &cbSig, BCRYPT_PAD_PKCS1);
    if(0 != Status) {
        sig.clear();
        return HRESULT_FROM_NT(Status);
    }

    return S_OK;
}

HRESULT nudf::crypto::RsaSign(_In_ const CRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& sig) throw()
{
    return RsaSign((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, sig);
}

HRESULT nudf::crypto::RsaSign(_In_ const CLegacyRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& sig) throw()
{
    return RsaSign((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, sig);
}

HRESULT nudf::crypto::RsaVerifySignature(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _In_ const void* sig, _In_ ULONG sigsize) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    std::vector<unsigned char> vHash;
    BCRYPT_PKCS1_PADDING_INFO Pkcs1PaddingInfo = {BCRYPT_SHA1_ALGORITHM};
    ULONG cbSig = 0;

    RSAKEYBLOBTYPE blobtype = CRsaPubKeyBlob::GetBlobType(keyblob, blobsize);
    if(RSAPUBKEYBLOBTYPE != blobtype && LEGACYRSAPUBKEYBLOBTYPE != blobtype) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if(!_algprov.initialized()) {return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);}
    

    Status = BCryptImportKeyPair(_algprov.AlgProvRsa(), NULL, (LEGACYRSAPUBKEYBLOBTYPE == blobtype) ? LEGACY_RSAPUBLIC_BLOB : BCRYPT_RSAPUBLIC_BLOB, &hKey, (PUCHAR)keyblob, blobsize, 0); 
    if(0 != Status) {
        return HRESULT_FROM_NT(Status);
    }

    if(!nudf::crypto::ToSha1(data, size, vHash)) {
        BCryptDestroyKey(hKey); 
        hKey = NULL; 
        return E_FAIL;
    }
    
    Status = BCryptVerifySignature(hKey, &Pkcs1PaddingInfo, (PUCHAR)&vHash[0], (ULONG)vHash.size(), (PUCHAR)sig, sigsize, BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey);
    hKey = NULL;

    return (0 == Status) ? S_OK : HRESULT_FROM_NT(Status);
}

HRESULT nudf::crypto::RsaVerifySignature(_In_ const CRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _In_ const void* sig, _In_ ULONG sigsize) throw()
{
    return RsaVerifySignature((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, sig, sigsize);
}

HRESULT nudf::crypto::RsaVerifySignature(_In_ const CLegacyRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _In_ const void* sig, _In_ ULONG sigsize) throw()
{
    CRsaPubKeyBlob rsablob;
    HRESULT hr = rsablob.SetBlob((const UCHAR*)blob.GetBlob(), blob.GetBlobSize());
    if(S_OK != hr) {
        return hr;
    }
    return RsaVerifySignature((const UCHAR*)rsablob.GetBlob(), rsablob.GetBlobSize(), data, size, sig, sigsize);
}

HRESULT nudf::crypto::RsaEncrypt(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& cipher) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    ULONG cbCipher = 0;

    RSAKEYBLOBTYPE blobtype = CRsaPubKeyBlob::GetBlobType(keyblob, blobsize);
    if(RSAPUBKEYBLOBTYPE != blobtype && LEGACYRSAPUBKEYBLOBTYPE != blobtype) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if(!_algprov.initialized()) {return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);}
    
    
    Status = BCryptImportKeyPair(_algprov.AlgProvRsa(), NULL, (LEGACYRSAPUBKEYBLOBTYPE == blobtype) ? LEGACY_RSAPUBLIC_BLOB : BCRYPT_RSAPUBLIC_BLOB, &hKey, (PUCHAR)keyblob, blobsize, 0);
    if(0 != Status) {
        return HRESULT_FROM_NT(Status);
    }
    
    Status = BCryptEncrypt(hKey, (PUCHAR)data, size, NULL, NULL, 0, NULL, 0, &cbCipher, BCRYPT_PAD_PKCS1); 
    if(0 == cbCipher) {
        BCryptDestroyKey(hKey); 
        hKey = NULL; 
        return HRESULT_FROM_NT(Status);
    }
    
    cipher.resize(cbCipher, 0);
    Status = BCryptEncrypt(hKey, (PUCHAR)data, size, NULL, NULL, 0, (PUCHAR)&cipher[0], cbCipher, &cbCipher, BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey); 
    hKey = NULL; 
    if(0 != Status) {
        cipher.clear();
        return HRESULT_FROM_NT(Status);
    }

    return S_OK;
}

HRESULT nudf::crypto::RsaEncrypt(_In_ const CRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& cipher) throw()
{
    return RsaEncrypt((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, cipher);
}

HRESULT nudf::crypto::RsaEncrypt(_In_ const CLegacyRsaPubKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& cipher) throw()
{
    CRsaPubKeyBlob rsablob;
    HRESULT hr = rsablob.SetBlob((const UCHAR*)blob.GetBlob(), blob.GetBlobSize());
    if(S_OK != hr) {
        return hr;
    }
    return RsaEncrypt((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, cipher);
}

HRESULT nudf::crypto::RsaDecrypt(_In_ const UCHAR* keyblob, _In_ ULONG blobsize, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& plain) throw()
{
    NTSTATUS Status = 0;
    BCRYPT_KEY_HANDLE hKey = NULL;
    ULONG cbPlain = 0;

    RSAKEYBLOBTYPE blobtype = CRsaPubKeyBlob::GetBlobType(keyblob, blobsize);
    if(RSAFULLPRIKEYBLOBTYPE != blobtype && LEGACYRSAPRIKEYBLOBTYPE != blobtype) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if(!_algprov.initialized()) {return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);}
    
    Status = BCryptImportKeyPair(_algprov.AlgProvRsa(), NULL, (LEGACYRSAPRIKEYBLOBTYPE == blobtype) ? LEGACY_RSAPRIVATE_BLOB : BCRYPT_RSAPRIVATE_BLOB, &hKey, (PUCHAR)keyblob, blobsize, 0);
    if(0 != Status) {
        return HRESULT_FROM_NT(Status);
    }
    
    Status = BCryptDecrypt(hKey, (PUCHAR)data, size, NULL, NULL, 0, NULL, 0, &cbPlain, BCRYPT_PAD_PKCS1); 
    if(0 == cbPlain) {
        BCryptDestroyKey(hKey); 
        hKey = NULL; 
        return HRESULT_FROM_NT(Status);
    }
    
    plain.resize(cbPlain, 0);
    Status = BCryptDecrypt(hKey, (PUCHAR)data, size, NULL, NULL, 0, (PUCHAR)&plain[0], cbPlain, &cbPlain, BCRYPT_PAD_PKCS1);
    BCryptDestroyKey(hKey); 
    hKey = NULL; 
    if(0 != Status) {
        plain.clear();
        return HRESULT_FROM_NT(Status);
    }

    return S_OK;
}

HRESULT nudf::crypto::RsaDecrypt(_In_ const CRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& plain) throw()
{
    return RsaEncrypt((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, plain);
}

HRESULT nudf::crypto::RsaDecrypt(_In_ const CLegacyRsaPriKeyBlob& blob, _In_ const void* data, _In_ ULONG size, _Out_ std::vector<UCHAR>& plain) throw()
{
    return RsaEncrypt((const UCHAR*)blob.GetBlob(), blob.GetBlobSize(), data, size, plain);
}

bool nudf::crypto::ToMd5(_In_ const void* data, _In_ unsigned long size, _Out_ std::vector<unsigned char>& md5) throw()
{
    NTSTATUS Status = 0;
    std::vector<unsigned char> vHashObject;
    HANDLE   hHash = NULL;

    if(!_algprov.initialized()) {return false;}


    vHashObject.resize(_algprov.GetMd5ObjectLength(), 0);
    Status = BCryptCreateHash(_algprov.AlgProvMd5(), &hHash, &vHashObject[0], (ULONG)vHashObject.size(), NULL, 0, 0);
	if (Status < 0) {
        return false;
    }

    Status = BCryptHashData(hHash, (PUCHAR)data, size, 0);
	if (Status < 0) {
        BCryptDestroyHash(hHash); hHash = NULL;
        return false;
    }

    md5.resize(_algprov.GetMd5HashLength(), 0);
    Status = BCryptFinishHash(hHash, &md5[0], (ULONG)md5.size(), 0);
    BCryptDestroyHash(hHash); hHash = NULL;
    return (Status >= 0) ? true : false;
}

bool nudf::crypto::ToSha1(_In_ const void* data, _In_ unsigned long size, _Out_ std::vector<unsigned char>& sha1) throw()
{
    NTSTATUS Status = 0;
    std::vector<unsigned char> vHashObject;
    HANDLE   hHash = NULL;

    if(!_algprov.initialized()) {return false;}


    vHashObject.resize(_algprov.GetSha1ObjectLength(), 0);
    Status = BCryptCreateHash(_algprov.AlgProvSha1(), &hHash, &vHashObject[0], (ULONG)vHashObject.size(), NULL, 0, 0);
	if (Status < 0) {
        return false;
    }

    Status = BCryptHashData(hHash, (PUCHAR)data, size, 0);
	if (Status < 0) {
        BCryptDestroyHash(hHash); hHash = NULL;
        return false;
    }

    sha1.resize(_algprov.GetSha1HashLength(), 0);
    Status = BCryptFinishHash(hHash, &sha1[0], (ULONG)sha1.size(), 0);
    BCryptDestroyHash(hHash); hHash = NULL;
    return (Status >= 0) ? true : false;
}

bool nudf::crypto::ToSha256(_In_ const void* data, _In_ unsigned long size, _Out_ std::vector<unsigned char>& sha256) throw()
{
    NTSTATUS Status = 0;
    std::vector<unsigned char> vHashObject;
    HANDLE   hHash = NULL;

    if(!_algprov.initialized()) {return false;}


    vHashObject.resize(_algprov.GetSha256ObjectLength(), 0);
    Status = BCryptCreateHash(_algprov.AlgProvSha256(), &hHash, &vHashObject[0], (ULONG)vHashObject.size(), NULL, 0, 0);
	if (Status < 0) {
        return false;
    }

    Status = BCryptHashData(hHash, (PUCHAR)data, size, 0);
	if (Status < 0) {
        BCryptDestroyHash(hHash); hHash = NULL;
        return false;
    }

    sha256.resize(_algprov.GetSha256HashLength(), 0);
    Status = BCryptFinishHash(hHash, &sha256[0], (ULONG)sha256.size(), 0);
    BCryptDestroyHash(hHash); hHash = NULL;
    return (Status >= 0) ? true : false;
}

unsigned long nudf::crypto::ToCrc32(_In_ unsigned long init_crc, _In_ const void* pb, _In_ unsigned long cb) throw()
{
    const unsigned char *p = (const unsigned char*)pb;
    init_crc ^= ~0U;
    while (cb--) {
        init_crc = crc32_tab[(init_crc ^ *p++) & 0xFF] ^ (init_crc >> 8);
    }
    init_crc ^= ~0U;
    return init_crc;
}

unsigned __int64 nudf::crypto::ToCrc64(_In_ unsigned __int64 init_crc, _In_ const void* pb, _In_ unsigned long cb) throw()
{
    const unsigned char *p = (const unsigned char*)pb;
    init_crc ^= ~0ULL;
    while (cb--) {
        init_crc = crc64_tab[((init_crc >> 56) ^ *p++) & 0xFF] ^ (init_crc << 8);
    }
    init_crc ^= ~0ULL;
    return init_crc;
}





//
//  class CAesKeyBlob
//

CAesKeyBlob::CAesKeyBlob() : CKeyBlob<BCRYPT_KEY_DATA_BLOB_HEADER>()
{
}

CAesKeyBlob::~CAesKeyBlob()
{
}

CAesKeyBlob& CAesKeyBlob::operator = (const CAesKeyBlob& blob)
{
    if(this != &blob) {
        CKeyBlob<BCRYPT_KEY_DATA_BLOB_HEADER>::operator=(blob);
    }
    return *this;
}

HRESULT CAesKeyBlob::Generate(_In_ ULONG bitslen) throw()
{
    UCHAR key[32] = {0};

    if(128!=bitslen && 192!=bitslen && 256!=bitslen) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    NTSTATUS Status = BCryptGenRandom(NULL, key, 32, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if(0 != Status) {
        return HRESULT_FROM_NT(Status);
    }

    SetKey(key, bitslen/8);
    return S_OK;
}

HRESULT CAesKeyBlob::SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw()
{
    return CKeyBlob<BCRYPT_KEY_DATA_BLOB_HEADER>::SetBlob(blob, size);
}

void CAesKeyBlob::SetKey(_In_ const UCHAR* key, _In_ ULONG size) throw()
{
    GetData().resize(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + 32, 0);
    GetBlob()->cbKeyData = size;
    GetBlob()->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
    GetBlob()->dwMagic   = BCRYPT_KEY_DATA_BLOB_MAGIC;
    memcpy(((PUCHAR)GetBlob()) + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), key, size);
}

const UCHAR* CAesKeyBlob::GetKey() const throw()
{
    return (GetBlobSize() ? (((PUCHAR)GetBlob()) + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER)) : NULL);
}

ULONG CAesKeyBlob::GetKeySize() const throw()
{
    return GetBlob()->cbKeyData;
}

ULONG CAesKeyBlob::GetKeyBitsLength() const throw()
{
    return (GetBlob()->cbKeyData * 8);
}


//
//  class CRsaPubKeyBlob
//

CRsaPubKeyBlob::CRsaPubKeyBlob() : CKeyBlob<BCRYPT_RSAKEY_BLOB>()
{
}

CRsaPubKeyBlob::~CRsaPubKeyBlob()
{
}

CRsaPubKeyBlob& CRsaPubKeyBlob::operator = (const CRsaPubKeyBlob& blob)
{
    if(this != &blob) {
        CKeyBlob<BCRYPT_RSAKEY_BLOB>::operator=(blob);
    }
    return *this;
}

HRESULT CRsaPubKeyBlob::SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw()
{
    HRESULT hr = S_OK;
    RSAKEYBLOBTYPE type = CRsaPubKeyBlob::GetBlobType(blob, size);

    if(UNKNWONRSAKEYBLOBTYPE == type) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
    }

    switch(type)
    {
    case RSAPUBKEYBLOBTYPE:
    case RSAPRIKEYBLOBTYPE:
    case RSAFULLPRIKEYBLOBTYPE:
        hr = SetBlob((const BCRYPT_RSAKEY_BLOB*)blob, size);
        break;
    case LEGACYRSAPUBKEYBLOBTYPE:
    case LEGACYRSAPRIKEYBLOBTYPE:
        hr = SetBlob((PCCRYPTO_RSAKEY_BLOB)blob, size);
        break;
        
    default:
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        break;
    }

    return hr;
}

ULONG CRsaPubKeyBlob::GetKeyBitsLength() const throw()
{
    return (GetBlob() ? GetBlob()->BitLength : 0);
}

const UCHAR* CRsaPubKeyBlob::GetPubexp() const throw()
{
    return (GetBlob() ? ((PUCHAR)(GetBlob()+1)) : NULL);
}

ULONG CRsaPubKeyBlob::GetPubexpLen() const throw()
{
    return (GetBlob() ? GetBlob()->cbPublicExp : 0);
}

const UCHAR* CRsaPubKeyBlob::GetModulus() const throw()
{
    return (GetBlob() ? ((PUCHAR)(GetBlob()+GetBlob()->cbPublicExp)) : NULL);
}

ULONG CRsaPubKeyBlob::GetModulusLen() const throw()
{
    return (GetBlob() ? GetBlob()->cbModulus : 0);
}

HRESULT CRsaPubKeyBlob::SetBlob(_In_ const BCRYPT_RSAKEY_BLOB* blob, _In_ ULONG size) throw()
{
    DWORD dwBcryptKeySize = 0;

    dwBcryptKeySize = (ULONG)sizeof(BCRYPT_RSAKEY_BLOB)
                        + blob->cbPublicExp // PublicExponent
                        + blob->cbModulus;  // Modulus

    GetData().clear();
    GetData().resize(dwBcryptKeySize, 0);

    GetBlob()->Magic      = BCRYPT_RSAPUBLIC_MAGIC;
    GetBlob()->BitLength  = blob->BitLength;
    GetBlob()->cbPublicExp= blob->cbPublicExp;
    GetBlob()->cbModulus  = blob->cbModulus;
    GetBlob()->cbPrime1   = 0;
    GetBlob()->cbPrime2   = 0;
    
    PUCHAR pDest = (PUCHAR)(GetBlob()+1);
    const UCHAR* pSource = (const UCHAR*)(blob+1);
    // copy PublicExponent
    memcpy(pDest, pSource, blob->cbPublicExp);
    // copy Modulus
    pDest += blob->cbPublicExp;
    pSource += blob->cbPublicExp;
    memcpy(pDest, pSource, blob->cbModulus);

    return S_OK;
}

HRESULT CRsaPubKeyBlob::SetBlob(_In_ PCCRYPTO_RSAKEY_BLOB blob, _In_ ULONG size) throw()
{
    DWORD cbPublicExp = 0;
    DWORD cbModulus = 0;
    DWORD dwBcryptKeySize = 0;
    
    // Set CryptoBlob and Calculate Size
    cbModulus = (blob->PubKey.bitlen + 7) / 8;
    cbPublicExp = (blob->PubKey.pubexp & 0xFF000000) ? 4 :
                  (blob->PubKey.pubexp & 0x00FF0000) ? 3 :
                  (blob->PubKey.pubexp & 0x0000FF00) ? 2 : 1;
    dwBcryptKeySize = (unsigned long)sizeof(BCRYPT_RSAKEY_BLOB)
                       + cbPublicExp    // PublicExponent
                       + cbModulus;     // Modulus

    GetData().clear();
    GetData().resize(dwBcryptKeySize, 0);
    
    GetBlob()->Magic      = BCRYPT_RSAPUBLIC_MAGIC;
    GetBlob()->BitLength  = blob->PubKey.bitlen;
    GetBlob()->cbPublicExp= cbPublicExp;
    GetBlob()->cbModulus  = cbModulus;
    GetBlob()->cbPrime1   = 0;
    GetBlob()->cbPrime2   = 0;
    
    PUCHAR pDest = (PUCHAR)(GetBlob()+1);
    const UCHAR* pSource = (PUCHAR)(blob+1);
    // copy PublicExponent
    nudf::util::convert::ReverseMemCopy(pDest, &blob->PubKey.pubexp, cbPublicExp);  // Little-endian to Big-endian
    // copy Modulus
    pDest += cbPublicExp;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbModulus);                 // Little-endian to Big-endian

    return S_OK;
}

RSAKEYBLOBTYPE CRsaPubKeyBlob::GetBlobType(_In_ const UCHAR* blob, _In_ ULONG size) throw()
{
    assert(sizeof(CRYPTO_RSAKEY_BLOB) < sizeof(BCRYPT_RSAKEY_BLOB));

    if(size < (ULONG)sizeof(CRYPTO_RSAKEY_BLOB)) {
        return UNKNWONRSAKEYBLOBTYPE;
    }

    const BLOBHEADER*   legacy_blob = (const BLOBHEADER*)blob;

    if(legacy_blob->bType == PUBLICKEYBLOB) {
        return LEGACYRSAPUBKEYBLOBTYPE;
    }
    else if (legacy_blob->bType == PRIVATEKEYBLOB) {
        return LEGACYRSAPRIKEYBLOBTYPE;
    }
    else {

        if(size < (ULONG)sizeof(BCRYPT_RSAKEY_BLOB)) {
            return UNKNWONRSAKEYBLOBTYPE;
        }

        const BCRYPT_RSAKEY_BLOB* bcrypt_blob = (const BCRYPT_RSAKEY_BLOB*)blob;

        if(bcrypt_blob->Magic == BCRYPT_RSAPUBLIC_MAGIC) {
            return RSAPUBKEYBLOBTYPE;
        }
        else if (bcrypt_blob->Magic == BCRYPT_RSAPRIVATE_MAGIC) {
            return RSAPRIKEYBLOBTYPE;
        }
        else if (bcrypt_blob->Magic == BCRYPT_RSAFULLPRIVATE_MAGIC) {
            return RSAFULLPRIKEYBLOBTYPE;
        }
        else {
            return UNKNWONRSAKEYBLOBTYPE;
        }
    }

    return UNKNWONRSAKEYBLOBTYPE;
}

//
// class CRsaPriKeyBlob
//

CRsaPriKeyBlob::CRsaPriKeyBlob() : CRsaPubKeyBlob()
{
}

CRsaPriKeyBlob::~CRsaPriKeyBlob()
{
}

CRsaPriKeyBlob& CRsaPriKeyBlob::operator = (const CRsaPriKeyBlob& blob)
{
    if(this != &blob) {
        CRsaPubKeyBlob::operator=(blob);
    }
    return *this;
}

HRESULT CRsaPriKeyBlob::SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw()
{
    HRESULT hr = S_OK;
    RSAKEYBLOBTYPE type = CRsaPubKeyBlob::GetBlobType(blob, size);

    if(UNKNWONRSAKEYBLOBTYPE == type) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
    }

    switch(type)
    {        
    case RSAPRIKEYBLOBTYPE:
    case RSAFULLPRIKEYBLOBTYPE:
        hr = SetBlob((const BCRYPT_RSAKEY_BLOB*)blob, size);
        break;
    case LEGACYRSAPRIKEYBLOBTYPE:
        hr = SetBlob((PCCRYPTO_RSAKEY_BLOB)blob, size);
        break;

    case RSAPUBKEYBLOBTYPE:
    case LEGACYRSAPUBKEYBLOBTYPE:
    default:
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
        break;
    }

    return hr;
}

HRESULT CRsaPriKeyBlob::SetBlob(_In_ const BCRYPT_RSAKEY_BLOB* blob, _In_ ULONG size) throw()
{
    return CKeyBlob<BCRYPT_RSAKEY_BLOB>::SetBlob((const UCHAR*)blob, size);
}

HRESULT CRsaPriKeyBlob::SetBlob(_In_ PCCRYPTO_RSAKEY_BLOB blob, _In_ ULONG size) throw()
{
    PCCRYPTO_RSAKEY_BLOB CryptKeyBlob = NULL;
    DWORD cbPublicExp   = 0;
    DWORD cbModulus     = 0;
    DWORD cbPrime1      = 0;
    DWORD cbPrime2      = 0;
    DWORD dwBcryptKeySize = 0;


    // Get Crypto RSA Key Blob
    if(size < sizeof(CRYPTO_RSAKEY_BLOB)) {
        return HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
    }

    // Set CryptoBlob and Calculate Size
    cbModulus   = (blob->PubKey.bitlen + 7) / 8;
    cbPublicExp = (blob->PubKey.pubexp & 0xFF000000) ? 4 :
                  (blob->PubKey.pubexp & 0x00FF0000) ? 3 :
                  (blob->PubKey.pubexp & 0x0000FF00) ? 2 : 1;
    cbPrime1 = (blob->PubKey.bitlen + 15) / 16;
    cbPrime2 = (blob->PubKey.bitlen + 15) / 16;
    dwBcryptKeySize = (ULONG)sizeof(BCRYPT_RSAKEY_BLOB)
                       + cbPublicExp    // PublicExponent
                       + cbModulus      // Modulus
                       + cbPrime1       // Prime1
                       + cbPrime2       // Prime2
                       + cbPrime1       // Exponent1
                       + cbPrime2       // Exponent2
                       + cbPrime1       // Coefficient
                       + cbModulus;     // PrivateExponent

    if(size < (sizeof(CRYPTO_RSAKEY_BLOB) + cbModulus + cbPrime1 + cbPrime2 + cbPrime1 + cbPrime2 + cbPrime1 + cbModulus)) {
        return HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
    }
    
    GetData().clear();
    GetData().resize(dwBcryptKeySize, 0);
    GetBlob()->Magic      = BCRYPT_RSAFULLPRIVATE_MAGIC;
    GetBlob()->BitLength  = CryptKeyBlob->PubKey.bitlen;
    GetBlob()->cbPublicExp= cbPublicExp;
    GetBlob()->cbModulus  = cbModulus;
    GetBlob()->cbPrime1   = cbPrime1;
    GetBlob()->cbPrime2   = cbPrime2;

    PUCHAR pDest = (PUCHAR)(GetBlob()+1);
    const UCHAR* pSource = (PUCHAR)(blob+1);

    // copy PublicExponent
    nudf::util::convert::ReverseMemCopy(pDest, &CryptKeyBlob->PubKey.pubexp, cbPublicExp);   // Little-endian to Big-endian
    // copy Modulus
    pDest += cbPublicExp;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbModulus);                          // Little-endian to Big-endian
    // copy Prime1
    pDest += cbModulus;
    pSource += cbModulus;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbPrime1);                           // Little-endian to Big-endian
    // copy Prime2
    pDest += cbPrime1;
    pSource += cbPrime1;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbPrime2);                           // Little-endian to Big-endian
    // copy Exponent1
    pDest += cbPrime2;
    pSource += cbPrime2;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbPrime1);                           // Little-endian to Big-endian
    // copy Exponent2
    pDest += cbPrime1;
    pSource += cbPrime1;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbPrime2);                           // Little-endian to Big-endian
    // copy Coefficient
    pDest += cbPrime2;
    pSource += cbPrime2;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbPrime1);                           // Little-endian to Big-endian
    // copy PrivateExponent
    pDest += cbPrime1;
    pSource += cbPrime1;
    nudf::util::convert::ReverseMemCopy(pDest, pSource, cbModulus);                          // Little-endian to Big-endian

    return S_OK;
}

HRESULT CRsaPriKeyBlob::Generate(_In_ ULONG bitslen) throw()
{
    NTSTATUS            Status = 0;
    BCRYPT_KEY_HANDLE   hKey = NULL;
    ULONG               size = 0;

    Status = BCryptGenerateKeyPair(_algprov.AlgProvRsa(), &hKey, bitslen, 0);
    if(0 != Status) {
        return HRESULT_FROM_NT(Status);
    }

    Status = BCryptExportKey(hKey, NULL, BCRYPT_RSAFULLPRIVATE_BLOB, NULL, 0, &size, 0);
    if(0 == size) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        return HRESULT_FROM_NT(Status);
    }

    GetData().resize(size, 0);
    Status = BCryptExportKey(hKey, NULL, BCRYPT_RSAFULLPRIVATE_BLOB, &(GetData()[0]), size, &size, 0);
    if(0 != Status) {
        BCryptDestroyKey(hKey);
        hKey = NULL;
        Clear();
        return HRESULT_FROM_NT(Status);
    }

    return S_OK;
}

bool CRsaPriKeyBlob::IsRsaFullPriKeyBlob() const throw()
{
    return (NULL != GetBlob() && RSAFULLPRIKEYBLOBTYPE == GetBlob()->Magic);
}

const UCHAR* CRsaPriKeyBlob::GetPrime1() const throw()
{
    return (NULL == GetBlob()) ? NULL :
                                (((PUCHAR)(GetBlob()+1))
                                 + GetBlob()->cbPublicExp   // PublicExponent
                                 + GetBlob()->cbModulus     // Modulus
                                 );
}

ULONG CRsaPriKeyBlob::GetPrime1Len() const throw()
{
    return (NULL == GetBlob()) ? 0 : GetBlob()->cbPrime1;
}

const UCHAR* CRsaPriKeyBlob::GetPrime2() const throw()
{
    return (NULL == GetBlob()) ? NULL :
                                (((PUCHAR)(GetBlob()+1))
                                 + GetBlob()->cbPublicExp   // PublicExponent
                                 + GetBlob()->cbModulus     // Modulus
                                 + GetBlob()->cbPrime1      // Prime1
                                 );
}

ULONG CRsaPriKeyBlob::GetPrime2Len() const throw()
{
    return (NULL == GetBlob()) ? 0 : GetBlob()->cbPrime2;
}

const UCHAR* CRsaPriKeyBlob::GetExponent1() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? NULL :
                                     (((PUCHAR)(GetBlob()+1))
                                      + GetBlob()->cbPublicExp   // PublicExponent
                                      + GetBlob()->cbModulus     // Modulus
                                      + GetBlob()->cbPrime1      // Prime1
                                      + GetBlob()->cbPrime2      // Prime2
                                      );
}

ULONG CRsaPriKeyBlob::GetExponent1Len() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? 0 : GetBlob()->cbPrime1;
}

const UCHAR* CRsaPriKeyBlob::GetExponent2() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? NULL :
                                      (((PUCHAR)(GetBlob()+1))
                                       + GetBlob()->cbPublicExp   // PublicExponent
                                       + GetBlob()->cbModulus     // Modulus
                                       + GetBlob()->cbPrime1      // Prime1
                                       + GetBlob()->cbPrime2      // Prime2
                                       + GetBlob()->cbPrime1      // Exponent1
                                       );
}

ULONG CRsaPriKeyBlob::GetExponent2Len() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? 0 : GetBlob()->cbPrime2;
}

const UCHAR* CRsaPriKeyBlob::GetCoefficient() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? NULL :
                                      (((PUCHAR)(GetBlob()+1))
                                       + GetBlob()->cbPublicExp   // PublicExponent
                                       + GetBlob()->cbModulus     // Modulus
                                       + GetBlob()->cbPrime1      // Prime1
                                       + GetBlob()->cbPrime2      // Prime2
                                       + GetBlob()->cbPrime1      // Exponent1
                                       + GetBlob()->cbPrime2      // Exponent2
                                       );
}

ULONG CRsaPriKeyBlob::GetCoefficientLen() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? 0 : GetBlob()->cbPrime1;
}

const UCHAR* CRsaPriKeyBlob::GetPrivateExponent() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? NULL :
                                      (((PUCHAR)(GetBlob()+1))
                                       + GetBlob()->cbPublicExp   // PublicExponent
                                       + GetBlob()->cbModulus     // Modulus
                                       + GetBlob()->cbPrime1      // Prime1
                                       + GetBlob()->cbPrime2      // Prime2
                                       + GetBlob()->cbPrime1      // Exponent1
                                       + GetBlob()->cbPrime2      // Exponent2
                                       + GetBlob()->cbPrime1      // Coefficient
                                       );
}

ULONG CRsaPriKeyBlob::GetPrivateExponentLen() const throw()
{
    return (!IsRsaFullPriKeyBlob()) ? 0 : GetBlob()->cbModulus;
}


//
//  class CLegacyRsaPubKeyBlob
//

CLegacyRsaPubKeyBlob::CLegacyRsaPubKeyBlob() : CKeyBlob<CRYPTO_RSAKEY_BLOB>()
{
}

CLegacyRsaPubKeyBlob::~CLegacyRsaPubKeyBlob()
{
}

CLegacyRsaPubKeyBlob& CLegacyRsaPubKeyBlob::operator = (const CLegacyRsaPubKeyBlob& blob)
{
    CKeyBlob<CRYPTO_RSAKEY_BLOB>::operator=(blob);
    return *this;
}

HRESULT CLegacyRsaPubKeyBlob::SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw()
{
    const CRYPTO_RSAKEY_BLOB* cblob = (const CRYPTO_RSAKEY_BLOB*)blob;
    DWORD cbModulus     = 0;
    DWORD cbPrime1      = 0;
    DWORD cbPrime2      = 0;
    DWORD dwKeyBlobSize = 0;

    if(size < sizeof(CRYPTO_RSAKEY_BLOB)) {
        return HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
    }

    cbModulus   = (cblob->PubKey.bitlen + 7) / 8;
    dwKeyBlobSize = (ULONG)sizeof(CRYPTO_RSAKEY_BLOB) + cbModulus;

    if(size < dwKeyBlobSize) {
        return HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
    }
    
    GetData().clear();
    GetData().resize(dwKeyBlobSize, 0);
    GetBlob()->Header.bType = PUBLICKEYBLOB;
    GetBlob()->Header.bVersion = cblob->Header.bVersion;
    GetBlob()->Header.aiKeyAlg = cblob->Header.aiKeyAlg;
    GetBlob()->Header.reserved = 0;
    GetBlob()->PubKey.magic = 0x31415352;   // 'RSA1'
    GetBlob()->PubKey.bitlen = cblob->PubKey.bitlen;
    GetBlob()->PubKey.pubexp = cblob->PubKey.pubexp;
    memcpy(GetBlob()+1, blob+sizeof(CRYPTO_RSAKEY_BLOB), cbModulus);

    return S_OK;
}

ULONG CLegacyRsaPubKeyBlob::GetKeyBitsLength() const throw()
{
    return GetBlob() ? GetBlob()->PubKey.bitlen : 0;
}

const UCHAR* CLegacyRsaPubKeyBlob::GetPubexp() const throw()
{
    return GetBlob() ? ((PUCHAR)(&GetBlob()->PubKey.pubexp)) : 0;
}

ULONG CLegacyRsaPubKeyBlob::GetPubexpLen() const throw()
{
    if(NULL == GetBlob()) {
        return 0;
    }

    return (GetBlob()->PubKey.pubexp & 0xFF000000) ? 4 :
                (GetBlob()->PubKey.pubexp & 0x00FF0000) ? 3 :
                (GetBlob()->PubKey.pubexp & 0x0000FF00) ? 2 : 1;
}

const UCHAR* CLegacyRsaPubKeyBlob::GetModulus() const throw()
{
    return GetBlob() ? ((PUCHAR)(GetBlob()+1)) : 0;
}

ULONG CLegacyRsaPubKeyBlob::GetModulusLen() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 7) / 8) : 0;
}

//
//  class CLegacyRsaPriKeyBlob
//

CLegacyRsaPriKeyBlob::CLegacyRsaPriKeyBlob() : CLegacyRsaPubKeyBlob()
{
}

CLegacyRsaPriKeyBlob::~CLegacyRsaPriKeyBlob()
{
}

CLegacyRsaPriKeyBlob& CLegacyRsaPriKeyBlob::operator = (const CLegacyRsaPriKeyBlob& blob)
{
    CKeyBlob<CRYPTO_RSAKEY_BLOB>::operator=(blob);
    return *this;
}

HRESULT CLegacyRsaPriKeyBlob::SetBlob(_In_ const UCHAR* blob, _In_ ULONG size) throw()
{
    const CRYPTO_RSAKEY_BLOB* cblob = (const CRYPTO_RSAKEY_BLOB*)blob;
    DWORD cbModulus     = 0;
    DWORD cbPrime1      = 0;
    DWORD cbPrime2      = 0;
    DWORD dwKeyBlobSize = 0;

    if(size < sizeof(CRYPTO_RSAKEY_BLOB)) {
        return HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
    }
    
    cbModulus= (cblob->PubKey.bitlen + 7) / 8;
    cbPrime1 = (cblob->PubKey.bitlen + 15) / 16;
    cbPrime2 = (cblob->PubKey.bitlen + 15) / 16;
    dwKeyBlobSize = sizeof(CRYPTO_RSAKEY_BLOB)
                    + cbModulus     // Modulus
                    + cbPrime1      // Prime1
                    + cbPrime2      // Prime2
                    + cbPrime1      // Exponent1
                    + cbPrime2      // Exponent2
                    + cbPrime1      // Coefficient
                    + cbModulus;    // PrivateExponent
    if(size < dwKeyBlobSize) {
        return HRESULT_FROM_WIN32(ERROR_INCORRECT_SIZE);
    }

    if(cblob->Header.bType != PRIVATEKEYBLOB) {
        HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
    }

    return CKeyBlob<CRYPTO_RSAKEY_BLOB>::SetBlob(blob, size);
}

const UCHAR* CLegacyRsaPriKeyBlob::GetPrime1() const throw()
{
    return (NULL == GetBlob()) ? NULL : 
                                 ((PUCHAR)(GetBlob()+1)
                                   + ((GetBlob()->PubKey.bitlen + 7) / 8)   // Modulus
                                   );
}

ULONG CLegacyRsaPriKeyBlob::GetPrime1Len() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 15) / 16) : 0;
}

const UCHAR* CLegacyRsaPriKeyBlob::GetPrime2() const throw()
{
    return (NULL == GetBlob()) ? NULL : 
                                 ((PUCHAR)(GetBlob()+1)
                                   + ((GetBlob()->PubKey.bitlen + 7) / 8)   // Modulus
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime1
                                   );
}

ULONG CLegacyRsaPriKeyBlob::GetPrime2Len() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 15) / 16) : 0;
}

const UCHAR* CLegacyRsaPriKeyBlob::GetExponent1() const throw()
{
    return (NULL == GetBlob()) ? NULL : 
                                 ((PUCHAR)(GetBlob()+1)
                                   + ((GetBlob()->PubKey.bitlen + 7) / 8)   // Modulus
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime1
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime2
                                   );
}

ULONG CLegacyRsaPriKeyBlob::GetExponent1Len() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 15) / 16) : 0;
}

const UCHAR* CLegacyRsaPriKeyBlob::GetExponent2() const throw()
{
    return (NULL == GetBlob()) ? NULL : 
                                 ((PUCHAR)(GetBlob()+1)
                                   + ((GetBlob()->PubKey.bitlen + 7) / 8)   // Modulus
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime1
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime2
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Exponent1
                                   );
}

ULONG CLegacyRsaPriKeyBlob::GetExponent2Len() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 15) / 16) : 0;
}

const UCHAR* CLegacyRsaPriKeyBlob::GetCoefficient() const throw()
{
    return (NULL == GetBlob()) ? NULL : 
                                 ((PUCHAR)(GetBlob()+1)
                                   + ((GetBlob()->PubKey.bitlen + 7) / 8)   // Modulus
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime1
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime2
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Exponent1
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Exponent2
                                   );
}

ULONG CLegacyRsaPriKeyBlob::GetCoefficientLen() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 15) / 16) : 0;
}

const UCHAR* CLegacyRsaPriKeyBlob::GetPrivateExponent() const throw()
{
    return (NULL == GetBlob()) ? NULL : 
                                 ((PUCHAR)(GetBlob()+1)
                                   + ((GetBlob()->PubKey.bitlen + 7) / 8)   // Modulus
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime1
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Prime2
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Exponent1
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Exponent2
                                   + ((GetBlob()->PubKey.bitlen + 15) / 16) // Coefficient
                                   );
}

ULONG CLegacyRsaPriKeyBlob::GetPrivateExponentLen() const throw()
{
    return GetBlob() ? ((GetBlob()->PubKey.bitlen + 7) / 8) : 0;
}

void CLegacyRsaPriKeyBlob::GetPublicKeyBlob(_Out_ CRsaPubKeyBlob& blob) const throw()
{
    blob.Clear();
    if(NULL != GetBlob()) {
        blob.SetBlob((const UCHAR*)GetBlob(), GetBlobSize());
    }
}
