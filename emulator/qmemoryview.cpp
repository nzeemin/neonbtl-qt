#include "stdafx.h"
#include <QtGui>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOptionFocusRect>
#include <QTabBar>
#include <QToolBar>
#include "main.h"
#include "qmemoryview.h"
#include "Emulator.h"
#include "emubase/Emubase.h"
#include "qdialogs.h"


enum MemoryViewMode
{
    MEMMODE_CPU = 0,   // CPU memory
    MEMMODE_HALT = 1,  // HALT mode memory
    MEMMODE_USER = 2,  // USER mode memory
    MEMMODE_PS = 3,    // Process List
    MEMMODE_LAST = 3,  // Last mode number
};

static const char * MemoryView_ModeNames[] =
{
    "CPU", "HALT", "USER", "Process List"
};

enum MemoryViewNumeralMode
{
    MEMMODENUM_OCT = 0,
    MEMMODENUM_HEX = 1,
};


QMemoryView::QMemoryView()
{
    m_Mode = Settings_GetDebugMemoryMode();
    if (m_Mode > MEMMODE_LAST) m_Mode = MEMMODE_LAST;
    m_ByteMode = Settings_GetDebugMemoryByte();
    m_NumeralMode = Settings_GetDebugMemoryNumeral();
    m_wBaseAddress = Settings_GetDebugMemoryAddress();
    m_cyLineMemory = 0;
    m_nPageSize = 0;

    QFont font = Common_GetMonospacedFont();
    QFontMetrics fontmetrics(font);
    //int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    m_cyLine = cyLine;

    this->setFont(font);

    m_scrollbar = new QScrollBar(Qt::Vertical, this);
    m_scrollbar->setRange(0, 65536 - 16);
    m_scrollbar->setSingleStep(16);
    QObject::connect(m_scrollbar, SIGNAL(valueChanged(int)), this, SLOT(scrollValueChanged()));

    m_tabbar = new QTabBar(this);
    m_tabbar->setShape(QTabBar::RoundedNorth);
    m_tabbar->setExpanding(false);
    m_tabbar->setDrawBase(true);
    m_tabbar->setFocusPolicy(Qt::NoFocus);  // The view keeps the keyboard focus
    for (int mode = 0; mode <= MEMMODE_LAST; mode++)
        m_tabbar->addTab(MemoryView_ModeNames[mode]);
    m_tabbar->setCurrentIndex(m_Mode);
    QObject::connect(m_tabbar, SIGNAL(currentChanged(int)), this, SLOT(setMode(int)));

    m_toolbar = new QToolBar(this);
    m_toolbar->setGeometry(4, 4, 36, 2000);
    m_toolbar->setOrientation(Qt::Vertical);
    m_toolbar->setIconSize(QSize(24, 24));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_toolbar->setFocusPolicy(Qt::NoFocus);
    m_toolbar->setStyle(QStyleFactory::create("windows"));  // fix for macOS to remove gradient background

    QAction* actionGotoAddr = m_toolbar->addAction(QIcon(":/images/iconEditAddress.svg"), "");
    m_toolbar->addSeparator();
    QAction* actionWordByte = m_toolbar->addAction(QIcon(":/images/iconWordByte.svg"), "");
    QAction* actionOctalHex = m_toolbar->addAction(QIcon(":/images/iconHex.svg"), "");
    m_memoryActions << actionGotoAddr << actionWordByte << actionOctalHex;

    QObject::connect(actionGotoAddr, SIGNAL(triggered()), this, SLOT(gotoAddress()));
    QObject::connect(actionWordByte, SIGNAL(triggered()), this, SLOT(changeWordByteMode()));
    QObject::connect(actionOctalHex, SIGNAL(triggered()), this, SLOT(changeNumeralMode()));

    setFocusPolicy(Qt::ClickFocus);

    updateLayout();
}

QMemoryView::~QMemoryView()
{
    delete m_scrollbar;
}

void QMemoryView::updateScrollPos()
{
    m_scrollbar->setValue(m_wBaseAddress);
}

static const char * GetMemoryModeName(int mode)
{
    if (mode < 0 || mode > MEMMODE_LAST)
        return "UKWN";  // Unknown mode
    return MemoryView_ModeNames[mode];
}

void QMemoryView::updateWindowText()
{
    QString buffer = tr("Memory - %1").arg(GetMemoryModeName(m_Mode));
    parentWidget()->setWindowTitle(buffer);
}

