﻿// Emulator.cpp

#include "stdafx.h"
#include "main.h"
#include "mainwindow.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
#include "qsoundout.h"
#include <QTime>
#include <QFile>


//////////////////////////////////////////////////////////////////////


CMotherboard* g_pBoard = nullptr;
static QSoundOut * g_sound = nullptr;
NeonConfiguration g_nEmulatorConfiguration;  // Current configuration
bool g_okEmulatorRunning = false;

int m_wEmulatorCPUBpsCount = 0;
quint16 m_EmulatorCPUBps[MAX_BREAKPOINTCOUNT + 1];
quint16 m_wEmulatorTempCPUBreakpoint = 0177777;
int m_wEmulatorWatchesCount = 0;
uint16_t m_EmulatorWatches[MAX_BREAKPOINTCOUNT];

static bool m_okEmulatorSound = false;

bool m_okEmulatorSerial = false;
FILE* m_fpEmulatorSerialOut = nullptr;

static long m_nFrameCount = 0;
static QTime m_emulatorTime;
static int m_nTickCount = 0;
static quint32 m_dwEmulatorUptime = 0;  // Device uptime, seconds, from turn on or reset, increments every 25 frames
static long m_nUptimeFrameCount = 0;

quint8* g_pEmulatorRam = nullptr;  // RAM values - for change tracking
quint8* g_pEmulatorChangedRam = nullptr;  // RAM change flags
quint16 g_wEmulatorCpuPC = 0177777;      // Current PC value
quint16 g_wEmulatorPrevCpuPC = 0177777;  // Previous PC value

void CALLBACK Emulator_SoundGenCallback(unsigned short L, unsigned short R);

void CALLBACK Emulator_FeedDAC(unsigned short l, unsigned short r);


//////////////////////////////////////////////////////////////////////


const size_t NEON_ROM_SIZE = 16384;

bool Emulator_LoadNeonRom()
{
    quint8 buffer[NEON_ROM_SIZE];

    // Load ROM file
    memset(buffer, 0, NEON_ROM_SIZE);
    QFile romFile(":/pk11.rom");
    if (!romFile.open(QIODevice::ReadOnly))
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to load ROM image file."));
        return false;
    }
    qint64 bytesRead = romFile.read((char*)buffer, NEON_ROM_SIZE);
    romFile.close();
    if (bytesRead != NEON_ROM_SIZE)
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to load ROM image file."));
        return false;
    }

    g_pBoard->LoadROM(buffer);
    return true;
}

bool Emulator_Init()
{
    ASSERT(g_pBoard == nullptr);

    CProcessor::Init();

    m_wEmulatorCPUBpsCount = 0;
    for (int i = 0; i <= MAX_BREAKPOINTCOUNT + 1; i++)
    {
        m_EmulatorCPUBps[i] = 0177777;
    }
    m_wEmulatorWatchesCount = 0;
    for (int i = 0; i <= MAX_WATCHESCOUNT; i++)
    {
        m_EmulatorWatches[i] = 0177777;
    }

    g_pBoard = new CMotherboard();

    // Allocate memory for old RAM values
    g_pEmulatorRam = static_cast<uint8_t*>(::calloc(65536, 1));
    g_pEmulatorChangedRam = static_cast<uint8_t*>(::calloc(65536, 1));

    g_pBoard->Reset();

    g_sound = new QSoundOut();
    if (m_okEmulatorSound)
    {
        g_pBoard->SetSoundGenCallback(Emulator_FeedDAC);
    }

    //Emulator_SetSerial(true);  //DEBUG

    return true;
}

void Emulator_Done()
{
    ASSERT(g_pBoard != nullptr);

    CProcessor::Done();

    g_pBoard->SetSoundGenCallback(nullptr);
    if (g_sound)
    {
        delete g_sound;
        g_sound = nullptr;
    }

    delete g_pBoard;
    g_pBoard = nullptr;

    // Free memory used for old RAM values
    ::free(g_pEmulatorRam);
    ::free(g_pEmulatorChangedRam);
}

bool Emulator_InitConfiguration(NeonConfiguration configuration)
{
    g_pBoard->SetConfiguration((uint16_t)configuration);

    if (!Emulator_LoadNeonRom())
    {
        AlertWarning(_T("Failed to load ROM file."));
        return false;
    }

    g_nEmulatorConfiguration = configuration;

    g_pBoard->Reset();

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;

    return true;
}

void Emulator_Start()
{
    g_okEmulatorRunning = true;

    // Set title bar text
    Global_getMainWindow()->updateWindowText();
    Global_getMainWindow()->updateMenu();

    m_nFrameCount = 0;
    m_emulatorTime.restart();
    m_nTickCount = 0;

    // For proper breakpoint processing
    if (m_wEmulatorCPUBpsCount != 0)
    {
        g_pBoard->GetCPU()->ClearInternalTick();
    }
}
void Emulator_Stop()
{
    g_okEmulatorRunning = false;

    Emulator_SetTempCPUBreakpoint(0177777);

    // Set title bar text
    Global_getMainWindow()->updateWindowText();

    // Reset title bar message
    Global_getMainWindow()->updateWindowText();
    Global_getMainWindow()->updateMenu();

    // Reset FPS indicator
    Global_showFps(-1.0);

    Global_UpdateAllViews();
}

void Emulator_Reset()
{
    ASSERT(g_pBoard != nullptr);

    g_pBoard->Reset();

    m_nUptimeFrameCount = 0;
    m_dwEmulatorUptime = 0;
    Global_showUptime(0);

    Global_UpdateAllViews();
}

