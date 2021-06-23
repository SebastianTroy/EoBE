#ifndef CONTROLSCHEMEPICKANDMOVEENTITY_H
#define CONTROLSCHEMEPICKANDMOVEENTITY_H

#include "ControlScheme.h"
#include "Entity.h"

#include <AutoClearingContainer.h>

#include <memory>

class ControlSchemePickAndMoveEntity : public ControlScheme {
public:
    ControlSchemePickAndMoveEntity(UniverseWidget& universeWidget);
    ~ControlSchemePickAndMoveEntity() override;

    void SetTrackSelectedEntity(bool track) { trackSelected_ = track; }
    void SelectFittestTrilobyte();

    virtual std::string GetName() const override { return "Pick and Move Entity"; }
    virtual std::string GetDescription() const override;

protected:
    virtual bool OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnMouseButtonReleased(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnMouseMoved(Point newLocation, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnWheelScrolled(Point /*scrolled*/) override { return false; }

private:
    Tril::Handle drawHandle_;
    bool trackSelected_;
    std::shared_ptr<Entity> selectedEntity_;
    std::shared_ptr<Entity> draggedEntity_;
    Point draggedEntityOffset_;
    Point draggedEntityOriginalPosition_;

    void ReturnEntityToUniverse();
};

#endif // CONTROLSCHEMEPICKANDMOVEENTITY_H