void QMemoryView::updateLayout()
{
    QFontMetrics fontmetrics(Common_GetMonospacedFont());
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();

    m_yTop = m_tabbar->sizeHint().height();
    m_toolbar->setGeometry(4, m_yTop + 4, 36, 2000);

    m_PostionIncrement = (m_NumeralMode == MEMMODENUM_OCT) ? cxChar * 7 : cxChar * 5;
    if (m_ByteMode)
        m_PostionIncrement += cxChar;

    // Page size in lines: below the tab bar and the address line
    m_nPageSize = (this->height() - m_yTop) / cyLine - 1;
    if (m_nPageSize < 1) m_nPageSize = 1;

    // The scrollbar goes right after the characters column, and has the height of the scrolling part
    int cxScroll = this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    int xFirst = 38 + 9 * cxChar;
    int xScroll = xFirst + 8 * m_PostionIncrement + cxChar + 16 * cxChar + cxChar;
    int yScroll = m_yTop + cyLine + fontmetrics.descent();
    m_scrollbar->setGeometry(xScroll, yScroll, cxScroll, cyLine * m_nPageSize);
    m_scrollbar->setPageStep(m_nPageSize * 16);

    // The tab bar spans from the address column to the right edge of the scrollbar
    m_tabbar->setGeometry(42, 0, xScroll + cxScroll - 42, m_yTop);

    // The process list header is 61 characters wide
    int cxMin = qMax(xScroll + cxScroll, 38 + cxChar * 63);
    this->setMinimumSize(cxMin, m_yTop + cyLine * 9 + cyLine / 2);

    // Scrollbar, toolbar and the address/format actions make sense for the memory modes only
    bool okMemory = m_Mode != MEMMODE_PS;
    m_scrollbar->setVisible(okMemory);
    m_toolbar->setVisible(okMemory);
for (QAction * action : m_memoryActions)
        action->setEnabled(okMemory);
}

void QMemoryView::updateData()
{
    if (m_Mode == MEMMODE_PS)
        update();
}

void QMemoryView::focusInEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}
void QMemoryView::focusOutEvent(QFocusEvent *)
{
    repaint();  // Need to draw focus rect
}

void QMemoryView::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_Mode == MEMMODE_PS) return;

    QMenu menu(this);
    menu.addAction(QIcon(":/images/iconEditAddress.svg"), tr("Go to Address..."), this, SLOT(gotoAddress()));
    menu.addSeparator();
    menu.addAction(QIcon(":/images/iconWordByte.svg"), tr("Words / Bytes"), this, SLOT(changeWordByteMode()));
    menu.addAction(QIcon(":/images/iconHex.svg"), tr("Octal / Hex"), this, SLOT(changeNumeralMode()));

    menu.exec(event->globalPos());
}

void QMemoryView::setMode(int mode)
{
    if (mode < 0 || mode > MEMMODE_LAST) return;

    m_Mode = mode;
    Settings_SetDebugMemoryMode(m_Mode);

    if (m_tabbar->currentIndex() != mode)  // Mode changed not by the tab bar
    {
        m_tabbar->blockSignals(true);
        m_tabbar->setCurrentIndex(mode);
        m_tabbar->blockSignals(false);
    }

    updateLayout();
    repaint();
    updateWindowText();
}

void QMemoryView::changeWordByteMode()
{
    if (m_Mode == MEMMODE_PS) return;

    m_ByteMode = !m_ByteMode;
    Settings_SetDebugMemoryByte(m_ByteMode);

    updateLayout();  // The column width changes, so does the scrollbar position
    repaint();
}

void QMemoryView::changeNumeralMode()
{
    if (m_Mode == MEMMODE_PS) return;

    int newMode = m_NumeralMode ^ 1;
    m_NumeralMode = newMode;
    Settings_SetDebugMemoryNumeral(newMode);

    updateLayout();  // The column width changes, so does the scrollbar position
    repaint();
}

void QMemoryView::scrollBy(qint16 delta)
{
    if (delta == 0 || m_Mode == MEMMODE_PS) return;

    m_wBaseAddress = (quint16)(m_wBaseAddress + delta);
    m_wBaseAddress = m_wBaseAddress & ((quint16)~15);
    Settings_SetDebugMemoryAddress(m_wBaseAddress);

    repaint();
    updateScrollPos();
}

