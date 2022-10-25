
/**
 * \file <nkdf/basic/registry.h>
 * \brief Header file for registry help routines
 * This header file declare registry help routines
 *
 * \author Gavin Ye
 * \version 1.0.0.0
 * \date 10/6/2014
 *
 */

#ifndef __NKDF_BASIC_REGISTRY_H__
#define __NKDF_BASIC_REGISTRY_H__



/**
 * \addtogroup nkdf-basic
 * @{
 */


/**
 * \defgroup nkdf-basic-registry Registry Support
 * @{
 */


/**
 * \defgroup nkdf-basic-registry-api Routines
 * @{
 */


/**
 * \brief Open a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyHandle Pointer to a HANDLE variable to receive key handle.
 * \param KeyPath Path of the key.
 * \param DesiredAccess Desired access mask.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegOpenKey(
             _Out_ PHANDLE KeyHandle,
             _In_ PUNICODE_STRING KeyPath,
             _In_ ACCESS_MASK DesiredAccess
             );

/**
 * \brief Create a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param KeyHandle Pointer to a HANDLE variable to receive key handle.
 * \param CreateNew Pointer to a BOOLEAN variable. If the key is created, it is set to TRUE, otherwise it is set to FALSE.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegCreateKey(
               _In_ PUNICODE_STRING KeyPath,
               _Out_ PHANDLE KeyHandle,
               _Out_opt_ PBOOLEAN CreateNew
               );

/**
 * \brief Check the existence of a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param CreateNew Pointer to a BOOLEAN variable. If the key exists, it is set to TRUE, otherwise it is set to FALSE.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegKeyExist(
              _In_ PUNICODE_STRING KeyPath,
              _Out_ PBOOLEAN Exist
              );

/**
 * \brief Delete a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegDeleteKey(
               _In_ PUNICODE_STRING KeyPath
               );

/**
 * \brief Delete a value in a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegDeleteValue(
                 _In_ PUNICODE_STRING KeyPath,
                 _In_ PUNICODE_STRING ValueName
                 );

/**
 * \brief Read integer value from a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param ValueData Pointer to a buffer receive data.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadIntValue(
                  _In_ PUNICODE_STRING KeyPath,
                  _In_ PUNICODE_STRING ValueName,
                  _Out_ PULONG ValueData
                  );

/**
 * \brief Read int64 value from a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param ValueData Pointer to a buffer receive data.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadInt64Value(
                    _In_ PUNICODE_STRING KeyPath,
                    _In_ PUNICODE_STRING ValueName,
                    _Out_ PULONGLONG ValueData
                    );


/**
 * \brief Read boolean value from a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param ValueData Pointer to a buffer receive data.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadBooleanValue(
                      _In_ PUNICODE_STRING KeyPath,
                      _In_ PUNICODE_STRING ValueName,
                      _Out_ PBOOLEAN ValueData
                      );

/**
 * \brief Read string value from a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param PagePool If it is TRUE, this function allocate paged pool for the value, otherwise it allocate non-paged pool.
 * \param PoolTag Tag of the pool.
 * \param ValueData Pointer to a UNICOED_STRING. Caller must free it after use.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadStringValue(
                     _In_ PUNICODE_STRING KeyPath,
                     _In_ PUNICODE_STRING ValueName,
                     _In_ BOOLEAN PagePool,
                     _In_ ULONG PoolTag,
                     _Out_ PUNICODE_STRING ValueData
                     );

/**
 * \brief Read binary value from a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param ValueType Expected type of the value.
 * \param ValueData Pointer to a UNICOED_STRING. Caller must free it after use.
 * \param ValueDataLength Pointer to a ULONG which contain valid buffer size and returned value size.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegReadBinaryValue(
                     _In_ PUNICODE_STRING KeyPath,
                     _In_ PUNICODE_STRING ValueName,
                     _Out_ PULONG ValueType,
                     _Out_writes_opt_(*ValueDataLength) PVOID ValueData,
                     _Inout_ PULONG ValueDataLength
                     );

/**
 * \brief Write int value to a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param Data Integer to write.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteIntValue(
                   _In_ PUNICODE_STRING KeyPath,
                   _In_ PUNICODE_STRING ValueName,
                   _In_ ULONG Data
                   );

/**
 * \brief Write int64 value to a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param Data 64 bits integer to write.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteInt64Value(
                     _In_ PUNICODE_STRING KeyPath,
                     _In_ PUNICODE_STRING ValueName,
                     _In_ ULONGLONG Data
                     );

/**
 * \brief Write boolean value to a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param Data Boolean data.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteBooleanValue(
                       _In_ PUNICODE_STRING KeyPath,
                       _In_ PUNICODE_STRING ValueName,
                       _In_ BOOLEAN Data
                       );

/**
 * \brief Write string value to a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param Data String to write.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteStringValue(
                      _In_ PUNICODE_STRING KeyPath,
                      _In_ PUNICODE_STRING ValueName,
                      _In_opt_ const WCHAR* Data
                      );

/**
 * \brief Write string value to a registry key.
 * \IRQL Requires PASSIVE_LEVEL.
 * \param KeyPath Path of the key.
 * \param ValueName Name of the value.
 * \param Data Pointer to the data to write.
 * \param DataSize Size of the data to write.
 * \return NTSTATUS.
 */
_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkRegWriteBinaryValue(
                      _In_ PUNICODE_STRING KeyPath,
                      _In_ PUNICODE_STRING ValueName,
                      _In_ PVOID Data,
                      _In_ ULONG DataSize
                      );


/**@}*/ // Group End: nkdf-basic-registry-api


/**@}*/ // Group End: nkdf-basic-registry


/**@}*/ // Group End: nkdf-basic




#endif  // #ifndef __NKDF_BASIC_REGISTRY_H__