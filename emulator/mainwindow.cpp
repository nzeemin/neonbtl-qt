#include "stdafx.h"
#include <QAction>
#include <QClipboard>
#include <QDateTime>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include "main.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdialogs.h"
#include "qscreen.h"
#include "qkeyboardview.h"
#include "qconsoleview.h"
#include "qdebugview.h"
#include "qdisasmview.h"
#include "qmemoryview.h"
#include "qscripting.h"
#include "Emulator.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("NEON Back to Life"));

    // Assign signals
    QObject::connect(ui->actionSaveStateImage, SIGNAL(triggered()), this, SLOT(saveStateImage()));
    QObject::connect(ui->actionLoadStateImage, SIGNAL(triggered()), this, SLOT(loadStateImage()));
    QObject::connect(ui->actionFileScreenshot, SIGNAL(triggered()), this, SLOT(saveScreenshot()));
    QObject::connect(ui->actionFileScreenshotAs, SIGNAL(triggered()), this, SLOT(saveScreenshotAs()));
    QObject::connect(ui->actionFileScreenshotToClipboard, SIGNAL(triggered()), this, SLOT(screenshotToClipboard()));
    QObject::connect(ui->actionScriptRun, SIGNAL(triggered()), this, SLOT(scriptRun()));
    QObject::connect(ui->actionFileExit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->actionEmulatorRun, SIGNAL(triggered()), this, SLOT(emulatorRun()));
    QObject::connect(ui->actionEmulatorReset, SIGNAL(triggered()), this, SLOT(emulatorReset()));
    QObject::connect(ui->actionactionEmulatorAutostart, SIGNAL(triggered()), this, SLOT(emulatorAutostart()));
    QObject::connect(ui->actionDrivesFloppy0, SIGNAL(triggered()), this, SLOT(emulatorFloppy0()));
    QObject::connect(ui->actionDrivesFloppy1, SIGNAL(triggered()), this, SLOT(emulatorFloppy1()));
    QObject::connect(ui->actionDrivesHard, SIGNAL(triggered()), this, SLOT(emulatorHardDrive()));
    QObject::connect(ui->actionDebugConsoleView, SIGNAL(triggered()), this, SLOT(debugConsoleView()));
    QObject::connect(ui->actionDebugDebugView, SIGNAL(triggered()), this, SLOT(debugDebugView()));
    QObject::connect(ui->actionDebugDisasmView, SIGNAL(triggered()), this, SLOT(debugDisasmView()));
    QObject::connect(ui->actionDebugMemoryView, SIGNAL(triggered()), this, SLOT(debugMemoryView()));
    QObject::connect(ui->actionDebugStepInto, SIGNAL(triggered()), this, SLOT(debugStepInto()));
    QObject::connect(ui->actionDebugStepOver, SIGNAL(triggered()), this, SLOT(debugStepOver()));
    QObject::connect(ui->actionDebugClearConsole, SIGNAL(triggered()), this, SLOT(debugClearConsole()));
    QObject::connect(ui->actionDebugRemoveAllBreakpoints, SIGNAL(triggered()), this, SLOT(debugRemoveAllBreakpoints()));
    QObject::connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(helpAbout()));
    QObject::connect(ui->actionViewKeyboard, SIGNAL(triggered()), this, SLOT(viewKeyboard()));
    QObject::connect(ui->actionViewMode0, SIGNAL(triggered()), this, SLOT(viewViewMode0()));
    QObject::connect(ui->actionViewMode1, SIGNAL(triggered()), this, SLOT(viewViewMode1()));
    QObject::connect(ui->actionViewMode2, SIGNAL(triggered()), this, SLOT(viewViewMode2()));
    QObject::connect(ui->actionViewMode3, SIGNAL(triggered()), this, SLOT(viewViewMode3()));
    QObject::connect(ui->actionViewMode4, SIGNAL(triggered()), this, SLOT(viewViewMode4()));
    QObject::connect(ui->actionConfRam512, SIGNAL(triggered()), this, SLOT(confRam512()));
    QObject::connect(ui->actionConfRam1024, SIGNAL(triggered()), this, SLOT(confRam1024()));
    QObject::connect(ui->actionConfRam2048, SIGNAL(triggered()), this, SLOT(confRam2048()));
    QObject::connect(ui->actionConfRam4096, SIGNAL(triggered()), this, SLOT(confRam4096()));
    QObject::connect(ui->actionSoundEnabled, SIGNAL(triggered()), this, SLOT(soundEnabled()));

    // Screen and keyboard
    m_screen = new QEmulatorScreen();
    m_keyboard = new QKeyboardView();
    m_console = new QConsoleView();
    m_debug = new QDebugView(this);
    m_disasm = new QDisasmView();
    m_memory = new QMemoryView();

    QVBoxLayout *vboxlayout = new QVBoxLayout;
    vboxlayout->setMargin(4);
    vboxlayout->setSpacing(4);
    vboxlayout->addWidget(m_screen);
    vboxlayout->addWidget(m_keyboard);
    ui->centralWidget->setLayout(vboxlayout);
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    int maxwid = m_screen->maximumWidth() > m_keyboard->maximumWidth() ? m_screen->maximumWidth() : m_keyboard->maximumWidth();
    ui->centralWidget->setMaximumWidth(maxwid);

    m_dockDebug = new QDockWidget(tr("Processor"));
    m_dockDebug->setObjectName("dockDebug");
    m_dockDebug->setWidget(m_debug);
    m_dockDebug->setFeatures(m_dockDebug->features() & ~QDockWidget::DockWidgetClosable);
    m_dockDisasm = new QDockWidget(tr("Disassemble"));
    m_dockDisasm->setObjectName("dockDisasm");
    m_dockDisasm->setWidget(m_disasm);
    m_dockMemory = new QDockWidget(tr("Memory"));
    m_dockMemory->setObjectName("dockMemory");
    m_dockMemory->setWidget(m_memory);
    m_dockConsole = new QDockWidget(tr("Debug Console"));
    m_dockConsole->setObjectName("dockConsole");
    m_dockConsole->setWidget(m_console);
    m_dockConsole->setFeatures(m_dockConsole->features() & ~QDockWidget::DockWidgetClosable);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDebug, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockDisasm, Qt::Vertical);
    this->addDockWidget(Qt::RightDockWidgetArea, m_dockMemory, Qt::Vertical);
    this->addDockWidget(Qt::BottomDockWidgetArea, m_dockConsole);

    m_statusLabelInfo = new QLabel(this);
    m_statusLabelFrames = new QLabel(this);
    m_statusLabelUptime = new QLabel(this);
    statusBar()->addWidget(m_statusLabelInfo, 600);
    statusBar()->addPermanentWidget(m_statusLabelFrames, 150);
    statusBar()->addPermanentWidget(m_statusLabelUptime, 150);

    this->setFocusProxy(m_screen);

    autoStartProcessed = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_screen;
    delete m_keyboard;
    delete m_console;
    delete m_debug;
    delete m_disasm;
    delete m_memory;
    delete m_dockConsole;
    delete m_dockDebug;
    delete m_dockDisasm;
    delete m_dockMemory;
    delete m_statusLabelInfo;
    delete m_statusLabelFrames;
    delete m_statusLabelUptime;
}