bool Emulator_AddCPUBreakpoint(quint16 address)
{
    if (m_wEmulatorCPUBpsCount == MAX_BREAKPOINTCOUNT - 1 || address == 0177777)
        return false;
    for (int i = 0; i < m_wEmulatorCPUBpsCount; i++)  // Check if the BP exists
    {
        if (m_EmulatorCPUBps[i] == address)
            return false;  // Already in the list
    }
    for (int i = 0; i < MAX_BREAKPOINTCOUNT; i++)  // Put in the first empty cell
    {
        if (m_EmulatorCPUBps[i] == 0177777)
        {
            m_EmulatorCPUBps[i] = address;
            break;
        }
    }
    m_wEmulatorCPUBpsCount++;
    return true;
}
bool Emulator_RemoveCPUBreakpoint(quint16 address)
{
    if (m_wEmulatorCPUBpsCount == 0 || address == 0177777)
        return false;
    for (int i = 0; i < MAX_BREAKPOINTCOUNT; i++)
    {
        if (m_EmulatorCPUBps[i] == address)
        {
            m_EmulatorCPUBps[i] = 0177777;
            m_wEmulatorCPUBpsCount--;
            if (m_wEmulatorCPUBpsCount > i)  // fill the hole
            {
                m_EmulatorCPUBps[i] = m_EmulatorCPUBps[m_wEmulatorCPUBpsCount];
                m_EmulatorCPUBps[m_wEmulatorCPUBpsCount] = 0177777;
            }
            return true;
        }
    }
    return false;
}
void Emulator_SetTempCPUBreakpoint(quint16 address)
{
    if (m_wEmulatorTempCPUBreakpoint != 0177777)
        Emulator_RemoveCPUBreakpoint(m_wEmulatorTempCPUBreakpoint);
    if (address == 0177777)
    {
        m_wEmulatorTempCPUBreakpoint = 0177777;
        return;
    }
    for (int i = 0; i < MAX_BREAKPOINTCOUNT; i++)
    {
        if (m_EmulatorCPUBps[i] == address)
            return;  // We have regular breakpoint with the same address
    }
    m_wEmulatorTempCPUBreakpoint = address;
    m_EmulatorCPUBps[m_wEmulatorCPUBpsCount] = address;
    m_wEmulatorCPUBpsCount++;
}
const quint16* Emulator_GetCPUBreakpointList() { return m_EmulatorCPUBps; }
bool Emulator_IsBreakpoint()
{
    quint16 address = g_pBoard->GetCPU()->GetPC();
    if (m_wEmulatorCPUBpsCount > 0)
    {
        for (int i = 0; i < m_wEmulatorCPUBpsCount; i++)
        {
            if (address == m_EmulatorCPUBps[i])
                return true;
        }
    }
    return false;
}
bool Emulator_IsBreakpoint(quint16 address)
{
    if (m_wEmulatorCPUBpsCount == 0)
        return false;
    for (int i = 0; i < m_wEmulatorCPUBpsCount; i++)
    {
        if (address == m_EmulatorCPUBps[i])
            return true;
    }
    return false;
}
void Emulator_RemoveAllBreakpoints()
{
    for (int i = 0; i < MAX_BREAKPOINTCOUNT; i++)
        m_EmulatorCPUBps[i] = 0177777;
    m_wEmulatorCPUBpsCount = 0;
}

const uint16_t* Emulator_GetWatchList() { return m_EmulatorWatches; }
bool Emulator_AddWatch(uint16_t address)
{
    if (m_wEmulatorWatchesCount == MAX_WATCHESCOUNT - 1 || address == 0177777)
        return false;
    for (int i = 0; i < m_wEmulatorWatchesCount; i++)  // Check if the BP exists
    {
        if (m_EmulatorWatches[i] == address)
            return false;  // Already in the list
    }
    for (int i = 0; i < MAX_BREAKPOINTCOUNT; i++)  // Put in the first empty cell
    {
        if (m_EmulatorWatches[i] == 0177777)
        {
            m_EmulatorWatches[i] = address;
            break;
        }
    }
    m_wEmulatorWatchesCount++;
    return true;
}
bool Emulator_RemoveWatch(uint16_t address)
{
    if (m_wEmulatorWatchesCount == 0 || address == 0177777)
        return false;
    for (int i = 0; i < MAX_WATCHESCOUNT; i++)
    {
        if (m_EmulatorWatches[i] == address)
        {
            m_EmulatorWatches[i] = 0177777;
            m_wEmulatorWatchesCount--;
            if (m_wEmulatorWatchesCount > i)  // fill the hole
            {
                m_EmulatorWatches[i] = m_EmulatorWatches[m_wEmulatorWatchesCount];
                m_EmulatorWatches[m_wEmulatorWatchesCount] = 0177777;
            }
            return true;
        }
    }
    return false;
}
void Emulator_RemoveAllWatches()
{
    for (int i = 0; i < MAX_WATCHESCOUNT; i++)
        m_EmulatorWatches[i] = 0177777;
    m_wEmulatorWatchesCount = 0;
}

void Emulator_SetSound(bool enable)
{
    if (g_pBoard != nullptr)
    {
        if (enable)
            g_pBoard->SetSoundGenCallback(Emulator_FeedDAC);
        else
            g_pBoard->SetSoundGenCallback(nullptr);
    }

    m_okEmulatorSound = enable;
}

void Emulator_UpdateKeyboardMatrix(const quint8 matrix[8])
{
    g_pBoard->UpdateKeyboardMatrix(matrix);
}


