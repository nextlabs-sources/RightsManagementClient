
#ifndef __NDF_ERROR_H__
#define __NDF_ERROR_H__

/* --------------------------------------------------------
 HEADER SECTION
*/



/* ------------------------------------------------------------------
 MESSAGE DEFINITION SECTION
*/
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTIME                 0x2
#define FACILITY_RMSP                    0x11
#define FACILITY_RMCSVC                  0x14
#define FACILITY_RMCREP                  0x15
#define FACILITY_PC                      0x13
#define FACILITY_NXL                     0x10
#define FACILITY_IO_ERROR_CODE           0x4
#define FACILITY_FSD                     0x12


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: CATEGORY_GENERAL
//
// MessageText:
//
// Category Unknown Exception
//
#define CATEGORY_GENERAL                 ((short)0x00000001L)

//
// MessageId: CATEGORY_WIN32
//
// MessageText:
//
// Category Win32 Error
//
#define CATEGORY_WIN32                   ((short)0x00000002L)

//
// MessageId: CATEGORY_NETWORK
//
// MessageText:
//
// Category Network Error
//
#define CATEGORY_NETWORK                 ((short)0x00000003L)

//
// MessageId: CATEGORY_COM
//
// MessageText:
//
// Category COM Error
//
#define CATEGORY_COM                     ((short)0x00000004L)

//
// MessageId: CATEGORY_ARGUMENT
//
// MessageText:
//
// Category Invalid Argument
//
#define CATEGORY_ARGUMENT                ((short)0x00000005L)

//
// MessageId: CATEGORY_RUNTIME
//
// MessageText:
//
// Category Runtime Error
//
#define CATEGORY_RUNTIME                 ((short)0x00000006L)

//
// MessageId: CATEGORY_ENVIRONMENT
//
// MessageText:
//
// Category Environment Error
//
#define CATEGORY_ENVIRONMENT             ((short)0x00000007L)

//
// MessageId: CATEGORY_3RDPARTYLIB
//
// MessageText:
//
// Category Third-party Library Error
//
#define CATEGORY_3RDPARTYLIB             ((short)0x00000008L)

//
// MessageId: CATEGORY_NEXTLABS
//
// MessageText:
//
// Category NextLabs Error
//
#define CATEGORY_NEXTLABS                ((short)0x00000009L)

//
// MessageId: CATEGORY_USER
//
// MessageText:
//
// Category User
//
#define CATEGORY_USER                    ((short)0x00000100L)

//
// MessageId: CATEGORY_RMCSVC
//
// MessageText:
//
// Category RMC Service
//
#define CATEGORY_RMCSVC                  ((short)0x00000101L)

//
// MessageId: CATEGORY_REP
//
// MessageText:
//
// Rep Events
//
#define CATEGORY_REP                     ((short)0x00000102L)

//
// MessageId: CATEGORY_CONTROLPANEL
//
// MessageText:
//
// Category Control Panel
//
#define CATEGORY_CONTROLPANEL            ((short)0x00000103L)

//
// MessageId: CATEGORY_UTILITY
//
// MessageText:
//
// Category Utility
//
#define CATEGORY_UTILITY                 ((short)0x00000104L)





/***************************************
  SUCCEED MESSAGE
 ***************************************/


//
// MessageId: NS_SUCCESS
//
// MessageText:
//
// System started. 
//
#define NS_SUCCESS                       ((long)0x00000000L)





/***************************************
  INFORMATIONAL MESSAGE
 ***************************************/


//
// MessageId: NI_SERVICE_STARTED
//
// MessageText:
//
// Service %1 has started
//
#define NI_SERVICE_STARTED               ((long)0x40000001L)

//
// MessageId: NI_SERVICE_STOPPED
//
// MessageText:
//
// Service %1 has stopped
//
#define NI_SERVICE_STOPPED               ((long)0x40000002L)

//
// MessageId: NI_LOG
//
// MessageText:
//
// Nextlabs log (Information): %n%n%1
//
#define NI_LOG                           ((long)0x40020001L)





/***************************************
  WARNING MESSAGE
 ***************************************/


//
// MessageId: NW_VOLUME_NOT_ATTACHED
//
// MessageText:
//
// NextLabs EFS doesn't attach to volume %1.
//
#define NW_VOLUME_NOT_ATTACHED           ((long)0x80020001L)

//
// MessageId: NW_LOG
//
// MessageText:
//
// Nextlabs log (Warning): %n%n%1
//
#define NW_LOG                           ((long)0x80020002L)





/***************************************
  ERROR MESSAGE
 ***************************************/




// System Error


