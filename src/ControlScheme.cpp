#include "ControlScheme.h"

ControlScheme::ControlScheme(UniverseWidget& universeWidget)
    : universeWidget_(universeWidget)
    , enabled_(true)
{
}

ControlScheme::~ControlScheme()
{
}

bool ControlScheme::OnMouseButtonPressed(QPoint eventLocation, Qt::MouseButton eventTrigger, Qt::MouseButtons currentButtonStates, Qt::KeyboardModifiers modifiers)
{
    return enabled_ && OnMouseButtonPressed(Convert(eventLocation), Convert(eventTrigger), Convert(currentButtonStates), Convert(modifiers));
}

bool ControlScheme::OnMouseButtonReleased(QPoint eventLocation, Qt::MouseButton eventTrigger, Qt::MouseButtons currentButtonStates, Qt::KeyboardModifiers modifiers)
{
    return enabled_ && OnMouseButtonReleased(Convert(eventLocation), Convert(eventTrigger), Convert(currentButtonStates), Convert(modifiers));
}

bool ControlScheme::OnMouseMoved(QPoint newLocation, Qt::MouseButtons currentButtonStates, Qt::KeyboardModifiers modifiers)
{
    return enabled_ && OnMouseMoved(Convert(newLocation), Convert(currentButtonStates), Convert(modifiers));
}

bool ControlScheme::OnWheelScrolled(QPoint scrolled)
{
    return enabled_ && OnWheelScrolled(Convert(scrolled));
}

Point ControlScheme::Convert(const QPoint& point)
{
    return { static_cast<double>(point.x()), static_cast<double>(point.y()) };
}

MouseButtonFlag ControlScheme::Convert(Qt::MouseButtons buttons)
{
    MouseButtonFlag flags = MouseButtonFlag::None;

    if ((buttons | Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) {
        flags |= MouseButtonFlag::Left;
    }
    if ((buttons | Qt::MouseButton::RightButton) == Qt::MouseButton::RightButton) {
        flags |= MouseButtonFlag::Right;
    }
    if ((buttons | Qt::MouseButton::MiddleButton) == Qt::MouseButton::MiddleButton) {
        flags |= MouseButtonFlag::Middle;
    }
    if ((buttons | Qt::MouseButton::ExtraButton1) == Qt::MouseButton::ExtraButton1) {
        flags |= MouseButtonFlag::Four;
    }
    if ((buttons | Qt::MouseButton::ExtraButton2) == Qt::MouseButton::ExtraButton2) {
        flags |= MouseButtonFlag::Five;
    }

    return flags;
}

ModifierFlag ControlScheme::Convert(Qt::KeyboardModifiers buttons)
{
    ModifierFlag flags = ModifierFlag::None;

    if ((buttons | Qt::KeyboardModifier::AltModifier) == Qt::KeyboardModifier::AltModifier) {
        flags |= ModifierFlag::Alt;
    }
    if ((buttons | Qt::KeyboardModifier::ControlModifier) == Qt::KeyboardModifier::ControlModifier) {
        flags |= ModifierFlag::Ctrl;
    }
    if ((buttons | Qt::KeyboardModifier::ShiftModifier) == Qt::KeyboardModifier::ShiftModifier) {
        flags |= ModifierFlag::Shift;
    }

    return flags;
}