void MainWindow::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);

    // Process Autostart on first show event
    if (!autoStartProcessed)
    {
        if (Settings_GetAutostart())
        {
            QTimer::singleShot(0, this, SLOT(emulatorRun()));
        }

        autoStartProcessed = true;
    }
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    Global_getSettings()->setValue("MainWindow/ScreenViewMode", m_screen->mode());

    Global_getSettings()->setValue("MainWindow/Geometry", saveGeometry());
    Global_getSettings()->setValue("MainWindow/WindowState", saveState());

    Global_getSettings()->setValue("MainWindow/ConsoleView", m_dockConsole->isVisible());
    Global_getSettings()->setValue("MainWindow/DebugView", m_dockDebug->isVisible());
    Global_getSettings()->setValue("MainWindow/DisasmView", m_dockDisasm->isVisible());
    Global_getSettings()->setValue("MainWindow/MemoryView", m_dockMemory->isVisible());
}

void MainWindow::restoreSettings()
{
    int scrViewMode = Global_getSettings()->value("MainWindow/ScreenViewMode").toInt();
    m_screen->setMode(scrViewMode);

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());

    //NOTE: Restore from maximized state fails, see https://bugreports.qt-project.org/browse/QTBUG-15080
    restoreGeometry(Global_getSettings()->value("MainWindow/Geometry").toByteArray());
    restoreState(Global_getSettings()->value("MainWindow/WindowState").toByteArray());

    m_dockConsole->setVisible(Global_getSettings()->value("MainWindow/ConsoleView", false).toBool());
    m_dockDebug->setVisible(Global_getSettings()->value("MainWindow/DebugView", false).toBool());
    m_dockDisasm->setVisible(Global_getSettings()->value("MainWindow/DisasmView", false).toBool());
    m_dockMemory->setVisible(Global_getSettings()->value("MainWindow/MemoryView", false).toBool());

    ui->actionSoundEnabled->setChecked(Settings_GetSound());
    m_debug->updateWindowText();
    m_disasm->updateWindowText();
