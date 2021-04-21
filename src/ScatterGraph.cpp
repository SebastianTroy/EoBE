#include "ScatterGraph.h"

#include <RangeConverter.h>

#include <QPaintEvent>
#include <QPainter>

#include <cmath>
#include <limits>

ScatterGraph::ScatterGraph(QWidget* parent, QString title, QString xAxisLabel, QString yAxisLabel)
    : QWidget(parent)
    , graphTitle_(title)
    , xRange_()
    , yRange_()
    , xAxisLabel_(xAxisLabel)
    , yAxisLabel_(yAxisLabel)
{
    QPalette p = palette();
    p.setColor(QPalette::ColorRole::Window, QColor(Qt::white));
    setPalette(p);
    setAutoFillBackground(true);
}

void ScatterGraph::SetPoints(std::vector<ScatterGraph::DataPoint>&& points)
{
    points_.swap(points);
    update();
}

void ScatterGraph::SetGraticuleHidden(bool hidden)
{
    graticuleHidden_ = hidden;
    setMouseTracking(!hidden);
    setCursor(hidden ? Qt::CursorShape::ArrowCursor : Qt::CursorShape::CrossCursor);
    update();
}

void ScatterGraph::Reset()
{
    xRange_.Reset();
    yRange_.Reset();
    points_.clear();
    update();
}

void ScatterGraph::RecalculateAxisBounds()
{
    xRange_.Reset();
    yRange_.Reset();
    for (const DataPoint& point : points_) {
        xRange_.ExpandToContain(point.x_);
        yRange_.ExpandToContain(point.y_);
    }
    update();
}

void ScatterGraph::mouseMoveEvent(QMouseEvent* event)
{
    graticuleLocation_ = event->pos();
    update();
}

void ScatterGraph::paintEvent(QPaintEvent* event)
{
    QPainter paint(this);
    paint.setClipRegion(event->region());

    const Tril::MinMax<qreal> xRange = { xAxisMinOverride_ ? xAxisMinOverrideValue_ : xRange_.Min(), xAxisMaxOverride_ ? xAxisMaxOverrideValue_ : xRange_.Max() };
    const Tril::MinMax<qreal> yRange = { yAxisMinOverride_ ? yAxisMinOverrideValue_ : yRange_.Min(), yAxisMaxOverride_ ? yAxisMaxOverrideValue_ : yRange_.Max() };

    const QPointF origin = PaintAxes(paint);
    PaintTitle(paint);

    const qreal xAxisLength = width() - origin.x();
    const qreal yAxisLength = origin.y();

    paint.setClipRegion(event->region().intersected(QRect(QPoint(origin.x(), 0.0), QPoint(width(), yAxisLength))));

    for (const DataPoint& point : points_) {
        paint.setPen(point.colour_);
        paint.setBrush(QColor(point.colour_));
        paint.drawEllipse(QPointF(point.x_, point.y_), 3.0, 3.0);
    }

    if (!graticuleHidden_) {
        PaintGraticule(paint, graticuleLocation_, QRectF(origin.x(), 0, xAxisLength, yAxisLength));
    }
}

