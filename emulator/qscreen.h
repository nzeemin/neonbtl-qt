#ifndef QEMULATORSCREEN_H
#define QEMULATORSCREEN_H

#include <QWidget>
#include "main.h"


class QEmulatorScreen : public QWidget
{
    Q_OBJECT
public:
    QEmulatorScreen(QWidget *parent = nullptr);
    ~QEmulatorScreen();

public:
    QImage getScreenshot();
    void setMode(int mode);
    int mode() const { return m_mode; }
    void processKeyboard(quint16 keyscan);

protected:
    void createDisplay();
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QImage* m_image;
    int m_mode;
    quint8 m_keyboardMatrix[8];

private:
    quint16 TranslateQtKeyToNeonKey(int qtkey);
};

#endif // QEMULATORSCREEN_H