void QMemoryView::gotoAddress()
{
    if (m_Mode == MEMMODE_PS) return;

    quint16 value = m_wBaseAddress;
    QInputOctalDialog dialog(this, tr("Go To Address"), &value);
    if (dialog.exec() == QDialog::Rejected) return;

    // Scroll to the address
    m_wBaseAddress = value & ((quint16)~15);
    Settings_SetDebugMemoryAddress(m_wBaseAddress);

    repaint();
    updateScrollPos();
}

void QMemoryView::resizeEvent(QResizeEvent *)
{
    updateLayout();
}

void QMemoryView::scrollValueChanged()
{
    int value = m_scrollbar->value();
    m_wBaseAddress = (unsigned short)value & ((quint16)~15);
    Settings_SetDebugMemoryAddress(m_wBaseAddress);

    this->repaint();
}

quint16 QMemoryView::getWordFromMemory(quint16 address, bool& okValid, int& addrtype, quint16& wChanged)
{
    bool okHalt;
    switch (m_Mode)
    {
    default:
    case MEMMODE_CPU:
        okHalt = g_pBoard->GetCPU()->IsHaltMode();
        break;
    case MEMMODE_HALT:
        okHalt = true;
        break;
    case MEMMODE_USER:
        okHalt = false;
        break;
    }

    quint32 offset;
    addrtype = g_pBoard->TranslateAddress(address, okHalt, false, &offset);
    okValid = (addrtype != ADDRTYPE_IO) && (addrtype != ADDRTYPE_DENY);
    if (!okValid)
        return 0;

    if (addrtype == ADDRTYPE_ROM)
    {
        wChanged = 0;
        return g_pBoard->GetROMWord((uint16_t)offset);
    }
    else
    {
        wChanged = Emulator_GetChangeRamStatus(offset);
        return g_pBoard->GetRAMWordView(offset);
    }
}

// Reads a word as seen in HALT mode; 0 if the address is not in RAM
quint16 QMemoryView::getWordFromHalt(quint16 address)
{
    quint32 offset;
    int addrtype = g_pBoard->TranslateAddress(address, true, false, &offset);
    if (addrtype < ADDRTYPE_RAM4)
        return g_pBoard->GetRAMWordView(offset);
    return 0;
}

void QMemoryView::paintProcessList(QPainter& painter)
{
    const int PROLENW = 35;  // process descriptor size, words
    const int MAXPROCESSES = 64;  // guard against a broken list in uninitialized RAM
    const char* PROCESS_LIST_HEADER  = "#   Name            Descript Priority State  Mem   Low   High";
    const char* PROCESS_LIST_DIVIDER = "--- ----------------  ------  ------  -----  ----  ----  ----";

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    painter.setPen(palette().color(QPalette::Text));

    quint16 pdptr = g_pBoard->GetRAMWordView(020572);   // pointer to first process-descriptor
    quint16 freepr = g_pBoard->GetRAMWordView(020006);  // pointer to first free process-descriptor
    quint16 running = g_pBoard->GetRAMWordView(020016); // pointer to current process
    int maplen = qMin((int)g_pBoard->GetRAMWordView(020002), 256);  // length of ram-bit-map in bytes

    int x = 38 + cxChar;
    int y = m_yTop + cyLine;
    painter.drawText(x, y, PROCESS_LIST_HEADER);
    y += cyLine;
    painter.drawText(x, y, PROCESS_LIST_DIVIDER);
    y += cyLine;

    char buffer[7];
    int procno = 1;
    while (pdptr != 0 && pdptr != freepr && procno <= MAXPROCESSES)
    {
        quint16 procinfo[PROLENW];
        for (int i = 0; i < PROLENW; i++)
            procinfo[i] = getWordFromHalt((quint16)(pdptr + i * 2));

        ushort wname[16];
        for (int i = 0; i < 16; i++)
        {
            quint8 ch = ((const quint8*)(procinfo + 24))[i];
            wname[i] = TranslateDeviceCharToUnicode(ch);
        }
        QString name = QString::fromUtf16(wname, 16);

        quint16 priority = procinfo[22];
        QString prio;
        if ((priority & 0100000) == 0)
        {
            PrintOctalValue(buffer, priority);
            prio = QString::fromLatin1(buffer);
        }
        else
        {
            PrintOctalValue(buffer, (quint16)(0200000 - priority));
            buffer[0] = '-';
            prio = QString::fromLatin1(buffer);
        }

        const char* statestr;
        if (pdptr == running)
            statestr = "Run ";
        else if ((procinfo[19] & 0040000) != 0)
            statestr = "Wait";
        else if ((procinfo[19] & 0000400) != 0)
            statestr = "Tim ";
        else if ((procinfo[19] & 0000377) != 0)
            statestr = "Int ";
        else
            statestr = "I/O ";

        // Memory map: one bit per 4K block, a zero bit means the block is used;
        // the first 4 words are the low memory, the rest is the high memory
        QString memstr = QStringLiteral("No memory map");
        quint16 pmem = procinfo[23];
        if (pmem != 0)
        {
            int memlow = 0, memhigh = 0;
            for (int i = 0; i < (maplen + 1) / 2; i++)
            {
                quint16 membits = getWordFromHalt((quint16)(pmem + i * 2));
                for (int b = 0; b < 16; b++)
                {
                    if ((membits & (1 << b)) == 0)
                    {
                        if (i < 4) memlow += 4; else memhigh += 4;
                    }
                }
            }
            memstr = QString("%1k  %2k  %3k").arg(memlow + memhigh, 3).arg(memlow, 3).arg(memhigh, 3);
        }

        PrintOctalValue(buffer, pdptr);
        QString line = QString("%1  %2  %3  %4  %5   %6")
                .arg(procno, 2).arg(name).arg(QString::fromLatin1(buffer)).arg(prio, 6)
                .arg(QLatin1String(statestr)).arg(memstr);
        painter.drawText(x, y, line);

        pdptr = procinfo[34];  // p.dsucc
        procno++;
        y += cyLine;
        if (y > this->height() + cyLine) break;
    }
}

