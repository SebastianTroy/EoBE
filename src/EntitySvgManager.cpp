#include "EntitySvgManager.h"

#include <QFile>
#include <QPainter>
#include <QtSvg/QSvgRenderer>

///
/// EntitySvgManager
///

std::shared_ptr<QPixmap> EntitySvgManager::GetPixmap(const std::string_view& entityName, const QColor& colour, qreal resolution)
{
    if (cachedRenderers_.count(entityName) == 0) {
        cachedRenderers_.emplace(std::piecewise_construct, std::forward_as_tuple(entityName), std::forward_as_tuple(entityName));
    }
    if (cachedPixmaps_.count(colour.rgb()) == 0) {
        cachedPixmaps_.emplace(colour.rgb(), std::weak_ptr<QPixmap>());
    }

    std::shared_ptr<QPixmap> pixmap = cachedPixmaps_.at(colour.rgb()).lock();

    if (!pixmap) {
        CachedSvg& svgRenderer = cachedRenderers_.at(entityName);
        pixmap = std::make_shared<QPixmap>(svgRenderer.GenerateRecolouredPixmap(colour, resolution));
        cachedPixmaps_.at(colour.rgb()) = pixmap;
    }

    return pixmap;
}

void EntitySvgManager::RecurseQDomElement(QDomElement node, const std::function<void (QDomElement&)>& perNodeAction)
{
    perNodeAction(node);
    QDomNodeList paths = node.childNodes();
    for (int i = 0; i < paths.count(); ++i) {
        QDomNode childNode = paths.at(i);
        if (childNode.isElement()) {
            RecurseQDomElement(childNode.toElement(), perNodeAction);
        }
    }
}

///
/// CachedSvg
///

EntitySvgManager::CachedSvg::CachedSvg(const std::string_view& entityName)
{
    QFile file(QString(":/images/images/%1.svg").arg(QString::fromStdString(std::string(entityName))));
    file.open(QFile::ReadOnly | QFile::Text);
    xml_.setContent(&file);

    RecurseQDomElement(xml_.documentElement(), [&](QDomElement& element)
    {
        if(element.attribute("id") == "indexColour") {
            indexColourElement_ = element;
        } else if(element.attribute("id") == "indexColourHighlight") {
            indexHighlightColourElement_ = element;
        } else if(element.attribute("id") == "indexColourLowlight") {
            indexLowlightColourElement_ = element;
        }
    });
}

QPixmap EntitySvgManager::CachedSvg::GenerateRecolouredPixmap(const QColor& colour, qreal resolution)
{
    Recolour(colour);

    QSvgRenderer svg(xml_.toByteArray());
    QRect imageRect(QPoint(0, 0), svg.defaultSize().scaled(resolution, resolution, Qt::KeepAspectRatio));
    QImage image(imageRect.size(), QImage::Format::Format_ARGB32);
    image.fill(QColor::fromRgba(0x000000FF));
    QPainter paint(&image);
    svg.render(&paint);
    return QPixmap::fromImage(image);
}

void EntitySvgManager::CachedSvg::Recolour(const QColor& colour)
{
    indexColourElement_.setAttribute("style", QString("%2;fill:%1").arg(colour.name(), indexColourElement_.attribute("style", "")));
    indexHighlightColourElement_.setAttribute("style", QString("%2;fill:%1").arg(colour.lighter(130).name(), indexHighlightColourElement_.attribute("style", "")));
    indexLowlightColourElement_.setAttribute("style", QString("%2;fill:%1").arg(colour.darker(130).name(), indexLowlightColourElement_.attribute("style", "")));
}
