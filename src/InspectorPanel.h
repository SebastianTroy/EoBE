#ifndef INSPECTORPANEL_H
#define INSPECTORPANEL_H

#include "Universe.h"
#include "Entity.h"
#include "EntityPropertyTableModel.h"

#include <QTabWidget>

namespace Ui {
class InspectorPanel;
}

class InspectorPanel : public QTabWidget {
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget *parent = nullptr);
    ~InspectorPanel();

public slots:
    void SetUniverse(std::shared_ptr<Universe> universe);
    void SetEntity(std::shared_ptr<Entity> selectedEntity);
    void OnUniverseTick();

private:
    constexpr static int ENTITY_TAB_INDEX = 0;
    constexpr static int BRAIN_TAB_INDEX = 1;
    constexpr static int GENOME_TAB_INDEX = 2;

    Ui::InspectorPanel *ui;
    QPixmap previewPixmap_;

    std::shared_ptr<Universe> universe_;
    std::shared_ptr<Entity> selectedEntity_;
    bool newEntity_;
    EntityPropertyTableModel entityPropertyModel_;
    MyDelegate entityPropertyDetailButtonDelegate_;

    void UpdateEntityTab();
    void UpdateEntityPreview();
    void SetProperties(std::vector<Property>&& properties);
};

#endif // INSPECTORPANEL_H
