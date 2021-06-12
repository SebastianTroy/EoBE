#ifndef UNIVERSEWIDGET_H
#define UNIVERSEWIDGET_H

#include "Universe.h"

#include <WindowedRollingStatistics.h>
#include <WindowedFrequencyStatistics.h>
#include <Shape.h>
#include <AutoClearingContainer.h>

// TODO QOpenGLWidget allows QPainter painting, but its messed up, consider moving over once everything is pixmap based
// see https://doc-snapshots.qt.io/qt6-dev/qopenglwidget.html for help re-implementing
#include <QWidget>
#include <QTimer>

class UniverseWidget final : public QWidget {
    Q_OBJECT
public:
    explicit UniverseWidget(QWidget* parent);
    virtual ~UniverseWidget();

    /**
     * @brief This allows for additional GUI elements to be added each time the
     * simulation redraws itself. For example, previewing where a new spawner
     * would be placed etc.
     *
     * @param drawTask The code to be run each time the simulation is redrawn.
     *
     * @return A Handle that defines the lifetime of the drawTask. When no
     * copies of the handle exist, the drawTask is removed from the list.
     */
    [[nodiscard]] Tril::Handle AddDrawOperation(std::function<void(QPainter& paint)>&& drawTask);

signals:
    void EntitySelected(const std::shared_ptr<Entity>& newSelection);
    void Ticked();

public slots:
    /*
     * The fllowing slots should be Queued to prevent multi-threaded universe
     * interactions
     */

    void SetUniverse(std::shared_ptr<Universe> newUniverse);

    void SetFpsTarget(double fps);
    void SetTpsTarget(double tps);
    void SetLimitTickRate(bool limit);
    void SetTicksPaused(bool paused);
    void StepForwards(unsigned ticksToStep);

    DrawSettings& DrawOptions() { return drawOptions_; }

    void SetDisplayDurationStats(bool display) { displayDurationStats_ = display; };
    void SetDisplayRateStats(bool display) { displayRateStats_ = display; };

    void SelectFittestTrilobyte();
    void SetTrackSelectedEntity(bool track) { trackSelected_ = track; }

    void RemoveAllTrilobytes();
    void RemoveAllFood();

    void ZoomIn();
    void ZoomOut();
    void ZoomReset();
    void PanReset();

    // Universe params
    void SetMeanGeneMutationCount(double mean) { universeParameters_.meanGeneMutationCount_ = mean; }
    void SetGeneMutationStdDev(double stdDev) { universeParameters_.geneMutationCountStdDev_ = stdDev; }
    void SetMeanChromosomeMutationCount(double mean) { universeParameters_.meanStructuralMutationCount_ = mean; }
    void SetChromosomeMutationStdDev(double stdDev) { universeParameters_.structuralMutationCountStdDev_ = stdDev; }

    const Tril::WindowedRollingStatistics& GetTickDurationStats() const { return tickDurationStats_; }
    const Tril::WindowedRollingStatistics& GetDrawDurationStats() const{ return drawDurationStats_; }

protected:
    virtual void wheelEvent(QWheelEvent* event) override final;
    virtual void mouseReleaseEvent(QMouseEvent* event) override final;
    virtual void mousePressEvent(QMouseEvent* event) override final;
    virtual void mouseMoveEvent(QMouseEvent* event) override final;

    virtual void resizeEvent(QResizeEvent* event) override final;

    virtual void paintEvent(QPaintEvent* event) override final;

private slots:
    void OnTickTimerElapsed();
    void OnDrawTimerElapsed();

private:
    QTimer tickThread_;
    QTimer drawThread_;
    bool limitTickRate_ = true;
    bool ticksPaused_ = false;
    double ticksPerSecondTarget_ = 60.0;

    Tril::WindowedRollingStatistics tickDurationStats_;
    Tril::WindowedRollingStatistics drawDurationStats_;
    Tril::WindowedFrequencyStatistics tickRateStats_;
    Tril::WindowedFrequencyStatistics drawRateStats_;
    bool displayRateStats_ = false;
    bool displayDurationStats_ = false;

    // TODO update to using a matrix based transform
    qreal transformX_ = 0.0;
    qreal transformY_ = 0.0;
    qreal transformScale_ = 1.0;
    qreal dragX_ = 0.0;
    qreal dragY_ = 0.0;
    bool dragging_ = false;

    UniverseParameters universeParameters_;
    std::shared_ptr<Universe> universe_;
    bool trackSelected_ = false;
    std::shared_ptr<Entity> selectedEntity_;
    std::shared_ptr<Entity> draggedEntity_;

    DrawSettings drawOptions_;
    Tril::AutoClearingContainer<std::function<void(QPainter& paint)>> perDrawTasks_;

    Point TransformLocalToSimCoords(const Point& local) const;
    Point TransformSimToLocalCoords(const Point& sim) const;

    void Tick();
    void UpdateTps();
};

#endif // UNIVERSEWIDGET_H
