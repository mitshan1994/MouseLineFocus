#pragma once

#include <QColor>
#include <QDataStream>
#include <QFile>
#include <QString>
#include <memory>

#define SCHEME_MAGIC_NUMBER 0x202309

// Represent a scheme for overlaying lines and background.
struct OverlayScheme {
    using Ptr = std::shared_ptr<OverlayScheme>;

    int version;        // Structure version.

    QString schemeName;

    bool bEnableHLine;
    int hLineWidth;     // Horizontal line width.
    QColor hLineColor;  // Horizontal line color.

    bool bEnableVLine;
    int vLineWidth;     // Vertical line width.
    QColor vLineColor;  // Vertical line color.

    // Inverted mode.
    QColor invertedBgColor;

    // Constructor.
    OverlayScheme() {
        schemeName = "";

        bEnableHLine = true;
        hLineWidth = 25;
        hLineColor = QColor{ 0, 255, 0, 51 };

        bEnableVLine = true;
        vLineWidth = 25;
        vLineColor = QColor{ 0, 255, 0, 51 };

        invertedBgColor = QColor{ 0, 255, 0, 51 };

        version = 1000;
    }
};

// Load scheme from file.
inline OverlayScheme::Ptr LoadSchemeFromFile(QString filePath)
{
    OverlayScheme::Ptr scheme = std::make_shared<OverlayScheme>();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);
    int magicNumber;
    
    stream
        >> magicNumber
        >> scheme->version
        >> scheme->schemeName
        >> scheme->bEnableHLine >> scheme->hLineWidth >> scheme->hLineColor
        >> scheme->bEnableVLine >> scheme->vLineWidth >> scheme->vLineColor
        >> scheme->invertedBgColor
        ;

    if (magicNumber != SCHEME_MAGIC_NUMBER) {
        return nullptr;
    }

    return scheme;
}

// Save scheme to file. Overwrite if file exists.
inline bool SaveSchemeToFile(OverlayScheme::Ptr scheme, QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);

    stream
        << (int)SCHEME_MAGIC_NUMBER
        << scheme->version
        << scheme->schemeName
        << scheme->bEnableHLine << scheme->hLineWidth << scheme->hLineColor
        << scheme->bEnableVLine << scheme->vLineWidth << scheme->vLineColor
        << scheme->invertedBgColor
        ;

    return true;
}
