#include "stdafx.h"
#include <QtGui>
#include "qkeyboardview.h"


// Keyboard key mapping to bitmap
const quint16 m_arrKeyboardKeys[] =
{
    /*   x1,y1    w,h    NEONscan  */
    18,  15,  42, 27,    0x002,     // K1
    62,  15,  42, 27,    0x004,     // K2
    106, 15,  42, 27,    0x003,     // K3
    151, 15,  42, 27,    0x010,     // K4
    195, 15,  42, 27,    0x005,     // K5
    343, 15,  42, 27,    0x703,     // POM
    387, 15,  42, 27,    0x603,     // UST
    431, 15,  42, 27,    0x604,     // ISP
    506, 15,  42, 27,    0x704,     // SBROS (RESET)
    551, 15,  42, 27,    0x240,     // STOP

    18,  56,  28, 27,    0x080,     // AR2
    47,  56,  28, 27,    0x001,     // ; +
    77,  56,  27, 27,    0x102,     // 1
    106, 56,  28, 27,    0x104,     // 2
    136, 56,  27, 27,    0x103,     // 3
    165, 56,  28, 27,    0x008,     // 4
    195, 56,  27, 27,    0x110,     // 5
    224, 56,  28, 27,    0x105,     // 6
    254, 56,  27, 27,    0x020,     // 7
    283, 56,  28, 27,    0x006,     // 8
    313, 56,  27, 27,    0x706,     // 9
    342, 56,  28, 27,    0x720,     // 0
    372, 56,  27, 27,    0x705,     // - =
    401, 56,  28, 27,    0x710,     // / ?
    431, 56,  42, 27,    0x503,     // Backspace

    18,  86,  42, 27,    0x180,     // TAB
    62,  86,  27, 27,    0x101,     // Й J
    91,  86,  28, 27,    0x202,     // Ц C
    121, 86,  27, 27,    0x204,     // У U
    150, 86,  28, 27,    0x203,     // К K
    180, 86,  27, 27,    0x108,     // Е E
    210, 86,  28, 27,    0x210,     // Н N
    239, 86,  27, 27,    0x205,     // Г G
    269, 86,  27, 27,    0x120,     // Ш [
    298, 86,  28, 27,    0x106,     // Щ ]
    328, 86,  27, 27,    0x606,     // З Z
    357, 86,  28, 27,    0x620,     // Х H
    387, 86,  27, 27,    0x605,     // Ъ
    416, 86,  28, 27,    0x708,     // : *

    18,  115, 49, 27,    0x280,     // UPR
    69,  115, 28, 27,    0x201,     // Ф F
    99,  115, 27, 27,    0x302,     // Ы Y
    128, 115, 28, 27,    0x304,     // В W
    158, 115, 27, 27,    0x303,     // А A
    187, 115, 28, 27,    0x208,     // П P
    217, 115, 27, 27,    0x310,     // Р R
    246, 115, 28, 27,    0x305,     // О O
    276, 115, 27, 27,    0x220,     // Л L
    305, 115, 28, 27,    0x206,     // Д D
    335, 115, 27, 27,    0x506,     // Ж V
    364, 115, 28, 27,    0x520,     // Э Backslash
    394, 115, 35, 27,    0x505,     // . >
    431, 115, 15, 27,    0x608,     // ENTER - left part
    446, 86,  27, 56,    0x608,     // ENTER - right part

    18,  145, 34, 27,    0x480,     // ALF
    55,  145, 27, 27,    0x380,     // GRAF
    84,  145, 27, 27,    0x301,     // Я Q
    114, 145, 27, 27,    0x402,     // Ч ^
    143, 145, 27, 27,    0x404,     // С S
    173, 145, 27, 27,    0x403,     // М
    202, 145, 27, 27,    0x308,     // И I
    232, 145, 27, 27,    0x410,     // Т
    261, 145, 27, 27,    0x405,     // Ь X
    291, 145, 27, 27,    0x320,     // Б B
    320, 145, 28, 27,    0x306,     // Ю @
    350, 145, 34, 27,    0x406,     // , <

    18,  174, 56, 27,    0x440,     // Left Shift
    77,  174, 34, 27,    0x401,     // FIKS
    114, 174, 211, 27,   0x408,     // Space bar
    328, 174, 56, 27,    0x440,     // Right Shift

    387, 145, 27, 56,    0x420,     // Left
    416, 145, 28, 27,    0x610,     // Up
    416, 174, 28, 27,    0x510,     // Down
    446, 145, 27, 56,    0x508,     // Right

    506, 56,  28, 27,    0x504,     // + NumPad
    536, 56,  27, 27,    0x140,     // - NumPad
    565, 56,  28, 27,    0x040,     // , NumPad
    506, 86,  28, 27,    0x540,     // 7 NumPad
    536, 86,  27, 27,    0x640,     // 8 NumPad
    565, 86,  28, 27,    0x740,     // 9 NumPad
    506, 115, 28, 27,    0x502,     // 4 NumPad
    536, 115, 27, 27,    0x602,     // 5 NumPad
    565, 115, 28, 27,    0x702,     // 6 NumPad
    506, 145, 28, 27,    0x501,     // 1 NumPad
    536, 145, 27, 27,    0x601,     // 2 NumPad
    565, 145, 28, 27,    0x701,     // 3 NumPad
    506, 174, 28, 27,    0x580,     // 0 NumPad
    536, 174, 27, 27,    0x680,     // . NumPad
    565, 174, 28, 27,    0x780,     // ENTER NumPad

};
const int m_nKeyboardKeysCount = sizeof(m_arrKeyboardKeys) / sizeof(unsigned short) / 5;

