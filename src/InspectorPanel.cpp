#include "InspectorPanel.h"
#include "ui_InspectorPanel.h"

#include "Swimmer.h"
#include "Genome/Genome.h"

#include <QFileDialog>
#include <QSpacerItem>

#include <fstream>

InspectorPanel::InspectorPanel(QWidget *parent)
    : QTabWidget(parent)
    , ui(new Ui::InspectorPanel)
    , previewPixmap_(50, 50)
{
    ui->setupUi(this);
    ui->entityPreview->setAlignment(Qt::AlignCenter);
    ui->entityPreview->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Preferred);
    ui->entityPreviewScale->setRange(10, 120);
    ui->entityProperties->setContentsMargins(8, 3, 5, 3);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 5);
    ui->splitter->setStretchFactor(2, 3);

    /// Entity Inspector controlls
    connect(ui->refreshButton, &QPushButton::pressed, this, &InspectorPanel::UpdateEntityTab, Qt::QueuedConnection);
    connect(ui->entityPreviewScale, &QSlider::sliderMoved, this, &InspectorPanel::UpdateEntityPreview, Qt::QueuedConnection);

    /// NeuralNetowrk Inspector controlls
    connect(ui->liveUpdateSelector, &QCheckBox::toggled, [&](bool checked) { ui->brainInspector->SetUpdateLive(checked); });
    connect(ui->resetInspectorView, &QPushButton::pressed, ui->brainInspector, &NeuralNetworkInspector::ResetViewTransform, Qt::QueuedConnection);

    /// File Menu Options
    connect(ui->saveGenomeButton, &QPushButton::pressed, [&]()
    {
        if (auto swimmerPointer = std::dynamic_pointer_cast<Swimmer>(selectedEntity_)) {
            if (auto genome = swimmerPointer->InspectGenome(); genome != nullptr) {
                std::string saveFileName = QFileDialog::getSaveFileName(this, "Save Genome", "./SavedGenomes/", "Genome (*.genome)").toStdString();
                nlohmann::json serialised = Genome::Serialise(genome);
                // std::filesystem::create_directories(saveFileName);
                std::ofstream fileWriter(saveFileName);
                fileWriter << serialised.dump(2);
            }
        }
    });
}

InspectorPanel::~InspectorPanel()
{
    delete ui;
}

void InspectorPanel::SetUniverse(std::shared_ptr<Universe> universe)
{
    universe_ = universe;
}

void InspectorPanel::SetEntity(std::shared_ptr<Entity> selectedEntity)
{
    if (selectedEntity != selectedEntity_) {
        selectedEntity_ = selectedEntity;
        // Entity tab
        ui->entityPropertyDescription->setText("Press '...' in the table above for more information about a value.");
        UpdateEntityTab();

        // Brain & Genome tabs
        std::shared_ptr<Swimmer> swimmerPointer = std::dynamic_pointer_cast<Swimmer>(selectedEntity_);
        ui->brainTab->setVisible(swimmerPointer != nullptr);
        ui->genomeTab->setVisible(swimmerPointer != nullptr);
        if (swimmerPointer || selectedEntity == nullptr) {
            ui->brainInspector->SetSwimmer(swimmerPointer); // TODO switch to SetEntity and deal with non Swimmer entities in the tab itself
            ui->brainInspector->UpdateConnectionStrengths(universe_->GetEntityContainer(), universe_->GetParameters());
        }
    }
}

void InspectorPanel::OnUniverseTick()
{
    // Entity tab
    if (ui->refreshAutoCheckBox->isChecked()) {
        UpdateEntityTab();
    }
    // Brain tab
    ui->brainInspector->UpdateConnectionStrengths(universe_->GetEntityContainer(), universe_->GetParameters());
}

void InspectorPanel::UpdateEntityTab()
{
    if (isVisible()) {
        ui->entityProperties->hide();
        ui->entityProperties->clear();
        UpdateEntityPreview();
        if (selectedEntity_) {
            ui->entityProperties->setRowCount(0);
            ui->entityProperties->setColumnCount(3);

            for (const auto& property : selectedEntity_->GetProperties()) {
                int rowIndex = ui->entityProperties->rowCount();
                ui->entityProperties->insertRow(rowIndex);
                QPushButton* moreInfoAction = new QPushButton("...");
                moreInfoAction->setFixedWidth(30);
                connect(moreInfoAction, &QPushButton::pressed, [&, description = QString::fromStdString(property.description_)]()
                {
                    ui->entityPropertyDescription->setText(description);
                });
                ui->entityProperties->setCellWidget(rowIndex, 0, moreInfoAction);
                ui->entityProperties->setItem(rowIndex, 1, new QTableWidgetItem(QString::fromStdString(property.name_)));
                ui->entityProperties->setItem(rowIndex, 2, new QTableWidgetItem(QString::fromStdString(property.value_())));
            }
        } else {
            ui->entityProperties->setRowCount(1);
            ui->entityProperties->setColumnCount(2);

            QPushButton* moreInfoAction = new QPushButton("...");
            moreInfoAction->setFixedWidth(30);
            connect(moreInfoAction, &QPushButton::pressed, ui->entityPropertyDescription, [&]()
            {
                ui->entityPropertyDescription->setText("You can select an entity within the simulation by right clicking on it. Left click and drag can be used to move them around.");
            });
            ui->entityProperties->setCellWidget(0, 0, moreInfoAction);
            ui->entityProperties->setItem(0, 1, new QTableWidgetItem("When an entity is selected, details about it will appear here."));
        }
        ui->entityProperties->resizeColumnsToContents();
        ui->entityProperties->resizeRowsToContents();
        ui->entityProperties->show();
    }
}

void InspectorPanel::UpdateEntityPreview()
{
    if (selectedEntity_) {
        ui->entityPreview->clear();

        Transform entityTransform = selectedEntity_->GetTransform();
        double scale = (0.01 * ui->entityPreviewScale->value() * std::min(ui->entityPreview->width(), ui->entityPreview->height())) / (selectedEntity_->GetRadius() * 2);
        previewPixmap_ = QPixmap(ui->entityPreview->width(), ui->entityPreview->height());
        previewPixmap_.fill(Qt::white);

        QPainter paint(&previewPixmap_);
        paint.translate(ui->entityPreview->width() / 2.0, ui->entityPreview->height() / 2.0);
        paint.scale(scale, scale);
        paint.translate(-entityTransform.x, -entityTransform.y);
        selectedEntity_->Draw(paint);

        ui->entityPreview->setPixmap(previewPixmap_);
    } else {
        ui->entityPreview->clear();
        ui->entityPreview->setText("No preview available");
    }
}
