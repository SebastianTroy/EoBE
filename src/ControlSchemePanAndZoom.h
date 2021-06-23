#ifndef CONTROLSCHEMEPANANDZOOM_H
#define CONTROLSCHEMEPANANDZOOM_H

#include "ControlScheme.h"

class ControlSchemePanAndZoom : public ControlScheme {
public:
    ControlSchemePanAndZoom(UniverseWidget& universeWidget);
    ~ControlSchemePanAndZoom() override;

    virtual std::string GetName() const override { return "Pan and Zoom Simulation"; }
    virtual std::string GetDescription() const override;

protected:
    virtual bool OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnMouseButtonReleased(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnMouseMoved(Point newLocation, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnWheelScrolled(Point scrolled) override;

private:
    Point dragBegin_;
    bool dragging_;
};

#endif // CONTROLSCHEMEPANANDZOOM_H
