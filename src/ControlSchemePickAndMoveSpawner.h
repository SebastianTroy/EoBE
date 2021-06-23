#ifndef CONTROLSCHEMEPICKANDMOVESPAWNER_H
#define CONTROLSCHEMEPICKANDMOVESPAWNER_H

#include "ControlScheme.h"
#include "Spawner.h"

#include <AutoClearingContainer.h>

#include <memory>

class ControlSchemePickAndMoveSpawner : public ControlScheme {
public:
    ControlSchemePickAndMoveSpawner(UniverseWidget& universeWidget);
    ~ControlSchemePickAndMoveSpawner() override;

    virtual std::string GetName() const override { return "Pick and Move Spawner"; }
    virtual std::string GetDescription() const override;

protected:
    virtual bool OnMouseButtonPressed(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnMouseButtonReleased(Point eventLocation, MouseButtonFlag eventTrigger, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnMouseMoved(Point newLocation, MouseButtonFlag currentButtonStates, ModifierFlag modifiers) override;
    virtual bool OnWheelScrolled(Point /*scrolled*/) override { return false; }

private:
    Tril::Handle drawHandle_;
    std::shared_ptr<Spawner> selectedSpawner_;
    std::shared_ptr<Spawner> draggedSpawner_;
    Point draggedSpawnerOffset_;
    Point draggedSpawnerOriginalPosition_;

    void ReturnSpawnerToUniverse();
};

#endif // CONTROLSCHEMEPICKANDMOVESPAWNER_H
