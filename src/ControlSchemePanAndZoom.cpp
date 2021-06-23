#include "ControlSchemePanAndZoom.h"

#include "UniverseWidget.h"

ControlSchemePanAndZoom::ControlSchemePanAndZoom(UniverseWidget& universeWidget)
    : ControlScheme(universeWidget)
    , dragBegin_({ 0.0, 0.0 })
    , dragging_(false)
{
}

ControlSchemePanAndZoom::~ControlSchemePanAndZoom()
{
}

std::string ControlSchemePanAndZoom::GetDescription() const
{
    return "<p>Use the mouse to adjust the simulation view.</p>"
           "<p>Click and hold the left mouse button to drag the view around."
             "</p>"
           "<p>Scroll the mouse wheel to zoom in and out.</p>";
}

bool ControlSchemePanAndZoom::OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    if ((eventTrigger | MouseButtonFlag::Left) == MouseButtonFlag::Left) {
        dragging_ = true;
        dragBegin_ = eventLocation;
        return true;
    }
    return false;
}

bool ControlSchemePanAndZoom::OnMouseButtonReleased(Point /*eventLocation*/, MouseButtonFlag eventTrigger, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    if ((eventTrigger | MouseButtonFlag::Left) == MouseButtonFlag::Left) {
        dragging_ = false;
    }
    return false;
}

bool ControlSchemePanAndZoom::OnMouseMoved(Point newLocation, MouseButtonFlag /*currentButtonStates*/, ModifierFlag /*modifiers*/)
{
    if (dragging_) {
        double deltaX = (newLocation.x - dragBegin_.x) / universeWidget_.GetZoomTransform();
        double deltaY = (newLocation.y - dragBegin_.y) / universeWidget_.GetZoomTransform();
        universeWidget_.Pan(deltaX, deltaY);
        dragBegin_ = newLocation;
        return true;
    }
    return false;
}

bool ControlSchemePanAndZoom::OnWheelScrolled(Point scrolled)
{
    universeWidget_.Zoom(scrolled.y / 10.0);
    return true;
}
