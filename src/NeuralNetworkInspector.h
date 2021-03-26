#ifndef NEURALNETWORKINSPECTOR_H
#define NEURALNETWORKINSPECTOR_H

#include "UniverseParameters.h"
#include <Shape.h>

#include <QWidget>

#include <memory>

class Swimmer;
class NeuralNetwork;
class EntityContainerInterface;

// TODO low priority, make groups movable and resizable and hideable
class NeuralNetworkInspector : public QWidget {
    Q_OBJECT
public:
    explicit NeuralNetworkInspector(QWidget *parent);

    void SetSwimmer(std::shared_ptr<Swimmer> toInspect);
    std::shared_ptr<Swimmer> GetSwimmer() const { return inspectedSwimmer_; }
    void UpdateConnectionStrengths(const EntityContainerInterface& container, const UniverseParameters& universeParameters);

    void SetUpdateLive(bool update);
    void ResetViewTransform();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;

private:
    enum class GroupType {
        Sense,
        Brain,
        Effector,
    };

    struct Node {
        double x;
        double y;
        double value;
        // Nodes are arranged from 0 -> 1 on the x & y axis
        std::vector<std::pair<std::reference_wrapper<Node>, double>> connections;
    };

    struct Group {
        std::string name;
        QRectF area;
        std::map<std::pair<unsigned, unsigned>, Node> nodes;
        unsigned horizontalNodes;
        unsigned verticalNodes;
        bool selected;
        QString description;
    };

    const qreal nodeHeight_ = 15.0;
    const qreal nodeWidth_ = 15.0;
    const qreal nodeHSpacing_ = 35.0;
    const qreal nodeVSpacing_ = nodeHSpacing_ * 1.5;
    const qreal groupPadding_ = 5.0;
    const qreal groupHSpacing_ = 5.0;
    const qreal groupVSpacing_ = groupHSpacing_ * 2;

    std::vector<Group> sensorGroups_;
    Group brainGroup_;
    std::vector<Group> effectorGroups_;

    double transformX_ = 0.0;
    double transformY_ = 0.0;
    double transformScale_ = 1.0;
    double dragX_;
    double dragY_;
    bool dragging_ = false;

    std::shared_ptr<Swimmer> inspectedSwimmer_;
    bool liveUpdate_ = false;

    QPointF TransformLocalToSimCoords(const QPointF& local) const;
    QPointF TransformSimToLocalCoords(const QPointF& sim) const;

    std::string GetBrainDescription() const;

    Group CreateGroup(const NeuralNetwork& network, const std::string& name, const std::string& description);

    void LayoutGroupsInView();
    void LayoutNodesInGroup(GroupType type, Group& group);

    void ForwardPropogate();

    void ForEachGroup(std::function<void(GroupType, Group&)>&& action);
    void ForEachGroup(std::function<void(GroupType, const Group&)>&& action) const;
};

#endif // NEURALNETWORKINSPECTOR_H
