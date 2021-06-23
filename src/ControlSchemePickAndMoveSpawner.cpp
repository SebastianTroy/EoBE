#include "ControlSchemePickAndMoveSpawner.h"

#include "UniverseWidget.h"

ControlSchemePickAndMoveSpawner::ControlSchemePickAndMoveSpawner(UniverseWidget& universeWidget)
    : ControlScheme(universeWidget)
{
    drawHandle_ = universeWidget_.AddDrawOperation([&](QPainter& p)
    {
        std::shared_ptr<Universe> universe = universeWidget_.GetUniverse(); {}

        if (universe) {
            if (draggedSpawner_) {
                draggedSpawner_->Draw(p, universeWidget_.GetDrawOptions());
            }
        }
    });
}

ControlSchemePickAndMoveSpawner::~ControlSchemePickAndMoveSpawner()
{
}

std::string ControlSchemePickAndMoveSpawner::GetDescription() const
{
    return "<p>Use the mouse to select, and move Spawners.</p>"
           "<p>Right click on any spawner to select it, more details about the "
             "spawner will appear in a tab in the inspector view on the right."
             "</p>"
           "<p>Left click and hold to drag a spawner around. If a spawner was "
             "dragged accidentally, click the right mouse button to cancel the "
             "drag before releasing the left button.</p>";
}

bool ControlSchemePickAndMoveSpawner::OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();

    if (universe) {
        Point simLocation = universeWidget_.TransformLocalToSimCoords(eventLocation);
        if ((eventTrigger | MouseButtonFlag::Right) == MouseButtonFlag::Right) {
            if (draggedSpawner_) {
                draggedSpawner_->SetLocation(draggedSpawnerOriginalPosition_);
                ReturnSpawnerToUniverse();
            } else {
                selectedSpawner_ = universe->PickSpawner(simLocation, false);
                emit universeWidget_.SpawnerSelected(selectedSpawner_);
            }
        }
        if ((eventTrigger | MouseButtonFlag::Left) == MouseButtonFlag::Left) {
            draggedSpawner_ = universe->PickSpawner(simLocation, true);
            draggedSpawnerOriginalPosition_ = draggedSpawner_ ? draggedSpawner_->GetLocation() : Point{};
            draggedSpawnerOffset_ = draggedSpawner_ ? draggedSpawner_->GetLocation() - simLocation : Point{};
            return draggedSpawner_ != nullptr;
        }
    }
    return false;
}

bool ControlSchemePickAndMoveSpawner::OnMouseButtonReleased(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers)
{
    if ((eventTrigger | MouseButtonFlag::Left) == MouseButtonFlag::Left && draggedSpawner_) {
        ReturnSpawnerToUniverse();
    }
    return false;
}

bool ControlSchemePickAndMoveSpawner::OnMouseMoved(Point newLocation, MouseButtonFlag currentButtonStates, ModifierFlag modifiers)
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();

    if (draggedSpawner_) {
        Point simLocation = universeWidget_.TransformLocalToSimCoords(newLocation);
        draggedSpawner_->SetLocation(simLocation + draggedSpawnerOffset_);
        universeWidget_.update();
        return true;
    }
    return false;
}



void ControlSchemePickAndMoveSpawner::ReturnSpawnerToUniverse()
{
    std::shared_ptr<Universe> universe = universeWidget_.GetUniverse();
    if (universe && draggedSpawner_) {
        universe->AddSpawner(draggedSpawner_);
        draggedSpawner_ = nullptr;
        universeWidget_.update();
    }
}
