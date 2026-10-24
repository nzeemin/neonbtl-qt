#ifndef QMEMORYVIEW_H
#define QMEMORYVIEW_H

#include <QAction>
#include <QList>
#include <QWidget>

class QPainter;
class QScrollBar;
class QTabBar;
class QToolBar;

class QMemoryView : public QWidget
{
    Q_OBJECT
public:
    QMemoryView();
    ~QMemoryView();

    void updateData();
    void updateWindowText();

public slots:
    void setMode(int mode);  // Select the memory mode: from the tab bar, the Space key
    void changeWordByteMode();
    void changeNumeralMode();
    void gotoAddress();
    void scrollBy(qint16 delta);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void wheelEvent(QWheelEvent *);

    void updateScrollPos();
    void updateLayout();  // Column widths, page size, geometry and visibility of the tab bar, toolbar and scrollbar
    quint16 getWordFromMemory(quint16 address, bool& okValid, int& addrtype, quint16& wChanged);
    quint16 getWordFromHalt(quint16 address);
    void paintProcessList(QPainter& painter);

protected slots:
    void scrollValueChanged();

private:
    int m_Mode;
    bool m_ByteMode;  // false - word mode, true - byte mode
    quint16 m_NumeralMode = 0;
    quint16 m_wBaseAddress;
    int m_cyLineMemory;  // Line height in pixels
    int m_nPageSize;  // Page size in lines
    int m_cyLine;
    int m_PostionIncrement = 100;  // Increment by X to the next word
    int m_yTop = 0;  // Height of the tab bar: the content is shifted down by this
    QScrollBar *m_scrollbar;
    QToolBar* m_toolbar;
    QTabBar* m_tabbar;
    QList<QAction*> m_memoryActions;  // Toolbar actions disabled in the Process List mode
};


#endif // QMEMORYVIEW_H
