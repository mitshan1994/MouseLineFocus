#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "AnchorSettings.h"
#include "GetInputDialog.h"
#include "mylog.h"

#include <QColorDialog>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QTimer>
#include <QtMath>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Hide question mark.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle("Configurations");

    ui->labelScreens->hide();
    ui->comboScreens->hide();

    //ui->btnSave->hide();

    // Connect some signals.
    connect(ui->checkEnableEdit, &QCheckBox::stateChanged,
        this, &SettingsDialog::OnEnableEditChanged);
    connect(ui->comboProfiles, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &SettingsDialog::OnProfileCurrentIndexChanged);

    connect(ui->btnHLineColor, &QPushButton::clicked, 
        this, &SettingsDialog::OnBtnChooseHLineColorClicked);
    connect(ui->btnVLineColor, &QPushButton::clicked,
        this, &SettingsDialog::OnBtnChooseVLineColorClicked);
    connect(ui->btnInvertBgColor, &QPushButton::clicked,
        this, &SettingsDialog::OnBtnChooseInvertBgColorClicked);

    connect(ui->btnCreateProfile, &QPushButton::clicked, 
        this, &SettingsDialog::OnBtnCreateProfileClicked);
    connect(ui->btnDeleteProfile, &QPushButton::clicked,
        this, &SettingsDialog::OnBtnDeleteProfileClicked);
    connect(ui->btnApply, &QPushButton::clicked, this, &SettingsDialog::OnApplySettings);
    connect(ui->btnSave, &QPushButton::clicked, this, &SettingsDialog::OnBtnSaveClicked);

    RefreshScreenList();

    // Connect some signals after UI is updated.
    connect(ui->comboScreens, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &SettingsDialog::OnScreenCurrentIndexChanged);
    connect(ui->checkEnabled, &QCheckBox::stateChanged,
        this, &SettingsDialog::OnEnabled);
    connect(ui->checkInverted, &QCheckBox::stateChanged,
        this, &SettingsDialog::OnEnableInvertedMode);

    // Update UI after the constructor, for MainWindow to connect signals.
    QTimer::singleShot(0, this, &SettingsDialog::UpdateUI);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::SetOverlayEnabledUI(bool bEnabled)
{
    ui->checkEnabled->blockSignals(true);
    ui->checkEnabled->setChecked(bEnabled);
    ui->checkEnabled->blockSignals(false);

    SaveEnabled(bEnabled);
}

void SettingsDialog::SetOverlayInvertedUI(bool bInverted)
{
    ui->checkInverted->blockSignals(true);
    ui->checkInverted->setChecked(bInverted);
    ui->checkInverted->blockSignals(false);

    SaveInverted(bInverted);
}

void SettingsDialog::SetCurrentProfile(QString profileName)
{
    int index = ui->comboProfiles->findText(profileName);
    if (index == -1) {
        L_WARN("profile name not found: {}", profileName);
        return;
    }

    ui->comboProfiles->setCurrentIndex(index);
}

void SettingsDialog::hideEvent(QHideEvent *event)
{
    emit SigDialogHided();
}

void SettingsDialog::RefreshScreenList()
{
    QList<QScreen *> screens = QGuiApplication::screens();

    ui->comboScreens->clear();

    for (int i = 0; i != screens.size(); ++i) {
        QString screenName = QString("%1 (%2x%3)")
                .arg(screens[i]->name())
                .arg(screens[i]->size().width())
                .arg(screens[i]->size().height());

        ui->comboScreens->addItem(screenName);
    }
}

