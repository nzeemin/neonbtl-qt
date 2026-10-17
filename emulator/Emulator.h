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

bool Emulator_AddCPUBreakpoint(quint16 address, bool ishalt);
bool Emulator_RemoveCPUBreakpoint(quint16 address, bool ishalt);
bool Emulator_RemoveCPUBreakpoint(quint32 bpvalue);
void Emulator_SetTempCPUBreakpoint(quint16 address, bool ishalt);
const quint32* Emulator_GetCPUBreakpointList();
bool Emulator_IsBreakpoint();
bool Emulator_IsBreakpoint(quint16 address, bool ishalt);
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

void Emulator_UpdateKeyboardMatrix(const quint8 matrix[8]);

void Emulator_GetScreenSize(int scrmode, int* pwid, int* phei);
void Emulator_PrepareScreenRGB32(void* pImageBits, int screenMode);

// Instruction history, always recorded
const int INSTR_HISTORY_SIZE = 256;  // Must be a power of two

// A record is either an executed instruction or an interrupt taken by the processor
struct InstructionRecord
{
    quint16 pc;        // Instruction: its address; interrupt: PC where the interrupt appeared
    quint16 words[4];  // Instruction: opcode and the next words, enough for the longest instruction;
    // interrupt: vector, new PC, new PSW
    quint16 psw;       // Instruction: PSW before it; interrupt: new PSW
    bool    halt;      // true = HALT mode, false = USER mode (for an interrupt: mode of the interrupted code)
    bool    interrupt; // true = interrupt record
};

int Emulator_GetHistoryLength();
const InstructionRecord& Emulator_GetHistoryRecord(int index);  // index 0 = most recent
void Emulator_ClearHistory();
LPCTSTR Emulator_GetInterruptName(quint16 vector);  // Short name of the interrupt vector

// Update cached values after Run or Step
void Emulator_OnUpdate();
quint16 Emulator_GetChangeRamStatus(quint16 address);

bool Emulator_SaveImage(const QString &sFilePath);
bool Emulator_LoadImage(const QString &sFilePath);


//////////////////////////////////////////////////////////////////////
