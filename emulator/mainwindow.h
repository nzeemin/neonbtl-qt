#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QEmulatorScreen;
class QKeyboardView;
class QConsoleView;
class QDebugView;
class QDisasmView;
class QMemoryView;
class QLabel;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void updateMenu();
    void updateAllViews();
    void redrawDebugView();
    void redrawDisasmView();
    void updateWindowText();
    void restoreSettings();
    void showUptime(int uptimeMillisec);
    void showFps(double framesPerSecond);

public:
    void saveStateImage(const QString& filename);
    void loadStateImage(const QString& filename);
    void saveScreenshot(const QString& filename);
    bool attachFloppy(int slot, const QString& filename);
    void detachFloppy(int slot);
    bool attachHardDrive(const QString& filename);
    void detachHardDrive();
    void consolePrint(const QString&);

public slots:
    void saveStateImage();
    void loadStateImage();
    void saveScreenshot();
    void saveScreenshotAs();
    void screenshotToClipboard();
    void helpAbout();
    void emulatorFrame();
    void emulatorRun();
    void emulatorReset();
    void emulatorAutostart();
    void emulatorFloppy0();
    void emulatorFloppy1();
    void emulatorHardDrive();
    void debugConsoleView();
    void debugDebugView();
    void debugDisasmView();
    void debugMemoryView();
    void debugStepInto();
    void debugStepOver();
    void debugClearConsole();
    void debugRemoveAllBreakpoints();
    void viewKeyboard();
    void viewViewMode0();
    void viewViewMode1();
    void viewViewMode2();
    void viewViewMode3();
    void viewViewMode4();
    void confRam512();
    void confRam1024();
    void confRam2048();
    void confRam4096();
    void soundEnabled();

protected:
    void showEvent(QShowEvent *);
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
    bool autoStartProcessed;

    QEmulatorScreen *m_screen;
    QKeyboardView *m_keyboard;
    QConsoleView *m_console;
    QDockWidget* m_dockConsole;
    QDebugView *m_debug;
    QDockWidget* m_dockDebug;
    QDisasmView *m_disasm;
    QDockWidget* m_dockDisasm;
    QMemoryView * m_memory;
    QDockWidget* m_dockMemory;

    QLabel* m_statusLabelInfo;
    QLabel* m_statusLabelFrames;
    QLabel* m_statusLabelUptime;

    void changeConfiguration(int configuration);
    void emulatorFloppy(int slot);
};

#endif // MAINWINDOW_H
