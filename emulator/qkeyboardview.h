#ifndef QKEYBOARDVIEW_H
#define QKEYBOARDVIEW_H

#include <QWidget>

class QKeyboardView : public QWidget
{
    Q_OBJECT
public:
    QKeyboardView(QWidget *parent = nullptr);

    quint16 getKeyPressed();  // Get pressed key if any, to call from ScreenView

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    int m_nImageLeft;
    int m_nImageTop;
    quint16 m_nKeyPressed;

private:
    quint16 getKeyByPoint(int x, int y);
};

#endif // QKEYBOARDVIEW_H
