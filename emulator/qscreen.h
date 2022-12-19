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
    bool getScreenText(uint8_t* buffer);
    void setMode(int mode);
    int mode() const { return m_mode; }

protected:
    void createDisplay();
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QImage* m_image;
    int m_mode;

private:
    unsigned char TranslateQtKeyToUkncKey(int qtkey);
};

#endif // QEMULATORSCREEN_H
