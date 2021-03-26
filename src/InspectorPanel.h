#ifndef INSPECTORPANEL_H
#define INSPECTORPANEL_H

#include "Universe.h"
#include "Entity.h"

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
    Ui::InspectorPanel *ui;
    QPixmap previewPixmap_;

    std::shared_ptr<Universe> universe_;
    std::shared_ptr<Entity> selectedEntity_;

    void UpdateEntityTab();
    void UpdateEntityPreview();
};

#endif // INSPECTORPANEL_H
