// Emulator.h

#pragma once

#include "main.h"
#include "emubase/Board.h"

//////////////////////////////////////////////////////////////////////

const int MAX_BREAKPOINTCOUNT = 16;
const int MAX_WATCHESCOUNT = 16;

extern CMotherboard* g_pBoard;
extern NeonConfiguration g_nEmulatorConfiguration;  // Current configuration
extern bool g_okEmulatorRunning;

extern quint8* g_pEmulatorRam;  // RAM values - for change tracking
extern quint8* g_pEmulatorChangedRam;  // RAM change flags
extern quint16 g_wEmulatorCpuPC;      // Current PC value
extern quint16 g_wEmulatorPrevCpuPC;  // Previous PC value


//////////////////////////////////////////////////////////////////////


bool Emulator_Init();
bool Emulator_InitConfiguration(NeonConfiguration configuration);
void Emulator_Done();

bool Emulator_AddCPUBreakpoint(quint16 address);
bool Emulator_RemoveCPUBreakpoint(quint16 address);
void Emulator_SetTempCPUBreakpoint(quint16 address);
const quint16* Emulator_GetCPUBreakpointList();
bool Emulator_IsBreakpoint();
bool Emulator_IsBreakpoint(quint16 address);
void Emulator_RemoveAllBreakpoints();

const quint16* Emulator_GetWatchList();
bool Emulator_AddWatch(uint16_t address);
bool Emulator_RemoveWatch(uint16_t address);
void Emulator_RemoveAllWatches();

void Emulator_SetSound(bool enable);

void Emulator_Start();
void Emulator_Stop();
void Emulator_Reset();
bool Emulator_SystemFrame();
float Emulator_GetUptime();  // Device uptime, in seconds

void Emulator_GetScreenSize(int scrmode, int* pwid, int* phei);
void Emulator_PrepareScreenRGB32(void* pImageBits, int screenMode);

// Update cached values after Run or Step
void Emulator_OnUpdate();
quint16 Emulator_GetChangeRamStatus(quint16 address);

bool Emulator_SaveImage(const QString &sFilePath);
bool Emulator_LoadImage(const QString &sFilePath);


//////////////////////////////////////////////////////////////////////