bool Emulator_SystemFrame()
{
    g_pBoard->SetCPUBreakpoints(m_wEmulatorCPUBpsCount > 0 ? m_EmulatorCPUBps : nullptr);

    //Emulator_ProcessKeyEvent();

    if (!g_pBoard->SystemFrame())  // Breakpoint hit
    {
        Emulator_SetTempCPUBreakpoint(0177777);
        return false;
    }

    // Calculate frames per second
    m_nFrameCount++;
    int nCurrentTicks = m_emulatorTime.elapsed();
    long nTicksElapsed = nCurrentTicks - m_nTickCount;
    if (nTicksElapsed >= 1200)
    {
        double dFramesPerSecond = m_nFrameCount * 1000.0 / nTicksElapsed;
        Global_showFps(dFramesPerSecond);

        m_nFrameCount = 0;
        m_nTickCount = nCurrentTicks;
    }

    // Calculate emulator uptime (25 frames per second)
    m_nUptimeFrameCount++;
    if (m_nUptimeFrameCount >= 25)
    {
        m_dwEmulatorUptime++;
        m_nUptimeFrameCount = 0;

        Global_showUptime(m_dwEmulatorUptime);
    }

    return true;
}

void CALLBACK Emulator_FeedDAC(unsigned short l, unsigned short r)
{
    if (g_sound)
    {
        if (m_okEmulatorSound)
            g_sound->FeedDAC(l, r);
    }
}

float Emulator_GetUptime()
{
    return (float)m_dwEmulatorUptime + float(m_nUptimeFrameCount) / 25.0f;
}

// Update cached values after Run or Step
void Emulator_OnUpdate()
{
    // Update stored PC value
    g_wEmulatorPrevCpuPC = g_wEmulatorCpuPC;
    g_wEmulatorCpuPC = g_pBoard->GetCPU()->GetPC();

    // Update memory change flags
    quint8* pOld = g_pEmulatorRam;
    quint8* pChanged = g_pEmulatorChangedRam;
    quint32 addr = 0;
    do
    {
        quint8 newvalue = g_pBoard->GetRAMByte(addr);
        quint8 oldvalue = *pOld;
        *pChanged = (newvalue != oldvalue) ? 255 : 0;
        *pOld = newvalue;
        addr++;
        pOld++;  pChanged++;
    }
    while (addr < 65535);//TODO
}

// Get RAM change flag
//   addrtype - address mode - see ADDRTYPE_XXX constants
quint16 Emulator_GetChangeRamStatus(quint16 address)
{
    return *((uint16_t*)(g_pEmulatorChangedRam + address));
}

// Прототип функции, вызываемой для каждой сформированной строки экрана
typedef void (CALLBACK* SCREEN_LINE_CALLBACK)(uint32_t* pImageBits, const quint32* pLineBits, int line);

void CALLBACK PrepareScreenLine416x300(uint32_t* pImageBits, const quint32* pLineBits, int line);
void CALLBACK PrepareScreenLine624x450(uint32_t* pImageBits, const quint32* pLineBits, int line);
void CALLBACK PrepareScreenLine832x600(uint32_t* pImageBits, const quint32* pLineBits, int line);
void CALLBACK PrepareScreenLine1040x750(uint32_t* pImageBits, const quint32* pLineBits, int line);
void CALLBACK PrepareScreenLine1248x900(uint32_t* pImageBits, const quint32* pLineBits, int line);

struct ScreenModeStruct
{
    int width;
    int height;
    SCREEN_LINE_CALLBACK lineCallback;
}
static ScreenModeReference[] =
{
    // wid  hei  callback                           size      scaleX scaleY   notes
    {  416, 300, PrepareScreenLine416x300  },  //  416 x 300   0.5     1      Debug mode
    {  624, 450, PrepareScreenLine624x450  },  //  624 x 450   0.75    1.5
    {  832, 600, PrepareScreenLine832x600  },  //  832 x 600   1       2
    { 1040, 750, PrepareScreenLine1040x750 },  // 1040 x 750   1.25    2.5
    { 1248, 900, PrepareScreenLine1248x900 },  // 1248 x 900   1.5     3
};

void Emulator_GetScreenSize(int scrmode, int* pwid, int* phei)
{
    if (scrmode < 0 || scrmode >= sizeof(ScreenModeReference) / sizeof(ScreenModeStruct))
        return;
    ScreenModeStruct* pinfo = ScreenModeReference + scrmode;
    *pwid = pinfo->width;
    *phei = pinfo->height;
}

void Emulator_PrepareScreenLines(void* pImageBits, SCREEN_LINE_CALLBACK lineCallback);

void Emulator_PrepareScreenRGB32(void* pImageBits, int screenMode)
{
    if (pImageBits == nullptr) return;

    // Render to bitmap
    SCREEN_LINE_CALLBACK lineCallback = ScreenModeReference[screenMode].lineCallback;
    Emulator_PrepareScreenLines(pImageBits, lineCallback);
}

uint32_t Color16Convert(uint16_t color)
{
    return
        ((color & 0x0300) >> 2 | (color & 0x0007) << 3 | (color & 0x0300) >> 7) |
        ((color & 0xe000) >> 8 | (color & 0x00e0) >> 3 | (color & 0xC000) >> 14) << 8 |
        ((color & 0x1C00) >> 5 | (color & 0x0018) | (color & 0x1C00) >> 10) << 16 |
        0xFF000000;
}

#define FILL1PIXEL(color) { *plinebits++ = color; }
#define FILL2PIXELS(color) { *plinebits++ = color; *plinebits++ = color; }
#define FILL4PIXELS(color) { *plinebits++ = color; *plinebits++ = color; *plinebits++ = color; *plinebits++ = color; }
#define FILL8PIXELS(color) { \
    *plinebits++ = color; *plinebits++ = color; *plinebits++ = color; *plinebits++ = color; \
    *plinebits++ = color; *plinebits++ = color; *plinebits++ = color; *plinebits++ = color; \
}
// Выражение для получения 16-разрядного цвета из палитры; pala = адрес старшего байта
#define GETPALETTEHILO(pala) ((uint16_t)(pBoard->GetRAMByteView(pala) << 8) | pBoard->GetRAMByteView((pala) + 256))

