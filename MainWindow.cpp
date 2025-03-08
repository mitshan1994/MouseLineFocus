#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "mylog/mylog.h"
#include "ShortcutDefine.h"
#include "HotkeyHook/KeyboardHook.h"

static OverlayScheme::Ptr GetNormalScheme()
{
    OverlayScheme::Ptr pScheme = std::make_shared<OverlayScheme>();

    pScheme->invertedBgColor = QColor{ 0, 0, 0, 0 };
    pScheme->hLineWidth = 1;
    pScheme->vLineWidth = 1;
    pScheme->hLineColor = QColor{ 0, 255, 0, 100 };
    pScheme->vLineColor = QColor{ 0, 255, 0, 100 };

    return pScheme;
}

static OverlayScheme::Ptr GetInvertedScheme()
{
    OverlayScheme::Ptr pScheme = std::make_shared<OverlayScheme>();

    pScheme->invertedBgColor = QColor{ 0, 255, 0, 70 };
    pScheme->hLineWidth = 1;
    pScheme->vLineWidth = 1;
    pScheme->hLineColor = QColor{ 255, 0, 0, 0 };
    pScheme->vLineColor = QColor{ 255, 0, 0, 0 };

    return pScheme;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    //setWindowFlag(Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_TranslucentBackground);

    InitSettings();

    InitOverlayWidget();

    InitTrayIcon();

    InitHotkeys();

    QTimer::singleShot(50, [this]() {
        hide();
        });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QTimer::singleShot(0, [this]() {
        // Make overlay widget the same size.
        m_overlayWidget->resize(size());

        // Get global pos.
        QPoint globalStartPos = mapToGlobal(QPoint(0, 0));

        //m_overlayWidget->move(0, 0);
        m_overlayWidget->move(globalStartPos);
    });
}

void MainWindow::InitTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/resources/icon.png"));
    m_trayIcon->setToolTip("Track and Read");
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);

    m_trayIcon->show();

    m_menuTray = new QMenu(this);

    QAction *actionSettings = new QAction("Configurations", this);
    connect(actionSettings, &QAction::triggered, this, &MainWindow::OnShowSettings);
    m_menuTray->addAction(actionSettings);
    m_actionSettings = actionSettings;
    m_menuTray->addSeparator();

    QAction *actionToggle = new QAction("Enabled (click to toggle)", this);
    actionToggle->setCheckable(true);
    actionToggle->setChecked(true);
    connect(actionToggle, &QAction::toggled, this, &MainWindow::OnOverlayEnabled);
    m_menuTray->addAction(actionToggle);
    m_actionEnable = actionToggle;

    QAction *actionInverted = new QAction("Inverted", this);
    actionInverted->setCheckable(true);
    connect(actionInverted, &QAction::toggled, this, &MainWindow::OnOverlayInverted);
    actionInverted->setChecked(false);
    m_menuTray->addAction(actionInverted);
    m_actionInverted = actionInverted;

    m_menuTray->addSeparator();

    QAction *actionToggleHLine = new QAction("Toggle h-line", this);
    actionToggleHLine->setCheckable(true);
    connect(actionToggleHLine, &QAction::toggled, this, &MainWindow::OnToggleHLineFromAction);
    m_menuTray->addAction(actionToggleHLine);
    m_actionToggleHLine = actionToggleHLine;

    QAction *actionToggleVLine = new QAction("Toggle v-line", this);
    actionToggleVLine->setCheckable(true);
    connect(actionToggleVLine, &QAction::toggled, this, &MainWindow::OnToggleVLineFromAction);
    m_menuTray->addAction(actionToggleVLine);
    m_actionToggleVLine = actionToggleVLine;

    m_menuTray->addSeparator();

    m_subMenuProfiles = new QMenu("Profiles", this);
    m_menuTray->addMenu(m_subMenuProfiles);

    m_menuTray->addAction("Exit", this, &MainWindow::OnExit);

    m_trayIcon->setContextMenu(m_menuTray);

    //connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayIconActivated);
}

void MainWindow::InitOverlayWidget()
{
    m_overlayWidget = new OverlayWidget(this);
    m_overlayWidget->show();

    m_overlayWidget->SetOverlayScheme(GetNormalScheme());
}

void MainWindow::InitSettings()
{
    Q_ASSERT(m_settingsDialog == nullptr);

    // Create settings dialog.
    m_settingsDialog = new SettingsDialog(this);

    // Connect.
    connect(m_settingsDialog, &SettingsDialog::SigOverlayEnabled, 
        this, &MainWindow::OnOverlayEnabled);
    connect(m_settingsDialog, &SettingsDialog::SigOverlayInverted,
        this, &MainWindow::OnOverlayInverted);
    connect(m_settingsDialog, &SettingsDialog::SigOverlaySchemeChanged,
        this, &MainWindow::OnOverlaySchemeChanged);
    connect(m_settingsDialog, &SettingsDialog::SigScreenChanged,
        this, &MainWindow::OnScreenChanged);
    connect(m_settingsDialog, &SettingsDialog::SigProfilesUpdated,
        this, &MainWindow::OnProfilesUpdate);

    connect(m_settingsDialog, &SettingsDialog::SigDialogHided,
        [this]() {
            hide();
        });
}

