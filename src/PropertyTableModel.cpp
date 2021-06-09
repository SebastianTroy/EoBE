#include "PropertyTableModel.h"

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

            if (auto* propertyModel = qobject_cast<PropertyTableModel*>(model)) {
                QModelIndex descriptionIndex = propertyModel->index(index.row(), PropertyTableModel::MORE_INFO_COLUMN_INDEX, {});
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

PropertyTableModel::PropertyTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int PropertyTableModel::rowCount(const QModelIndex& /*parent*/) const
{
    return properties_.size();
}

int PropertyTableModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
}

QVariant PropertyTableModel::data(const QModelIndex& index, int role) const
{
    const Property& property = properties_.at(index.row());

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

void PropertyTableModel::SetProperties(std::vector<Property>&& properties)
{
    bool addingRows = properties.size() > properties_.size();
    bool removingRows = properties.size() < properties_.size();

    if (addingRows) {
        beginInsertRows({}, properties_.size(), properties.size() - 1);
    } else if (removingRows) {
        beginRemoveRows({}, properties.size(), properties_.size() - 1);
    }

    properties_.swap(properties);
    emit dataChanged(index(0, 0), index(properties_.empty() ? 0 : properties_.size() - 1, columnCount({})));

    if (addingRows) {
        endInsertRows();
    } else if (removingRows) {
        endRemoveRows();
    }
}

void PropertyTableModel::UpdateValues()
{
    emit dataChanged(index(0, VALUE_COLUMN_INDEX), index(properties_.empty() ? 0 : properties_.size() - 1, VALUE_COLUMN_INDEX), { Qt::DisplayRole });
}