void QMemoryView::paintEvent(QPaintEvent * /*event*/)
{
    if (g_pBoard == nullptr) return;

    QColor colorBackground = palette().color(QPalette::Base);

    QPainter painter(this);
    painter.fillRect(36 + 4, 0, this->width(), this->height(), colorBackground);

    if (m_Mode == MEMMODE_PS)
    {
        paintProcessList(painter);
        return;
    }

    QFont font = Common_GetMonospacedFont();
    painter.setFont(font);
    QFontMetrics fontmetrics(font);
    int cxChar = fontmetrics.averageCharWidth();
    int cyLine = fontmetrics.height();
    QColor colorText = palette().color(QPalette::Text);
    QColor colorChanged = Common_GetColorShifted(palette(), COLOR_VALUECHANGED);
    QColor colorMemoryRom = Common_GetColorShifted(palette(), COLOR_MEMORYROM);
    QColor colorMemoryIO = Common_GetColorShifted(palette(), COLOR_MEMORYIO);
    QColor colorMemoryNA = Common_GetColorShifted(palette(), COLOR_MEMORYNA);

    m_cyLineMemory = cyLine;

    char buffer[7];
    // Header line: each column label is drawn at the same x as the values below it, not as one
    // string, so the columns stay aligned with any font metrics
    painter.setPen(colorText);
    int xFirst = 38 + 9 * cxChar;  // x of the first value
    painter.drawText(38 + 3 * cxChar, m_yTop + cyLine, "addr");
    int numBase = (m_NumeralMode == MEMMODENUM_OCT) ? 8 : 16;
    if (!m_ByteMode)
    {
        for (int j = 0; j < 8; j++)  // One label per word, the offset of the word
            painter.drawText(xFirst + j * m_PostionIncrement, m_yTop + cyLine, QString::number(j * 2, numBase));
    }
    else
    {
        int cxSecondByte = ((m_NumeralMode == MEMMODENUM_OCT) ? 4 : 3) * cxChar;  // The high byte is drawn here from the low one
        for (int k = 0; k < 16; k++)  // One label per byte, the offset of the byte
            painter.drawText(xFirst + (k / 2) * m_PostionIncrement + (k & 1) * cxSecondByte, m_yTop + cyLine, QString::number(k, numBase));
    }

    quint16 address = m_wBaseAddress;
    int y = m_yTop + 2 * cyLine;
    for (;;)    // Draw lines
    {
        if (m_NumeralMode == MEMMODENUM_OCT)
            DrawOctalValue(painter, 38 + 1 * cxChar, y, address);
        else
            DrawHexValue(painter, 38 + 3 * cxChar, y, address);

        int x = xFirst;
        ushort wchars[16];
        for (int j = 0; j < 8; j++)    // Draw words as octal value
        {
            bool okValid = false;
            int addrtype;
            quint16 wChanged = 0;
            quint16 word = getWordFromMemory(address, okValid, addrtype, wChanged);

            if (okValid)
            {
                if (addrtype == ADDRTYPE_ROM)
                    painter.setPen(colorMemoryRom);
                else
                    painter.setPen(wChanged != 0 ? colorChanged : colorText);

                if (m_NumeralMode == MEMMODENUM_OCT && !m_ByteMode)
                    DrawOctalValue(painter, x, y, word);
                else if (m_NumeralMode == MEMMODENUM_OCT && m_ByteMode)
                {
                    PrintOctalValue(buffer, (word & 0xff));
                    painter.drawText(x, y, buffer + 3);
                    PrintOctalValue(buffer, (word >> 8));
                    painter.drawText(x + 4 * cxChar, y, buffer + 3);
                }
                else if (m_NumeralMode == MEMMODENUM_HEX && !m_ByteMode)
                    DrawHexValue(painter, x, y, word);
                else if (m_NumeralMode == MEMMODENUM_HEX && m_ByteMode)
                {
                    PrintHexValue(buffer, word);
                    painter.drawText(x, y, buffer + 2);
                    buffer[2] = 0;
                    painter.drawText(x + 3 * cxChar, y, buffer);
                }
            }
            else  // No value
            {
                if (addrtype == ADDRTYPE_IO)
                {
                    painter.setPen(colorMemoryIO);
                    painter.drawText(x + (m_PostionIncrement - cxChar) / 2 - cxChar, y, "IO");
                }
                else
                {
                    painter.setPen(colorMemoryNA);
                    painter.drawText(x + (m_PostionIncrement - cxChar) / 2 - cxChar, y, "NA");
                }
            }

            // Prepare characters to draw at right
            quint8 ch1 = (quint8)(word & 0xff);
            ushort wch1 = Translate_KOI8R(ch1);
            if (ch1 < 32) wch1 = 0x00b7;
            wchars[j * 2] = wch1;
            quint8 ch2 = (quint8)((word >> 8) & 0xff);
            ushort wch2 = Translate_KOI8R(ch2);
            if (ch2 < 32) wch2 = 0x00b7;
            wchars[j * 2 + 1] = wch2;

            address += 2;
            x += m_PostionIncrement;
        }
        painter.setPen(colorText);

        // Draw characters at right
        int xch = x + cxChar;
        QString wstr = QString::fromUtf16(wchars, 16);
        painter.drawText(xch, y, wstr);

        y += cyLine;
        if (y > this->height()) break;
    }  // Draw lines

    // Draw focus rect
    if (hasFocus())
    {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.state |= QStyle::State_KeyboardFocusChange;
        option.backgroundColor = QColor(Qt::gray);
        option.rect = QRect(38, m_yTop + cyLine + fontmetrics.descent(), 28 * cxChar + 8 * m_PostionIncrement, cyLine * m_nPageSize);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
    }
}