// Формирует 300 строк экрана; для каждой сформированной строки вызывает функцию lineCallback
void Emulator_PrepareScreenLines(void* pImageBits, SCREEN_LINE_CALLBACK lineCallback)
{
    if (pImageBits == nullptr || lineCallback == nullptr || g_pBoard == nullptr) return;

    uint32_t linebits[NEON_SCREEN_WIDTH];  // буфер под строку

    const CMotherboard* pBoard = g_pBoard;

    uint16_t vdptaslo = pBoard->GetRAMWordView(0000010);  // VDPTAS
    uint16_t vdptashi = pBoard->GetRAMWordView(0000012);  // VDPTAS
    uint16_t vdptaplo = pBoard->GetRAMWordView(0000004);  // VDPTAP
    uint16_t vdptaphi = pBoard->GetRAMWordView(0000006);  // VDPTAP

    uint32_t tasaddr = (((uint32_t)vdptaslo) << 2) | (((uint32_t)(vdptashi & 0x000f)) << 18);
    uint32_t tapaddr = (((uint32_t)vdptaplo) << 2) | (((uint32_t)(vdptaphi & 0x000f)) << 18);
    uint16_t pal0 = GETPALETTEHILO(tapaddr);
    uint32_t colorBorder = Color16Convert(pal0);  // Глобальный цвет бордюра

    for (int line = -2; line < 300; line++)  // Цикл по строкам -2..299, первые две строки не видны
    {
        uint16_t linelo = pBoard->GetRAMWordView(tasaddr);
        uint16_t linehi = pBoard->GetRAMWordView(tasaddr + 2);
        tasaddr += 4;
        if (line < 0)
            continue;

        uint32_t* plinebits = linebits;
        uint32_t lineaddr = (((uint32_t)linelo) << 2) | (((uint32_t)(linehi & 0x000f)) << 18);
        bool firstOtr = true;  // Признак первого отрезка в строке
        uint32_t colorbprev = 0;  // Цвет бордюра предыдущего отрезка
        int bar = 52;  // Счётчик полосок от 52 к 0
        for (;;)  // Цикл по видеоотрезкам строки, до полного заполнения строки
        {
            uint16_t otrlo = pBoard->GetRAMWordView(lineaddr);
            uint16_t otrhi = pBoard->GetRAMWordView(lineaddr + 2);
            lineaddr += 4;
            // Получаем параметры отрезка
            int otrcount = 32 - (otrhi >> 10) & 037;  // Длина отрезка в 32-разрядных словах
            if (otrcount == 0) otrcount = 32;
            uint32_t otraddr = (((uint32_t)otrlo) << 2) | (((uint32_t)otrhi & 0x000f) << 18);
            uint16_t otrvn = (otrhi >> 6) & 3;  // VN1 VN0 - бит/точку
            bool otrpb = (otrhi & 0x8000) != 0;
            uint16_t vmode = (otrhi >> 6) & 0x0f;  // биты VD1 VD0 VN1 VN0
            // Получить адрес палитры
            uint32_t paladdr = tapaddr;
            if (otrvn == 3 && otrpb)  // Многоцветный режим
            {
                paladdr += (otrhi & 0x10) ? 1024 + 512 : 1024;
            }
            else
            {
                paladdr += (otrpb ? 512 : 0) + (otrvn * 64);
                uint32_t otrpn = (otrhi >> 4) & 3;  // PN1 PN0 - номер палитры
                paladdr += otrpn * 16;
            }
            // Бордюр
            uint16_t palbhi = pBoard->GetRAMWordView(paladdr);
            uint16_t palblo = pBoard->GetRAMWordView(paladdr + 256);
            uint32_t colorb = Color16Convert((uint16_t)((palbhi & 0xff) << 8 | (palblo & 0xff)));
            if (!firstOtr)  // Это не первый отрезок - будет бордюр, цвета по пикселям: AAAAAAAAABBCCCCC
            {
                FILL8PIXELS(colorbprev)  FILL1PIXEL(colorbprev)
                FILL2PIXELS(colorBorder)
                FILL4PIXELS(colorb)  FILL1PIXEL(colorb)
                bar--;  if (bar == 0) break;
            }
            colorbprev = colorb;  // Запоминаем цвет бордюра
            // Определяем, сколько 16-пиксельных полосок нужно заполнить
            int barcount = otrcount * 2;
            if (!firstOtr) barcount--;
            if (barcount > bar) barcount = bar;
            bar -= barcount;
            // Заполняем отрезок
            if (vmode == 0)  // VM1, плотность видео-строки 52 байта, со сдвигом влево на 2 байта
            {
                uint16_t pal14hi = pBoard->GetRAMWordView(paladdr + 14);
                uint16_t pal14lo = pBoard->GetRAMWordView(paladdr + 14 + 256);
                uint32_t color0 = Color16Convert((uint16_t)((pal14hi & 0xff) << 8 | (pal14lo & 0xff)));
                uint32_t color1 = Color16Convert((uint16_t)((pal14hi & 0xff00) | (pal14lo & 0xff00) >> 8));
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMByteView(otraddr);
                    otraddr++;
                    uint32_t color = (bits & 1) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 2) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 4) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 8) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 16) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 32) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 64) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 128) ? color1 : color0;
                    FILL2PIXELS(color)
                    barcount--;
                }
            }
            else if (vmode == 1)  // VM2, плотность видео-строки 52 байта
            {
                while (barcount > 0)
                {
                    uint8_t bits = pBoard->GetRAMByteView(otraddr);  // читаем байт - выводим 16 пикселей
                    otraddr++;
                    uint32_t palc = paladdr + (bits & 3);
                    uint16_t c = GETPALETTEHILO(palc);
                    uint32_t color = Color16Convert(c);
                    FILL4PIXELS(color)
                    palc = paladdr + ((bits >> 2) & 3);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL4PIXELS(color)
                    palc = paladdr + ((bits >> 4) & 3);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL4PIXELS(color)
                    palc = paladdr + (bits >> 6);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL4PIXELS(color)
                    barcount--;
                }
            }
            else if (vmode == 2 || vmode == 6 ||
                    (vmode == 3 && !otrpb) ||
                    (vmode == 7 && !otrpb))  // VM4, плотность видео-строки 52 байта
            {
                while (barcount > 0)
                {
                    uint8_t bits = pBoard->GetRAMByteView(otraddr);  // читаем байт - выводим 16 пикселей
                    otraddr++;
                    uint32_t palc = paladdr + (bits & 15);
                    uint16_t c = GETPALETTEHILO(palc);
                    uint32_t color = Color16Convert(c);
                    FILL8PIXELS(color)
                    palc = paladdr + (bits >> 4);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL8PIXELS(color)
                    barcount--;
                }
            }
            else if ((vmode == 3 && otrpb) ||
                    (vmode == 7 && otrpb))  // VM8, плотность видео-строки 52 байта
            {
                while (barcount > 0)
                {
                    uint8_t bits = pBoard->GetRAMByteView(otraddr);  // читаем байт - выводим 16 пикселей
                    otraddr++;
                    uint32_t palc = paladdr + bits;
                    uint16_t c = GETPALETTEHILO(palc);
                    uint32_t color = Color16Convert(c);
                    FILL8PIXELS(color)
                    FILL8PIXELS(color)
                    barcount--;
                }
            }
            else if (vmode == 4)  // VM1, плотность видео-строки 52 байта
            {
                uint16_t pal14hi = pBoard->GetRAMWordView(paladdr + 14);
                uint16_t pal14lo = pBoard->GetRAMWordView(paladdr + 14 + 256);
                uint32_t color0 = Color16Convert((uint16_t)((pal14hi & 0xff) << 8 | (pal14lo & 0xff)));
                uint32_t color1 = Color16Convert((uint16_t)((pal14hi & 0xff00) | (pal14lo & 0xff00) >> 8));
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr & ~1);
                    if (otraddr & 1) bits = bits >> 8;
                    otraddr++;
                    uint32_t color = (bits & 1) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 2) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 4) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 8) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 16) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 32) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 64) ? color1 : color0;
                    FILL2PIXELS(color)
                    color = (bits & 128) ? color1 : color0;
                    FILL2PIXELS(color)
                    barcount--;
                }
            }
            else if (vmode == 5)  // VM2, плотность видео-строки 52 байта
            {
                while (barcount > 0)
                {
                    uint8_t bits = pBoard->GetRAMByteView(otraddr);  // читаем байт - выводим 16 пикселей
                    otraddr++;
                    uint32_t palc0 = (paladdr + 12 + (bits & 3));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL4PIXELS(color0)
                    uint32_t palc1 = (paladdr + 12 + ((bits >> 2) & 3));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL4PIXELS(color1)
                    uint32_t palc2 = (paladdr + 12 + ((bits >> 4) & 3));
                    uint16_t c2 = GETPALETTEHILO(palc2);
                    uint32_t color2 = Color16Convert(c2);
                    FILL4PIXELS(color2)
                    uint32_t palc3 = (paladdr + 12 + ((bits >> 6) & 3));
                    uint16_t c3 = GETPALETTEHILO(palc3);
                    uint32_t color3 = Color16Convert(c3);
                    FILL4PIXELS(color3)
                    barcount--;
                }
            }
            else if (vmode == 8)  // VM1, плотность видео-строки 104 байта
            {
                uint16_t pal14hi = pBoard->GetRAMWordView(paladdr + 14);
                uint16_t pal14lo = pBoard->GetRAMWordView(paladdr + 14 + 256);
                uint32_t color0 = Color16Convert((uint16_t)((pal14hi & 0xff) << 8 | (pal14lo & 0xff)));
                uint32_t color1 = Color16Convert((uint16_t)((pal14hi & 0xff00) | (pal14lo & 0xff00) >> 8));
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);
                    otraddr += 2;
                    uint32_t color = (bits & 1) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 2) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 4) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 8) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 16) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 32) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 64) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 128) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x0100) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x0200) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x0400) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x0800) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x1000) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x2000) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x4000) ? color1 : color0;
                    FILL1PIXEL(color)
                    color = (bits & 0x8000) ? color1 : color0;
                    FILL1PIXEL(color)
                    barcount--;
                }
            }
            else if (vmode == 9)
            {
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 16 пикселей
                    otraddr += 2;
                    uint32_t palc0 = (paladdr + 12 + (bits & 3));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL2PIXELS(color0)
                    uint32_t palc1 = (paladdr + 12 + ((bits >> 2) & 3));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL2PIXELS(color1)
                    uint32_t palc2 = (paladdr + 12 + ((bits >> 4) & 3));
                    uint16_t c2 = GETPALETTEHILO(palc2);
                    uint32_t color2 = Color16Convert(c2);
                    FILL2PIXELS(color2)
                    uint32_t palc3 = (paladdr + 12 + ((bits >> 6) & 3));
                    uint16_t c3 = GETPALETTEHILO(palc3);
                    uint32_t color3 = Color16Convert(c3);
                    FILL2PIXELS(color3)
                    uint32_t palc4 = (paladdr + 12 + ((bits >> 8) & 3));
                    uint16_t c4 = GETPALETTEHILO(palc4);
                    uint32_t color4 = Color16Convert(c4);
                    FILL2PIXELS(color4)
                    uint32_t palc5 = (paladdr + 12 + ((bits >> 10) & 3));
                    uint16_t c5 = GETPALETTEHILO(palc5);
                    uint32_t color5 = Color16Convert(c5);
                    FILL2PIXELS(color5)
                    uint32_t palc6 = (paladdr + 12 + ((bits >> 12) & 3));
                    uint16_t c6 = GETPALETTEHILO(palc6);
                    uint32_t color6 = Color16Convert(c6);
                    FILL2PIXELS(color6)
                    uint32_t palc7 = (paladdr + 12 + ((bits >> 14) & 3));
                    uint16_t c7 = GETPALETTEHILO(palc7);
                    uint32_t color7 = Color16Convert(c7);
                    FILL2PIXELS(color7)
                    barcount--;
                }
            }
            else if (vmode == 10)  // VM4, плотность видео-строки 104 байта
            {
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 16 пикселей
                    otraddr += 2;
                    uint32_t palc = paladdr + (bits & 15);
                    uint16_t c = GETPALETTEHILO(palc);
                    uint32_t color = Color16Convert(c);
                    FILL4PIXELS(color)
                    palc = paladdr + ((bits >> 4) & 15);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL4PIXELS(color)
                    palc = paladdr + ((bits >> 8) & 15);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL4PIXELS(color)
                    palc = paladdr + ((bits >> 12) & 15);
                    c = GETPALETTEHILO(palc);
                    color = Color16Convert(c);
                    FILL4PIXELS(color)
                    barcount--;
                }
            }
            else if (vmode == 11 && !otrpb)  // VM41, плотность видео-строки 104 байта
            {
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 16 пикселей
                    otraddr += 2;
                    uint32_t palc0 = (paladdr + (bits & 15));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL4PIXELS(color0)
                    uint32_t palc1 = (paladdr + ((bits >> 4) & 15));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL4PIXELS(color1)
                    uint32_t palc2 = (paladdr + ((bits >> 8) & 15));
                    uint16_t c2 = GETPALETTEHILO(palc2);
                    uint32_t color2 = Color16Convert(c2);
                    FILL4PIXELS(color2)
                    uint32_t palc3 = (paladdr + ((bits >> 12) & 15));
                    uint16_t c3 = GETPALETTEHILO(palc3);
                    uint32_t color3 = Color16Convert(c3);
                    FILL4PIXELS(color3)
                    barcount--;
                }
            }
            else if (vmode == 11 && otrpb)  // VM8, плотность видео-строки 104 байта
            {
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 16 пикселей
                    otraddr += 2;
                    uint32_t palc0 = (paladdr + (bits & 0xff));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL8PIXELS(color0)
                    uint32_t palc1 = (paladdr + (bits >> 8));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL8PIXELS(color1)
                    barcount--;
                }
            }
            else if (vmode == 13)  // VM2, плотность видео-строки 208 байт
            {
                for (int j = 0; j < barcount * 2; j++)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 8 пикселей
                    otraddr += 2;
                    uint32_t palc0 = (paladdr + 12 + (bits & 3));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL1PIXEL(color0)
                    uint32_t palc1 = (paladdr + 12 + ((bits >> 2) & 3));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL1PIXEL(color1)
                    uint32_t palc2 = (paladdr + 12 + ((bits >> 4) & 3));
                    uint16_t c2 = GETPALETTEHILO(palc2);
                    uint32_t color2 = Color16Convert(c2);
                    FILL1PIXEL(color2)
                    uint32_t palc3 = (paladdr + 12 + ((bits >> 6) & 3));
                    uint16_t c3 = GETPALETTEHILO(palc3);
                    uint32_t color3 = Color16Convert(c3);
                    FILL1PIXEL(color3)
                    uint32_t palc4 = (paladdr + 12 + ((bits >> 8) & 3));
                    uint16_t c4 = GETPALETTEHILO(palc4);
                    uint32_t color4 = Color16Convert(c4);
                    FILL1PIXEL(color4)
                    uint32_t palc5 = (paladdr + 12 + ((bits >> 10) & 3));
                    uint16_t c5 = GETPALETTEHILO(palc5);
                    uint32_t color5 = Color16Convert(c5);
                    FILL1PIXEL(color5)
                    uint32_t palc6 = (paladdr + 12 + ((bits >> 12) & 3));
                    uint16_t c6 = GETPALETTEHILO(palc6);
                    uint32_t color6 = Color16Convert(c6);
                    FILL1PIXEL(color6)
                    uint32_t palc7 = (paladdr + 12 + ((bits >> 14) & 3));
                    uint16_t c7 = GETPALETTEHILO(palc7);
                    uint32_t color7 = Color16Convert(c7);
                    FILL1PIXEL(color7)
                }
            }
            else if ((vmode == 14) ||  // VM4, плотность видео-строки 208 байт
                    (vmode == 15 && !otrpb))  // VM41, плотность видео-строки 208 байт
            {
                for (int j = 0; j < barcount * 2; j++)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 8 пикселей
                    otraddr += 2;
                    uint32_t palc0 = (paladdr + (bits & 15));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL2PIXELS(color0)
                    uint32_t palc1 = (paladdr + ((bits >> 4) & 15));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL2PIXELS(color1)
                    uint32_t palc2 = (paladdr + ((bits >> 8) & 15));
                    uint16_t c2 = GETPALETTEHILO(palc2);
                    uint32_t color2 = Color16Convert(c2);
                    FILL2PIXELS(color2)
                    uint32_t palc3 = (paladdr + ((bits >> 12) & 15));
                    uint16_t c3 = GETPALETTEHILO(palc3);
                    uint32_t color3 = Color16Convert(c3);
                    FILL2PIXELS(color3)
                }
            }
            else if (vmode == 15 && otrpb)  // VM8, плотность видео-строки 208 байт
            {
                while (barcount > 0)
                {
                    uint16_t bits0 = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 8 пикселей
                    otraddr += 2;
                    uint32_t palc0 = (paladdr + (bits0 & 0xff));
                    uint16_t c0 = GETPALETTEHILO(palc0);
                    uint32_t color0 = Color16Convert(c0);
                    FILL4PIXELS(color0)
                    uint32_t palc1 = (paladdr + (bits0 >> 8));
                    uint16_t c1 = GETPALETTEHILO(palc1);
                    uint32_t color1 = Color16Convert(c1);
                    FILL4PIXELS(color1)
                    uint16_t bits1 = pBoard->GetRAMWordView(otraddr);  // читаем слово - выводим 8 пикселей
                    otraddr += 2;
                    uint32_t palc2 = (paladdr + (bits1 & 0xff));
                    uint16_t c2 = GETPALETTEHILO(palc2);
                    uint32_t color2 = Color16Convert(c2);
                    FILL4PIXELS(color2)
                    uint32_t palc3 = (paladdr + (bits1 >> 8));
                    uint16_t c3 = GETPALETTEHILO(palc3);
                    uint32_t color3 = Color16Convert(c3);
                    FILL4PIXELS(color3)
                    barcount--;
                }
            }
            else //if (vmode == 12)  // VM1, плотность видео-строки 208 байт - запрещенный режим
            {
                //NOTE: Как выяснилось, берутся только чётные биты из строки в 208 байт
                uint16_t pal14hi = pBoard->GetRAMWordView(paladdr + 14);
                uint16_t pal14lo = pBoard->GetRAMWordView(paladdr + 14 + 256);
                uint32_t color0 = Color16Convert((uint16_t)((pal14hi & 0xff) << 8 | (pal14lo & 0xff)));
                uint32_t color1 = Color16Convert((uint16_t)((pal14hi & 0xff00) | (pal14lo & 0xff00) >> 8));
                while (barcount > 0)
                {
                    uint16_t bits = pBoard->GetRAMWordView(otraddr);
                    otraddr += 2;
                    for (uint16_t k = 0; k < 8; k++)
                    {
                        uint32_t color = (bits & 2) ? color1 : color0;
                        FILL1PIXEL(color)
                        bits = bits >> 2;
                    }
                    bits = pBoard->GetRAMWordView(otraddr);
                    otraddr += 2;
                    for (uint16_t k = 0; k < 8; k++)
                    {
                        uint32_t color = (bits & 2) ? color1 : color0;
                        FILL1PIXEL(color)
                        bits = bits >> 2;
                    }
                    barcount--;
                }
            }

            if (bar <= 0) break;
            firstOtr = false;
        }

        (*lineCallback)((uint32_t*)pImageBits, linebits, line);
    }
}

