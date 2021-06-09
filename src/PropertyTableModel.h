#ifndef PROPERTYTABLEMODEL_H
#define PROPERTYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QItemDelegate>

/**
 * Designed to allow inspection of values of unspecified type.
 */
struct Property{
    std::string name_;
    std::function<std::string()> value_;
    std::string description_;
};

class MyDelegate : public QItemDelegate {
    Q_OBJECT
public:
    explicit MyDelegate(QObject* parent);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    constexpr static int MARGIN = 3;
    int pressedIndex_;
};

class PropertyTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    constexpr static int MORE_INFO_COLUMN_INDEX = 0;
    constexpr static int NAME_COLUMN_INDEX = 1;
    constexpr static int VALUE_COLUMN_INDEX = 2;

    explicit PropertyTableModel(QObject* parent);

    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;

    void SetProperties(std::vector<Property>&& properties);
    void UpdateValues();

signals:
    void DescriptionRequested(QString description);

private:
    std::vector<Property> properties_;
};

#endif // PROPERTYTABLEMODEL_H