void SettingsDialog::UpdateUI()
{
    AnchorSettings *settings = AnchorSettings::Instance();

    // Update screen index.
    int screenIndex = settings->GetScreenIndex();
    if (screenIndex >= 0 && screenIndex < ui->comboScreens->count()) {
        ui->comboScreens->setCurrentIndex(screenIndex);
    }
    emit SigScreenChanged(screenIndex);

    // Update global settings.
    bool bEnabled = settings->GetEnabled();
    bool bInverted = settings->GetInverted();

    // Emit here to notify MainWindow.
    emit SigOverlayEnabled(bEnabled);
    emit SigOverlayInverted(bInverted);

    bool bEnableEdit = settings->GetEnableEdit();
    OnEnableEditChanged(-1);

    ui->checkEnabled->setChecked(bEnabled);
    ui->checkInverted->setChecked(bInverted);
    ui->checkEnableEdit->setChecked(bEnableEdit);

    ReadAllProfiles();

    RefreshProfileList();

    // Make height minimum.
    resize(width(), 100);
}

void SettingsDialog::ReadAllProfiles()
{
    AnchorSettings *settings = AnchorSettings::Instance();

    // Get all profiles.
    QVector<OverlayScheme::Ptr> profiles = settings->GetAllProfiles();

    m_profiles = profiles;
}

void SettingsDialog::RefreshProfileList()
{
    AnchorSettings *settings = AnchorSettings::Instance();

    ui->comboProfiles->blockSignals(true);

    // Update current profiles.
    QString currentProfile = settings->GetCurrentProfile();
    int currentProfileIndex = -1;
    for (int i = 0; i != m_profiles.size(); ++i) {
        OverlayScheme::Ptr scheme = m_profiles[i];

        ui->comboProfiles->addItem(scheme->schemeName);

        if (scheme->schemeName == currentProfile) {
            currentProfileIndex = i;
        }
    }
    
    // Default to first profile if no current profile found.
    if (currentProfileIndex < 0 && m_profiles.size() > 0) {
        currentProfileIndex = 0;
    }

    ui->comboProfiles->setCurrentIndex(currentProfileIndex);

    ui->comboProfiles->blockSignals(false);

    if (currentProfileIndex == -1) {
        L_WARN("current profile not found: {}", currentProfile);
        return;
    }

    EmitProfilesUpdate();

    // Emit current profile for first time.
    emit SigOverlaySchemeChanged(GetCurrentProfile());

    UpdateCurrentProfileToUI();
}

void SettingsDialog::UpdateCurrentProfileToUI()
{
    AnchorSettings *settings = AnchorSettings::Instance();

    OverlayScheme::Ptr scheme = GetCurrentProfile();
    if (!scheme) {
        L_ERROR("current profile not found");
        return;
    }

    qreal opacity;
    int opacityInt;

    // Update.
    ui->checkEnableHLine->setChecked(scheme->bEnableHLine);
    ui->spinHLineThick->setValue(scheme->hLineWidth);
    ui->btnHLineColor->setStyleSheet(QString("background-color: %1").arg(scheme->hLineColor.name()));
    opacity = 1.0 * scheme->hLineColor.alpha() * 100 / 255;
    opacityInt = qFloor(opacity + 0.5);
    ui->spinHLineOpacity->setValue(opacityInt);

    ui->checkEnableVLine->setChecked(scheme->bEnableVLine);
    ui->spinVLineThick->setValue(scheme->vLineWidth);
    ui->btnVLineColor->setStyleSheet(QString("background-color: %1").arg(scheme->vLineColor.name()));
    opacity = 1.0 * scheme->vLineColor.alpha() * 100 / 255;
    opacityInt = qFloor(opacity + 0.5);
    ui->spinVLineOpacity->setValue(opacityInt);

    ui->btnInvertBgColor->setStyleSheet(QString("background-color: %1").arg(scheme->invertedBgColor.name()));
    opacity = 1.0 * scheme->invertedBgColor.alpha() * 100 / 255;
    opacityInt = qFloor(opacity + 0.5);
    ui->spinInvertBgOpacity->setValue(opacityInt);
}

