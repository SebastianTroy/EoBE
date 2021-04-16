#ifndef ENTITYSVGMANAGER_H
#define ENTITYSVGMANAGER_H

#include <QPixmap>
#include <QtXml/QDomDocument>

#include <functional>

class EntitySvgManager {
public:
    static std::shared_ptr<QPixmap> GetPixmap(const std::string_view& entityName, const QColor& colour, qreal resolution);

private:
    struct CachedSvg {
        CachedSvg(const std::string_view& entityName);

        QPixmap GenerateRecolouredPixmap(const QColor& colour, qreal resolution);

    private:
        QDomDocument xml_;
        QDomElement indexColourElement_;
        QDomElement indexHighlightColourElement_;
        QDomElement indexLowlightColourElement_;

        void Recolour(const QColor& colour);
    };

    inline static std::map<std::string_view, CachedSvg> cachedRenderers_ = {};
    inline static std::map<QRgb, std::weak_ptr<QPixmap>> cachedPixmaps_ = {};

    static void RecurseQDomElement(QDomElement node, const std::function<void (QDomElement&)>& perNodeAction);
};

#endif // ENTITYSVGMANAGER_H