// 1/2 part of "a" plus 1/2 part of "b"
#define AVERAGERGB(a, b)  ( (((a) & 0xfefefeffUL) + ((b) & 0xfefefeffUL)) >> 1 )

// 1/4 part of "a" plus 3/4 parts of "b"
#define AVERAGERGB13(a, b)  ( ((a) == (b)) ? a : (((a) & 0xfcfcfcffUL) >> 2) + ((b) - (((b) & 0xfcfcfcffUL) >> 2)) )

void CALLBACK PrepareScreenLine416x300(uint32_t* pImageBits, const uint32_t* pLineBits, int line)
{
    uint32_t* pBits = pImageBits + line * 416;
    for (int x = 0; x < 832; x += 2)
    {
        uint32_t color1 = *pLineBits++;
        uint32_t color2 = *pLineBits++;
        uint32_t color = AVERAGERGB(color1, color2);
        *pBits++ = color;
    }
}

void CALLBACK PrepareScreenLine624x450(uint32_t* pImageBits, const uint32_t* pLineBits, int line)
{
    bool even = (line & 1) == 0;
    uint32_t* pBits = pImageBits + (line / 2 * 3) * 624;
    if (!even)
        pBits += 624 * 2;

    uint32_t* p = pBits;
    for (int x = 0; x < 832; x += 4)  // x0.75 - mapping every 4 pixels into 3 pixels
    {
        uint32_t color1 = *pLineBits++;
        uint32_t color2 = *pLineBits++;
        uint32_t color3 = *pLineBits++;
        uint32_t color4 = *pLineBits++;
        *p++ = AVERAGERGB13(color2, color1);
        *p++ = AVERAGERGB(color2, color3);
        *p++ = AVERAGERGB13(color3, color4);
    }

    if (!even)  // odd line
    {
        uint32_t* pBits1 = pBits;
        uint32_t* pBits12 = pBits1 - 624;
        uint32_t* pBits2 = pBits12 - 624;
        for (int x = 0; x < 624; x++)
        {
            uint32_t color1 = *pBits1++;
            uint32_t color2 = *pBits2++;
            uint32_t color12 = AVERAGERGB(color1, color2);
            *pBits12++ = color12;
        }
    }
}

