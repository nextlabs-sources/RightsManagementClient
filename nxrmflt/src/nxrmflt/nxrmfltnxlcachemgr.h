#pragma once

NXL_CACHE_NODE *
FindNXLNodeInCache(
rb_root			*Cache,
PUNICODE_STRING	FileName
);

BOOLEAN
AddNXLNodeToCache(
rb_root			*CacheMap,
NXL_CACHE_NODE	*pNXLCacheNode
);

NTSTATUS
DeleteNXLNodeInCacheByName(
rb_root			*Cache,
PUNICODE_STRING	FileName
);

VOID
DeleteNXLNodeInCache(
rb_root			*Cache,
NXL_CACHE_NODE	*pNXLCacheNode
);

VOID
FreeNXLCacheNode(
NXL_CACHE_NODE	*pNXLCacheNode
);