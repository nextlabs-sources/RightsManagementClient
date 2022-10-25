#include "nxrmfltdef.h"
#include "nxrmflt.h"
#include "nxrmfltcommunication.h"
#include "nxrmfltutils.h"

extern NXRMFLT_GLOBAL_DATA	Global;

static NTSTATUS handle_set_keys_command(NXRM_KEY_BLOB *KeyChain, ULONG KeyChainSizeInByte);

NTSTATUS nxrmfltPrepareServerPort(VOID)
{
	NTSTATUS Status = STATUS_SUCCESS;

	OBJECT_ATTRIBUTES	ObjectAttributes = { 0 };
	UNICODE_STRING		nxrmfltPortName = { 0 };
	LONG				maxConnections = 1;
	SECURITY_DESCRIPTOR	*pSD = NULL;

	RtlInitUnicodeString(&nxrmfltPortName, NXRMFLT_MSG_PORT_NAME);

	do 
	{
		Status = FltBuildDefaultSecurityDescriptor(&pSD, FLT_PORT_ALL_ACCESS);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		InitializeObjectAttributes(&ObjectAttributes,
								   &nxrmfltPortName,
								   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
								   NULL,
								   pSD);

		Status = FltCreateCommunicationPort(Global.Filter,
											&Global.ServerPort,
											&ObjectAttributes,
											NULL,
											nxrmfltConnectNotifyCallback,
											nxrmfltDisconnectNotifyCallback,
											nxrmfltMessageNotifyCallback,
											maxConnections);

		FltFreeSecurityDescriptor(pSD);
		
	} while (FALSE);

	return Status;
}

NTSTATUS
	nxrmfltConnectNotifyCallback (
	_In_ PFLT_PORT ClientPort,
	_In_ PVOID ServerPortCookie,
	_In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID *ConnectionCookie
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_CONNECTION_CONTEXT *ConnCtx = (NXRMFLT_CONNECTION_CONTEXT*)ConnectionContext;

	LIST_ENTRY KeyChain = {0};

	NXL_KEYCHAIN_NODE *pNode = NULL;

	ULONG i = 0;

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	NXL_KEKEY_BLOB PrimaryKey = {0};

	do 
	{
		if(!ConnCtx || SizeOfContext < sizeof(NXRMFLT_CONNECTION_CONTEXT))
		{
			Status = STATUS_INVALID_PARAMETER_3;
			break;
		}

		if (SizeOfContext != ConnCtx->NumberOfKey * sizeof(NXRM_KEY_BLOB) + sizeof(NXRMFLT_CONNECTION_CONTEXT) - sizeof(NXRM_KEY_BLOB))
		{
			Status = STATUS_INVALID_PARAMETER_3;
			break;
		}

		//
		// build primary key
		//
		memset(&PrimaryKey, 0, sizeof(PrimaryKey));

		memcpy(&PrimaryKey.KeyId,
			   &ConnCtx->KeyChain[0].KeKeyId,
			   min(sizeof(PrimaryKey.KeyId), sizeof(ConnCtx->KeyChain[0].KeKeyId)));

		PrimaryKey.keySize = sizeof(ConnCtx->KeyChain[0].Key);

		memcpy(&PrimaryKey.Key, 
			   ConnCtx->KeyChain[0].Key,
			   min(sizeof(PrimaryKey.Key), sizeof(ConnCtx->KeyChain[0].Key)));

		//
		// build a on stack list to avoid allocating memory while holding the push lock
		//
		InitializeListHead(&KeyChain);

		for (i=0; i < ConnCtx->NumberOfKey; i++)
		{
			pNode = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_KEYCHAIN_NODE), NXRMFLT_KEYCHAINNODE_TAG);

			if (pNode)
			{
				memset(pNode, 0, sizeof(NXL_KEYCHAIN_NODE));

				memcpy(&pNode->KeyBlob.KeyId,
					   &ConnCtx->KeyChain[i].KeKeyId,
					   min(sizeof(pNode->KeyBlob.KeyId), sizeof(ConnCtx->KeyChain[i].KeKeyId)));

				pNode->KeyBlob.keySize = sizeof(ConnCtx->KeyChain[i].Key);

				memcpy(&pNode->KeyBlob.Key, 
					   ConnCtx->KeyChain[i].Key,
					   min(sizeof(pNode->KeyBlob.Key), sizeof(ConnCtx->KeyChain[i].Key)));

				InsertTailList(&KeyChain, &pNode->Link);
			}
		}

		FltAcquirePushLockExclusive(&Global.KeyChainLock);

		NT_ASSERT(KeyChain.Flink);

		FOR_EACH_LIST_SAFE(ite, tmp, &KeyChain)
		{
			NT_ASSERT(ite->Flink);

			pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

			RemoveEntryList(ite);

			InsertTailList(&Global.KeyChain, &pNode->Link);
		}

		FltReleasePushLock(&Global.KeyChainLock);

		FltAcquirePushLockExclusive(&Global.PrimaryKeyLock);

		memcpy(&Global.PrimaryKey,
			   &PrimaryKey,
			   min(sizeof(Global.PrimaryKey), sizeof(PrimaryKey)));

		FltReleasePushLock(&Global.PrimaryKeyLock);

		Global.HideNXLExtension = ConnCtx->HideNxlExtension;
		Global.PortProcessId	= PsGetCurrentProcessId();
		Global.ClientPort		= ClientPort;
		
	} while (FALSE);

	//
	// KeyChain list should be empty if there is no error
	// free all node if there is a error
	//
	NT_ASSERT(KeyChain.Flink);

	FOR_EACH_LIST_SAFE(ite, tmp, &KeyChain)
	{
		pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

		ExFreePoolWithTag(pNode, NXRMFLT_KEYCHAINNODE_TAG);

		pNode = NULL;
	}

	return Status;
}

