#include "NeuralNetworkInspector.h"

#include "Trilobyte.h"
#include "EntityContainerInterface.h"

#include <NeuralNetwork.h>
#include <Algorithm.h>

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

NeuralNetworkInspector::NeuralNetworkInspector(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

void NeuralNetworkInspector::SetTrilobyte(std::shared_ptr<Trilobyte> toInspect)
{
    if (inspectedTrilobyte_ != toInspect) {
        inspectedTrilobyte_ = toInspect;
        sensorGroups_.clear();
        effectorGroups_.clear();
        brainGroup_ = {};

        if (inspectedTrilobyte_ != nullptr) {
            if (inspectedTrilobyte_->InspectBrain()) {
                brainGroup_ = CreateGroup(*inspectedTrilobyte_->InspectBrain(), "Brain", GetBrainDescription());
            }

            for (auto& sense : inspectedTrilobyte_->InspectSenses()) {
                sensorGroups_.push_back(CreateGroup(sense->Inspect(), std::string(sense->GetName()), sense->GetDescription()));
                Group& senseGroup = sensorGroups_.back();

                unsigned neuronIndex = 0;
                for (auto& neuron : sense->InspectConnection().Inspect()) {
                    unsigned weightIndex = 0;
                    for (auto& weight : neuron) {
                        /*
                         * Some senses are simple passthroughs and do not have any
                         * neurons.
                         */
                        if (senseGroup.nodes.empty()) {
                            senseGroup.horizontalNodes = sense->InspectConnection().GetInputCount();
                            senseGroup.verticalNodes = 1;
                            senseGroup.nodes[{ 0, 0 }];
                        }
                        Node& node = senseGroup.nodes[{ neuronIndex, senseGroup.verticalNodes - 1 }];
                        if (brainGroup_.horizontalNodes > 0 && brainGroup_.verticalNodes > 0) {
                            node.connections.push_back({ brainGroup_.nodes[{ weightIndex, 0 }], weight });
                        }
                        node.value = 0;
                        ++weightIndex;
                    }
                    ++neuronIndex;
                }
            }

            for (auto& effector : inspectedTrilobyte_->InspectEffectors()) {
                effectorGroups_.push_back(CreateGroup(effector->Inspect(), std::string(effector->GetName()), std::string(effector->GetDescription())));
                Group& effectorGroup = effectorGroups_.back();

                unsigned neuronIndex = 0;
                for (auto& neuron : effector->InspectConnection().Inspect()) {
                    unsigned weightIndex = 0;
                    for (auto& weight : neuron) {
                        /*
                         * Some effectors are simple passthroughs and do not have any
                         * neurons.
                         */
                        if (effectorGroup.nodes.empty()) {
                            effectorGroup.horizontalNodes = effector->InspectConnection().GetOutputCount();
                            effectorGroup.verticalNodes = 1;
                            effectorGroup.nodes[{ 0, 0 }];
                        }
                        if (brainGroup_.horizontalNodes > 0 && brainGroup_.verticalNodes > 0) {
                            Node& node = brainGroup_.nodes.at({ neuronIndex, brainGroup_.verticalNodes - 1 });
                            node.connections.push_back({ effectorGroup.nodes[{ weightIndex, 0 }], weight });
                        }
                        effectorGroup.nodes[{ weightIndex, 0 }].value = 0;
                        ++weightIndex;
                    }
                    ++neuronIndex;
                }
            }
        }
        LayoutGroupsInView();
        update();
    }
    ResetViewTransform();
}

void NeuralNetworkInspector::UpdateConnectionStrengths(const EntityContainerInterface& container, const UniverseParameters& universeParameters)
{
    if (inspectedTrilobyte_ && liveUpdate_) {
        // zeroise every input value everywhere
        ForEachGroup([](GroupType /*type*/, Group& group)
        {
            for (auto& [ location, node ] : group.nodes) {
                (void) location; // unused
                node.value = 0.0;
            }
        });

        Tril::IterateBoth<std::shared_ptr<Sense>, Group>(inspectedTrilobyte_->InspectSenses(), sensorGroups_, [&](const std::shared_ptr<Sense>& sense, NeuralNetworkInspector::Group& senseGroup)
        {
            std::vector<double> inputs(senseGroup.horizontalNodes);
            sense->PrimeInputs(inputs, container, universeParameters);
            for (unsigned x = 0; x < senseGroup.horizontalNodes; ++x) {
                senseGroup.nodes.at({ x, 0 }).value = inputs.at(x);
            }
        });
        ForwardPropogate();
        update();
    }
}

void NeuralNetworkInspector::SetUpdateLive(bool liveUpdate)
{
    liveUpdate_ = liveUpdate;
    update();
}

void NeuralNetworkInspector::ResetViewTransform()
{
    // y-axis, top < bottom
    qreal top = std::numeric_limits<qreal>::max();
    qreal bottom = std::numeric_limits<qreal>::lowest();
    // x-axis, left < right
    qreal left = std::numeric_limits<qreal>::max();
    qreal right = std::numeric_limits<qreal>::lowest();

    ForEachGroup([&](GroupType, const Group& group)
    {
        top = std::min(top, group.area.top());
        bottom = std::max(bottom, group.area.bottom());
        left = std::min(left, group.area.left());
        right = std::max(right, group.area.right());
    });

    qreal areaWidth = right - left;
    qreal areaHeight = bottom - top;

    // +5 to add a little margin around the edge
    qreal verticalTransform = height() / (areaHeight + 5);
    qreal horizontalTransform = width() / (areaWidth + 5);

    transformScale_ = std::min(verticalTransform, horizontalTransform);
    transformX_ = -(left + right) / 2.0;
    transformY_ = -(top + bottom) / 2.0;
    update();
}

void NeuralNetworkInspector::wheelEvent(QWheelEvent* event)
{
    double d = 1.0 + (0.001 * double(event->angleDelta().y()));
    transformScale_ *= d;
    // QT paint call
    update();
}

void NeuralNetworkInspector::resizeEvent(QResizeEvent* /*event*/)
{
    update();
}

void NeuralNetworkInspector::mouseReleaseEvent(QMouseEvent*)
{
    dragging_ = false;
}

void NeuralNetworkInspector::mousePressEvent(QMouseEvent* event)
{
    dragging_ = true;
    dragX_ = event->pos().x();
    dragY_ = event->pos().y();
}

void NeuralNetworkInspector::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging_) {
        transformX_ += ((event->pos().x() - dragX_) / transformScale_);
        transformY_ += ((event->pos().y() - dragY_) / transformScale_);
        dragX_ = event->pos().x();
        dragY_ = event->pos().y();
        // QT paint call
        update();
    }

    ForEachGroup([&](GroupType /*type*/, Group& group)
    {
        bool selected = group.area.contains(TransformLocalToSimCoords(event->pos()));
        if (selected != group.selected) {
            group.selected = selected;
            if (group.selected) {
                QToolTip::showText(TransformSimToLocalCoords(group.area.bottomLeft()).toPoint() + mapToGlobal(QPoint(0, 0)), group.description, this);
            }
            update();
        }
    });
}