void MainWindow::InitHotkeys()
{
    connect(&KeyboardHook::getInstance(), &KeyboardHook::keyPressed,
            this, &MainWindow::OnHotkeyPressed);

    // Register hotkeys.
    Hotkey hotkeyToggleOverlay("Ctrl+Shift+T");
    Hotkey hotkeyToggleInverted("Ctrl+Shift+I");

    Hotkey hotkeyToggleHLine("Ctrl+Shift+H");
    Hotkey hotkeyToggleVLine("Ctrl+Shift+V");

    Hotkey hotkeyPreviousProfile("Ctrl+Shift+P");
    Hotkey hotkyeNextProfile("Ctrl+Shift+N");

    KeyboardHook::getInstance().addHotkey(SC_ID_TOGGLE_OVERLAY, hotkeyToggleOverlay);
    KeyboardHook::getInstance().addHotkey(SC_ID_TOGGLE_INVERTED, hotkeyToggleInverted);
    KeyboardHook::getInstance().addHotkey(SC_ID_TOGGLE_HLINE, hotkeyToggleHLine);
    KeyboardHook::getInstance().addHotkey(SC_ID_TOGGLE_VLINE, hotkeyToggleVLine);
    KeyboardHook::getInstance().addHotkey(SC_ID_PREVIOUS_PROFILE, hotkeyPreviousProfile);
    KeyboardHook::getInstance().addHotkey(SC_ID_NEXT_PROFILE, hotkyeNextProfile);
}

void MainWindow::SetActionEnabledUI(bool bEnabled)
{
    m_actionEnable->blockSignals(true);
    m_actionEnable->setChecked(bEnabled);
    m_actionEnable->blockSignals(false);
}

void MainWindow::SetActionInvertedUI(bool bEnabled)
{
    m_actionInverted->blockSignals(true);
    m_actionInverted->setChecked(bEnabled);
    m_actionInverted->blockSignals(false);
}

void MainWindow::MoveToScreen(QScreen *screen)
{
    bool bWholeScreens = false;
    if (nullptr == screen) {
        screen = QApplication::primaryScreen();
        bWholeScreens = true;
    }

    if (nullptr == screen) {
        L_ERROR("No screen detected!");
        qApp->exit(1);
    }

    QRect rect = screen->geometry();
    if (bWholeScreens) {
        rect = screen->virtualGeometry();
    }
    L_INFO("Screen: {}, {}, {}, {}", rect.x(), rect.y(), rect.width(), rect.height());

    // Resize window to fit screen.
    move(rect.x(), rect.y());
    resize(rect.width(), rect.height());

    show();
}

void MainWindow::UpdateTrayProfileActive(QString profileName)
{
    L_TRACE("Active profile name: {}", profileName);

    for (auto actionProfile : m_actionProfiles) {
        actionProfile->blockSignals(true);

        if (actionProfile->text() == profileName) {
            actionProfile->setChecked(true);
        } else {
            actionProfile->setChecked(false);
        }

        actionProfile->blockSignals(false);
    }
}

void MainWindow::SwitchProfileRelatively(int offset)
{
    L_TRACE("Switch profile relatively. Offset: {}", offset);

    int index = GetCurrentActiveProfileIndex();
    index += offset;

    if (index < 0) {
        index += m_actionProfiles.size();
    } else if (index >= m_actionProfiles.size()) {
        index -= m_actionProfiles.size();
    }

    if (index < 0 || index >= m_actionProfiles.size()) {
        L_ERROR("Invalid index: {}", index);
        return;
    }

    m_actionProfiles[index]->trigger();
}

int MainWindow::GetCurrentActiveProfileIndex()
{
    int index = -1;
    for (int i = 0; i != m_actionProfiles.size(); ++i) {
        if (m_actionProfiles[i]->isChecked()) {
            index = i;
            break;
        }
    }

    return index;
}

void MainWindow::OnHotkeyPressed(int id)
{
    L_TRACE("MainWindow::OnHotkeyPressed: {}", id);

    if (id == SC_ID_TOGGLE_OVERLAY) {
        bool bEnabled = !m_actionEnable->isChecked();
        OnOverlayEnabled(bEnabled);
    } else if (id == SC_ID_TOGGLE_INVERTED) {
        bool bInverted = !m_actionInverted->isChecked();
        OnOverlayInverted(bInverted);
    } else if (id == SC_ID_TOGGLE_HLINE) {
        //m_actionToggleHLine->toggle();
        m_actionToggleHLine->toggle();
    } else if (id == SC_ID_TOGGLE_VLINE) {
        m_actionToggleVLine->toggle();
    } else if (id == SC_ID_PREVIOUS_PROFILE) {
        SwitchProfileRelatively(-1);
    } else if (id == SC_ID_NEXT_PROFILE) {
        SwitchProfileRelatively(1);
    }
}