VOID
	nxrmfltDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
	)
{
	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	NXL_KEYCHAIN_NODE *pNode = NULL;

	FltCloseClientPort(Global.Filter, &Global.ClientPort);

	Global.ClientPort			= NULL;
	Global.PortProcessId		= 0;
	Global.HideNXLExtension		= TRUE;
	
	FltAcquirePushLockExclusive(&Global.KeyChainLock);

	FOR_EACH_LIST_SAFE(ite, tmp, &Global.KeyChain)
	{
		pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

		RemoveEntryList(ite);

		ExFreePoolWithTag(pNode, NXRMFLT_KEYCHAINNODE_TAG);
	}
	
	InitializeListHead(&Global.KeyChain);

	FltReleasePushLock(&Global.KeyChainLock);

	return;
}

NTSTATUS
	nxrmfltMessageNotifyCallback (
	_In_ PVOID ConnectionCookie,
	_In_reads_bytes_opt_(InputBufferSize) PVOID InputBuffer,
	_In_ ULONG InputBufferSize,
	_Out_writes_bytes_to_opt_(OutputBufferSize,*ReturnOutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferSize,
	_Out_ PULONG ReturnOutputBufferLength
	)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXRMFLT_COMMAND		Command = nxrmfltInvalidCommand;

	NXRMFLT_COMMAND_MSG *Msg = (NXRMFLT_COMMAND_MSG *)InputBuffer;

	NXRMFLT_SAVEAS_FORECAST *pSaveAsForecast = NULL;
	
	NXL_SAVEAS_NODE *pSaveAsNode = NULL;

	PVOID CommandData = NULL;

	ULONG CommandDataLength = 0;

	LIST_ENTRY *ite = NULL;

	*ReturnOutputBufferLength = 0;

	do 
	{
		if ((InputBuffer == NULL) ||
			(InputBufferSize < (FIELD_OFFSET(NXRMFLT_COMMAND_MSG, Command) +
								sizeof(NXRMFLT_COMMAND)))) 
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		__try
		{
			Command = ((PNXRMFLT_COMMAND_MSG) InputBuffer)->Command;

			CommandData = (PVOID)(((PNXRMFLT_COMMAND_MSG) InputBuffer)->Data);

			CommandDataLength = ((PNXRMFLT_COMMAND_MSG) InputBuffer)->Size;
		}
		__except(nxrmfltExceptionFilter(NULL, GetExceptionInformation()))
		{
			Status = GetExceptionCode();
			break;
		}
		
		switch (Command)
		{
		case nxrmfltSaveAsForecast:

			do 
			{
				if (CommandDataLength != sizeof(NXRMFLT_SAVEAS_FORECAST))
				{
					Status = STATUS_INVALID_PARAMETER;
					break;
				}

				pSaveAsNode = ExAllocateFromPagedLookasideList(&Global.SaveAsExpireLookaside);

				if (!pSaveAsNode)
				{
					Status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				memset(pSaveAsNode, 0, sizeof(NXL_SAVEAS_NODE));

				__try
				{
					pSaveAsForecast = (NXRMFLT_SAVEAS_FORECAST *)CommandData;

					memcpy(pSaveAsNode->SaveAsFileNameBuf,
						   pSaveAsForecast->SaveAsFileName,
						   min(sizeof(pSaveAsNode->SaveAsFileNameBuf) - sizeof(WCHAR), sizeof(pSaveAsForecast->SaveAsFileName) - sizeof(WCHAR)));

					memcpy(pSaveAsNode->SourceFileNameBuf,
						   pSaveAsForecast->SourceFileName,
						   min(sizeof(pSaveAsNode->SourceFileNameBuf) - sizeof(WCHAR), sizeof(pSaveAsForecast->SourceFileName) - sizeof(WCHAR)));

					pSaveAsNode->ProcessId = (HANDLE)pSaveAsForecast->ProcessId;
				}
				__except(nxrmfltExceptionFilter(NULL, GetExceptionInformation()))
				{
					Status = GetExceptionCode();
					break;
				}

				ExInitializeRundownProtection(&pSaveAsNode->NodeRundownRef);
				
				RtlInitUnicodeString(&pSaveAsNode->SaveAsFileName, pSaveAsNode->SaveAsFileNameBuf);

				pSaveAsNode->SourceFileName.Buffer			= pSaveAsNode->SourceFileNameBuf;
				pSaveAsNode->SourceFileName.Length			= wcslen(pSaveAsNode->SourceFileNameBuf) * sizeof(WCHAR);
				pSaveAsNode->SourceFileName.MaximumLength	= sizeof(pSaveAsNode->SourceFileNameBuf);

				FltAcquirePushLockExclusive(&Global.ExpireTableLock);

				InsertHeadList(&Global.ExpireTable, &pSaveAsNode->Link);

				FltReleasePushLock(&Global.ExpireTableLock);

				pSaveAsNode = NULL;

			} while (FALSE);

			break;

		case nxrmfltPolicyChanged:

			if (CommandDataLength != 0)
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			FltAcquirePushLockExclusive(&Global.NxlProcessListLock);

			FOR_EACH_LIST(ite, &Global.NxlProcessList)
			{
				rb_root RightsCache = {0};
				rb_node *rb_ite = NULL;
				rb_node *rb_tmp = NULL;

				NXL_PROCESS_NODE *pNode = CONTAINING_RECORD(ite, NXL_PROCESS_NODE, Link);

				FltAcquirePushLockExclusive(&pNode->RightsCacheLock);

				RightsCache = pNode->RightsCache;

				pNode->RightsCache.rb_node = NULL;

				FltReleasePushLock(&pNode->RightsCacheLock);

				RB_EACH_NODE_SAFE(rb_ite, rb_tmp, &RightsCache)
				{
					NXL_RIGHTS_CACHE_NODE *pRightsCacheNode = CONTAINING_RECORD(rb_ite, NXL_RIGHTS_CACHE_NODE, Node);

					rb_erase(rb_ite, &RightsCache);

					pRightsCacheNode->FileNameHash		= 0;
					pRightsCacheNode->RightsMask		= MAX_ULONGLONG;

					ExFreeToPagedLookasideList(&Global.NXLRightsCacheLookaside, pRightsCacheNode);
				}
			}

			FltReleasePushLock(&Global.NxlProcessListLock);

			break;

		case nxrmfltSetKeys:
			
			Status = handle_set_keys_command((NXRM_KEY_BLOB*)CommandData, CommandDataLength);

			break;

		default:
			break;
		}

	} while (FALSE);
	
	if (pSaveAsNode)
	{
		ExFreeToPagedLookasideList(&Global.SaveAsExpireLookaside, pSaveAsNode);
		pSaveAsNode = NULL;
	}

	return Status;
}

NTSTATUS nxrmfltCloseServerPort(VOID)
{
	NTSTATUS Status = STATUS_SUCCESS;

	do 
	{
		Global.PortProcessId = 0;
		
		if (Global.ServerPort)
		{
			FltCloseCommunicationPort(Global.ServerPort);
			Global.ServerPort = NULL;
		}

	} while (FALSE);

	return Status;
}

static NTSTATUS handle_set_keys_command(NXRM_KEY_BLOB *Keys, ULONG KeyChainSizeInByte)
{
	NTSTATUS Status = STATUS_SUCCESS;


	LIST_ENTRY KeyChain = {0};

	NXL_KEYCHAIN_NODE *pNode = NULL;

	ULONG i = 0;

	ULONG NumberOfKey = 0;

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	NXL_KEKEY_BLOB PrimaryKey = {0};

	do 
	{
		NumberOfKey = KeyChainSizeInByte/sizeof(NXRM_KEY_BLOB);

		//
		// protect with try/except because of accessing user mode memory
		//
		__try
		{

			//
			// build primary key
			//
			memset(&PrimaryKey, 0, sizeof(PrimaryKey));

			memcpy(&PrimaryKey.KeyId,
				   &Keys[0].KeKeyId,
				   min(sizeof(PrimaryKey.KeyId), sizeof(Keys[0].KeKeyId)));

			PrimaryKey.keySize = sizeof(Keys[0].Key);

			memcpy(&PrimaryKey.Key, 
				   &Keys[0].Key,
				   min(sizeof(PrimaryKey.Key), sizeof(Keys[0].Key)));

			//
			// build a on stack list to avoid allocating memory while holding the push lock
			//
			InitializeListHead(&KeyChain);

			for (i=0; i < NumberOfKey; i++)
			{
				pNode = ExAllocatePoolWithTag(PagedPool, sizeof(NXL_KEYCHAIN_NODE), NXRMFLT_KEYCHAINNODE_TAG);

				if (pNode)
				{
					memset(pNode, 0, sizeof(NXL_KEYCHAIN_NODE));

					memcpy(&pNode->KeyBlob.KeyId,
						   &Keys[i].KeKeyId,
						   min(sizeof(pNode->KeyBlob.KeyId), sizeof(Keys[i].KeKeyId)));

					pNode->KeyBlob.keySize = sizeof(Keys[i].Key);

					memcpy(&pNode->KeyBlob.Key, 
						   Keys[i].Key,
						   min(sizeof(pNode->KeyBlob.Key), sizeof(Keys[i].Key)));

					InsertTailList(&KeyChain, &pNode->Link);
				}
			}
		}
		__except(nxrmfltExceptionFilter(NULL, GetExceptionInformation()))
		{
			Status = GetExceptionCode();
			break;
		}

		FltAcquirePushLockExclusive(&Global.KeyChainLock);

		NT_ASSERT(KeyChain.Flink);

		FOR_EACH_LIST_SAFE(ite, tmp, &KeyChain)
		{
			NT_ASSERT(ite->Flink);

			pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

			RemoveEntryList(ite);

			InsertTailList(&Global.KeyChain, &pNode->Link);
		}

		FltReleasePushLock(&Global.KeyChainLock);

		FltAcquirePushLockExclusive(&Global.PrimaryKeyLock);

		memcpy(&Global.PrimaryKey,
			   &PrimaryKey,
			   min(sizeof(Global.PrimaryKey), sizeof(PrimaryKey)));

		FltReleasePushLock(&Global.PrimaryKeyLock);

	} while(FALSE);

	//
	// KeyChain list should be empty if there is no error
	// free all node if there is a error
	//
	FOR_EACH_LIST_SAFE(ite, tmp, &KeyChain)
	{
		pNode = CONTAINING_RECORD(ite, NXL_KEYCHAIN_NODE, Link);

		ExFreePoolWithTag(pNode, NXRMFLT_KEYCHAINNODE_TAG);

		pNode = NULL;
	}

	return Status;
}