void CALLBACK PrepareScreenLine832x600(uint32_t* pImageBits, const uint32_t* pLineBits, int line)
{
    uint32_t* pBits = pImageBits + line * 832 * 2;
    memcpy(pBits, pLineBits, sizeof(uint32_t) * 832);
    pBits += 832;
    memcpy(pBits, pLineBits, sizeof(uint32_t) * 832);
}

void CALLBACK PrepareScreenLine1040x750(uint32_t* pImageBits, const uint32_t* pLineBits, int line)
{
    bool even = (line & 1) == 0;
    uint32_t* pBits = pImageBits + (line / 2 * 5) * 1040;
    if (!even)
        pBits += 1040 * 3;

    uint32_t* p = pBits;
    for (int x = 0; x < 832; x += 4)  // x1.25 - mapping every 4 pixels into 5 pixels
    {
        uint32_t color1 = *pLineBits++;
        uint32_t color2 = *pLineBits++;
        uint32_t color3 = *pLineBits++;
        uint32_t color4 = *pLineBits++;
        *p++ = color1;
        *p++ = AVERAGERGB13(color1, color2);
        *p++ = AVERAGERGB(color2, color3);
        *p++ = AVERAGERGB13(color4, color3);
        *p++ = color4;
    }

    memcpy(pBits + 1040, pBits, sizeof(uint32_t) * 1040);

    if (!even)  // odd line
    {
        uint32_t* pBits1 = pBits;
        uint32_t* pBits12 = pBits1 - 1040;
        uint32_t* pBits2 = pBits12 - 1040;
        for (int x = 0; x < 1040; x++)
        {
            uint32_t color1 = *pBits1++;
            uint32_t color2 = *pBits2++;
            uint32_t color12 = AVERAGERGB(color1, color2);
            *pBits12++ = color12;
        }
    }
}

