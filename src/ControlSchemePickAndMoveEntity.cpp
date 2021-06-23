#include "ControlSchemePickAndMoveEntity.h"

#include "UniverseWidget.h"
#include "Trilobyte.h"

ControlSchemePickAndMoveEntity::ControlSchemePickAndMoveEntity(UniverseWidget& universeWidget)
    : ControlScheme(universeWidget)
    , trackSelected_(false)
{
    drawHandle_ = universeWidget_.AddDrawOperation([&](QPainter& p)
    {
        std::shared_ptr<Universe> universe = universeWidget_.GetUniverse(); {}

        if (universe) {
            if (trackSelected_ && selectedEntity_ && (selectedEntity_ != draggedEntity_)) {
                Point focus = { -selectedEntity_->GetTransform().x, -selectedEntity_->GetTransform().y };
                universeWidget_.SetPanTransform(focus);
            }

            if (draggedEntity_) {
                // TODO draw dragged entity slightly bigger and offset, with a shadow so it looks like it is above the sim
                draggedEntity_->Draw(p, universeWidget_.GetDrawOptions());
                p.setPen(Qt::GlobalColor::darkYellow);
                p.setBrush(Qt::BrushStyle::NoBrush);
                p.drawEllipse(QPointF(draggedEntity_->GetTransform().x, draggedEntity_->GetTransform().y), draggedEntity_->GetRadius(), draggedEntity_->GetRadius());
            }
        }
    });
}

ControlSchemePickAndMoveEntity::~ControlSchemePickAndMoveEntity()
{
}

void ControlSchemePickAndMoveEntity::SelectFittestTrilobyte()
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();

    if (universe) {
        unsigned mostLivingGrandChildren = 0;
        universe->ForEach([&](const std::shared_ptr<Entity>& e)
        {
            if (const auto* s = dynamic_cast<const Trilobyte*>(e.get())) {
                if (s->GetLivingDescendantsCount(2) > mostLivingGrandChildren) {
                    mostLivingGrandChildren = s->GetLivingDescendantsCount(2);
                    selectedEntity_ = e;
                }
            }
        });
        emit universeWidget_.EntitySelected(selectedEntity_);
    }
}

std::string ControlSchemePickAndMoveEntity::GetDescription() const
{
    return "<p>Use the mouse to select, and move Entities.</p>"
           "<p>Right click on any entity to select it, more details about the "
             "entity will appear in a tab in the inspector view on the right."
             "</p>"
           "<p>Left click and hold to drag an entity around. If an entity was "
             "dragged accidentally, click the right mouse button to cancel the "
             "drag before releasing the left button.</p>";
}

bool ControlSchemePickAndMoveEntity::OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();

    if (universe) {
        Point simLocation = universeWidget_.TransformLocalToSimCoords(eventLocation);
        if ((eventTrigger | MouseButtonFlag::Right) == MouseButtonFlag::Right) {
            if (draggedEntity_) {
                draggedEntity_->SetLocation(draggedEntityOriginalPosition_);
                ReturnEntityToUniverse();
            } else {
                selectedEntity_ = universe->PickEntity(simLocation, false);
                emit universeWidget_.EntitySelected(selectedEntity_);
            }
        }
        if ((eventTrigger | MouseButtonFlag::Left) == MouseButtonFlag::Left) {
            draggedEntity_ = universe->PickEntity(simLocation, true);
            draggedEntityOriginalPosition_ = draggedEntity_ ? draggedEntity_->GetLocation() : Point{};
            draggedEntityOffset_ = draggedEntity_ ? draggedEntity_->GetLocation() - simLocation : Point{};
            return draggedEntity_ != nullptr;
        }
    }
    return false;
}

bool ControlSchemePickAndMoveEntity::OnMouseButtonReleased(Point /*eventLocation*/, MouseButtonFlag eventTrigger, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    if ((eventTrigger | MouseButtonFlag::Left) == MouseButtonFlag::Left && draggedEntity_) {
        ReturnEntityToUniverse();
    }
    return false;
}

bool ControlSchemePickAndMoveEntity::OnMouseMoved(Point newLocation, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();

    if (draggedEntity_) {
        Point simLocation = universeWidget_.TransformLocalToSimCoords(newLocation);
        draggedEntity_->SetLocation(simLocation + draggedEntityOffset_);
        universeWidget_.update();
        return true;
    }
    return false;
}

void ControlSchemePickAndMoveEntity::ReturnEntityToUniverse()
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();
    if (universe && draggedEntity_) {
        universe->AddEntity(draggedEntity_);
        draggedEntity_ = nullptr;
        universeWidget_.update();
    }
}