void NeuralNetworkInspector::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (isVisible()) {
        QPainter p(this);
        p.setClipRegion(event->region());
        p.translate(width() / 2, height() / 2);
        p.scale(transformScale_, transformScale_);
        p.translate(transformX_, transformY_);

        ForEachGroup([&](GroupType type, const Group& group)
        {
            // Draw the connections
            for (auto& [ nodeCoordinates, node ] : group.nodes) {
                (void) nodeCoordinates; // unused
                p.setBrush(Qt::BrushStyle::NoBrush);
                for (auto& [ target, strength ] : node.connections) {
                    qreal penWidth = (std::abs(strength) / 1.5) * 8;
                    if (penWidth * transformScale_ >= 0.75) {
                        unsigned opacity = liveUpdate_ ? std::clamp(std::abs(node.value) * 200, 0.0, 255.0) : 255.0;
                        QPen pen(QColor(strength > 0 ? 0 : 255, 0, 0, opacity));
                        pen.setWidthF(penWidth);
                        p.setPen(pen);

                        p.drawLine(QPointF{ node.x, node.y }, QPointF{ target.get().x, target.get().y });
                    }
                }
            }

            // Draw the Nodes over the top
            for (auto& [ nodeCoordinates, node ] : group.nodes) {
                (void) nodeCoordinates; // unused
                // Draw the Node
                QColor nodeCol(liveUpdate_ ? (node.value > 0 ? Qt::blue : Qt::red) : Qt::magenta);
                qreal transparency = liveUpdate_ ? std::abs(node.value) : 1.0;
                p.setPen(QPen(nodeCol, 3.0));
                nodeCol.setAlphaF(std::min(1.0, transparency));
                p.setBrush(nodeCol);
                // scale /should/ be 1.0, if it isn't something needs normalising!
                double scale = std::max(1.0, transparency);
                p.drawEllipse(QPointF(node.x, node.y), (nodeWidth_ / 2) * scale, (nodeHeight_ / 2) * scale);
            }

            // Draw last so can be seen over over stimulated neurons
            p.setPen(Qt::black);
            p.setBrush(group.selected ? QBrush(QColor::fromRgb(90, 25, 110, 45)): QBrush(Qt::NoBrush));
            p.drawRect(group.area);
            p.setCompositionMode(QPainter::CompositionMode_Exclusion);
            p.setPen(Qt::white);
            if (type == GroupType::Effector) {
                // Draw effector text below the neurons
                QRectF titleRect = p.boundingRect(group.area, Qt::AlignBottom | Qt::AlignHCenter, QString(group.name.c_str()));
                p.drawText(titleRect, QString(group.name.c_str()));
            } else {
                // Draw Sensor/Brain text above the neurons
                QRectF titleRect = p.boundingRect(group.area, Qt::AlignTop | Qt::AlignHCenter, QString(group.name.c_str()));
                p.drawText(titleRect, QString(group.name.c_str()));
            }
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        });
    }
}