//
// MessageId: NE_FAIL
//
// MessageText:
//
// General failure.
//
#define NE_FAIL                          ((long)0xC0000000L)


//
// Runtime Error
//

//
// MessageId: NE_LOG
//
// MessageText:
//
// Nextlabs log (Error): %n%n%1
//
#define NE_LOG                           ((long)0xC0020000L)

//
// MessageId: NE_INVALID_POINTER
//
// MessageText:
//
// Invalid pointer
//
#define NE_INVALID_POINTER               ((long)0xC0020001L)

//
// MessageId: NE_INVALID_HANDLE
//
// MessageText:
//
// Invalid handle
//
#define NE_INVALID_HANDLE                ((long)0xC0020002L)

//
// MessageId: NE_INVALID_VALUE
//
// MessageText:
//
// Invalid value
//
#define NE_INVALID_VALUE                 ((long)0xC0020003L)

//
// MessageId: NE_INVALID_TYPE
//
// MessageText:
//
// Invalid type
//
#define NE_INVALID_TYPE                  ((long)0xC0020004L)

//
// MessageId: NE_INSUFFICIENT_MEMORY
//
// MessageText:
//
// Insufficient memory
//
#define NE_INSUFFICIENT_MEMORY           ((long)0xC0020005L)

//
// MessageId: NE_BUFFER_OVERFLOW
//
// MessageText:
//
// Buffer overflow.
//
#define NE_BUFFER_OVERFLOW               ((long)0xC0020006L)

//
// MessageId: NE_OUT_OF_RANGE
//
// MessageText:
//
// The value is out of range
//
#define NE_OUT_OF_RANGE                  ((long)0xC0020007L)

//
// MessageId: NE_INVALID_RESULT
//
// MessageText:
//
// Invalid result
//
#define NE_INVALID_RESULT                ((long)0xC0020008L)

//
// MessageId: NE_NOT_IMPL
//
// MessageText:
//
// Method is not implemented.
//
#define NE_NOT_IMPL                      ((long)0xC0020009L)

//
// MessageId: NE_NOT_FOUND
//
// MessageText:
//
// Object is not found.
//
#define NE_NOT_FOUND                     ((long)0xC002000AL)

//
// MessageId: NE_NOT_OPENED
//
// MessageText:
//
// Object is not opened.
//
#define NE_NOT_OPENED                    ((long)0xC002000BL)

//
// MessageId: NE_NOT_CLOSED
//
// MessageText:
//
// Object is not closed.
//
#define NE_NOT_CLOSED                    ((long)0xC002000CL)

//
// MessageId: NE_ALREADY_EXIST
//
// MessageText:
//
// Object exists already.
//
#define NE_ALREADY_EXIST                 ((long)0xC002000DL)

//
// MessageId: NE_JVM_FAIL_TO_START
//
// MessageText:
//
// JVM fail to start
//
#define NE_JVM_FAIL_TO_START             ((long)0xC002000EL)

//
// MessageId: NE_JVM_FAIL_TO_STOP
//
// MessageText:
//
// JVM fail to stop
//
#define NE_JVM_FAIL_TO_STOP              ((long)0xC002000FL)

//
// MessageId: NE_JVM_CLASS_NOT_FOUND
//
// MessageText:
//
// JVM fail to find a class
//
#define NE_JVM_CLASS_NOT_FOUND           ((long)0xC0020010L)

//
// MessageId: NE_JVM_METHOD_NOT_FOUND
//
// MessageText:
//
// JVM fail to find a method
//
#define NE_JVM_METHOD_NOT_FOUND          ((long)0xC0020011L)

//
// MessageId: NE_JVM_CALL_FAILED
//
// MessageText:
//
// JVM fail to call a method or that call return failure
//
#define NE_JVM_CALL_FAILED               ((long)0xC0020012L)


//
// I/O Error
//

//
// MessageId: NE_IO_GENERAL_FAILURE
//
// MessageText:
//
// General I/O failure
//
#define NE_IO_GENERAL_FAILURE            ((long)0xC0040000L)


//
// NXL Format Error
//

//
// MessageId: NE_NXL_INVALID_SIGNATURE
//
// MessageText:
//
// The NXL signature is invalid
//
#define NE_NXL_INVALID_SIGNATURE         ((long)0xC0100000L)

//
// MessageId: NE_NXL_INVALID_THUMBPRINT
//
// MessageText:
//
// The NXL thumbprint is invalid
//
#define NE_NXL_INVALID_THUMBPRINT        ((long)0xC0100001L)

//
// MessageId: NE_NXL_INVALID_VERSION
//
// MessageText:
//
// The NXL version is invalid
//
#define NE_NXL_INVALID_VERSION           ((long)0xC0100002L)

