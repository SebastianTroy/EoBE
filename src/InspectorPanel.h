#ifndef INSPECTORPANEL_H
#define INSPECTORPANEL_H

#include "Universe.h"
#include "Entity.h"
#include "PropertyTableModel.h"
#include "DrawSettings.h"

#include <QTabWidget>
#include <QTimer>

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

private:
    constexpr static int SIM_TAB_INDEX = 0;
    constexpr static int ENTITY_TAB_INDEX = 1;
    constexpr static int BRAIN_TAB_INDEX = 2;
    constexpr static int GENOME_TAB_INDEX = 3;

    Ui::InspectorPanel *ui;
    QPixmap previewPixmap_;
    DrawSettings drawSettings_;

    std::shared_ptr<Universe> universe_;
    std::shared_ptr<Entity> selectedEntity_;
    PropertyTableModel simPropertyModel_;
    MyDelegate simPropertyDetailButtonDelegate_;
    PropertyTableModel entityPropertyModel_;
    MyDelegate entityPropertyDetailButtonDelegate_;
    QTimer propertyUpdateThread_;

    void UpdateSimTab();
    void UpdateEntityTab();
    void UpdateEntityPreview();
    void SetSimProperties(std::vector<Property>&& properties);
    void SetEntityProperties(std::vector<Property>&& properties);
};

#endif // INSPECTORPANEL_H
