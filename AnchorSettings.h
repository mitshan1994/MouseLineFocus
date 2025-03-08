#ifndef ANCHORSETTINGS_H
#define ANCHORSETTINGS_H

#include "OverlayScheme.h"

#include <QSettings>
#include <QVector>

// Fake singleton. Rely on main() to initialize it.
class AnchorSettings : public QSettings
{
    Q_OBJECT
public:
    static AnchorSettings *Instance() { return s_instance; }

    AnchorSettings(QObject *parent = nullptr);

    // Get stored screen index. Default: 0
    int GetScreenIndex();
    void SetScreenIndex(int index);

    // Last profile name.
    QString GetCurrentProfile();
    void SetCurrentProfile(QString profileName);

    // Get all profiles from settings.
    QVector<OverlayScheme::Ptr> GetAllProfiles();
    bool SaveProfile(OverlayScheme::Ptr scheme);

    // Delete profile with name.
    void DeleteProfile(QString profileName);

    // Whether the application is enabled.
    bool GetEnabled();
    void SetEnabled(bool enabled);

    bool GetInverted();
    void SetInverted(bool inverted);

    bool GetEnableEdit();
    void SetEnableEdit(bool bEnable);

private:
    static AnchorSettings *s_instance;
};

#endif // ANCHORSETTINGS_H
