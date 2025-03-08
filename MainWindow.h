#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "OverlayWidget.h"
#include "OverlayScheme.h"
#include "SettingsDialog.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QScreen>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void InitTrayIcon();

    void InitOverlayWidget();

    void InitSettings();

    void InitHotkeys();

    // Update UI according to parameters. (No triggers)
    void SetActionEnabledUI(bool bEnabled);
    void SetActionInvertedUI(bool bEnabled);

    // Move to specified screen, or to all screens if nullptr.
    void MoveToScreen(QScreen *screen = nullptr);

    // Update system tray menu profiles current active one.
    void UpdateTrayProfileActive(QString profileName);

    // Switch to profile relative to current profile in tray menu.
    void SwitchProfileRelatively(int offset);

    // Get active profile index in m_actionProfiles.
    int GetCurrentActiveProfileIndex();

private slots:
    void OnHotkeyPressed(int id);
    
    void OnExit() { qApp->quit(); }

    void OnToggleOverlayFromAction(bool bChecked);

    void OnToggleInvertedFromAction(bool bChecked);

    void OnToggleHLineFromAction();
    void OnToggleVLineFromAction();

    // Update system tray action state. (No triggers)
    void OnUpdateLinesToggleState(bool hLineChecked, bool vLineChecked);

    void OnShowSettings();

    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

    /// Receive signals from SettingsDialog.
    void OnOverlayEnabled(bool bEnabled);
    void OnOverlayInverted(bool bInverted);

    void OnOverlaySchemeChanged(OverlayScheme::Ptr pOverlayScheme);
    void OnScreenChanged(int screenIndex);

    void OnProfilesUpdate(QVector<OverlayScheme::Ptr> profiles);
    void OnTrayProfileActionTriggered();

private:
    Ui::MainWindow *ui;

    OverlayWidget *m_overlayWidget = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;

    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_menuTray = nullptr;
    QAction *m_actionSettings = nullptr;
    QAction *m_actionEnable = nullptr;
    QAction *m_actionInverted = nullptr;

    QAction *m_actionToggleHLine = nullptr;
    QAction *m_actionToggleVLine = nullptr;

    // Sub menu of profiles.
    QMenu *m_subMenuProfiles = nullptr;
    QVector<QAction *> m_actionProfiles;
};
#endif // MAINWINDOW_H
