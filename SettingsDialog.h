#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "OverlayScheme.h"

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // Only update ui widgets without triggering. (Also save to settings)
    void SetOverlayEnabledUI(bool bEnabled);
    void SetOverlayInvertedUI(bool bInverted);

    // Set currrent profile with no changing signal emitting.
    void SetCurrentProfile(QString profileName);

protected:
    void hideEvent(QHideEvent *event) override;

signals:
    void SigOverlayEnabled(bool bEnabled);
    void SigOverlayInverted(bool bInverted);

    void SigOverlaySchemeChanged(OverlayScheme::Ptr pOverlayScheme);
    void SigScreenChanged(int screenIndex);

    void SigDialogHided();

    // Emitted when profiles list updated (first, add, delete).
    // This signal should usually be followd by a SigOverlaySchemeChanged signal.
    void SigProfilesUpdated(QVector<OverlayScheme::Ptr> profiles);

private:
    void RefreshScreenList();

    // Update UI according to current settings.
    void UpdateUI();

    // Read all profiles from settings.
    void ReadAllProfiles();

    void RefreshProfileList();

    // Update current selected profile to UI widgets.
    void UpdateCurrentProfileToUI();

    // Get profile by name. Search only in m_profiles.
    OverlayScheme::Ptr GetProfileByName(QString profileName);

    OverlayScheme::Ptr GetCurrentProfile();
    int GetCurrentProfileIndex();

    // Get current profile settings from UI.
    // OverlayScheme::schemeName is not set.
    OverlayScheme::Ptr GetCurrentProfileFromUI();

    // Save value to settings.
    void SaveEnabled(bool bEnabled);
    void SaveInverted(bool bInverted);

    void EmitProfilesUpdate();

private slots:
    void OnProfileCurrentIndexChanged(int index);

    /// Global settings.
    void OnScreenCurrentIndexChanged(int index);
    void OnEnableEditChanged(int state);

    void OnEnabled(int state);
    void OnEnableInvertedMode(int state);

    /// Profile settings.
    void OnBtnCreateProfileClicked();
    void OnBtnDeleteProfileClicked();

    void OnBtnChooseHLineColorClicked();
    void OnBtnChooseVLineColorClicked();
    void OnBtnChooseInvertBgColorClicked();

    // Apply current settings.
    void OnApplySettings();

    void OnBtnSaveClicked();

private:
    Ui::SettingsDialog *ui;

    QVector<OverlayScheme::Ptr> m_profiles;
};

#endif // SETTINGSDIALOG_H