//    m_memory->updateWindowText();
}

void MainWindow::updateMenu()
{
    ui->actionEmulatorRun->setChecked(g_okEmulatorRunning);
    ui->actionactionEmulatorAutostart->setChecked(Settings_GetAutostart());
    ui->actionViewMode0->setChecked(m_screen->mode() == 0);
    ui->actionViewMode1->setChecked(m_screen->mode() == 1);
    ui->actionViewMode2->setChecked(m_screen->mode() == 2);
    ui->actionViewMode3->setChecked(m_screen->mode() == 3);
    ui->actionViewMode4->setChecked(m_screen->mode() == 4);

    ui->actionViewKeyboard->setChecked(m_keyboard->isVisible());

    int conf = Settings_GetConfiguration();
    ui->actionConfRam512->setChecked((conf & NEON_COPT_RAMSIZE_MASK) <= 512);
    ui->actionConfRam1024->setChecked((conf & NEON_COPT_RAMSIZE_MASK) == 1024);
    ui->actionConfRam2048->setChecked((conf & NEON_COPT_RAMSIZE_MASK) == 2048);
    ui->actionConfRam4096->setChecked((conf & NEON_COPT_RAMSIZE_MASK) == 4096);

    ui->actionDrivesFloppy0->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(0) ? ":/images/iconFloppy.svg" : ":/images/iconFloppySlot.svg" ));
    ui->actionDrivesFloppy1->setIcon(QIcon(
            g_pBoard->IsFloppyImageAttached(1) ? ":/images/iconFloppy.svg" : ":/images/iconFloppySlot.svg" ));

//    ui->actionDrivesHard->setIcon(QIcon(
//            g_pBoard->IsHardImageAttached() ? ":/images/iconHdd.svg" : ":/images/iconHddSlot.svg" ));

    ui->actionDebugConsoleView->setChecked(m_console->isVisible());
    ui->actionDebugDebugView->setChecked(m_dockDebug->isVisible());
    ui->actionDebugDisasmView->setChecked(m_dockDisasm->isVisible());
    ui->actionDebugMemoryView->setChecked(m_dockMemory->isVisible());
}

void MainWindow::updateAllViews()
{
    Emulator_OnUpdate();

    if (m_debug != nullptr)
        m_debug->updateData();
    if (m_disasm != nullptr)
        m_disasm->updateData();
    if (m_memory != nullptr)
        m_memory->updateData();
    if (m_console != nullptr)
        m_console->updatePrompt();

    m_screen->repaint();
    if (m_debug != nullptr)
        m_debug->repaint();
    if (m_disasm != nullptr)
        m_disasm->repaint();
    if (m_memory != nullptr)
        m_memory->repaint();

    updateMenu();
}

void MainWindow::redrawDebugView()
{
    if (m_debug != nullptr)
        m_debug->repaint();
}
void MainWindow::redrawDisasmView()
{
    if (m_disasm != nullptr)
        m_disasm->repaint();
}

void MainWindow::updateWindowText()
{
    if (g_okEmulatorRunning)
        this->setWindowTitle(tr("NEON Back to Life [run]"));
    else
        this->setWindowTitle(tr("NEON Back to Life [stop]"));
}

void MainWindow::showUptime(int uptimeMillisec)
{
    int seconds = (int) (uptimeMillisec % 60);
    int minutes = (int) (uptimeMillisec / 60 % 60);
    int hours   = (int) (uptimeMillisec / 3600 % 60);

    char buffer[20];
    _snprintf(buffer, 20, "%02d:%02d:%02d", hours, minutes, seconds);
    m_statusLabelUptime->setText(tr("Uptime: %1").arg(buffer));
}
void MainWindow::showFps(double framesPerSecond)
{
    if (framesPerSecond <= 0)
    {
        m_statusLabelFrames->setText("");
    }
    else
    {
        double speed = framesPerSecond / 25.0 * 100.0;
        char buffer[16];
        _snprintf(buffer, 16, "%03.f%%", speed);
        m_statusLabelFrames->setText(buffer);
    }
}