void CALLBACK PrepareScreenLine1248x900(uint32_t* pImageBits, const uint32_t* pLineBits, int line)
{
    uint32_t* pBits = pImageBits + line * 1248 * 3;
    uint32_t* p = pBits;
    for (int x = 0; x < 832 / 2; x++)
    {
        uint32_t color1 = *pLineBits++;
        uint32_t color2 = *pLineBits++;
        uint32_t color12 = AVERAGERGB(color1, color2);
        *p++ = color1;
        *p++ = color12;
        *p++ = color2;
    }

    uint32_t* pBits2 = pBits + 1248;
    memcpy(pBits2, pBits, sizeof(uint32_t) * 1248);
    uint32_t* pBits3 = pBits2 + 1248;
    memcpy(pBits3, pBits, sizeof(uint32_t) * 1248);
}


//////////////////////////////////////////////////////////////////////
//
// Emulator image format - see CMotherboard::SaveToImage()
// Image header format (32 bytes):
//   4 bytes        NEON_IMAGE_HEADER1
//   4 bytes        NEON_IMAGE_HEADER2
//   4 bytes        NEON_IMAGE_VERSION
//   4 bytes        state size = 20K + 512/1024/2048/4096 KB
//   4 bytes        NEON uptime
//   4 bytes        state body compressed size
//   8 bytes        RESERVED

