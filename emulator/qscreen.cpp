#include "stdafx.h"
#include <QtGui>
#include <QMenu>
#include "qscreen.h"
#include "mainwindow.h"
#include "Emulator.h"
#include "emubase/Emubase.h"


//////////////////////////////////////////////////////////////////////


QEmulatorScreen::QEmulatorScreen(QWidget *parent) :
    QWidget(parent), m_image(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);

    m_image = nullptr;
    m_mode = 1;

    createDisplay();
}

QEmulatorScreen::~QEmulatorScreen()
{
    delete m_image;
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
    menu.addAction(tr("Screenshot"), Global_getMainWindow(), SLOT(saveScreenshot()));
    menu.addAction(tr("Screenshot to Clipboard"), Global_getMainWindow(), SLOT(screenshotToClipboard()));
    menu.exec(event->globalPos());
}

void QEmulatorScreen::keyPressEvent(QKeyEvent *event)
{
    if (! g_okEmulatorRunning) return;
    if (event->isAutoRepeat()) return;

    unsigned char neonscan = TranslateQtKeyToUkncKey(event->key());
    if (neonscan == 0) return;

    //Emulator_KeyEvent(neonscan, true);
    event->accept();
}

void QEmulatorScreen::keyReleaseEvent(QKeyEvent *event)
{
    unsigned char neonscan = TranslateQtKeyToUkncKey(event->key());
    if (neonscan == 0) return;

    //Emulator_KeyEvent(neonscan, false);
    event->accept();
}

const unsigned char arrQtkey2UkncscanLat[256] =    // ЛАТ
{
    /*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
    /*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*2*/    0113, 0004, 0151, 0172, 0000, 0116, 0154, 0133, 0134, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*3*/    0176, 0030, 0031, 0032, 0013, 0034, 0035, 0016, 0017, 0177, 0000, 0000, 0000, 0000, 0000, 0000,
    /*4*/    0000, 0072, 0076, 0050, 0057, 0033, 0047, 0055, 0156, 0073, 0027, 0052, 0056, 0112, 0054, 0075,
    /*5*/    0053, 0067, 0074, 0111, 0114, 0051, 0137, 0071, 0115, 0070, 0157, 0000, 0000, 0000, 0000, 0000,
    /*6*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*7*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*c*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
};
const unsigned char arrQtkey2UkncscanRus[256] =    // РУС
{
    /*       0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f  */
    /*0*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*1*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*2*/    0113, 0004, 0151, 0172, 0000, 0116, 0154, 0133, 0134, 0000, 0000, 0000, 0000, 0171, 0152, 0000,
    /*3*/    0176, 0030, 0031, 0032, 0013, 0034, 0035, 0016, 0017, 0177, 0000, 0000, 0000, 0000, 0000, 0000,
    /*4*/    0000, 0047, 0073, 0111, 0071, 0051, 0072, 0053, 0074, 0036, 0075, 0056, 0057, 0115, 0114, 0037,
    /*5*/    0157, 0027, 0052, 0070, 0033, 0055, 0112, 0050, 0110, 0054, 0067, 0000, 0000, 0000, 0000, 0000,
    /*6*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*7*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*8*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*9*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*a*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*b*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*c*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*d*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*e*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
    /*f*/    0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000,
};


unsigned char QEmulatorScreen::TranslateQtKeyToUkncKey(int qtkey)
{
    switch (qtkey)
    {
    case Qt::Key_Down:      return 0134;
    case Qt::Key_Up:        return 0154;
    case Qt::Key_Left:      return 0116;
    case Qt::Key_Right:     return 0133;
    case Qt::Key_Enter:     return 0166;
    case Qt::Key_Return:    return 0166;
    case Qt::Key_Tab:       return 0026;
    case Qt::Key_Shift:     return 0105;  // HP key
    case Qt::Key_Space:     return 0113;
    case Qt::Key_Backspace: return 0132;  // ZB (BACKSPACE) key
    case Qt::Key_Control:   return 0046;  // SU (UPR) key
    case Qt::Key_F1:        return 0010;  // K1 / K6
    case Qt::Key_F2:        return 0011;  // K1 / K6
    case Qt::Key_F3:        return 0012;  // K1 / K6
    case Qt::Key_F4:        return 0014;  // K1 / K6
    case Qt::Key_F5:        return 0015;  // K1 / K6
    case Qt::Key_F7:        return 0152;  // UST key
    case Qt::Key_F8:        return 0151;  // ISP key
    }

//    if (qtkey >= 32 && qtkey <= 255)
//    {
//        unsigned short ukncRegister = g_pBoard->GetKeyboardRegister();
//        const unsigned char * pTable = ((ukncRegister & KEYB_LAT) != 0) ? arrQtkey2UkncscanLat : arrQtkey2UkncscanRus;
//        return pTable[qtkey];
//    }

    return 0;
}
