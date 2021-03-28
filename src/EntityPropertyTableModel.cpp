#include "EntityPropertyTableModel.h"

#include <QApplication>
#include <QMouseEvent>

///
/// Button delegate
///

MyDelegate::MyDelegate(QObject *parent)
    : QItemDelegate(parent)
    , pressedIndex_(-1)
{
}

void MyDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionButton button;
    button.rect = option.rect.adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN);
    button.text = "...";
    button.state = QStyle::State_Enabled | (index.row() == pressedIndex_ ? QStyle::State_Sunken : QStyle::State_Raised);
    QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
}

bool MyDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QRect buttonRect = option.rect.adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN);
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (buttonRect.contains(mouseEvent->pos())) {
            pressedIndex_ = index.row();

            if (auto* propertyModel = qobject_cast<EntityPropertyTableModel*>(model)) {
                QModelIndex descriptionIndex = propertyModel->index(index.row(), EntityPropertyTableModel::MORE_INFO_COLUMN_INDEX, {});
                emit propertyModel->DescriptionRequested(propertyModel->data(descriptionIndex, Qt::UserRole).toString());
            }
        }
    } else {
        pressedIndex_ = -1;
    }

    return true;
}

///
/// Model
///

EntityPropertyTableModel::EntityPropertyTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int EntityPropertyTableModel::rowCount(const QModelIndex& /*parent*/) const
{
    return entityProperties_.size();
}

int EntityPropertyTableModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
}

QVariant EntityPropertyTableModel::data(const QModelIndex& index, int role) const
{
    const Property& property = entityProperties_.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case NAME_COLUMN_INDEX :
            return QString::fromStdString(property.name_);
        case VALUE_COLUMN_INDEX :
            return QString::fromStdString(property.value_());
        case MORE_INFO_COLUMN_INDEX :
            return QString::fromStdString("...");
        }
    } else if (role == Qt::UserRole && index.column() == MORE_INFO_COLUMN_INDEX) {
        return QString::fromStdString(property.description_);
    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case VALUE_COLUMN_INDEX :
            return QString::fromStdString(property.description_);
            break;
        }
    }
    return {};
}

void EntityPropertyTableModel::SetProperties(std::vector<Property>&& properties)
{
    if (properties.empty()) {
        properties.push_back({
                                 "Example",
                                 [](){ return "When an entity is selected, details about it will appear here."; },
                                 "You can select an entity within the simulation by right clicking on it. Left click and drag can be used to move them around.",
                             });
    }

    bool addingRows = properties.size() > entityProperties_.size();
    bool removingRows = properties.size() < entityProperties_.size();

    if (addingRows) {
        beginInsertRows({}, entityProperties_.size(), properties.size() - 1);
    } else if (removingRows) {
        beginRemoveRows({}, properties.size(), entityProperties_.size() - 1);
    }

    entityProperties_.swap(properties);
    emit dataChanged(index(0, 0), index(entityProperties_.empty() ? 0 : entityProperties_.size() - 1, columnCount({})));

    if (addingRows) {
        endInsertRows();
    } else if (removingRows) {
        endRemoveRows();
    }
}

void EntityPropertyTableModel::UpdateValues()
{
    emit dataChanged(index(0, VALUE_COLUMN_INDEX), index(entityProperties_.empty() ? 0 : entityProperties_.size() - 1, VALUE_COLUMN_INDEX), { Qt::DisplayRole });
}
