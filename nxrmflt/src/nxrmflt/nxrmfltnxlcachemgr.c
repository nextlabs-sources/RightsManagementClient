#include "nxrmfltdef.h"
#include "nxrmfltnxlcachemgr.h"

extern DECLSPEC_CACHEALIGN PFLT_FILTER				gFilterHandle;
extern DECLSPEC_CACHEALIGN ULONG					gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA		Global;

NXL_CACHE_NODE *
FindNXLNodeInCache(
rb_root			*Cache,
PUNICODE_STRING	FileName
)
{
	NXL_CACHE_NODE *pCacheNode = NULL;

	rb_node *ite = NULL;

	LONG CompareRet = 0;

	ite = Cache->rb_node;

	while (ite)
	{
		pCacheNode = CONTAINING_RECORD(ite, NXL_CACHE_NODE, Node);

		CompareRet = RtlCompareUnicodeString(&pCacheNode->FileName, FileName, TRUE);

		if (CompareRet < 0)
			ite = ite->rb_left;
		else if (CompareRet > 0)
			ite = ite->rb_right;
		else
			return pCacheNode;
	}

	return NULL;
}

BOOLEAN
AddNXLNodeToCache(
rb_root			*CacheMap,
NXL_CACHE_NODE	*pNXLCacheNode
)
{
	NXL_CACHE_NODE *pNode = NULL;

	rb_node **ite = NULL;
	rb_node *parent = NULL;

	LONG CompareRet = 0;

	ite = &(CacheMap->rb_node);
	parent = NULL;


	while (*ite)
	{
		pNode = CONTAINING_RECORD(*ite, NXL_CACHE_NODE, Node);

		parent = *ite;

		CompareRet = RtlCompareUnicodeString(&pNode->FileName, &pNXLCacheNode->FileName, TRUE);

		if (CompareRet < 0)
			ite = &((*ite)->rb_left);
		else if (CompareRet > 0)
			ite = &((*ite)->rb_right);
		else
			return FALSE;
	}

	rb_link_node(&pNXLCacheNode->Node, parent, ite);
	rb_insert_color(&pNXLCacheNode->Node, CacheMap);

	Global.NxlFileCount++;

	return TRUE;
}

NTSTATUS
DeleteNXLNodeInCacheByName(
rb_root			*Cache,
PUNICODE_STRING	FileName
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	NXL_CACHE_NODE *pCacheNode = NULL;

	pCacheNode = FindNXLNodeInCache(Cache, FileName);

	if (pCacheNode)
	{
		rb_erase(&pCacheNode->Node, Cache);

		Global.NxlFileCount--;
	}
	else
	{
		Status = STATUS_NOT_FOUND;
	}

	return Status;
}


VOID
DeleteNXLNodeInCache(
rb_root			*Cache,
NXL_CACHE_NODE	*pNXLCacheNode
)
{
	if (pNXLCacheNode)
	{
		rb_erase(&pNXLCacheNode->Node, Cache);

		Global.NxlFileCount--;
	}

	return;
}

VOID
FreeNXLCacheNode(
NXL_CACHE_NODE	*pNXLCacheNode
)
{
	//
	// Wait for all other threads rundown
	//
	ExWaitForRundownProtectionRelease(&pNXLCacheNode->NodeRundownRef);

	ExRundownCompleted(&pNXLCacheNode->NodeRundownRef);

	//
	// Free resources here
	//
	if (pNXLCacheNode->ReleaseFileName)
	{
		ExFreePoolWithTag(pNXLCacheNode->FileName.Buffer, NXRMFLT_NXLCACHE_TAG);

	}

	RtlInitUnicodeString(&pNXLCacheNode->FileName, NULL);

	if (pNXLCacheNode->ReleaseReparseName)
	{
		ExFreePoolWithTag(pNXLCacheNode->ReparseFileName.Buffer, NXRMFLT_NXLCACHE_TAG);

	}

	RtlInitUnicodeString(&pNXLCacheNode->ReparseFileName, NULL);

	if (pNXLCacheNode->ReleaseSourceFileName)
	{
		ExFreePoolWithTag(pNXLCacheNode->SourceFileName.Buffer, NXRMFLT_NXLCACHE_TAG);
	}

	RtlInitUnicodeString(&pNXLCacheNode->SourceFileName, NULL);
	//
	// Free to look aside list
	//
	ExFreeToPagedLookasideList(&Global.NXLCacheLookaside, pNXLCacheNode);
}