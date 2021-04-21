#include "InspectorPanel.h"
#include "ui_InspectorPanel.h"

#include "Trilobyte.h"
#include "Genome/Genome.h"

#include <QFileDialog>
#include <QSpacerItem>

#include <fstream>

InspectorPanel::InspectorPanel(QWidget *parent)
    : QTabWidget(parent)
    , ui(new Ui::InspectorPanel)
    , previewPixmap_(50, 50)
    , newEntity_(true)
    , entityPropertyModel_(this)
    , entityPropertyDetailButtonDelegate_(this)
{
    ui->setupUi(this);
    ui->entityPreview->setAlignment(Qt::AlignCenter);
    ui->entityPreviewScale->setRange(10, 120);
    ui->entityPreviewScale->setValue(70);
    ui->entityProperties->setModel(&entityPropertyModel_);
    ui->entityProperties->setItemDelegateForColumn(EntityPropertyTableModel::MORE_INFO_COLUMN_INDEX, &entityPropertyDetailButtonDelegate_);
    ui->entityProperties->setColumnWidth(EntityPropertyTableModel::MORE_INFO_COLUMN_INDEX, 30);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 5);
    ui->splitter->setStretchFactor(2, 3);
    connect(ui->splitter, &QSplitter::splitterMoved, this, &InspectorPanel::UpdateEntityTab, Qt::QueuedConnection);

    /// TODO draw option controlls
    // Only interested in Trilobyte based draw settings for this view
    // (unless there is the option to render the Universe in the view, centred
    //  on the selected entity, then the other options are emaningful)
    drawSettings_.showTrilobyteDebug_ = true;

    /// Entity Inspector controlls
    connect(&entityPropertyModel_, &EntityPropertyTableModel::DescriptionRequested, [&](QString description)
    {
        ui->entityPropertyDescription->setText(description);
    });
    connect(ui->refreshButton, &QPushButton::pressed, this, &InspectorPanel::UpdateEntityTab, Qt::QueuedConnection);
    connect(ui->entityPreviewScale, &QSlider::valueChanged, this, &InspectorPanel::UpdateEntityPreview, Qt::QueuedConnection);

    /// NeuralNetowrk Inspector controlls
    connect(ui->liveUpdateSelector, &QCheckBox::toggled, [&](bool checked) { ui->brainInspector->SetUpdateLive(checked); });
    connect(ui->resetInspectorView, &QPushButton::pressed, ui->brainInspector, &NeuralNetworkInspector::ResetViewTransform, Qt::QueuedConnection);

    /// File Menu Options
    connect(ui->saveGenomeButton, &QPushButton::pressed, [&]()
    {
        if (auto trilobytePointer = std::dynamic_pointer_cast<Trilobyte>(selectedEntity_)) {
            if (auto genome = trilobytePointer->InspectGenome(); genome != nullptr) {
                std::string saveFileName = QFileDialog::getSaveFileName(this, "Save Genome", "./SavedGenomes/", "Genome (*.genome)").toStdString();
                nlohmann::json serialised = Genome::Serialise(genome);
                // std::filesystem::create_directories(saveFileName);
                std::ofstream fileWriter(saveFileName);
                fileWriter << serialised.dump(2);
            }
        }
    });

    SetEntity({});
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
    if (!selectedEntity_ || selectedEntity != selectedEntity_) {
        newEntity_ = true;
        selectedEntity_ = selectedEntity;
        if (selectedEntity_) {
            SetProperties(selectedEntity_->GetProperties());
        } else {
            SetProperties({});
        }
        // Entity tab
        ui->entityPropertyDescription->setText("Press '...' in the table above for more information about a value.");
        UpdateEntityTab();

        // Brain & Genome tabs
        std::shared_ptr<Trilobyte> trilobytePointer = std::dynamic_pointer_cast<Trilobyte>(selectedEntity_);
        setTabVisible(BRAIN_TAB_INDEX, trilobytePointer != nullptr);
        setTabVisible(GENOME_TAB_INDEX, trilobytePointer != nullptr);
        if (trilobytePointer || selectedEntity == nullptr) {
            ui->brainInspector->SetTrilobyte(trilobytePointer); // TODO switch to SetEntity and deal with non Trilobyte entities in the tab itself
            ui->brainInspector->UpdateConnectionStrengths(universe_->GetEntityContainer(), universe_->GetParameters());
        }

        update();
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
    if (currentIndex() == ENTITY_TAB_INDEX) {
        UpdateEntityPreview();
        entityPropertyModel_.UpdateValues();
    }
}

void InspectorPanel::UpdateEntityPreview()
{
    if (selectedEntity_) {
        ui->entityPreview->clear();

        Transform entityTransform = selectedEntity_->GetTransform();
        double scale = (0.01 * ui->entityPreviewScale->value() * std::min(ui->entityPreview->width(), ui->entityPreview->height())) / (Entity::MAX_RADIUS * 2);
        previewPixmap_ = QPixmap(ui->entityPreview->width(), ui->entityPreview->height());
        previewPixmap_.fill(Qt::white);

        QPainter paint(&previewPixmap_);
        paint.translate(ui->entityPreview->width() / 2.0, ui->entityPreview->height() / 2.0);
        paint.scale(scale, scale);
        paint.translate(-entityTransform.x, -entityTransform.y);
        selectedEntity_->Draw(paint, drawSettings_);

        ui->entityPreview->setPixmap(previewPixmap_);
    } else {
        ui->entityPreview->clear();
        ui->entityPreview->setText("No preview available");
    }
}

void InspectorPanel::SetProperties(std::vector<Property>&& properties)
{
    entityPropertyModel_.SetProperties(std::move(properties));
    ui->entityProperties->resizeColumnsToContents();
    ui->entityProperties->setColumnWidth(EntityPropertyTableModel::MORE_INFO_COLUMN_INDEX, 30);
}