bool isDarkPalette(const QPalette& palette)
{
    return palette.windowText().color().lightness() > palette.window().color().lightness();
}

void invertImageColors(QImage& image)
{
    for (int i = 1; i < image.colorCount(); i++)
    {
        QColor color = QColor(image.color(i));
        QColor invColor = QColor::fromHsl(color.hslHue(), color.hslSaturation(), 255 - color.lightness(), color.alpha());
        image.setColor(i, invColor.rgb());
    }
}

QKeyboardView::QKeyboardView(QWidget *parent) :
    QWidget(parent)
{
    m_nKeyPressed = 0;

    setMinimumSize(600, 200);
    setMaximumSize(600 + 60, 210 + 20);
}

void QKeyboardView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QImage imageKeyboard(":/images/keyboard.png");
    if (isDarkPalette(palette()))  // For dark theme, invert keyboard palette
        invertImageColors(imageKeyboard);

    // Center image
    int cxBitmap = imageKeyboard.width();
    int cyBitmap = imageKeyboard.height();
    m_nImageLeft = (this->width() - cxBitmap) / 2;
    m_nImageTop = (this->height() - cyBitmap) / 2;

    painter.drawImage(QPoint(m_nImageLeft, m_nImageTop), imageKeyboard);

    if (m_nKeyPressed != 0)
    {
        QImage imageKeyboard2(":/images/keyboard.png");
        if (!isDarkPalette(palette()))  // For dark theme, invert keyboard palette; invert for pressed
            invertImageColors(imageKeyboard2);

        for (int i = 0; i < m_nKeyboardKeysCount; i++)
            if (m_nKeyPressed == m_arrKeyboardKeys[i * 5 + 4])
            {
                int x = m_arrKeyboardKeys[i * 5];
                int y = m_arrKeyboardKeys[i * 5 + 1];
                int w = m_arrKeyboardKeys[i * 5 + 2];
                int h = m_arrKeyboardKeys[i * 5 + 3];
                painter.drawImage(m_nImageLeft + x, m_nImageTop + y, imageKeyboard2, x, y, w, h);
            }
    }

    //// Show key mappings
    //for (int i = 0; i < m_nKeyboardKeysCount; i++)
    //{
    //    QRect rcKey;
    //    rcKey.setLeft(m_nImageLeft + m_arrKeyboardKeys[i * 5]);
    //    rcKey.setTop(m_nImageTop + m_arrKeyboardKeys[i * 5 + 1]);
    //    rcKey.setRight(rcKey.left() + m_arrKeyboardKeys[i * 5 + 2]);
    //    rcKey.setBottom(rcKey.top() + m_arrKeyboardKeys[i * 5 + 3]);

    //    painter.drawRect(rcKey);
    //}
}

void QKeyboardView::mousePressEvent(QMouseEvent *event)
{
    quint16 keyscan = getKeyByPoint(event->x(), event->y());
    if (keyscan == 0) return;

    grabMouse();

    m_nKeyPressed = keyscan;

    repaint();
}

void QKeyboardView::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    if (m_nKeyPressed == 0) return;

    releaseMouse();

    m_nKeyPressed = 0;

    repaint();
}

quint16 QKeyboardView::getKeyPressed()
{
    return m_nKeyPressed;
}

// Returns: NEON scan-code of key under the cursor position, or 0 if not found
quint16 QKeyboardView::getKeyByPoint(int x, int y)
{
    for (int i = 0; i < m_nKeyboardKeysCount; i++)
    {
        int x1 = m_nImageLeft + m_arrKeyboardKeys[i * 5];
        int y1 = m_nImageTop + m_arrKeyboardKeys[i * 5 + 1];
        int x2 = x1 + m_arrKeyboardKeys[i * 5 + 2];
        int y2 = y1 + m_arrKeyboardKeys[i * 5 + 3];
        if (x >= x1 && x < x2 && y >= y1 && y < y2)
            return m_arrKeyboardKeys[i * 5 + 4];
    }

    return 0;
}
