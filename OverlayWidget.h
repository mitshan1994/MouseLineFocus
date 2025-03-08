#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include "OverlayScheme.h"

#include <QPainter>
#include <QPoint>
#include <QTimer>
#include <QWidget>

namespace Ui {
class OverlayWidget;
}

class OverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayWidget(QWidget *parent = nullptr);
    ~OverlayWidget();

    // Set background color.
    void SetBackgroundColor(QString bgColor);

    // Enable/disable the functions.
    void SetEnabled(bool bEnabled);

    // Set inverted or not.
    void SetInverted(bool bInverted);

    void SetOverlayScheme(OverlayScheme::Ptr pOverlayScheme);

    // Toggle H/V line.
    void ToggleHLine();
    void ToggleVLine();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    /// Some painting functions.
    void DrawTwoLines(QPainter &painter);
    void DrawBgRectangles(QPainter &painter);

    void DrawHorizontalLine(QPainter &painter);
    void DrawHorizontalRect(QPainter &painter);

    void DrawVerticalLine(QPainter &painter);
    void DrawVerticalRect(QPainter &painter);

    // Return non-transparent line width. 0 if transparent.
    int GetHorizontalLineWidth();
    int GetVertialLineWidth();

private slots:
    void OnTimerRefreshTimeout();

private:
    Ui::OverlayWidget *ui;

    bool m_bEnabled = true;
    bool m_bInverted = false;

    QTimer m_timerRefresh;
    QPoint m_mousePos;

    OverlayScheme::Ptr m_scheme;
};

#endif // OVERLAYWIDGET_H
