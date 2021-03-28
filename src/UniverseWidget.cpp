#include "UniverseWidget.h"

#include "Swimmer.h"
#include "FoodPellet.h"
#include "Egg.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPalette>

UniverseWidget::UniverseWidget(QWidget* parent)
    : QWidget(parent)
    , renderThread_(this)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setColor(QPalette::ColorRole::Window, QColor(200, 255, 255));
    setPalette(p);

    renderThread_.setSingleShot(false);
    renderThread_.connect(&renderThread_, &QTimer::timeout, [&]()
    {
        if (updateToRender_) {
            update();
        }
    });

    tickThread_.setSingleShot(false);
    tickThread_.connect(&tickThread_, &QTimer::timeout, [&]()
    {
        if (universe_ && !ticksPaused_) {
            universe_->Tick();
            updateToRender_ = true;
            emit Ticked();
        }
    });

    SetFpsTarget(40);
    SetTpsTarget(60);
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
        universe_->Tick();
    }
    updateToRender_ = true;
    update();
    emit Ticked();

    if (tickThreadWasRunning) {
        tickThread_.start();
    }
    if (renderThreadWasRunning) {
        renderThread_.start();
    }
}

void UniverseWidget::SelectFittestSwimmer()
{
    unsigned mostLivingChildren = 0;
    universe_->ForEach([&](const std::shared_ptr<Entity>& e)
    {
        if (const auto* s = dynamic_cast<const Swimmer*>(e.get())) {
            if (s->GetLivingDescendantsCount(2) > mostLivingChildren) {
                mostLivingChildren = s->GetLivingDescendantsCount(2);
                selectedEntity_ = e;
            }
        }
    });
    emit EntitySelected(selectedEntity_);
    updateToRender_ = true;
}

void UniverseWidget::RemoveAllSwimmers()
{
    universe_->ClearAllEntitiesOfType<Swimmer, Egg>();
}

void UniverseWidget::RemoveAllFood()
{
    universe_->ClearAllEntitiesOfType<FoodPellet>();
}

void UniverseWidget::wheelEvent(QWheelEvent* event)
{
    double d = 1.0 + (0.001 * double(event->angleDelta().y()));
    transformScale_ *= d;
    updateToRender_ = true;
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
        updateToRender_ = true;
        update();
    } else if (draggedEntity_) {
        Point simLocation = TransformLocalToSimCoords({ static_cast<double>(event->pos().x()), static_cast<double>(event->pos().y()) });
        draggedEntity_->SetLocation(simLocation);
        update();
    }
}

void UniverseWidget::resizeEvent(QResizeEvent* /*event*/)
{
    updateToRender_ = true;
    update();
}

void UniverseWidget::paintEvent(QPaintEvent* event)
{
    updateToRender_ = false;

    // TODO add some indication that sim is still running when 0 FPS selected
    if (universe_) {
        if (trackSelected_ && selectedEntity_ && (selectedEntity_ != draggedEntity_)) {
            Point focus = { selectedEntity_->GetTransform().x, selectedEntity_->GetTransform().y };
            transformX_ = -focus.x;
            transformY_ = -focus.y;
        }
        QPainter p(this);
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
    }
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