void MainWindow::saveStateImage()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("NEON state images (*.neonst)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveStateImage(strFileName);
}
void MainWindow::saveStateImage(const QString& strFileName)
{
    const char * sFileName = qPrintable(strFileName);
    Emulator_SaveImage(sFileName);
}
void MainWindow::loadStateImage()
{
    QFileDialog dlg;
    dlg.setNameFilter(tr("NEON state images (*.neonst)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    loadStateImage(strFileName);
}
void MainWindow::loadStateImage(const QString& strFileName)
{
    const char * sFileName = qPrintable(strFileName);
    Emulator_LoadImage(sFileName);

    updateAllViews();
}

void MainWindow::saveScreenshot()
{
    QString strFileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshotAs()
{
    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setNameFilter(tr("PNG images (*.png)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);

    saveScreenshot(strFileName);
}
void MainWindow::saveScreenshot(const QString& strFileName)
{
    QImage image = m_screen->getScreenshot();
    image.save(strFileName, "PNG", -1);
}

void MainWindow::screenshotToClipboard()
{
    QImage image = m_screen->getScreenshot();

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->clear();
    clipboard->setImage(image);
}

void MainWindow::helpAbout()
{
    QAboutDialog dialog(nullptr);
    dialog.exec();
}

void MainWindow::viewKeyboard()
{
    m_keyboard->setVisible(!m_keyboard->isVisible());
    updateMenu();
}

void MainWindow::viewViewMode0()
{
    m_screen->setMode(0);
    updateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewViewMode1()
{
    m_screen->setMode(1);
    updateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewViewMode2()
{
    m_screen->setMode(2);
    updateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewViewMode3()
{
    m_screen->setMode(3);
    updateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}
void MainWindow::viewViewMode4()
{
    m_screen->setMode(4);
    updateMenu();

    //Update centralWidget size
    ui->centralWidget->setMaximumHeight(m_screen->maximumHeight() + m_keyboard->maximumHeight());
    ui->centralWidget->setMaximumWidth(m_screen->maximumWidth());
}

void MainWindow::confRam512()
{
    changeConfiguration(Settings_GetConfiguration() & ~NEON_COPT_RAMSIZE_MASK | 512);
}
void MainWindow::confRam1024()
{
    changeConfiguration(Settings_GetConfiguration() & ~NEON_COPT_RAMSIZE_MASK | 1024);
}
void MainWindow::confRam2048()
{
    changeConfiguration(Settings_GetConfiguration() & ~NEON_COPT_RAMSIZE_MASK | 2048);
}
void MainWindow::confRam4096()
{
    changeConfiguration(Settings_GetConfiguration() & ~NEON_COPT_RAMSIZE_MASK | 4096);
}

void MainWindow::changeConfiguration(int configuration)
{
    int oldConf = Settings_GetConfiguration();
    if (oldConf == configuration)
        return;

    // Ask user -- we have to reset machine to change configuration
    if (!AlertOkCancel(tr("Reset required after configuration change.\nAre you agree?")))
        return;

    // Change configuration
    Emulator_InitConfiguration((NeonConfiguration)configuration);

    Settings_SetConfiguration(configuration);

    updateMenu();
    updateAllViews();
}

void MainWindow::emulatorFrame()
{
    if (!g_okEmulatorRunning)
        return;
    if (!isActiveWindow())
        return;

    if (!Emulator_SystemFrame())
        Emulator_Stop();  // Breakpoint hit

    m_screen->repaint();
}

void MainWindow::emulatorRun()
{
    if (g_okEmulatorRunning)
        Emulator_Stop();
    else
        Emulator_Start();
    updateWindowText();
    updateMenu();
}

void MainWindow::emulatorReset()
{
    Emulator_Reset();

    m_screen->repaint();
}

void MainWindow::emulatorAutostart()
{
    Settings_SetAutostart(!Settings_GetAutostart());
    updateMenu();
}

void MainWindow::soundEnabled()
{
    bool sound = ui->actionSoundEnabled->isChecked();
    Emulator_SetSound(sound);
    Settings_SetSound(sound);
}

void MainWindow::emulatorFloppy0() { emulatorFloppy(0); }
void MainWindow::emulatorFloppy1() { emulatorFloppy(1); }
void MainWindow::emulatorFloppy(int slot)
{
    if (g_pBoard->IsFloppyImageAttached(slot))
    {
        detachFloppy(slot);
    }
    else
    {
        QFileDialog dlg;
        dlg.setNameFilter(tr("NEON floppy images (*.dsk)"));
        if (dlg.exec() == QDialog::Rejected)
            return;

        QString strFileName = dlg.selectedFiles().at(0);

        if (! attachFloppy(slot, strFileName))
        {
            AlertWarning(tr("Failed to attach floppy image."));
            return;
        }
    }
}
bool MainWindow::attachFloppy(int slot, const QString & strFileName)
{
    QFileInfo fi(strFileName);
    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

    QByteArray baFullName = strFullName.toLocal8Bit();
    if (! g_pBoard->AttachFloppyImage(slot, baFullName.constData()))
        return false;

    Settings_SetFloppyFilePath(slot, strFullName);

    updateMenu();

    return true;
}
void MainWindow::detachFloppy(int slot)
{
    g_pBoard->DetachFloppyImage(slot);

    Settings_SetFloppyFilePath(slot, nullptr);

    updateMenu();
}

void MainWindow::emulatorHardDrive()
{
//    if (g_pBoard->IsHardImageAttached())
//    {
//        detachHardDrive();
//    }
//    else
//    {
//        // Select HDD disk image
//        QFileDialog dlg;
//        dlg.setNameFilter(tr("NEON HDD images (*.img)"));
//        if (dlg.exec() == QDialog::Rejected)
//            return;

//        QString strFileName = dlg.selectedFiles().at(0);
//        if (! attachHardDrive(strFileName))
//        {
//            AlertWarning(tr("Failed to attach hard drive image."));
//            return;
//        }
//    }
}
bool MainWindow::attachHardDrive(const QString & strFileName)
{
//    QFileInfo fi(strFileName);
//    QString strFullName(fi.canonicalFilePath());  // Get absolute file name

//    LPCTSTR sFileName = qPrintable(strFullName);
//    if (!g_pBoard->AttachHardImage(sFileName))
//        return false;

//    Settings_SetHardFilePath(strFullName);

//    updateMenu();

//    return true;
    return false;
}
void MainWindow::detachHardDrive()
{
//    g_pBoard->DetachHardImage();
//    Settings_SetHardFilePath(nullptr);
}

void MainWindow::debugConsoleView()
{
    bool okShow = !m_dockConsole->isVisible();
    m_dockConsole->setVisible(okShow);
    m_dockDebug->setVisible(okShow);
    m_dockDisasm->setVisible(okShow);
    m_dockMemory->setVisible(okShow);

    if (!okShow)
    {
        this->adjustSize();
    }

    updateMenu();
}
void MainWindow::debugDebugView()
{
    m_dockDebug->setVisible(!m_dockDebug->isVisible());
    updateMenu();
}
void MainWindow::debugDisasmView()
{
    m_dockDisasm->setVisible(!m_dockDisasm->isVisible());
    updateMenu();
}
void MainWindow::debugMemoryView()
{
    m_dockMemory->setVisible(!m_dockMemory->isVisible());
    updateMenu();
}

void MainWindow::debugStepInto()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("s");
}
void MainWindow::debugStepOver()
{
    if (!g_okEmulatorRunning)
        m_console->execConsoleCommand("so");
}

void MainWindow::debugClearConsole()
{
    m_console->clear();
}
void MainWindow::debugRemoveAllBreakpoints()
{
    m_console->execConsoleCommand("bc");
}

void MainWindow::scriptRun()
{
    if (g_okEmulatorRunning)
        emulatorRun();  // Stop the emulator

    QFileDialog dlg;
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setNameFilter(tr("Script files (*.js)"));
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString strFileName = dlg.selectedFiles().at(0);
    QFile file(strFileName);
    file.open(QIODevice::ReadOnly);
    QString strScript = file.readAll();

    QScriptWindow window(this);
    window.runScript(strScript);
}

void MainWindow::consolePrint(const QString &message)
{
    m_console->printLine(message);
}