void QMemoryView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Space:
        if (event->isAutoRepeat()) return;
        event->accept();
        setMode(m_Mode == MEMMODE_LAST ? 0 : m_Mode + 1);
        break;

    case Qt::Key_G:
        event->accept();
        gotoAddress();
        break;
    case Qt::Key_B:
    case Qt::Key_W:
        event->accept();
        changeWordByteMode();
        break;
    case Qt::Key_H:
    case Qt::Key_O:
        event->accept();
        changeNumeralMode();
        break;

    case Qt::Key_Up:
        event->accept();
        scrollBy(-16);
        break;
    case Qt::Key_Down:
        event->accept();
        scrollBy(16);
        break;

    case Qt::Key_PageUp:
        event->accept();
        scrollBy(-m_nPageSize * 16);
        break;
    case Qt::Key_PageDown:
        event->accept();
        scrollBy(m_nPageSize * 16);
        break;
    }
}

void QMemoryView::wheelEvent(QWheelEvent * event)
{
    QPoint angleDelta = event->angleDelta();
    if (qAbs(angleDelta.x()) > qAbs(angleDelta.y()))
        return;
    event->accept();

    int steps = -angleDelta.y() / 60;
    scrollBy(steps * 16);
}


//////////////////////////////////////////////////////////////////////
