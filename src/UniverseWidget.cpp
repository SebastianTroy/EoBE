#include "UniverseWidget.h"

#include "Trilobyte.h"
#include "FoodPellet.h"
#include "Egg.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPalette>

#include <chrono>

UniverseWidget::UniverseWidget(QWidget* parent)
    : QWidget(parent)
    , renderThread_(this)
    , tickDurationStats_(100)
    , paintDurationStats_(100)
    , tickRateStats_(30)
    , paintRateStats_(30)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(QPalette::ColorRole::Window, QColor(200, 255, 255));
    setPalette(p);

    renderThread_.setSingleShot(false);
    renderThread_.setTimerType(Qt::PreciseTimer);
    renderThread_.connect(&renderThread_, &QTimer::timeout, this, &UniverseWidget::OnPaintTimerElapsed, Qt::QueuedConnection);

    tickThread_.setSingleShot(false);
    tickThread_.setTimerType(Qt::PreciseTimer);
    tickThread_.connect(&tickThread_, &QTimer::timeout, this, &UniverseWidget::OnTickTimerElapsed, Qt::QueuedConnection);

    SetTpsTarget(40);
    SetFpsTarget(40);
}

UniverseWidget::~UniverseWidget()
{
}

void UniverseWidget::SetUniverse(std::shared_ptr<Universe> universe)
{
    universe_ = universe;
    emit EntitySelected(nullptr);
}

void UniverseWidget::SetFpsTarget(double fps)
{
    if (fps <= 0.0) {
        renderThread_.stop();
    } else if (renderThread_.isActive()) {
        renderThread_.setInterval(1000.0 / fps);
    } else {
        renderThread_.start(1000.0 / fps);
    }
}

void UniverseWidget::SetTpsTarget(double tps)
{
    ticksPerSecondTarget_ = tps;
    UpdateTps();
}

void UniverseWidget::SetLimitTickRate(bool limit)
{
    limitTickRate_ = limit;
    UpdateTps();
}

void UniverseWidget::SetTicksPaused(bool paused)
{
    ticksPaused_ = paused;
    UpdateTps();
}

void UniverseWidget::StepForwards(unsigned ticksToStep)
{
    bool tickThreadWasRunning = tickThread_.isActive();
    bool renderThreadWasRunning = renderThread_.isActive();
    tickThread_.stop();
    renderThread_.stop();

    for (unsigned i = 0; i < ticksToStep; ++i) {
        Tick();
    }
    update();

    if (tickThreadWasRunning) {
        tickThread_.start();
    }
    if (renderThreadWasRunning) {
        renderThread_.start();
    }
}

void UniverseWidget::SelectFittestTrilobyte()
{
    unsigned mostLivingChildren = 0;
    universe_->ForEach([&](const std::shared_ptr<Entity>& e)
    {
        if (const auto* s = dynamic_cast<const Trilobyte*>(e.get())) {
            if (s->GetLivingDescendantsCount(2) > mostLivingChildren) {
                mostLivingChildren = s->GetLivingDescendantsCount(2);
                selectedEntity_ = e;
            }
        }
    });
    emit EntitySelected(selectedEntity_);
}

void UniverseWidget::RemoveAllTrilobytes()
{
    universe_->ClearAllEntitiesOfType<Trilobyte, Egg>();
}

void UniverseWidget::RemoveAllFood()
{
    universe_->ClearAllEntitiesOfType<FoodPellet>();
}

void UniverseWidget::ZoomIn()
{
    transformScale_ *= 1.0 + 0.01;
    update();
}

void UniverseWidget::ZoomOut()
{
    transformScale_ *= 1.0 - 0.01;
    update();
}

void UniverseWidget::ZoomReset()
{
    transformScale_ = 1.0;
    update();
}

void UniverseWidget::PanReset()
{
    transformX_ = transformY_ = 0.0;
    update();
}

void UniverseWidget::wheelEvent(QWheelEvent* event)
{
    double d = 1.0 + (0.001 * double(event->angleDelta().y()));
    transformScale_ *= d;
    update();
}

void UniverseWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && draggedEntity_) {
        universe_->AddEntity(draggedEntity_);
        draggedEntity_ = nullptr;
        update();
    } else if (event->button() == Qt::MiddleButton) {
        dragging_ = false;
    }
}

void UniverseWidget::mousePressEvent(QMouseEvent* event)
{
    if (universe_) {
        Point simLocation = TransformLocalToSimCoords({ static_cast<double>(event->pos().x()), static_cast<double>(event->pos().y()) });
        if (event->button() == Qt::RightButton) {
            selectedEntity_ = universe_->PickEntity(simLocation, false);
            emit EntitySelected(selectedEntity_);
        } else if (event->button() == Qt::LeftButton) {
            draggedEntity_ = universe_->PickEntity(simLocation, true);
        } else if (event->button() == Qt::MiddleButton) {
            dragging_ = true;
            dragX_ = event->pos().x();
            dragY_ = event->pos().y();
        }
    }
}

void UniverseWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging_) {
        transformX_ += ((event->pos().x() - dragX_) / transformScale_);
        transformY_ += ((event->pos().y() - dragY_) / transformScale_);
        dragX_ = event->pos().x();
        dragY_ = event->pos().y();
        update();
    } else if (draggedEntity_) {
        Point simLocation = TransformLocalToSimCoords({ static_cast<double>(event->pos().x()), static_cast<double>(event->pos().y()) });
        draggedEntity_->SetLocation(simLocation);
        update();
    }
}

void UniverseWidget::resizeEvent(QResizeEvent* /*event*/)
{
    update();
}

void UniverseWidget::paintEvent(QPaintEvent* event)
{
    auto begin = std::chrono::steady_clock::now();

    if (universe_) {
        if (trackSelected_ && selectedEntity_ && (selectedEntity_ != draggedEntity_)) {
            Point focus = { selectedEntity_->GetTransform().x, selectedEntity_->GetTransform().y };
            transformX_ = -focus.x;
            transformY_ = -focus.y;
        }
        QPainter p(this);
        p.save();
        p.setClipRegion(event->region());
        p.translate(width() / 2, height() / 2);
        p.scale(transformScale_, transformScale_);
        p.translate(transformX_, transformY_);

        Point topLeft = TransformLocalToSimCoords(Point{ 0, 0 });
        Point bottomRight = TransformLocalToSimCoords(Point{ static_cast<double>(width()), static_cast<double>(height()) });
        universe_->Render(p, Rect{ topLeft.x, topLeft.y, bottomRight.x, bottomRight.y });

        if (draggedEntity_) {
            // TODO draw dragged entity slightly bigger and offset, with a shadow so it looks like it is above the sim
            draggedEntity_->Draw(p);
            p.setPen(Qt::GlobalColor::darkYellow);
            p.setBrush(Qt::BrushStyle::NoBrush);
            p.drawEllipse(QPointF(draggedEntity_->GetTransform().x, draggedEntity_->GetTransform().y), draggedEntity_->GetRadius(), draggedEntity_->GetRadius());
        }

        p.restore();
        qreal textY = 15.0;
        if (displayRateStats_ || (!ticksPaused_ && !limitTickRate_)) {
            p.fillRect(QRect(0, textY - 15.0, displayDurationStats_ ? 110 : 80, 35), QColor(200, 255, 255));
            p.drawText(QPointF(5.0, textY), QString("Tick (Hz): %1").arg(std::round(tickRateStats_.MeanHz())));
            textY += 15.0;
            p.drawText(QPointF(5.0, textY), QString("Paint (Hz): %1").arg(std::round(paintRateStats_.MeanHz())));
            textY += 15.0;
        }
        if (displayDurationStats_) {
            p.fillRect(QRect(0, textY - 15.0, 110, 35), QColor(200, 255, 255));
            p.drawText(QPointF(5.0, textY), QString("Tick (ms): %1").arg(tickDurationStats_.Mean() * 1.e3));
            textY += 15.0;
            p.drawText(QPointF(5.0, textY), QString("Paint (ms): %1").arg(paintDurationStats_.Mean() * 1.e3));
            textY += 15.0;
        }
    }

    // Only if we painted the whole region
    if (event->rect() == rect()) {
        auto end = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
        paintDurationStats_.AddValue(seconds);
        paintRateStats_.AddValue();
    }
}

void UniverseWidget::OnTickTimerElapsed()
{
    if (universe_ && !ticksPaused_) {
        Tick();
    }
}

void UniverseWidget::OnPaintTimerElapsed()
{
    update();
}

Point UniverseWidget::TransformLocalToSimCoords(const Point& local) const
{
    double x = local.x;
    double y = local.y;
    // Sim is centred on screen
    x -= (width() / 2);
    y -= (height() / 2);
    // Sim is scaled
    x /= transformScale_;
    y /= transformScale_;
    // Sim is transformed
    x -= transformX_;
    y -= transformY_;
    return { x, y };
}

Point UniverseWidget::TransformSimToLocalCoords(const Point& sim) const
{
    double x = sim.x;
    double y = sim.y;
    // Sim is transformed
    x += transformX_;
    y += transformY_;
    // Sim is scaled
    x *= transformScale_;
    y *= transformScale_;
    // Sim is centred on screen
    x += (width() / 2);
    y += (height() / 2);
    return { x, y };
}

void UniverseWidget::Tick()
{
    auto begin = std::chrono::steady_clock::now();

    universe_->Tick();

    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
    tickDurationStats_.AddValue(seconds);
    tickRateStats_.AddValue();
    emit Ticked();
}

void UniverseWidget::UpdateTps()
{
    if (limitTickRate_ || ticksPaused_) {
        if (ticksPerSecondTarget_ <= 0.0 || ticksPaused_) {
            tickThread_.stop();
        } else if (tickThread_.isActive()) {
            tickThread_.setInterval(1000.0 / ticksPerSecondTarget_);
        } else {
            tickThread_.start(1000.0 / ticksPerSecondTarget_);
        }
    } else {
        if (tickThread_.isActive()) {
            tickThread_.setInterval(0);
        } else {
            tickThread_.start(0);
        }
    }
    update();
}
