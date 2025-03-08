#include "OverlayWidget.h"
#include "ui_OverlayWidget.h"

#include "mylog/mylog.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTimer>

OverlayWidget::OverlayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverlayWidget)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog | Qt::Tool);
    setWindowFlag(Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_TranslucentBackground);

    //setAttribute(Qt::WA_ShowWithoutActivating);
    //setMouseTracking(true);

    connect(&m_timerRefresh, &QTimer::timeout,
        this, &OverlayWidget::OnTimerRefreshTimeout);
    m_timerRefresh.setSingleShot(false);
    m_timerRefresh.start(1000 / 60);

    // Create a default overlay scheme.
    m_scheme = std::make_shared<OverlayScheme>();
}

OverlayWidget::~OverlayWidget()
{
    delete ui;
}

void OverlayWidget::SetBackgroundColor(QString bgColor)
{
    //// Set background color using qt stylesheet.
    //QString style = QString("background-color: %1;").arg(bgColor);
    //setStyleSheet(style);
}

void OverlayWidget::SetEnabled(bool bEnabled)
{
    m_bEnabled = bEnabled;

    update();
}

void OverlayWidget::SetInverted(bool bInverted)
{
    m_bInverted = bInverted;

    update();
}

void OverlayWidget::SetOverlayScheme(OverlayScheme::Ptr pOverlayScheme)
{
    m_scheme = std::make_shared<OverlayScheme>(*pOverlayScheme);
    //m_scheme = pOverlayScheme;

    update();
}

void OverlayWidget::ToggleHLine()
{
    m_scheme->bEnableHLine = !m_scheme->bEnableHLine;

    update();
}

void OverlayWidget::ToggleVLine()
{
    m_scheme->bEnableVLine = !m_scheme->bEnableVLine;

    update();
}

void OverlayWidget::paintEvent(QPaintEvent *event)
{
    //L_TRACE("paintEvent. mouse pos: ({},{})", m_mousePos.x(), m_mousePos.y());

    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (!m_bEnabled) {
        return;
    }

    int w = width();
    int h = height();

    DrawTwoLines(painter);
    DrawBgRectangles(painter);
}

void OverlayWidget::mouseMoveEvent(QMouseEvent *event)
{
    L_TRACE("mouseMoveEvent: ({}, {})", event->x(), event->y());
}

void OverlayWidget::DrawTwoLines(QPainter &painter)
{
    if (m_bInverted) {
        return;
    }

    int w = width();
    int h = height();

    if (m_scheme->bEnableHLine && m_scheme->hLineColor.alpha() != 0) {
        QColor color = m_scheme->hLineColor;
        QBrush brush(color);
        painter.setPen(color);
        painter.setBrush(brush);

        if (m_scheme->hLineWidth == 1) {
            DrawHorizontalLine(painter);
        }
        else if (m_scheme->hLineWidth > 1) {
            DrawHorizontalRect(painter);
        }
        else {
            // No drawing.
        }
    }

    if (m_scheme->bEnableVLine && m_scheme->vLineColor.alpha() != 0) {
        QColor color = m_scheme->vLineColor;
        QBrush brush(color);
        painter.setPen(color);
        painter.setBrush(brush);

        if (m_scheme->vLineWidth == 1) {
            DrawVerticalLine(painter);
        }
        else if (m_scheme->vLineWidth > 1) {
            DrawVerticalRect(painter);
        }
        else {
            // No drawing.
        }
    }
}

void OverlayWidget::DrawBgRectangles(QPainter &painter)
{
    if (!m_bInverted) {
        return;
    }

    if (m_scheme->invertedBgColor.alpha() == 0) {
        return;
    }

    QBrush brush(m_scheme->invertedBgColor);
    painter.setBrush(brush);

    QPen oldPen = painter.pen();
    painter.setPen(Qt::NoPen);

    int w = width();
    int h = height();

    int vLineWidth = m_scheme->vLineWidth;
    int hLineWidth = m_scheme->hLineWidth;

    if (!m_scheme->bEnableHLine) {
        hLineWidth = 0;
    }
    if (!m_scheme->bEnableVLine) {
        vLineWidth = 0;
    }

    int vLeftWidth = vLineWidth / 2;
    int vRightWidth = vLineWidth - vLeftWidth;
    int hUpWidth = hLineWidth / 2;
    int hDownWidth = hLineWidth - hUpWidth;

    // Draw four rectangles.
    {
        int startX = 0;
        int startY = 0;
        int rectWidth = m_mousePos.x() - vLeftWidth;
        int rectHeight = m_mousePos.y() - hUpWidth;
        painter.drawRect(startX, startY, rectWidth, rectHeight);
    }
    {
        int startX = m_mousePos.x() + vRightWidth;
        int startY = 0;
        int rectWidth = w - startX;
        int rectHeight = m_mousePos.y() - hUpWidth;
        painter.drawRect(startX, startY, rectWidth, rectHeight);
    }
    {
        int startX = 0;
        int startY = m_mousePos.y() + hDownWidth;
        int rectWidth = m_mousePos.x() - vLeftWidth;
        int rectHeight = h - startY;
        painter.drawRect(startX, startY, rectWidth, rectHeight);
    }
    {
        int startX = m_mousePos.x() + vRightWidth;
        int startY = m_mousePos.y() + hDownWidth;
        int rectWidth = w - startX;
        int rectHeight = h - startY;
        painter.drawRect(startX, startY, rectWidth, rectHeight);
    }

    painter.setPen(oldPen);
}

