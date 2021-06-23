#ifndef CONTROLSCHEME_H
#define CONTROLSCHEME_H

#include "Shape.h"

#include <QMouseEvent>

class UniverseWidget;

enum class MouseButtonFlag : unsigned {
    None =   0,
    Left =   1 << 0,
    Middle = 1 << 1,
    Right =  1 << 2,
    Four =   1 << 3,
    Five =   1 << 4,
};

enum class ModifierFlag : unsigned {
    None =  0,
    Ctrl =  1 << 0,
    Shift = 1 << 1,
    Alt =   1 << 2,
};

/**
 * @brief The ControlScheme class exists to allow multiple control modes to
 * exist, which can be switched between easily, simplifying control logic. Each
 * event type can return a boolean to indicate whether it has consumed the
 * event, so if multiple control schemes are being used in parallel, they can
 * indicate whether an event can be shared or not.
 */
class ControlScheme {
public:
    ControlScheme(UniverseWidget& universeWidget);
    virtual ~ControlScheme();

    void SetEnabled(bool enabled) { enabled_ = enabled; }

    bool OnMouseButtonPressed(QPoint eventLocation, Qt::MouseButton eventTrigger, Qt::MouseButtons currentButtonStates, Qt::KeyboardModifiers modifiers);
    bool OnMouseButtonReleased(QPoint eventLocation, Qt::MouseButton eventTrigger, Qt::MouseButtons currentButtonStates, Qt::KeyboardModifiers modifiers);
    bool OnMouseMoved(QPoint newLocation, Qt::MouseButtons currentButtonStates, Qt::KeyboardModifiers modifiers);
    bool OnWheelScrolled(QPoint scrolled);

    virtual std::string GetName() const = 0;
    virtual std::string GetDescription() const = 0;

    bool IsEnabled() const { return enabled_; }

protected:
    UniverseWidget& universeWidget_;

    /**
     * @param eventLocation The mouse location on screen where the button press
     * was detected.
     * @param eventTrigger The mouse button that triggered the event.
     * @param currentButtonStates Flags indicating which mouse buttons are
     * currently held down.
     * @param modifiers Keyboard modifiers which are currrently held down.
     * @return If this returns true, the event will be consumed
     */
    virtual bool OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) = 0;
    /**
     * @param eventLocation The mouse location on screen where the button
     * release was detected.
     * @param eventTrigger The mouse button that triggered the event.
     * @param currentButtonStates Flags indicating which mouse buttons are
     * currently held down.
     * @param modifiers Keyboard modifiers which are currrently held down.
     * @return true to indicate that this event should be consumed, false to
     * indicate that this event is safe to pass onto any parallel ControlSchemes
     */
    virtual bool OnMouseButtonReleased(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) = 0;
    /**
     * @param eventLocation The mouse location on screen after the mouse has
     * moved.
     * @param currentButtonStates Flags indicating which mouse buttons are
     * currently held down.
     * @param modifiers Keyboard modifiers which are currrently held down.
     * @return true to indicate that this event should be consumed, false to
     * indicate that this event is safe to pass onto any parallel ControlSchemes
     */
    virtual bool OnMouseMoved(Point newLocation, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) = 0;
    /**
     * @param scrolled The scroll is made up of horizontal (x) and vertical (y)
     * components.
     * @return true to indicate that this event should be consumed, false to
     * indicate that this event is safe to pass onto any parallel ControlSchemes
     */
    virtual bool OnWheelScrolled(Point scrolled) = 0;

private:
    bool enabled_;

    Point Convert(const QPoint& point);
    MouseButtonFlag Convert(Qt::MouseButtons buttons);
    ModifierFlag Convert(Qt::KeyboardModifiers buttons);
};

// TODO use C++20 to define a bunch of templates for flags that only work on enumerated type, preferable with some sort of EnableFlagFunctions<MyEnum> opt-in system
// might be able to use bit_cast too?
inline MouseButtonFlag operator~ (MouseButtonFlag a) { return static_cast<MouseButtonFlag>(~static_cast<std::underlying_type_t<MouseButtonFlag>>(a)); }
inline MouseButtonFlag operator| (MouseButtonFlag a, MouseButtonFlag b) { return static_cast<MouseButtonFlag>(static_cast<std::underlying_type_t<MouseButtonFlag>>(a) | static_cast<std::underlying_type_t<MouseButtonFlag>>(b)); }
inline MouseButtonFlag operator& (MouseButtonFlag a, MouseButtonFlag b) { return static_cast<MouseButtonFlag>(static_cast<std::underlying_type_t<MouseButtonFlag>>(a) & static_cast<std::underlying_type_t<MouseButtonFlag>>(b)); }
inline MouseButtonFlag operator^ (MouseButtonFlag a, MouseButtonFlag b) { return static_cast<MouseButtonFlag>(static_cast<std::underlying_type_t<MouseButtonFlag>>(a) ^ static_cast<std::underlying_type_t<MouseButtonFlag>>(b)); }
inline MouseButtonFlag& operator|= (MouseButtonFlag& a, MouseButtonFlag b) { a = a | b; return a; }
inline MouseButtonFlag& operator&= (MouseButtonFlag& a, MouseButtonFlag b) { a = a & b; return a; }
inline MouseButtonFlag& operator^= (MouseButtonFlag& a, MouseButtonFlag b) { a = a ^ b; return a; }

inline ModifierFlag operator~ (ModifierFlag a) { return static_cast<ModifierFlag>(~static_cast<std::underlying_type_t<ModifierFlag>>(a)); }
inline ModifierFlag operator| (ModifierFlag a, ModifierFlag b) { return static_cast<ModifierFlag>(static_cast<std::underlying_type_t<ModifierFlag>>(a) | static_cast<std::underlying_type_t<ModifierFlag>>(b)); }
inline ModifierFlag operator& (ModifierFlag a, ModifierFlag b) { return static_cast<ModifierFlag>(static_cast<std::underlying_type_t<ModifierFlag>>(a) & static_cast<std::underlying_type_t<ModifierFlag>>(b)); }
inline ModifierFlag operator^ (ModifierFlag a, ModifierFlag b) { return static_cast<ModifierFlag>(static_cast<std::underlying_type_t<ModifierFlag>>(a) ^ static_cast<std::underlying_type_t<ModifierFlag>>(b)); }
inline ModifierFlag& operator|= (ModifierFlag& a, ModifierFlag b) { a = a | b; return a; }
inline ModifierFlag& operator&= (ModifierFlag& a, ModifierFlag b) { a = a & b; return a; }
inline ModifierFlag& operator^= (ModifierFlag& a, ModifierFlag b) { a = a ^ b; return a; }

#endif // CONTROLSCHEME_H