OverlayScheme::Ptr SettingsDialog::GetProfileByName(QString profileName)
{
    for (int i = 0; i != m_profiles.size(); ++i) {
        OverlayScheme::Ptr scheme = m_profiles[i];
        if (scheme->schemeName == profileName) {
            return scheme;
        }
    }

    return nullptr;
}

OverlayScheme::Ptr SettingsDialog::GetCurrentProfile()
{
    QString currentProfile = ui->comboProfiles->currentText();
    return GetProfileByName(currentProfile);
}

int SettingsDialog::GetCurrentProfileIndex()
{
    return ui->comboProfiles->currentIndex();
}

OverlayScheme::Ptr SettingsDialog::GetCurrentProfileFromUI()
{
    OverlayScheme::Ptr scheme = std::make_shared<OverlayScheme>();

    qreal opacity;
    int opacityInt;

    scheme->bEnableHLine = ui->checkEnableHLine->isChecked();
    scheme->hLineWidth = ui->spinHLineThick->value();
    scheme->hLineColor = ui->btnHLineColor->palette().color(QPalette::Button);
    opacity = 1.0 * ui->spinHLineOpacity->value() * 255 / 100;
    opacityInt = qFloor(opacity + 0.5);
    scheme->hLineColor.setAlpha(opacityInt);

    scheme->bEnableVLine = ui->checkEnableVLine->isChecked();
    scheme->vLineWidth = ui->spinVLineThick->value();
    scheme->vLineColor = ui->btnVLineColor->palette().color(QPalette::Button);
    opacity = 1.0 * ui->spinVLineOpacity->value() * 255 / 100;
    opacityInt = qFloor(opacity + 0.5);
    scheme->vLineColor.setAlpha(opacityInt);

    scheme->invertedBgColor = ui->btnInvertBgColor->palette().color(QPalette::Button);
    opacity = 1.0 * ui->spinInvertBgOpacity->value() * 255 / 100;
    opacityInt = qFloor(opacity + 0.5);
    scheme->invertedBgColor.setAlpha(opacityInt);

    return scheme;
}

void SettingsDialog::SaveEnabled(bool bEnabled)
{
    AnchorSettings *settings = AnchorSettings::Instance();
    settings->SetEnabled(bEnabled);
}

void SettingsDialog::SaveInverted(bool bInverted)
{
    AnchorSettings *settings = AnchorSettings::Instance();
    settings->SetInverted(bInverted);
}

void SettingsDialog::EmitProfilesUpdate()
{
    emit SigProfilesUpdated(m_profiles);
}

void SettingsDialog::OnProfileCurrentIndexChanged(int index)
{
    QString profileName = ui->comboProfiles->itemText(index);

    L_TRACE("current profile index changed. index: {}, profile: {}",
        index, profileName);

    // Save current profile name.
    AnchorSettings *settings = AnchorSettings::Instance();
    settings->SetCurrentProfile(profileName);

    // Update UI.
    UpdateCurrentProfileToUI();

    emit SigOverlaySchemeChanged(GetCurrentProfile());
}

void SettingsDialog::OnScreenCurrentIndexChanged(int index)
{
    // Save to settings.
    AnchorSettings *settings = AnchorSettings::Instance();
    settings->SetScreenIndex(index);

    // Move to the selected screen.
    emit SigScreenChanged(index);
}

void SettingsDialog::OnEnabled(int state)
{
    bool bEnabled;
    if (state == Qt::Checked) {
        bEnabled = true;
    } else {
        bEnabled = false;
    }
    emit SigOverlayEnabled(bEnabled);

    // Save to settings.
    SaveEnabled(bEnabled);
}

void SettingsDialog::OnEnableInvertedMode(int state)
{
    bool bEnabled;
    if (state == Qt::Checked) {
        bEnabled = true;
    } else {
        bEnabled = false;
    }
    emit SigOverlayInverted(bEnabled);

    // Save to settings.
    SaveInverted(bEnabled);
}