//
// MessageId: NE_NXL_VERSION_IS_TOO_LOW
//
// MessageText:
//
// The NXL version is too low
//
#define NE_NXL_VERSION_IS_TOO_LOW        ((long)0xC0100003L)

//
// MessageId: NE_NXL_VERSION_IS_TOO_HIGH
//
// MessageText:
//
// The NXL version is too high
//
#define NE_NXL_VERSION_IS_TOO_HIGH       ((long)0xC0100004L)

//
// MessageId: NE_NXL_INVALID_ALIGNMENT
//
// MessageText:
//
// The NXL alignment is invalid
//
#define NE_NXL_INVALID_ALIGNMENT         ((long)0xC0100005L)

//
// MessageId: NE_NXL_INVALID_CONTENT_OFFSET
//
// MessageText:
//
// The NXL content offset is invalid
//
#define NE_NXL_INVALID_CONTENT_OFFSET    ((long)0xC0100006L)

//
// MessageId: NE_NXL_INVALID_ENCRYPT_KEY
//
// MessageText:
//
// The NXL encrypt key is invalid
//
#define NE_NXL_INVALID_ENCRYPT_KEY       ((long)0xC0100007L)

//
// MessageId: NE_NXL_INVALID_ENCRYPT_ALGORITHM
//
// MessageText:
//
// The NXL encrypt algorithm is invalid
//
#define NE_NXL_INVALID_ENCRYPT_ALGORITHM ((long)0xC0100008L)

//
// MessageId: NE_NXL_ALGORITHM_NOTIMPL
//
// MessageText:
//
// The NXL encrypt algorithm is not implemented
//
#define NE_NXL_ALGORITHM_NOTIMPL         ((long)0xC0100009L)

//
// MessageId: NE_NXL_INVALID_CBC_SIZE
//
// MessageText:
//
// The NXL cbc size is invalid
//
#define NE_NXL_INVALID_CBC_SIZE          ((long)0xC010000AL)

//
// MessageId: NE_NXL_INVALID_FILE_SIZE
//
// MessageText:
//
// The NXL file size is invalid
//
#define NE_NXL_INVALID_FILE_SIZE         ((long)0xC010000BL)

//
// MessageId: NE_NXL_INVALID_KEY_ALGORITHM
//
// MessageText:
//
// The NXL key algorithm is invalid
//
#define NE_NXL_INVALID_KEY_ALGORITHM     ((long)0xC010000CL)

//
// MessageId: NE_NXL_INVALID_KEYID
//
// MessageText:
//
// The NXL key id is invalid
//
#define NE_NXL_INVALID_KEYID             ((long)0xC010000DL)

//
// MessageId: NE_NXL_INVALID_KEYID_SIZE
//
// MessageText:
//
// The NXL key id size is invalid
//
#define NE_NXL_INVALID_KEYID_SIZE        ((long)0xC010000EL)

//
// MessageId: NE_NXL_KEY_NOT_EXIST
//
// MessageText:
//
// The NXL key does not exist
//
#define NE_NXL_KEY_NOT_EXIST             ((long)0xC010000FL)

//
// MessageId: NE_NXL_KEY_IS_ENCRYPTED
//
// MessageText:
//
// The NXL key is encrypted
//
#define NE_NXL_KEY_IS_ENCRYPTED          ((long)0xC0100010L)

//
// MessageId: NE_NXL_INVALID_CONTENT_SIZE
//
// MessageText:
//
// The NXL content size is invalid
//
#define NE_NXL_INVALID_CONTENT_SIZE      ((long)0xC0100011L)

//
// MessageId: NE_NXL_INVALID_ALLOCATION_SIZE
//
// MessageText:
//
// The NXL allocation size is invalid
//
#define NE_NXL_INVALID_ALLOCATION_SIZE   ((long)0xC0100012L)

//
// MessageId: NE_NXL_DEFAULT_SECTION_NOT_EXIST
//
// MessageText:
//
// The NXL default section does not exist
//
#define NE_NXL_DEFAULT_SECTION_NOT_EXIST ((long)0xC0100013L)

//
// MessageId: NE_NXL_TOO_MANY_SECTIONS
//
// MessageText:
//
// The NXL has too many sections
//
#define NE_NXL_TOO_MANY_SECTIONS         ((long)0xC0100014L)

//
// MessageId: NE_NXL_TOO_FEW_SECTIONS
//
// MessageText:
//
// The NXL has too few sections
//
#define NE_NXL_TOO_FEW_SECTIONS          ((long)0xC0100015L)