bool Emulator_SaveImage(const QString& sFilePath)
{
    // Create file
    QFile file(sFilePath);
    if (! file.open(QIODevice::Truncate | QIODevice::WriteOnly))
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to save image file."));
        return false;
    }

//    // Allocate memory
//    quint8* pImage = (quint8*) ::calloc(NEONIMAGE_SIZE, 1);
//    if (pImage == nullptr)
//    {
//        file.close();
//        return false;
//    }
//    // Prepare header
//    quint32* pHeader = (quint32*) pImage;
//    *pHeader++ = NEONIMAGE_HEADER1;
//    *pHeader++ = NEONIMAGE_HEADER2;
//    *pHeader++ = NEONIMAGE_VERSION;
//    *pHeader++ = NEONIMAGE_SIZE;
//    // Store emulator state to the image
//    g_pBoard->SaveToImage(pImage);
//    *(quint32*)(pImage + 16) = m_dwEmulatorUptime;

//    // Save image to the file
//    qint64 bytesWritten = file.write((const char *)pImage, NEONIMAGE_SIZE);
//    if (bytesWritten != NEONIMAGE_SIZE)
//    {
//        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to save image file data."));
//        return false;
//    }

//    // Free memory, close file
//    ::free(pImage);
    file.close();

    return true;
}

bool Emulator_LoadImage(const QString &sFilePath)
{
    Emulator_Stop();

    // Open file
    QFile file(sFilePath);
    if (! file.open(QIODevice::ReadOnly))
    {
        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to load image file."));
        return false;
    }

//    // Read header
//    quint32 bufHeader[NEONIMAGE_HEADER_SIZE / sizeof(quint32)];
//    qint64 bytesRead = file.read((char*)bufHeader, NEONIMAGE_HEADER_SIZE);
//    if (bytesRead != NEONIMAGE_HEADER_SIZE)
//    {
//        file.close();
//        return false;
//    }

//    //TODO: Check version and size

//    // Allocate memory
//    quint8* pImage = (quint8*) ::malloc(NEONIMAGE_SIZE);
//    if (pImage == nullptr)
//    {
//        file.close();
//        return false;
//    }

//    // Read image
//    file.seek(0);
//    bytesRead = file.read((char*)pImage, NEONIMAGE_SIZE);
//    if (bytesRead != NEONIMAGE_SIZE)
//    {
//        ::free(pImage);
//        file.close();
//        AlertWarning(QT_TRANSLATE_NOOP("Emulator", "Failed to load image file data."));
//        return false;
//    }
//    else
//    {
//        // Restore emulator state from the image
//        g_pBoard->Reset();
//        g_pBoard->LoadFromImage(pImage);

//        m_dwEmulatorUptime = *(quint32*)(pImage + 16);
//    }

//    // Free memory, close file
//    ::free(pImage);
    file.close();

    return true;
}


//////////////////////////////////////////////////////////////////////