void NeuralNetworkInspector::showEvent(QShowEvent* /*event*/)
{
    ResetViewTransform();
}

QPointF NeuralNetworkInspector::TransformLocalToSimCoords(const QPointF& local) const
{
    double x = local.x();
    double y = local.y();
    // Sim is centred on screen
    x -= (width() / 2);
    y -= (height() / 2);
    // Sim is scaled
    x /= transformScale_;
    y /= transformScale_;
    // Sim is transformed
    x -= transformX_;
    y -= transformY_;
    return QPointF(x, y);
}

QPointF NeuralNetworkInspector::TransformSimToLocalCoords(const QPointF& sim) const
{
    double x = sim.x();
    double y = sim.y();
    // Sim is transformed
    x += transformX_;
    y += transformY_;
    // Sim is scaled
    x *= transformScale_;
    y *= transformScale_;
    // Sim is centred on screen
    x += (width() / 2);
    y += (height() / 2);
    return QPointF(x, y);
}

std::string NeuralNetworkInspector::GetBrainDescription() const
{
    return "<p>Each of the senses at the top, effectors at the bottom and the "
    "brain here in the middle are neural networks. The values are propogated "
    "from the top to the bottom.</p>"

    "<p>Each simulation tick, the sense's input nodes (the top most row) are "
    "set, and then all of the sense's neural networks are propogated in "
    "parallel, they each output the same number of values, which are summed and"
    " passed to the brain's input layer. The brain's network is then "
    "propogated, and the output values are then copied to each of the "
    "effector's networks, which are in turn propogated. The end result is that "
    "each of the effectors has a series of values which it translates into "
    "actions. Mouse over each sense for details about how the input values are "
    "calculated, and each effector to see how the output values are translated "
    "into actions.</p>"

    "<p>When viewing the live values, blue nodes contain a positive value, red "
    "nodes contain a negative value and transparency denotes the magnitude, "
    "where opaque represents -1 or 1, depending on the colour, and transparent "
    "represents 0. The connections are also colour coded, black for positive, "
    "and red for negative, but their values are not constrained between -1 and "
    "1, the width of the connection denotes the strength of the connection and "
    "the transparency shows how active the connected input node is blue "
    "denoting it contains a positive value, red denoting a negative value.</p>"

    "<p>Sometimes an input node will contain a value greater than 1 and will be"
    " drawn larger than normal, this occurs because input values are not "
    "constrained, however when they are passed forward, part of the process "
    "normalises the values to between -1 and 1.</p>";
}