void SettingsDialog::OnEnableEditChanged(int /*state*/)
{
    bool bEnabled = ui->checkEnableEdit->isChecked();

    ui->groupHorizontal->setEnabled(bEnabled);
    ui->groupVertical->setEnabled(bEnabled);
    ui->groupInverted->setEnabled(bEnabled);

    // Save to settings.
    AnchorSettings *settings = AnchorSettings::Instance();
    settings->SetEnableEdit(bEnabled);

    //ui->comboScreens->setEnabled(bEnabled);
    //ui->checkEnabled->setEnabled(bEnabled);
    //ui->checkInverted->setEnabled(bEnabled);

    //ui->comboProfiles->setEnabled(bEnabled);
    //ui->btnCreateProfile->setEnabled(bEnabled);
    //ui->btnDeleteProfile->setEnabled(bEnabled);

    //ui->checkEnableHLine->setEnabled(bEnabled);
    //ui->spinHLineThick->setEnabled(bEnabled);
    //ui->btnHLineColor->setEnabled(bEnabled);
    //ui->spinHLineOpacity->setEnabled(bEnabled);

    //ui->checkEnableVLine->setEnabled(bEnabled);
    //ui->spinVLineThick->setEnabled(bEnabled);
    //ui->btnVLineColor->setEnabled(bEnabled);
    //ui->spinVLineOpacity->setEnabled(bEnabled);

    //ui->btnInvertBgColor->setEnabled(bEnabled);
    //ui->spinInvertBgOpacity->setEnabled(bEnabled);

    //ui->btnApply->setEnabled(bEnabled);
    //ui->btnSave->setEnabled(bEnabled);
}

void SettingsDialog::OnBtnCreateProfileClicked()
{
    AnchorSettings *settings = AnchorSettings::Instance();

    GetInputDialog dialog(this);
    dialog.setWindowTitle("Input new profile name");

    if (dialog.exec() != QDialog::Accepted) {
        L_TRACE("create profile dialog canceled.");
        return;
    }

    QString profileName = dialog.GetInput();
    L_INFO("new profile name: {}", profileName);

    if (profileName.isEmpty()) {
        L_WARN("profile name is empty.");

        QMessageBox::warning(this, "Warning", 
            "Profile name cannot be empty! Please create with a non-empty name.");  
        return;
    }

    // Check if the profile name already exists.
    if (GetProfileByName(profileName)) {
        L_WARN("profile name already exists.");

        QMessageBox::warning(this, "Warning", 
            "Profile name already exists! Please create with another name.");  
        return;
    }

    // Create a new profile.
    OverlayScheme::Ptr scheme = std::make_shared<OverlayScheme>();
    scheme->schemeName = profileName;
    
    // Save to settings.
    if (!settings->SaveProfile(scheme)) {
        L_ERROR("save profile failed.");
        QMessageBox::warning(this, "Warning", "Failed to save profile!");  
        return;
    }

    // Add to m_profiles.
    m_profiles.push_back(scheme);

    // Add to combo box.
    ui->comboProfiles->addItem(profileName);

    EmitProfilesUpdate();

    // Set current profile.
    ui->comboProfiles->setCurrentIndex(ui->comboProfiles->count() - 1);

    L_INFO("create profile success: {}", profileName);

    QString info = QString("Profile \"%1\" created.").arg(profileName);
    QMessageBox::information(this, "Information", info);
}