void OverlayWidget::DrawHorizontalLine(QPainter &painter)
{
    Q_ASSERT(m_scheme->hLineWidth == 1);

    int w = width();
    int vLineWidth = GetVertialLineWidth();

    if (vLineWidth <= 1) {
        painter.drawLine(0, m_mousePos.y(), w, m_mousePos.y());
    } else {
        // Draw two line segments.
        painter.drawLine(0, m_mousePos.y(), m_mousePos.x() - vLineWidth / 2 - 1, m_mousePos.y());
        painter.drawLine(m_mousePos.x() + vLineWidth / 2, m_mousePos.y(), w, m_mousePos.y());
    }
}

void OverlayWidget::DrawHorizontalRect(QPainter &painter)
{
    Q_ASSERT(m_scheme->hLineWidth > 1);

    int w = width();
    int vLineWidth = GetVertialLineWidth();

    QPen oldPen = painter.pen();
    painter.setPen(Qt::NoPen);

    int startX = 0;
    int startY = m_mousePos.y() - m_scheme->hLineWidth / 2;
    int rectWidth = w;
    int rectHeight = m_scheme->hLineWidth;
    painter.drawRect(startX, startY, rectWidth, rectHeight);

    painter.setPen(oldPen);
}

void OverlayWidget::DrawVerticalLine(QPainter &painter)
{
    Q_ASSERT(m_scheme->vLineWidth == 1);

    int h = height();
    int hLineWidth = GetHorizontalLineWidth();

    if (hLineWidth <= 1 || !m_scheme->bEnableHLine) {
        painter.drawLine(m_mousePos.x(), 0, m_mousePos.x(), h);
    } else {
        // Draw two line segments.
        int upperY = m_mousePos.y() - hLineWidth / 2 - 1;
        painter.drawLine(m_mousePos.x(), 0, m_mousePos.x(), upperY);
        painter.drawLine(m_mousePos.x(), upperY + hLineWidth, m_mousePos.x(), h);
    }
}

void OverlayWidget::DrawVerticalRect(QPainter &painter)
{
    Q_ASSERT(m_scheme->vLineWidth > 1);

    int h = height();
    int hLineWidth = GetHorizontalLineWidth();

    QPen oldPen = painter.pen();
    painter.setPen(Qt::NoPen);

    if (hLineWidth <= 1 || !m_scheme->bEnableHLine) {
        int startX = m_mousePos.x() - m_scheme->vLineWidth / 2;
        int startY = 0;
        int rectWidth = m_scheme->vLineWidth;
        int rectHeight = h;
        painter.drawRect(startX, startY, rectWidth, rectHeight);
    } else {
        // Draw two rectangles.
        int upperY = m_mousePos.y() - hLineWidth / 2;
        {
            int startX = m_mousePos.x() - m_scheme->vLineWidth / 2;
            int startY = 0;
            int rectWidth = m_scheme->vLineWidth;
            int rectHeight = upperY;
            painter.drawRect(startX, startY, rectWidth, rectHeight);
        }
        {
            int startX = m_mousePos.x() - m_scheme->vLineWidth / 2;
            int startY = upperY + hLineWidth;
            int rectWidth = m_scheme->vLineWidth;
            int rectHeight = h - startY;
            painter.drawRect(startX, startY, rectWidth, rectHeight);
        }
    }

    painter.setPen(oldPen);
}

int OverlayWidget::GetHorizontalLineWidth()
{
    if (m_scheme->hLineColor.alpha() == 0)
        return 0;
    return m_scheme->hLineWidth;
}

int OverlayWidget::GetVertialLineWidth()
{
    if (m_scheme->vLineColor.alpha() == 0)
        return 0;
    return m_scheme->vLineWidth;
}

void OverlayWidget::OnTimerRefreshTimeout()
{
    // Get current mouse position.
    QPoint pos = QCursor::pos();
    if (pos == m_mousePos) {
        return;
    }

    m_mousePos = mapFromGlobal(pos);

    update();
}