QPointF ScatterGraph::PaintAxes(QPainter& p) const
{
    Tril::MinMax<qreal> xRange = { xAxisMinOverride_ ? xAxisMinOverrideValue_ : xRange_.Min(), xAxisMaxOverride_ ? xAxisMaxOverrideValue_ : xRange_.Max() };
    Tril::MinMax<qreal> yRange = { yAxisMinOverride_ ? yAxisMinOverrideValue_ : yRange_.Min(), yAxisMaxOverride_ ? yAxisMaxOverrideValue_ : yRange_.Max() };

    QString yMinLabel = QString("%1").arg(yRange.Min(), 0, 'f', 2);
    QString yMaxLabel = QString("%1").arg(yRange.Max(), 0, 'f', 2);
    QString xMinLabel = QString("%1").arg(xRange.Min(), 0, 'f', 2);
    QString xMaxLabel = QString("%1").arg(xRange.Max(), 0, 'f', 2);

    /*
     * Draw the X and Y min/max values
     */

    QRectF yMinRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, yMinLabel);
    QRectF yMaxRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, yMaxLabel);
    QRectF xMinRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, xMinLabel);
    QRectF xMaxRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, xMaxLabel);

    QRectF xLabelRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, xAxisLabel_);
    QRectF yLabelRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, yAxisLabel_);

    // The bottem left of the graph, aka (minX, minY)
    // TODO (xLabelRect.height() * 2) is lazy, needs to be (xLabelRect.height() + keyHeight)
    QPointF origin(std::max(yMinRect.width(), yMaxRect.width()) + yLabelRect.height(), height() - (xLabelRect.height() * 2));

    yMinRect.moveBottomRight(QPointF(origin.x() - 1, origin.y()));
    yMaxRect.moveTopRight(QPointF(origin.x() - 1, 0.0));
    xMinRect.moveTopLeft(origin);
    xMaxRect.moveTopRight(QPointF(width() - 1, origin.y()));

    p.drawText(yMinRect, Qt::AlignTop | Qt::AlignLeft, yMinLabel);
    p.drawText(yMaxRect, Qt::AlignTop | Qt::AlignLeft, yMaxLabel);
    p.drawText(xMinRect, Qt::AlignTop | Qt::AlignLeft, xMinLabel);
    p.drawText(xMaxRect, Qt::AlignTop | Qt::AlignLeft, xMaxLabel);

    /*
     * Draw the Axis labels & axis lines
     */
    qreal xAxisLength = width() - origin.x();
    qreal yAxisLength = origin.y();

    p.setPen(Qt::black);
    p.drawLine(QLineF(origin, QPointF(origin.x(), 0)));
    p.drawLine(QLineF(origin, QPointF(width(), origin.y())));

    xLabelRect.translate(((xAxisLength - xLabelRect.width()) / 2.0) + origin.x(), height() - (xLabelRect.height() * 2));
    yLabelRect.translate((yLabelRect.height() / 2.0) - (yLabelRect.width() / 2.0), (yAxisLength / 2.0) - (yLabelRect.height() / 2.0));

    p.drawText(xLabelRect, Qt::AlignLeft, xAxisLabel_);
    p.translate(yLabelRect.center());
    yLabelRect.translate(-yLabelRect.center());
    p.rotate(270.0);
    p.drawText(yLabelRect, Qt::AlignLeft, yAxisLabel_);
    p.resetTransform();

    return origin;
}

void ScatterGraph::PaintTitle(QPainter& p) const
{
    p.setPen(Qt::black);

    QRectF titleRect = p.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, graphTitle_);
    titleRect.moveCenter(QPointF(width() / 2.0, height() - (titleRect.height() / 2.0)));
    p.drawText(titleRect, Qt::AlignLeft, graphTitle_);
}

void ScatterGraph::PaintGraticule(QPainter& painter, const QPointF& target, const QRectF& area) const
{
    if (area.contains(target)) {
        // Draw lines inverse so they show up over plots
        painter.save();
        painter.setClipRect(area);
        painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
        painter.setPen(Qt::white);
        painter.drawLines({ QLineF(QPointF(0, target.y()), QPointF(width(), target.y())), QLineF(QPointF(target.x(), 0), QPointF(target.x(), height())) });
        painter.restore();

        // Calculate axis values for point
        Tril::RangeConverter xConverter(Tril::Range(area.left(), area.right()), xRange_);
        Tril::RangeConverter yConverter(Tril::Range(area.bottom(), area.top()), yRange_);

        qreal xValueAtCrosshair = xConverter.Convert(target.x());
        qreal yValueAtCrosshair = yConverter.Convert(target.y());

        // Display axis values
        QString coordsText = QString("(%1, %2)").arg(xValueAtCrosshair).arg(yValueAtCrosshair);

        // Weird bug where half the text doesn't draw without expanding the rect!
        QRectF coordsRect = painter.boundingRect(QRectF(0, 0, 50, 50), Qt::AlignTop | Qt::AlignLeft, coordsText).adjusted(0, 0, 1, 0);

        // Make sure we don't draw them off screen
        bool left = target.x() > width() / 2;
        bool bottom = target.y() < height() / 2;

        if (left) {
            if (bottom) {
                coordsRect.moveTopRight(target);
            } else {
                coordsRect.moveBottomRight(target);
            }
        } else {
            if (bottom) {
                coordsRect.moveTopLeft(target);
            } else {
                coordsRect.moveBottomLeft(target);
            }
        }

        painter.setPen(Qt::black);
        painter.fillRect(coordsRect.adjusted(1, 1, -1, -1), Qt::white);
        painter.drawText(coordsRect, Qt::AlignLeft, coordsText);
    }
}