NeuralNetworkInspector::Group NeuralNetworkInspector::CreateGroup(const NeuralNetwork& network, const std::string& name, const std::string& description)
{
    Group group = { name, rect(), {}, 0, 0, false, QString::fromStdString(description) };

    network.ForEach([&](unsigned x, unsigned y, const NeuralNetwork::Node& node)
    {
        /*
         * NeuralNetwork::Nodes are simply an array of input weights, from the
         * previous layer, but we want actual nodes for all layers, including
         * the input layer.
         */
        unsigned inputX = 0;
        unsigned inputY = y - 1;
        for (const auto& connectionWeight : node) {
            group.nodes[{ inputX, inputY }].connections.push_back({ group.nodes[{ x, y }], connectionWeight });
            group.nodes.insert({ { x, y }, { 0, 0, 0, {}} }); // Ensure even nodes without connections are added
            inputX++;
        }
    });

    group.horizontalNodes = 0;
    group.verticalNodes = 0;
    for (const auto& pair : group.nodes) {
        auto& [ x, y ] = pair.first; {}
        group.horizontalNodes = std::max(group.horizontalNodes, x + 1);
        group.verticalNodes = std::max(group.verticalNodes, y + 1);
    }

    return group;
}

void NeuralNetworkInspector::LayoutGroupsInView()
{
    /*
     * The aim here is to give each group an appropriate size based on its
     * content, then to arrange them so that senses are at the top,  the brain
     * in the middle and effectors at the bottom.
     */

    qreal totalSenseWidth = (sensorGroups_.size() * groupHSpacing_) - groupHSpacing_;
    qreal totalEffectorWidth = (effectorGroups_.size() * groupHSpacing_) - groupHSpacing_;

    qreal maxSenseHeight = 0.0;
    qreal maxEffectorHeight = 0.0;

    QFontMetricsF metrics = fontMetrics();

    ForEachGroup([&](GroupType type, Group& group)
    {
        QRectF titleRect = metrics.boundingRect(QString(group.name.c_str()));
        qreal nodesWidth = (2 * groupPadding_) + (group.horizontalNodes * (nodeWidth_ + nodeHSpacing_)) - (group.horizontalNodes ? nodeHSpacing_ : 0);
        qreal nodesHeight = (2 * groupPadding_) + (group.verticalNodes * (nodeHeight_ + nodeVSpacing_)) - (group.verticalNodes ? nodeVSpacing_ : 0);

        group.area.setSize(QSizeF(std::max(titleRect.width(), nodesWidth), titleRect.height() + nodesHeight));

        if (type == GroupType::Sense) {
            totalSenseWidth += group.area.width();
            maxSenseHeight = std::max(maxSenseHeight, group.area.height());
        } else if (type == GroupType::Effector) {
            totalEffectorWidth += group.area.width();
            maxEffectorHeight = std::max(maxEffectorHeight, group.area.height());
        }
    });

    /*
     * Centre everything around {0, 0}, senses at the top, effectors at the bottom
     */

    QRectF sensesRect(-totalSenseWidth / 2, -(maxSenseHeight + (groupVSpacing_ + brainGroup_.area.height() / 2)), totalSenseWidth, maxSenseHeight);
    qreal sensorsWidthSoFar = 0.0;
    for (auto& sense : sensorGroups_) {
        sense.area.moveTopLeft(sensesRect.topLeft());
        // Move sense groups down so their bases are lined up
        sense.area.translate(sensorsWidthSoFar, maxSenseHeight - sense.area.height());
        sensorsWidthSoFar += sense.area.width() + groupHSpacing_;
    }

    brainGroup_.area.moveTopLeft(QPointF(-brainGroup_.area.width() / 2.0, -(brainGroup_.area.height() / 2)));

    QRectF effectorsRect(-totalEffectorWidth / 2, (brainGroup_.area.height() / 2) + groupVSpacing_, totalEffectorWidth, maxEffectorHeight);
    qreal effectorsWidthSoFar = 0.0;
    for (auto& effector : effectorGroups_) {
        effector.area.moveTopLeft(effectorsRect.topLeft());
        // Do not move effector groups down so their tops remain aligned
        effector.area.translate(effectorsWidthSoFar, 0);
        effectorsWidthSoFar += effector.area.width() + groupHSpacing_;
    }

    ForEachGroup([&](GroupType type, Group& group)
    {
        LayoutNodesInGroup(type, group);
    });

    ResetViewTransform();
}

