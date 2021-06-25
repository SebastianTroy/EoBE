#include "UniverseWidget.h"

#include "Trilobyte.h"
#include "FoodPellet.h"
#include "Egg.h"
#include "ControlSchemePanAndZoom.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPalette>

#include <chrono>

UniverseWidget::UniverseWidget(QWidget* parent)
    : QWidget(parent)
    , drawThread_(this)
    , tickDurationStats_(100)
    , drawDurationStats_(100)
    , tickRateStats_(30)
    , drawRateStats_(30)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(QPalette::ColorRole::Window, QColor(200, 255, 255));
    setPalette(p);

    drawOptions_.showQuadTreeGrid_ = false;
    drawOptions_.showEntityImages_ = true;
    drawOptions_.showSpawners_ = true;
    drawOptions_.showTrilobyteDebug_ = true;

    drawThread_.setSingleShot(false);
    drawThread_.setTimerType(Qt::PreciseTimer);
    drawThread_.connect(&drawThread_, &QTimer::timeout, this, &UniverseWidget::OnDrawTimerElapsed, Qt::QueuedConnection);

    tickThread_.setSingleShot(false);
    tickThread_.setTimerType(Qt::PreciseTimer);
    tickThread_.connect(&tickThread_, &QTimer::timeout, this, &UniverseWidget::OnTickTimerElapsed, Qt::QueuedConnection);

    SetTpsTarget(40);
    SetFpsTarget(40);
}

UniverseWidget::~UniverseWidget()
{
}

Tril::Handle UniverseWidget::AddDrawOperation(std::function<void (QPainter&)>&& drawTask)
{
    return perDrawTasks_.PushBack(std::move(drawTask));
}

void UniverseWidget::SetUniverse(std::shared_ptr<Universe> universe)
{
    universe_ = universe;
    emit EntitySelected(nullptr);
}

void UniverseWidget::SetFpsTarget(double fps)
{
    if (fps <= 0.0) {
        drawThread_.stop();
    } else if (drawThread_.isActive()) {
        drawThread_.setInterval(1000.0 / fps);
    } else {
        drawThread_.start(1000.0 / fps);
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
    bool drawThreadWasRunning = drawThread_.isActive();
    tickThread_.stop();
    drawThread_.stop();

    for (unsigned i = 0; i < ticksToStep; ++i) {
        Tick();
    }
    update();

    if (tickThreadWasRunning) {
        tickThread_.start();
    }
    if (drawThreadWasRunning) {
        drawThread_.start();
    }
}

void UniverseWidget::RemoveAllTrilobytes()
{
    universe_->ClearAllEntitiesOfType<Trilobyte, Egg>();
}

void UniverseWidget::RemoveAllFood()
{
    universe_->ClearAllEntitiesOfType<FoodPellet>();
}

void UniverseWidget::Zoom(int ticks)
{
    transformScale_ *= 1.0 + (0.01 * ticks);
    update();
}

void UniverseWidget::ZoomIn()
{
    Zoom(+1);
}

void UniverseWidget::ZoomOut()
{
    Zoom(-1);
}

void UniverseWidget::ZoomReset()
{
    transformScale_ = 1.0;
    update();
}

void UniverseWidget::Pan(double xDistance, double yDistance)
{
    transformX_ += xDistance;
    transformY_ += yDistance;
    update();
}

void UniverseWidget::PanReset()
{
    transformX_ = transformY_ = 0.0;
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

void UniverseWidget::wheelEvent(QWheelEvent* event)
{
    bool eventConsumed = false;
    for (std::shared_ptr<ControlScheme>& controlScheme : controlSchemes_) {
        if (!eventConsumed) {
            eventConsumed = controlScheme->OnWheelScrolled(event->angleDelta());
        }
    }
}

void UniverseWidget::mouseReleaseEvent(QMouseEvent* event)
{
    bool eventConsumed = false;
    for (std::shared_ptr<ControlScheme>& controlScheme : controlSchemes_) {
        if (!eventConsumed) {
            eventConsumed = controlScheme->OnMouseButtonReleased(event->pos(), event->button(), event->buttons(), event->modifiers());
        }
    }
}

void UniverseWidget::mousePressEvent(QMouseEvent* event)
{
    bool eventConsumed = false;
    for (std::shared_ptr<ControlScheme>& controlScheme : controlSchemes_) {
        if (!eventConsumed) {
            eventConsumed = controlScheme->OnMouseButtonPressed(event->pos(), event->button(), event->buttons(), event->modifiers());
        }
    }
}

void UniverseWidget::mouseMoveEvent(QMouseEvent* event)
{
    bool eventConsumed = false;
    for (std::shared_ptr<ControlScheme>& controlScheme : controlSchemes_) {
        if (!eventConsumed) {
            eventConsumed = controlScheme->OnMouseMoved(event->pos(), event->buttons(), event->modifiers());
        }
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
        QPainter p(this);
        p.save();
        p.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform);
        p.setClipRegion(event->region());
        p.translate(width() / 2, height() / 2);
        p.scale(transformScale_, transformScale_);
        p.translate(transformX_, transformY_);

        Point topLeft = TransformLocalToSimCoords(Point{ 0, 0 });
        Point bottomRight = TransformLocalToSimCoords(Point{ static_cast<double>(width()), static_cast<double>(height()) });
        universe_->Draw(p, drawOptions_, Rect{ topLeft.x, topLeft.y, bottomRight.x, bottomRight.y });

        perDrawTasks_.ForEach([&](auto& paintAction)
        {
            paintAction(p);
        });

        p.restore();
        qreal textY = 15.0;
        if (displayRateStats_ || (!ticksPaused_ && !limitTickRate_)) {
            p.fillRect(QRect(0, textY - 15.0, displayDurationStats_ ? 110 : 80, 35), QColor(200, 255, 255));
            p.drawText(QPointF(5.0, textY), QString("Tick (Hz): %1").arg(std::round(tickRateStats_.MeanHz())));
            textY += 15.0;
            p.drawText(QPointF(5.0, textY), QString("Paint (Hz): %1").arg(std::round(drawRateStats_.MeanHz())));
            textY += 15.0;
        }
        if (displayDurationStats_) {
            p.fillRect(QRect(0, textY - 15.0, 110, 35), QColor(200, 255, 255));
            p.drawText(QPointF(5.0, textY), QString("Tick (ms): %1").arg(tickDurationStats_.Mean() * 1.e3));
            textY += 15.0;
            p.drawText(QPointF(5.0, textY), QString("Paint (ms): %1").arg(drawDurationStats_.Mean() * 1.e3));
            textY += 15.0;
        }
    }

    // Only if we painted the whole region
    if (event->rect() == rect()) {
        auto end = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count();
        drawDurationStats_.AddValue(seconds);
        drawRateStats_.AddValue();
        emit Painted();
    }
}

void UniverseWidget::OnTickTimerElapsed()
{
    if (universe_ && !ticksPaused_) {
        Tick();
    }
}

void UniverseWidget::OnDrawTimerElapsed()
{
    update();
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
