#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Universe.h"
#include "ScatterGraph.h"
#include "Entity.h"

#include <QMainWindow>
#include <QImage>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void UniverseReset(std::shared_ptr<Universe> newUniverse);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Universe> universe_;

    void SetupPlayControls();
    void ResetGraphs();
    void AddLineGraph(QString graphTitle, std::vector<std::pair<QRgb, QString>>&& plots, QString xAxisTitle, QString yAxisTitle, std::function<void(uint64_t tick, LineGraph& graph)>&& task);
    void AddScatterGraph(QString graphTitle, QString xAxisTitle, QString yAxisTitle, std::function<void(uint64_t tick, ScatterGraph& graph)>&& task);
};
#endif // MAINWINDOW_H
