#include "AnchorSettings.h"
#include "SettingKeys.h"

#include <QDir>

#define SETTINGS_FILE "MouseLineFocus.ini"
#define PROFILE_SUB_DIR "profiles"

AnchorSettings *AnchorSettings::s_instance = nullptr;

AnchorSettings::AnchorSettings(QObject *parent)
    : QSettings(SETTINGS_FILE, QSettings::IniFormat, parent)
{
    s_instance = this;
}

int AnchorSettings::GetScreenIndex()
{
    return value(GROUP_COMMON "/" COMMON_SCREEN_INDEX, 0).toInt();
}

void AnchorSettings::SetScreenIndex(int index)
{
    setValue(GROUP_COMMON "/" COMMON_SCREEN_INDEX, index);
}

QString AnchorSettings::GetCurrentProfile()
{
    return value(GROUP_COMMON "/" COMMON_CURRENT_PROFILE, "").toString();
}

void AnchorSettings::SetCurrentProfile(QString profileName)
{
    setValue(GROUP_COMMON "/" COMMON_CURRENT_PROFILE, profileName);
}

QVector<OverlayScheme::Ptr> AnchorSettings::GetAllProfiles()
{
    QVector<OverlayScheme::Ptr> profiles;

    // Ensure sub dir exists. Create if not exist.
    QDir dir(PROFILE_SUB_DIR);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Get all files with extension ".dat".
    QStringList files = dir.entryList(QStringList() << "*.dat", QDir::Files);
    for (int i = 0; i != files.size(); ++i) {
        QString fileName = files[i];
        QString filePath = dir.absoluteFilePath(fileName);

        OverlayScheme::Ptr scheme = LoadSchemeFromFile(filePath);
        if (scheme) {
            profiles.push_back(scheme);
        }
    }

    return profiles;
}

// Create or Update profile.
bool AnchorSettings::SaveProfile(OverlayScheme::Ptr scheme)
{
    QString filename = scheme->schemeName + ".dat";

    QDir dir(PROFILE_SUB_DIR);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = dir.absoluteFilePath(filename);

    return SaveSchemeToFile(scheme, filePath);
}

void AnchorSettings::DeleteProfile(QString profileName)
{
    QString filename = profileName + ".dat";

    QDir dir(PROFILE_SUB_DIR);
    if (!dir.exists()) {
        return;
    }
    QString filePath = dir.absoluteFilePath(filename);

    QFile::remove(filePath);
}

bool AnchorSettings::GetEnabled()
{
    return value(GROUP_COMMON "/" COMMON_ENABLED, true).toBool();
}

void AnchorSettings::SetEnabled(bool enabled)
{
    setValue(GROUP_COMMON "/" COMMON_ENABLED, enabled);
}

bool AnchorSettings::GetInverted()
{
    return value(GROUP_COMMON "/" COMMON_INVERTED, false).toBool();
}

void AnchorSettings::SetInverted(bool inverted)
{
    setValue(GROUP_COMMON "/" COMMON_INVERTED, inverted);
}

bool AnchorSettings::GetEnableEdit()
{
    return value(GROUP_COMMON "/" COMMON_ENABLE_EDIT, true).toBool();
}

void AnchorSettings::SetEnableEdit(bool bEnable)
{
    setValue(GROUP_COMMON "/" COMMON_ENABLE_EDIT, bEnable);
}
