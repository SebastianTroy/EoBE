#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "FoodPellet.h"
#include "Egg.h"
#include "Trilobyte.h"
#include "MeatChunk.h"
#include "LineGraphContainerWidget.h"
#include "UniverseWidget.h"
#include "InspectorPanel.h"
#include "Genome/GeneFactory.h"

#include <RollingStatistics.h>

#include <nlohmann/json.hpp>
#include <fmt/core.h>

#include <QFileDialog>
#include <QCheckBox>
#include <QPainter>
#include <QGridLayout>
#include <QToolTip>
#include <QSpacerItem>

#include <fstream>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SetupPlayControls();

    setWindowTitle(QString::fromStdString(fmt::format("Trilobytes - Evolution Simulator v{}.{} {}", TOSTRING(VERSION_MAJOR), TOSTRING(VERSION_MINOR), TOSTRING(VERSION_ADDITIONAL))));

    universe_ = std::make_shared<Universe>(Rect{ -500, -500, 500, 500 });

    ui->newGraphButtonsContainer->setLayout(new QVBoxLayout());

    connect(this, &MainWindow::UniverseReset, ui->universe, &UniverseWidget::SetUniverse, Qt::QueuedConnection);
    connect(this, &MainWindow::UniverseReset, ui->inspectorPanel, &InspectorPanel::SetUniverse, Qt::QueuedConnection);
    connect(ui->universe, &UniverseWidget::EntitySelected, ui->inspectorPanel, &InspectorPanel::SetEntity, Qt::QueuedConnection);
    connect(ui->universe, &UniverseWidget::SpawnerSelected, ui->inspectorPanel, &InspectorPanel::SetSpawner, Qt::QueuedConnection);
    connect(ui->universe, &UniverseWidget::Painted, ui->inspectorPanel, &InspectorPanel::OnUniverseRedrawn, Qt::QueuedConnection);

    // Set reasonable initial proportions
    ui->horizontalSplitter->setSizes({ static_cast<int>(width() * 0.15), static_cast<int>(width() * 0.65), static_cast<int>(width() * 0.2) });
    ui->verticalSplitter->setSizes({ static_cast<int>(height() * 0.7), static_cast<int>(height() * 0.3) });

    /// Control Schemes
    /// Added in order of priority, highest priority first
    pickAndMoveEntityControlls_ = ui->universe->EmplaceBackControlScheme<ControlSchemePickAndMoveEntity>();
    pickAndMoveSpawnerControlls_ = ui->universe->EmplaceBackControlScheme<ControlSchemePickAndMoveSpawner>();
    panAndZoomControlls_ = ui->universe->EmplaceBackControlScheme<ControlSchemePanAndZoom>();

    QGridLayout* controlTabLayout = new QGridLayout();
    ui->controllsTab->setLayout(controlTabLayout);
    int rowIndex = 0;
    for (std::shared_ptr<ControlScheme> scheme : { std::static_pointer_cast<ControlScheme>(pickAndMoveEntityControlls_),
                                                   std::static_pointer_cast<ControlScheme>(pickAndMoveSpawnerControlls_),
                                                   std::static_pointer_cast<ControlScheme>(panAndZoomControlls_) }) {
        QString descriptionText = QString::fromStdString(scheme->GetDescription());

        QLabel* schemeNameLabel = new QLabel(QString::fromStdString(scheme->GetName()));
        schemeNameLabel->setToolTip(descriptionText);

        QCheckBox* schemeEnabledCheckbox = new QCheckBox("Enabled");
        schemeEnabledCheckbox->setToolTip(descriptionText);
        connect(schemeEnabledCheckbox, &QCheckBox::toggled, this, [=](bool enabled)
        {
            scheme->SetEnabled(enabled);
        }, Qt::QueuedConnection);
        schemeEnabledCheckbox->setChecked(true);

        QPushButton* schemeHelpButton = new QPushButton("Help");
        schemeHelpButton->setToolTip(descriptionText);
        connect(schemeHelpButton, &QPushButton::pressed, this, [=]()
        {
            QToolTip::showText(schemeHelpButton->mapToGlobal(schemeHelpButton->rect().center()), descriptionText);
        }, Qt::QueuedConnection);

        controlTabLayout->addWidget(schemeNameLabel, rowIndex, 0, Qt::AlignRight);
        controlTabLayout->addWidget(schemeEnabledCheckbox, rowIndex, 1, Qt::AlignLeft);
        controlTabLayout->addWidget(schemeHelpButton, rowIndex, 2);

        ++rowIndex;
    }
    controlTabLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Ignored, QSizePolicy::Expanding), rowIndex, 0, 1, 3);

    /// Zoom controlls
    connect(ui->zoomInButton, &QPushButton::pressed, ui->universe, &UniverseWidget::ZoomIn, Qt::QueuedConnection);
    connect(ui->zoomOutButton, &QPushButton::pressed, ui->universe, &UniverseWidget::ZoomOut, Qt::QueuedConnection);
    connect(ui->zoomHomeButton, &QPushButton::pressed, ui->universe, [&]()
    {
        ui->universe->ZoomReset();
        ui->universe->PanReset();
    }, Qt::QueuedConnection);

    /// Global controlls
    connect(ui->resetAllButton, &QPushButton::pressed, this, [&]()
    {
        universe_ = std::make_shared<Universe>(Rect{ -500, -500, 500, 500 });
        emit UniverseReset(universe_);
        ResetGraphs();
    }, Qt::QueuedConnection);
    connect(ui->removeAllTrilobytesButton, &QPushButton::pressed, this, [&]()
    {
        universe_->ClearAllEntitiesOfType<Trilobyte, Egg>();
    }, Qt::QueuedConnection);
    connect(ui->removeAllFoodButton, &QPushButton::pressed, this, [&]()
    {
        universe_->ClearAllEntitiesOfType<FoodPellet, MeatChunk>();
    }, Qt::QueuedConnection);
    connect(ui->addDefaultTrilobyteButton, &QPushButton::pressed, this, [&]()
    {
        auto point = ApplyOffset({0, 0}, Random::Bearing(), Random::Number(0.0, 1000.0));
        universe_->AddEntity(std::make_shared<Trilobyte>(300_mj, Transform{ point.x, point.y, Random::Bearing() }, GeneFactory::Get().GenerateDefaultGenome(NeuralNetwork::BRAIN_WIDTH)));
    }, Qt::QueuedConnection);
    connect(ui->addRandomTrilobyteButton, &QPushButton::pressed, this, [&]()
    {
        auto point = ApplyOffset({0, 0}, Random::Bearing(), Random::Number(0.0, 1000.0));
        universe_->AddEntity(std::make_shared<Trilobyte>(300_mj, Transform{ point.x, point.y, Random::Bearing() }, GeneFactory::Get().GenerateRandomGenome(NeuralNetwork::BRAIN_WIDTH)));
    }, Qt::QueuedConnection);
    connect(ui->quadCapacitySpinner, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](int capacity)
    {
        universe_->SetEntityTargetPerQuad(capacity, ui->quadLeewaySpinner->value());
    }, Qt::QueuedConnection);
    connect(ui->quadLeewaySpinner, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [&](int leeway)
    {
        universe_->SetEntityTargetPerQuad(ui->quadCapacitySpinner->value(), leeway);
    }, Qt::QueuedConnection);

    connect(ui->meanGeneMutationSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [&](double mean) { universe_->GetParameters().meanGeneMutationCount_ = mean; }, Qt::QueuedConnection);
    connect(ui->geneMutationStdDevSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [&](double stdDev) { universe_->GetParameters().geneMutationCountStdDev_ = stdDev; }, Qt::QueuedConnection);
    connect(ui->meanChromosomeMutationSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [&](double mean) { universe_->GetParameters().meanStructuralMutationCount_ = mean; }, Qt::QueuedConnection);
    connect(ui->chromosomeMutationStdDevSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [&](double stdDev) { universe_->GetParameters().structuralMutationCountStdDev_ = stdDev; }, Qt::QueuedConnection);

    /// Spawner Controlls
    connect(ui->spawnEntitiesToggle, &QPushButton::toggled, this, [&](bool state) { universe_->GetParameters().spawnRateModifier = state ? 1.0 : 0.0; }, Qt::QueuedConnection);

    ui->newSpawnerShapeCombo->addItem("Square", QVariant::fromValue(Spawner::Shape::Square));
    ui->newSpawnerShapeCombo->addItem("Circle", QVariant::fromValue(Spawner::Shape::Circle));

    ui->newSpawnerTypeCombo->addItem("FoodPellet", QVariant::fromValue(Spawner::Spawn::FoodPellet));
    ui->newSpawnerTypeCombo->addItem("MeatChunk", QVariant::fromValue(Spawner::Spawn::MeatChunk));
    ui->newSpawnerTypeCombo->addItem("Spike", QVariant::fromValue(Spawner::Spawn::Spike));

    // Keep for lifetime of UI
    static auto handle = ui->universe->AddDrawOperation([&](QPainter& paint)
    {
        if (ui->newSpawnerPreviewCheckBox->isChecked() && (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count() % 1000 > 150)) {
            double x = ui->newSpawnerXSpinBox->value();
            double y = ui->newSpawnerYSpinBox->value();
            double radius = ui->newSpawnerRadiusSpinBox->value();
            Spawner::Shape shape = static_cast<Spawner::Shape>(ui->newSpawnerShapeCombo->currentData().value<std::underlying_type_t<Spawner::Shape>>());
            Spawner::Spawn spawnType = static_cast<Spawner::Spawn>(ui->newSpawnerTypeCombo->currentData().value<std::underlying_type_t<Spawner::Spawn>>());
            Spawner::Draw(paint, spawnType, shape, x, y, radius, true);
        }
    });

    connect(ui->newSpawnerCreateButton, &QPushButton::clicked, this, &MainWindow::AddSpawner, Qt::QueuedConnection);

    /// Selected Trilobyte Controlls
    connect(ui->selectFittestButton, &QPushButton::pressed, this, [&]()
    {
        pickAndMoveEntityControlls_->SelectFittestTrilobyte();
    }, Qt::QueuedConnection);
    connect(ui->followSelectedToggle, &QPushButton::toggled, this, [&](bool checked)
    {
        pickAndMoveEntityControlls_->SetTrackSelectedEntity(checked);
    }, Qt::QueuedConnection);

    /// Graph Controlls
    connect(ui->graphs, &QTabWidget::tabCloseRequested, [&](int index)
    {
        if (index != 0) {
            ui->graphs->removeTab(index);
        }
    });

    /// Draw controlls
    ui->showGridCheckbox->setChecked(ui->universe->GetDrawOptions().showQuadTreeGrid_);
    ui->showSpawnersCheckbox->setChecked(ui->universe->GetDrawOptions().showSpawners_);
    ui->useImagesCheckbox->setChecked(ui->universe->GetDrawOptions().showEntityImages_);
    ui->trilobyteDegugCheckbox->setChecked(ui->universe->GetDrawOptions().showTrilobyteDebug_);

    connect(ui->showGridCheckbox, &QCheckBox::toggled, ui->universe, [&](bool enabled){ ui->universe->GetDrawOptions().showQuadTreeGrid_ = enabled; }, Qt::QueuedConnection);
    connect(ui->showSpawnersCheckbox, &QCheckBox::toggled, ui->universe, [&](bool enabled){ ui->universe->GetDrawOptions().showSpawners_ = enabled; }, Qt::QueuedConnection);
    connect(ui->useImagesCheckbox, &QCheckBox::toggled, ui->universe, [&](bool enabled){ ui->universe->GetDrawOptions().showEntityImages_ = enabled; }, Qt::QueuedConnection);
    connect(ui->trilobyteDegugCheckbox, &QCheckBox::toggled, ui->universe, [&](bool enabled){ ui->universe->GetDrawOptions().showTrilobyteDebug_ = enabled; }, Qt::QueuedConnection);

    /// Debug controlls
    connect(ui->showPaintAndTickDurationsCheckbox, &QCheckBox::toggled, ui->universe, &UniverseWidget::SetDisplayDurationStats, Qt::QueuedConnection);
    connect(ui->showPaintAndTickFrequencyCheckbox, &QCheckBox::toggled, ui->universe, &UniverseWidget::SetDisplayRateStats, Qt::QueuedConnection);

    ResetGraphs();
    emit UniverseReset(universe_);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupPlayControls()
{
    ui->forwardButton->setChecked(true);
    ui->playStepContainer->setHidden(true);

    // Only show speed buttons when playing and step buttons when paused
    connect(ui->playPauseButton, &QPushButton::toggled, ui->playSpeedContainer, &QWidget::setHidden, Qt::QueuedConnection);
    connect(ui->playPauseButton, &QPushButton::toggled, ui->playStepContainer, &QWidget::setVisible, Qt::QueuedConnection);

    connect(ui->playPauseButton, &QPushButton::toggled, ui->universe, &UniverseWidget::SetTicksPaused, Qt::QueuedConnection);
    connect(ui->forwardButton, &QPushButton::toggled, this, [&]()
    {
        ui->universe->SetLimitTickRate(true);
        ui->universe->SetTpsTarget(40);
        ui->universe->SetFpsTarget(40);
    }, Qt::QueuedConnection);
    connect(ui->fastForwardButton, &QPushButton::toggled, this, [&]()
    {
        ui->universe->SetLimitTickRate(true);
        ui->universe->SetTpsTarget(120);
        ui->universe->SetFpsTarget(60);
    }, Qt::QueuedConnection);
    connect(ui->fastFastForwardButton, &QPushButton::toggled, this, [&]()
    {
        ui->universe->SetLimitTickRate(true);
        ui->universe->SetTpsTarget(500);
        ui->universe->SetFpsTarget(30);
    }, Qt::QueuedConnection);
    connect(ui->fullSpeedButton, &QPushButton::toggled, this, [&]()
    {
        ui->universe->SetLimitTickRate(false);
        ui->universe->SetFpsTarget(0.25);
    }, Qt::QueuedConnection);

    connect(ui->stepOneButton, &QPushButton::pressed, this, [&]()
    {
        ui->universe->StepForwards(1);
    }, Qt::QueuedConnection);
    connect(ui->stepTenButton, &QPushButton::pressed, this, [&]()
    {
        ui->universe->StepForwards(10);
    }, Qt::QueuedConnection);
    connect(ui->stepHundredButton, &QPushButton::pressed, this, [&]()
    {
        ui->universe->StepForwards(100);
    }, Qt::QueuedConnection);

}