// TODO remove. Not used.
void MainWindow::OnToggleOverlayFromAction(bool bChecked)
{
    m_overlayWidget->SetEnabled(bChecked);
}

// TODO remove. Not used.
void MainWindow::OnToggleInvertedFromAction(bool bChecked)
{
    if (bChecked) {
        m_overlayWidget->SetOverlayScheme(GetInvertedScheme());
    } else {
        m_overlayWidget->SetOverlayScheme(GetNormalScheme());
    }
}

void MainWindow::OnToggleHLineFromAction()
{
    L_TRACE("toggle hline");

    m_overlayWidget->ToggleHLine();
}

void MainWindow::OnToggleVLineFromAction()
{
    L_TRACE("toggle vline");

    m_overlayWidget->ToggleVLine();
}

void MainWindow::OnUpdateLinesToggleState(bool hLineChecked, bool vLineChecked)
{
    m_actionToggleHLine->blockSignals(true);
    m_actionToggleHLine->setChecked(hLineChecked);
    m_actionToggleHLine->blockSignals(false);

    m_actionToggleVLine->blockSignals(true);
    m_actionToggleVLine->setChecked(vLineChecked);
    m_actionToggleVLine->blockSignals(false);
}

void MainWindow::OnOverlayEnabled(bool bEnabled)
{
    L_TRACE("MainWindow::OnOverlayEnabled: {}", bEnabled);

    m_overlayWidget->SetEnabled(bEnabled);

    QObject *objSender = sender();
    if (objSender == m_settingsDialog) {
        SetActionEnabledUI(bEnabled);
    } else if (objSender == m_actionEnable) {
        m_settingsDialog->SetOverlayEnabledUI(bEnabled);
    } else {
        // Triggered by hotkey.
        SetActionEnabledUI(bEnabled);
        m_settingsDialog->SetOverlayEnabledUI(bEnabled);
    }
}

void MainWindow::OnOverlayInverted(bool bInverted)
{
    L_TRACE("MainWindow::OnOverlayInverted: {}", bInverted);

    m_overlayWidget->SetInverted(bInverted);

    QObject *objSender = sender();
    if (objSender == m_settingsDialog) {
        SetActionInvertedUI(bInverted);
    } else if (objSender == m_actionInverted) {
        m_settingsDialog->SetOverlayInvertedUI(bInverted);
    } else {
        // Triggered by hotkey.
        SetActionInvertedUI(bInverted);
        m_settingsDialog->SetOverlayInvertedUI(bInverted);
    }
}

void MainWindow::OnOverlaySchemeChanged(OverlayScheme::Ptr pOverlayScheme)
{
    L_TRACE("MainWindow::OnOverlaySchemeChanged: {}", pOverlayScheme->schemeName);

    m_overlayWidget->SetOverlayScheme(pOverlayScheme);

    UpdateTrayProfileActive(pOverlayScheme->schemeName);

    // Update hline/vline toggle state.
    // Since this is the only point to receive scheme changes from settings dialog
    //   (and from system tray -> setting dialog -> here), it's sufficient to
    //   update the toggle state here.
    OnUpdateLinesToggleState(pOverlayScheme->bEnableHLine, pOverlayScheme->bEnableVLine);
}

void MainWindow::OnScreenChanged(int screenIndex)
{
    L_INFO("Screen changed. index: {}", screenIndex);

    // Currently we need to apply to all screens directly.
    MoveToScreen(nullptr);
    return;

    auto screens = QGuiApplication::screens();
    if (screenIndex < 0 || screenIndex >= screens.size()) {
        L_ERROR("No screen of index {}. Total screen count: {}.", screenIndex, screens.size());
        return;
    }

    MoveToScreen(screens[screenIndex]);
}

void MainWindow::OnProfilesUpdate(QVector<OverlayScheme::Ptr> profiles)
{
    // Update profiles in system tray.
    for (auto action : m_actionProfiles) {
        m_subMenuProfiles->removeAction(action);
        delete action;
    }
    m_actionProfiles.clear();

    // Add.
    for (auto scheme : profiles) {
        QAction *action = new QAction(scheme->schemeName, this);
        action->setCheckable(true);
        action->setChecked(false);
        connect(action, &QAction::triggered, this, &MainWindow::OnTrayProfileActionTriggered);
        m_subMenuProfiles->addAction(action);
        m_actionProfiles.push_back(action);
    }
}

void MainWindow::OnTrayProfileActionTriggered()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    Q_ASSERT(action);

    QString profileName = action->text();
    L_TRACE("Profile action triggered: {}", profileName);

    UpdateTrayProfileActive(profileName);

    // Update settings dialog.
    m_settingsDialog->SetCurrentProfile(profileName);
}

void MainWindow::OnShowSettings()
{
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainWindow::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        OnShowSettings();
    }
}