void SettingsDialog::OnBtnDeleteProfileClicked()
{
    // Get current profile name.
    QString currentProfileName = ui->comboProfiles->currentText();

    if (currentProfileName.isEmpty()) {
        L_WARN("current profile name is empty.");
        QMessageBox::warning(this, "Warning", "You need to select a valid profile first to delete.");
        return;
    }

    // Confirm.
    QString confirmInfo = QString("Are you sure to delete profile \"%1\"?").arg(currentProfileName);
    QMessageBox::StandardButton ret = QMessageBox::question(this, "Confirm", confirmInfo);
    if (ret != QMessageBox::Yes) {
        L_TRACE("delete profile canceled.");
        return;
    }

    // Delete from settings.
    AnchorSettings *settings = AnchorSettings::Instance();
    settings->DeleteProfile(currentProfileName);

    // Delete from m_profiles.
    for (int i = 0; i != m_profiles.size(); ++i) {
        OverlayScheme::Ptr scheme = m_profiles[i];
        if (scheme->schemeName == currentProfileName) {
            m_profiles.remove(i);
            break;
        }
    }

    EmitProfilesUpdate();

    // Delete from combo box.
    ui->comboProfiles->removeItem(ui->comboProfiles->currentIndex());

    L_INFO("delete profile success: {}", currentProfileName);

    QString info = QString("Profile \"%1\" deleted.").arg(currentProfileName);
    QMessageBox::information(this, "Information", info);
}

void SettingsDialog::OnBtnChooseHLineColorClicked()
{
    QColorDialog colorDialog(this);
    colorDialog.setWindowTitle("Choose horizontal line color");

    // Get current color on widget.
    QColor currentColor = ui->btnHLineColor->palette().color(QPalette::Button);
    colorDialog.setCurrentColor(currentColor);

    if (colorDialog.exec() != QDialog::Accepted) {
        L_TRACE("Choose horizontal line color canceled.");
        return;
    }

    QColor color = colorDialog.selectedColor();
    L_INFO("Horizontal line color chosen: {}", color.name());

    ui->btnHLineColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
}

void SettingsDialog::OnBtnChooseVLineColorClicked()
{
    QColorDialog colorDialog(this);
    colorDialog.setWindowTitle("Choose vertical line color");

    // Get current color on widget.
    QColor currentColor = ui->btnVLineColor->palette().color(QPalette::Button);
    colorDialog.setCurrentColor(currentColor);

    if (colorDialog.exec() != QDialog::Accepted) {
        L_TRACE("Choose vertical line color canceled.");
        return;
    }

    QColor color = colorDialog.selectedColor();
    L_INFO("Vertical line color chosen: {}", color.name());

    ui->btnVLineColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
}

void SettingsDialog::OnBtnChooseInvertBgColorClicked()
{
    QColorDialog colorDialog(this);
    colorDialog.setWindowTitle("Choose inverted background color");

    // Get current color on widget.
    QColor currentColor = ui->btnInvertBgColor->palette().color(QPalette::Button);
    colorDialog.setCurrentColor(currentColor);

    if (colorDialog.exec() != QDialog::Accepted) {
        L_TRACE("Choose inverted background color canceled.");
        return;
    }

    QColor color = colorDialog.selectedColor();
    L_INFO("Inverted background color chosen: {}", color.name());

    ui->btnInvertBgColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
}

void SettingsDialog::OnApplySettings()
{
    AnchorSettings *settings = AnchorSettings::Instance();
    OverlayScheme::Ptr scheme = GetCurrentProfileFromUI();

    Q_ASSERT(scheme != nullptr);

    QString currentProfileName = ui->comboProfiles->currentText();
    if (currentProfileName.isEmpty()) {
        L_WARN("current profile name is empty.");
        QMessageBox::warning(this, "Warning", "Please create a new profile first!");
        return;
    }

    scheme->schemeName = currentProfileName;

    if (!settings->SaveProfile(scheme)) {
        L_ERROR("Save profile failed. Current profile name: {}", scheme->schemeName);
        QMessageBox::warning(this, "Warning", "Failed to save profile!");
        return;
    }

    // Update m_profiles.
    int currentProfileIndex = GetCurrentProfileIndex();
    m_profiles[currentProfileIndex] = scheme;

    emit SigOverlaySchemeChanged(scheme);
}

void SettingsDialog::OnBtnSaveClicked()
{
    OnApplySettings();

    close();
}