void MainWindow::ResetGraphs()
{
    QString oldTitle = ui->graphs->tabText(ui->graphs->currentIndex());

    // Can't remove the current tab, so set to 1
    ui->graphs->setCurrentIndex(0);
    while (ui->graphs->count() > 1) {
        ui->graphs->removeTab(1);
    }

    // AddScatterGraph("Trilobyte speciation", "", "", [=](uint64_t tick, ScatterGraph& graph)
    // {
    //     std::vector<std::shared_ptr<Trilobyte>> trilobytes;
    //
    //     universe_->ForEach([&](const Entity& e) -> void
    //     {
    //         if (dynamic_cast<const Trilobyte*>(&e)) {
    //             trilobytes.push_back(std::dynamic_pointer_cast<Trilobyte>(e));
    //         }
    //     });
    //
    //     std::shared_ptr<Trilobyte> trilobyteA;
    //     std::shared_ptr<Trilobyte> trilobyteB;
    //
    //     for (auto& trilobyte : trilobytes) {
    //         for (auto& otherTrilobyte : trilobytes) {
    //             trilobyteA->
    //         }
    //     }
    //
    //     std::vector<ScatterGraph::DataPoint> points;
    //
    //
    //
    //     graph.SetPoints(std::move(points));
    // });
    AddLineGraph("Lunar Cycle", { {0x010101, "Moon Phase"} }, "Time (tick)", "Full (%)", [=](uint64_t tick, LineGraph& graph)
    {
        graph.AddPoint(0, tick, 50 * (universe_->GetParameters().lunarCycle_ + 1));
    });
    AddLineGraph("Population", { {0x00F100, "Food Pellet"}, {0xFF0000, "Meat Chunk"}, {0x3333FF, "Trilobyte"}, {0xFF44FF, "Egg"} }, "Time (tick)", "Number of Entities", [=](uint64_t tick, LineGraph& graph)
    {
        if (tick % 100 == 0) {
            uint64_t foodCount = 0;
            uint64_t chunkCount = 0;
            uint64_t trilobyteCount = 0;
            uint64_t eggCount = 0;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (dynamic_cast<const FoodPellet*>(&e)) {
                    ++foodCount;
                } else if (dynamic_cast<const MeatChunk*>(&e)) {
                    ++chunkCount;
                } else if (dynamic_cast<const Trilobyte*>(&e)) {
                    ++trilobyteCount;
                } else if (dynamic_cast<const Egg*>(&e)) {
                    ++eggCount;
                }
            });
            graph.AddPoint(0, tick, foodCount);
            graph.AddPoint(1, tick, chunkCount);
            graph.AddPoint(2, tick, trilobyteCount);
            graph.AddPoint(3, tick, eggCount);
        }
    });
    AddLineGraph("Total Energy", { {0x00F100, "Food Pellet"}, {0xFF0000, "Meat Chunk"}, {0x3333FF, "Trilobyte"}, {0xFF44FF, "Egg"} }, "Time (tick)", "Combined Energy (mj)", [=](uint64_t tick, LineGraph& graph)
    {
        if (tick % 100 == 0) {
            Energy foodEnergy = 0;
            Energy chunkEnergy = 0;
            Energy trilobyteEnergy = 0;
            Energy eggEnergy = 0;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (dynamic_cast<const FoodPellet*>(&e)) {
                    foodEnergy += e.GetEnergy();
                } else if (dynamic_cast<const MeatChunk*>(&e)) {
                    chunkEnergy += e.GetEnergy();
                } else if (dynamic_cast<const Trilobyte*>(&e)) {
                    trilobyteEnergy += e.GetEnergy();
                } else if (dynamic_cast<const Egg*>(&e)) {
                    eggEnergy += e.GetEnergy();
                }
            });
            graph.AddPoint(0, tick, foodEnergy / 1_mj);
            graph.AddPoint(1, tick, chunkEnergy / 1_mj);
            graph.AddPoint(2, tick, trilobyteEnergy / 1_mj);
            graph.AddPoint(3, tick, eggEnergy / 1_mj);
        }
    });
    AddLineGraph("Average Age", { {0x00FC00, "Food"}, {0x3333FF, "Trilobyte"} }, "Time (tick)", "Age (tick)", [=](uint64_t tick, LineGraph& graph)
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics foodStats;
            Tril::RollingStatistics trilobyteStats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (dynamic_cast<const FoodPellet*>(&e)) {
                    foodStats.AddValue(e.GetAge());
                } else if (dynamic_cast<const Trilobyte*>(&e)) {
                    trilobyteStats.AddValue(e.GetAge());
                }
            });
            graph.AddPoint(0, tick, foodStats.Mean());
            graph.AddPoint(1, tick, trilobyteStats.Mean());
        }
    });
    AddLineGraph("Max Age", { {0x00FC00, "Food"}, {0x3333FF, "Trilobyte"} }, "Time (tick)", "Age (tick)", [=](uint64_t tick, LineGraph& graph)
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics foodStats;
            Tril::RollingStatistics trilobyteStats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (dynamic_cast<const FoodPellet*>(&e)) {
                    foodStats.AddValue(e.GetAge());
                } else if (dynamic_cast<const Trilobyte*>(&e)) {
                    trilobyteStats.AddValue(e.GetAge());
                }
            });
            graph.AddPoint(0, tick, foodStats.Max());
            graph.AddPoint(1, tick, trilobyteStats.Max());
        }
    });
    AddLineGraph("Health (%)", { { 0xFFDFDF, "Min (%)" }, { 0xFF0000, "Mean (%)" }, { 0xFFDFDF, "Max (%)" }, { 0x00CF00, "StdDev lowerBound" }, { 0xCF3000, "StdDev upper bound" } }, "Time (tick)", "Trilobyte Health (%)", [=, previous = std::chrono::steady_clock::now()](uint64_t tick, LineGraph& graph) mutable
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics stats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (const Trilobyte* trilobyte = dynamic_cast<const Trilobyte*>(&e); trilobyte != nullptr) {
                    stats.AddValue(trilobyte->GetHealth());
                }
            });
            if (stats.Count() > 0) {
                graph.AddPoint(0, tick, stats.Min());
                graph.AddPoint(1, tick, stats.Mean());
                graph.AddPoint(2, tick, stats.Max());
                // Don't let our lower bound go below min
                graph.AddPoint(3, tick, std::max(stats.Min(), stats.Mean() - stats.StandardDeviation()));
                // Don't let our upper  bound go above max
                graph.AddPoint(4, tick, std::min(stats.Max(), stats.Mean() + stats.StandardDeviation()));
            }
        }
    });
    AddLineGraph("Generation", { { 0xDFDFDF, "Min" }, { 0x0000FF, "Mean" }, { 0xDFDFDF, "Max" }, { 0x00CF00, "StdDev lowerBound" }, { 0xCF3000, "StdDev upper bound" } }, "Time (tick)", "Trilobyte Generation", [=, previous = std::chrono::steady_clock::now()](uint64_t tick, LineGraph& graph) mutable
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics stats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (const Trilobyte* trilobyte = dynamic_cast<const Trilobyte*>(&e); trilobyte != nullptr) {
                    stats.AddValue(trilobyte->GetGeneration());
                }
            });
            if (stats.Count() > 0) {
                graph.AddPoint(0, tick, stats.Min());
                graph.AddPoint(1, tick, stats.Mean());
                graph.AddPoint(2, tick, stats.Max());
                // Don't let our lower bound go below min
                graph.AddPoint(3, tick, std::max(stats.Min(), stats.Mean() - stats.StandardDeviation()));
                // Don't let our upper  bound go above max
                graph.AddPoint(4, tick, std::min(stats.Max(), stats.Mean() + stats.StandardDeviation()));
            }
        }
    });
    AddLineGraph("Mutations", { { 0xF0F000, "Mean Gene Mutations" }, { 0xF000FF, "Mean Chromosome Mutations" } }, "Time (tick)", "Mutations per Genome", [=, previous = std::chrono::steady_clock::now()](uint64_t tick, LineGraph& graph) mutable
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics geneStats;
            Tril::RollingStatistics chromosomeStats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (const Trilobyte* trilobyte = dynamic_cast<const Trilobyte*>(&e); trilobyte != nullptr) {
                    geneStats.AddValue(trilobyte->GetGeneMutationCount());
                    chromosomeStats.AddValue(trilobyte->GetChromosomeMutationCount());
                }
            });
            if (geneStats.Count() > 0) {
                graph.AddPoint(0, tick, geneStats.Mean());
            }
            if (chromosomeStats.Count() > 0) {
                graph.AddPoint(1, tick, chromosomeStats.Mean());
            }
        }
    });
    AddLineGraph("Base Metabolism", { { 0xDFDFDF, "Min (uj)" }, { 0x0000FF, "Mean (uj)" }, { 0xDFDFDF, "Max (uj)" }, { 0x00CF00, "StdDev lowerBound" }, { 0xCF3000, "StdDev upper bound" } }, "Time (tick)", "Energy (uj)",
    [=, previous = std::chrono::steady_clock::now()](uint64_t tick, LineGraph& graph) mutable
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics stats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (const Trilobyte* trilobyte = dynamic_cast<const Trilobyte*>(&e); trilobyte != nullptr) {
                    stats.AddValue(trilobyte->GetBaseMetabolism());
                }
            });
            if (stats.Count() > 0) {
                graph.AddPoint(0, tick, stats.Min() / 1_uj);
                graph.AddPoint(1, tick, stats.Mean() / 1_uj);
                graph.AddPoint(2, tick, stats.Max() / 1_uj);
                // Don't let our lower bound go below min
                graph.AddPoint(3, tick, std::max(stats.Min(), stats.Mean() - stats.StandardDeviation()) / 1_uj);
                // Don't let our upper bound go above max
                graph.AddPoint(4, tick, std::min(stats.Max(), stats.Mean() + stats.StandardDeviation()) / 1_uj);
            }
        }
    });
    AddLineGraph("Trilobyte Velocity", { { 0xDFDFDF, "Min" }, { 0x0000FF, "Mean" }, { 0xDFDFDF, "Max" }, { 0x00CF00, "StdDev lowerBound" }, { 0xCF3000, "StdDev upper bound" } }, "Time (tick)", "Velocity (pixels per tick)",
    [=, previous = std::chrono::steady_clock::now()](uint64_t tick, LineGraph& graph) mutable
    {
        if (tick % 100 == 0) {
            Tril::RollingStatistics stats;
            universe_->ForEach([&](const Entity& e) -> void
            {
                if (const Trilobyte* trilobyte = dynamic_cast<const Trilobyte*>(&e); trilobyte != nullptr) {
                    stats.AddValue(std::abs(trilobyte->GetVelocity()));
                }
            });
            if (stats.Count() > 0) {
                graph.AddPoint(0, tick, stats.Min());
                graph.AddPoint(1, tick, stats.Mean());
                graph.AddPoint(2, tick, stats.Max());
                // Don't let our lower bound go below min
                graph.AddPoint(3, tick, std::max(stats.Min(), stats.Mean() - stats.StandardDeviation()));
                // Don't let our upper bound go above max
                graph.AddPoint(4, tick, std::min(stats.Max(), stats.Mean() + stats.StandardDeviation()));
            }
        }
    });
    AddLineGraph("Tick Performance", { { 0x0000FF, "Mean" }, { 0x00CF00, "StdDev lowerBound" }, { 0xCF3000, "StdDev upper bound" } }, "Time (tick)", "Tick Duration (ms)",
    [=](uint64_t tick, LineGraph& graph)
    {
        if (tick % 100 == 0) {
            const Tril::WindowedRollingStatistics& stats = ui->universe->GetTickDurationStats();
            graph.AddPoint(0, tick, 1000.0 * stats.Mean());
            graph.AddPoint(1, tick, 1000.0 * (stats.Mean() - stats.StandardDeviation()));
            graph.AddPoint(2, tick, 1000.0 * (stats.Mean() + stats.StandardDeviation()));
        }
    });
    AddLineGraph("Paint Performance", { { 0x0000FF, "Mean" }, { 0x00CF00, "StdDev lowerBound" }, { 0xCF3000, "StdDev upper bound" } }, "Time (tick)", "Paint Duration (ms)",
                 [=](uint64_t tick, LineGraph& graph)
    {
        if (tick % 100 == 0) {
            const Tril::WindowedRollingStatistics& stats = ui->universe->GetDrawDurationStats();
            graph.AddPoint(0, tick, 1000.0 * stats.Mean());
            graph.AddPoint(1, tick, 1000.0 * (stats.Mean() - stats.StandardDeviation()));
            graph.AddPoint(2, tick, 1000.0 * (stats.Mean() + stats.StandardDeviation()));
        }
    });

    for (int i = 0; i < ui->graphs->count(); ++i) {
        if (ui->graphs->tabText(i) == oldTitle) {
            ui->graphs->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::AddLineGraph(QString graphTitle, std::vector<std::pair<QRgb, QString> >&& plots, QString xAxisTitle, QString yAxisTitle, std::function<void (uint64_t tick, LineGraph& graph)>&& task)
{
    QPushButton* button = new QPushButton(QString("Create \"%1\" graph").arg(graphTitle));
    ui->newGraphButtonsContainer->layout()->addWidget(button);

    connect(button, &QPushButton::pressed, [=]()
    {
        LineGraph* lineGraph = new LineGraph(nullptr, xAxisTitle, yAxisTitle);
        for (const auto& [ colour, name ] : plots) {
            lineGraph->AddPlot(colour, name);
        }
        auto handle = universe_->AddTask([=, task = std::move(task)](uint64_t tick) { task(tick, *lineGraph); });
        ui->graphs->addTab(new LineGraphContainerWidget(nullptr, std::move(handle), lineGraph), graphTitle);
    });
    emit button->pressed();
}

void MainWindow::AddScatterGraph(QString graphTitle, QString xAxisTitle, QString yAxisTitle, std::function<void (uint64_t, ScatterGraph&)>&& task)
{
    QPushButton* button = new QPushButton(QString("Create \"%1\" graph").arg(graphTitle));
    ui->newGraphButtonsContainer->layout()->addWidget(button);

    connect(button, &QPushButton::pressed, [=]()
    {
        ScatterGraph* scatterGraph = new ScatterGraph(nullptr, graphTitle, xAxisTitle, yAxisTitle);
        // FIXME handle is going out of scope here, need to capture it !!!!!!!!

        auto handle = universe_->AddTask([=, task = std::move(task)](uint64_t tick) { task(tick, *scatterGraph); });
        ui->graphs->addTab(scatterGraph, graphTitle);
    });
    emit button->pressed();
}

void MainWindow::AddSpawner()
{
    if (universe_) {
        double x = ui->newSpawnerXSpinBox->value();
        double y = ui->newSpawnerYSpinBox->value();
        double radius = ui->newSpawnerRadiusSpinBox->value();
        unsigned spawnCap = ui->newSpawnerLimitSpinBox->value();
        double ticksPerSpawn = ui->newSpawnerRateSpinBox->value();
        Spawner::Shape shape = static_cast<Spawner::Shape>(ui->newSpawnerShapeCombo->currentData().value<std::underlying_type_t<Spawner::Shape>>());
        Spawner::Spawn spawnType = static_cast<Spawner::Spawn>(ui->newSpawnerTypeCombo->currentData().value<std::underlying_type_t<Spawner::Spawn>>());
        universe_->AddSpawner(std::make_shared<Spawner>(*universe_, x, y, radius, spawnCap, ticksPerSpawn, shape, spawnType));

        ui->newSpawnerPreviewCheckBox->setChecked(false);
    }
}