void NeuralNetworkInspector::LayoutNodesInGroup(GroupType type, Group& group)
{
    for (auto& [ nodeCoordinates, node ] : group.nodes) {
        auto& [column, row] = nodeCoordinates; {}
        qreal totalNodesWidth  = (nodeWidth_ + nodeHSpacing_) * (group.horizontalNodes - 1);
        qreal totalNodesHeight = (nodeHeight_ + nodeVSpacing_) * (group.verticalNodes - 1);
        qreal xOffset = (group.area.width() - totalNodesWidth) / 2;
        qreal yOffset = (((group.area.height() - fontMetrics().height()) - totalNodesHeight) / 2);
        // Effectors have their text below the nodes, every other group has it above
        if (type != GroupType::Effector) {
            yOffset += fontMetrics().height();
        }
        node.x = group.area.x() + xOffset + ((nodeWidth_ + nodeHSpacing_) * column);
        node.y = group.area.y() + yOffset + ((nodeHeight_ + nodeVSpacing_) * row);
    }
}

void NeuralNetworkInspector::ForwardPropogate()
{
    auto propogateGroup = [](Group& group)
    {
        for (unsigned y = 0; y < group.verticalNodes; ++y) {
            for (unsigned x = 0; x < group.horizontalNodes; ++x) {
                Node& node = group.nodes.at({ x, y });
                if (y != 0) {
                    // Don't normalise inputs
                    node.value = std::tanh(node.value);
                }
                for (auto& [ connectedNode, strength ] : node.connections) {
                    connectedNode.get().value += node.value * strength;
                }
            }
        }
    };

    for (auto& group : sensorGroups_) {
        propogateGroup(group);
    }
    propogateGroup(brainGroup_);
    for (auto& group : effectorGroups_) {
        propogateGroup(group);
    }
}

void NeuralNetworkInspector::ForEachGroup(std::function<void (GroupType type, Group& group)>&& action)
{
    for (auto& group : sensorGroups_) {
        action(GroupType::Sense, group);
    }

    action(GroupType::Brain, brainGroup_);

    for (auto& group : effectorGroups_) {
        action(GroupType::Effector, group);
    }}

void NeuralNetworkInspector::ForEachGroup(std::function<void (GroupType type, const Group&)>&& action) const
{
    for (const auto& group : sensorGroups_) {
        action(GroupType::Sense, group);
    }

    action(GroupType::Brain, brainGroup_);

    for (const auto& group : effectorGroups_) {
        action(GroupType::Effector, group);
    }
}