//
// MessageId: NE_NXL_INVALID_SECTION_SIZE
//
// MessageText:
//
// The NXL section size is invalid
//
#define NE_NXL_INVALID_SECTION_SIZE      ((long)0xC0100016L)

//
// MessageId: NE_NXL_INVALID_SECTION_NAME_SIZE
//
// MessageText:
//
// The NXL section name size is invalid
//
#define NE_NXL_INVALID_SECTION_NAME_SIZE ((long)0xC0100017L)

//
// MessageId: NE_NXL_INVALID_SECTION_NAME
//
// MessageText:
//
// The NXL section name is invalid
//
#define NE_NXL_INVALID_SECTION_NAME      ((long)0xC0100018L)

//
// MessageId: NE_NXL_INVALID_SECTION_TABLE_CHECKSUM
//
// MessageText:
//
// The NXL section table checksum is invalid
//
#define NE_NXL_INVALID_SECTION_TABLE_CHECKSUM ((long)0xC0100019L)

//
// MessageId: NE_NXL_INVALID_SECTION_DATA_CHECKSUM
//
// MessageText:
//
// The NXL section data checksum is invalid
//
#define NE_NXL_INVALID_SECTION_DATA_CHECKSUM ((long)0xC010001AL)


//
// RMSP Error
//

//
// MessageId: NE_RMSP_NO_LOGON
//
// MessageText:
//
// The RMSP has not logged on
//
#define NE_RMSP_NO_LOGON                 ((long)0xC0110000L)

//
// MessageId: NE_RMSP_CANNOT_LOGOUT
//
// MessageText:
//
// The NXL thumbprint is invalid
//
#define NE_RMSP_CANNOT_LOGOUT            ((long)0xC0110001L)


//
// RM FSD Error
//

//
// MessageId: NE_FSD_FAIL_TO_REGISTER
//
// MessageText:
//
// Fail to register File System.
//
#define NE_FSD_FAIL_TO_REGISTER          ((long)0xC0120000L)


//
// PC Error
//

//
// MessageId: NE_PC_INVALID_PARAMETERS
//
// MessageText:
//
// The start parameters are invalid
//
#define NE_PC_INVALID_PARAMETERS         ((long)0xC0130000L)

//
// MessageId: NE_PC_JAVA_EXCEPTION
//
// MessageText:
//
// PC throw a Java exception
//
#define NE_PC_JAVA_EXCEPTION             ((long)0xC0130001L)

//
// MessageId: NE_PC_CLASS_NOT_FOUND
//
// MessageText:
//
// A Java class is not found
//
#define NE_PC_CLASS_NOT_FOUND            ((long)0xC0130002L)

//
// MessageId: NE_PC_METHOD_NOT_FOUND
//
// MessageText:
//
// A Java method is not found
//
#define NE_PC_METHOD_NOT_FOUND           ((long)0xC0130003L)

//
// MessageId: NE_PC_FAIL_TO_CONNECT
//
// MessageText:
//
// Fail to connected to server
//
#define NE_PC_FAIL_TO_CONNECT            ((long)0xC0130004L)

//
// MessageId: NE_PC_INVALID_EVAL_RESULT
//
// MessageText:
//
// Evaluation result is invalid
//
#define NE_PC_INVALID_EVAL_RESULT        ((long)0xC0130005L)


//
// RMC SVC Error
//

//
// MessageId: NE_SVC_FAIL_TO_START
//
// MessageText:
//
// The RMC service cannot start
//
#define NE_SVC_FAIL_TO_START             ((long)0xC0140000L)

//
// MessageId: NE_SVC_FAIL_TO_STOP
//
// MessageText:
//
// The RMC service cannot stop
//
#define NE_SVC_FAIL_TO_STOP              ((long)0xC0140001L)

//
// MessageId: NE_SVC_NOT_RUNNING
//
// MessageText:
//
// The RMC service is not running
//
#define NE_SVC_NOT_RUNNING               ((long)0xC0140002L)


//
// RMC REP Error
//

//
// MessageId: NE_REP_NOT_AUTHORIZED
//
// MessageText:
//
// TNot authorized
//
#define NE_REP_NOT_AUTHORIZED            ((long)0xC0150000L)

//
// MessageId: NE_REP_NO_PERMISSION
//
// MessageText:
//
// No permission
//
#define NE_REP_NO_PERMISSION             ((long)0xC0150001L)

//
// MessageId: NE_REP_NO_SINATURE
//
// MessageText:
//
// No signature
//
#define NE_REP_NO_SINATURE               ((long)0xC0150002L)




#endif    // #ifndef __NDF_ERROR_H__