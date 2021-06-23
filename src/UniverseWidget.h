#ifndef UNIVERSEWIDGET_H
#define UNIVERSEWIDGET_H

#include "Universe.h"
#include "ControlScheme.h"

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

    template <typename ControlSchemeType, typename... Args>
    std::shared_ptr<ControlSchemeType> EmplaceBackControlScheme(Args... args)
    {
        controlSchemes_.push_back(std::make_shared<ControlSchemeType>(*this, std::forward<Args>(args)...));
        return std::dynamic_pointer_cast<ControlSchemeType>(controlSchemes_.back());
    }

signals:
    void EntitySelected(const std::shared_ptr<Entity>& newSelection);
    void SpawnerSelected(const std::shared_ptr<Spawner>& newSelection);
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

    void SetDisplayDurationStats(bool display) { displayDurationStats_ = display; };
    void SetDisplayRateStats(bool display) { displayRateStats_ = display; };

    DrawSettings& GetDrawOptions() { return drawOptions_; }
    std::shared_ptr<Universe> GetUniverse() { return universe_; }

    void RemoveAllTrilobytes();
    void RemoveAllFood();

    void Zoom(int ticks);
    void ZoomIn();
    void ZoomOut();
    void ZoomReset();
    void Pan(double xDistance, double yDistance);
    void PanReset();

    // Universe params
    void SetMeanGeneMutationCount(double mean) { universeParameters_.meanGeneMutationCount_ = mean; }
    void SetGeneMutationStdDev(double stdDev) { universeParameters_.geneMutationCountStdDev_ = stdDev; }
    void SetMeanChromosomeMutationCount(double mean) { universeParameters_.meanStructuralMutationCount_ = mean; }
    void SetChromosomeMutationStdDev(double stdDev) { universeParameters_.structuralMutationCountStdDev_ = stdDev; }

    void SetPanTransform(Point transform) { transformX_ = transform.x; transformY_ = transform.y; update(); }
    void SetZoomTransform(double transform) { transformScale_ = transform; update(); }

    Point GetPanTransform() const { return { transformX_, transformY_ }; }
    double GetZoomTransform() const { return transformScale_; }

    const Tril::WindowedRollingStatistics& GetTickDurationStats() const { return tickDurationStats_; }
    const Tril::WindowedRollingStatistics& GetDrawDurationStats() const{ return drawDurationStats_; }

    Point TransformLocalToSimCoords(const Point& local) const;
    Point TransformSimToLocalCoords(const Point& sim) const;

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

    std::vector<std::shared_ptr<ControlScheme>> controlSchemes_;

    qreal transformX_ = 0.0;
    qreal transformY_ = 0.0;
    qreal transformScale_ = 1.0;

    UniverseParameters universeParameters_;
    std::shared_ptr<Universe> universe_;

    DrawSettings drawOptions_;
    Tril::AutoClearingContainer<std::function<void(QPainter& paint)>> perDrawTasks_;

    void Tick();
    void UpdateTps();
};

#endif // UNIVERSEWIDGET_H
