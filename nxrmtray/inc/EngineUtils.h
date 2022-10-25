#pragma once

BOOL                EngineGetStatus();
void                EngineUpdatePolicy();
void                EngineEnableDebug(BOOL bEnable);
void                EngineCollectLog();
void                EngineLogIn(LPCWSTR wzDefaultDomain = NULL);
void                EngineLogOut();
