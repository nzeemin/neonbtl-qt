#include "stdafx.h"
#include <QtGui>
#include <QMenu>
#include "qscreen.h"
#include "mainwindow.h"
#include "Emulator.h"


//////////////////////////////////////////////////////////////////////


QEmulatorScreen::QEmulatorScreen(QWidget *parent) :
    QWidget(parent), m_image(nullptr), m_keysPressed()
{
    setFocusPolicy(Qt::StrongFocus);

    m_mode = 1;

    createDisplay();
}

QEmulatorScreen::~QEmulatorScreen()
{
    delete m_image;
    m_image = nullptr;
}

void QEmulatorScreen::setMode(int mode)
{
    bool needRecreate = (m_mode != mode);
    m_mode = mode;
    if (needRecreate)
        createDisplay();
}

QImage QEmulatorScreen::getScreenshot()
{
    QImage image(*m_image);
    return image;
}

void QEmulatorScreen::createDisplay()
{
    if (m_image != nullptr)
    {
        delete m_image;
        m_image = nullptr;
    }

    int cxScreenWidth, cyScreenHeight;
    Emulator_GetScreenSize(m_mode, &cxScreenWidth, &cyScreenHeight);

    m_image = new QImage(cxScreenWidth, cyScreenHeight, QImage::Format_RGB32);

    setMinimumSize(cxScreenWidth, cyScreenHeight);
    setMaximumSize(cxScreenWidth + 60, cyScreenHeight + 40);
}

void QEmulatorScreen::paintEvent(QPaintEvent * /*event*/)
{
    Emulator_PrepareScreenRGB32(m_image->bits(), m_mode);

    QPainter painter(this);
    painter.drawImage(0, 0, *m_image);
}

void QEmulatorScreen::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(QIcon(":/images/iconScreenshot.svg"), tr("Screenshot"), Global_getMainWindow(), SLOT(saveScreenshot()));
    menu.addAction(tr("Screenshot to Clipboard"), Global_getMainWindow(), SLOT(screenshotToClipboard()));
    menu.exec(event->globalPos());
}

void QEmulatorScreen::keyPressEvent(QKeyEvent *event)
{
    if (! g_okEmulatorRunning) return;
    if (event->isAutoRepeat()) return;

    quint16 keyscan = TranslateQtKeyToNeonKey(event->key());
    //DebugPrintFormat("Key %02x scan 0x%03x", event->key(), keyscan);
    if (keyscan == 0) return;

    event->accept();

    if (!m_keysPressed.contains(keyscan))
        m_keysPressed.append(keyscan);
}

void QEmulatorScreen::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;

    quint16 keyscan = TranslateQtKeyToNeonKey(event->key());
    if (keyscan == 0) return;

    event->accept();

    if (m_keysPressed.contains(keyscan))
        m_keysPressed.removeAll(keyscan);
}

const quint16 NOKEY = 0;

const quint16 arrQtkey2NeonscanLat[128 - 32] =
{
    /*       0      1      2      3      4      5      6      7      8      9      a      b      c      d      e      f  */
    /*2*/    0x408, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY,
    /*3*/    0x720, 0x102, 0x104, 0x103, 0x008, 0x110, 0x105, 0x020, 0x006, 0x706, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY,
    /*4*/    NOKEY, 0x303, 0x320, 0x202, 0x206, 0x108, 0x201, 0x205, 0x620, 0x308, 0x101, 0x203, 0x220, 0x403, 0x210, 0x305,
    /*5*/    0x208, 0x301, 0x310, 0x404, 0x410, 0x204, 0x506, 0x304, 0x405, 0x302, 0x606, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY,
    /*6*/    NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY,
    /*7*/    NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY, NOKEY,
};

void QEmulatorScreen::processKeyboard(quint16 vkeyscan)
{
    quint8 matrix[8];
    ::memset(matrix, 0, sizeof(matrix));
    foreach(quint16 keyscan, m_keysPressed)
    {
        matrix[(keyscan >> 8) & 7] |= (keyscan & 0xff);
    }

    if (vkeyscan != 0)  // Currently pressed key on KeyboardView
    {
        matrix[(vkeyscan >> 8) & 7] |= (vkeyscan & 0xff);
    }

    Emulator_UpdateKeyboardMatrix(matrix);
}

quint16 QEmulatorScreen::TranslateQtKeyToNeonKey(int qtkey)
{
    switch (qtkey)
    {
    case Qt::Key_Down:      return 0x510;
    case Qt::Key_Up:        return 0x610;
    case Qt::Key_Left:      return 0x420;
    case Qt::Key_Right:     return 0x508;
    case Qt::Key_Enter:     return 0x780;  // NUM Enter
    case Qt::Key_Return:    return 0x608;  // Enter
    case Qt::Key_Tab:       return 0x180;
    case Qt::Key_Shift:     return 0x404;  // HP key
    case Qt::Key_Space:     return 0x408;
    case Qt::Key_Backspace: return 0x503;  // ZB (BACKSPACE) key
    case Qt::Key_Control:   return 0x280;  // SU (UPR) key
    case Qt::Key_F1:        return 0x002;  // K1
    case Qt::Key_F2:        return 0x004;  // K2
    case Qt::Key_F3:        return 0x003;  // K3
    case Qt::Key_F4:        return 0x010;  // K4
    case Qt::Key_F5:        return 0x005;  // K5
    case Qt::Key_F6:        return 0x703;  // POM key
    case Qt::Key_F7:        return 0x603;  // UST key
    case Qt::Key_F8:        return 0x604;  // ISP key
    case Qt::Key_F11:       return 0x704;  // SBROS (RESET)
    case Qt::Key_F12:       return 0x240;  // STOP
    }

    if (qtkey >= 32 && qtkey < 128)
    {
        return arrQtkey2NeonscanLat[qtkey - 32];
    }

    return 0;
}
