#include "ui/shared_widgets/IconFactory.h"

#include <QHash>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

namespace courierman::ui {
namespace {

[[nodiscard]] QString materialPath(const QString& name) {
    static const QHash<QString, QString> paths{
        {QStringLiteral("add"), QStringLiteral("M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2Z")},
        {QStringLiteral("more"), QStringLiteral("M6 10c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2Zm6 0c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2Zm6 0c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2Z")},
        {QStringLiteral("collections"), QStringLiteral("M20 6h-8.17l-2-2H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2Z")},
        {QStringLiteral("api"), QStringLiteral("M7 7h4v2H9v6H7V7Zm6 0h4v8h-2V9h-2V7ZM4 11H2V9h2V7h2v2h2v2H6v2H4v-2Zm16-2h2v2h-2v2h-2v-2h-2V9h2V7h2v2ZM9 17h6v2H9v-2Z")},
        {QStringLiteral("history"), QStringLiteral("M13 3a9 9 0 1 1-8.95 10H2l3-3 3 3H6.05A7 7 0 1 0 8.1 6.1L6.7 4.7A8.96 8.96 0 0 1 13 3Zm-1 5h1.5v4.25l3.5 2.08-.75 1.23L12 13V8Z")},
        {QStringLiteral("environments"), QStringLiteral("M12 2a7 7 0 0 0-7 7c0 5.25 7 13 7 13s7-7.75 7-13a7 7 0 0 0-7-7Zm0 9.5A2.5 2.5 0 1 1 12 6a2.5 2.5 0 0 1 0 5.5Z")},
        {QStringLiteral("settings"), QStringLiteral("M19.43 12.98c.04-.32.07-.65.07-.98s-.02-.66-.07-.98l2.11-1.65-2-3.46-2.49 1a7.28 7.28 0 0 0-1.69-.98L15 3h-4l-.36 2.93c-.6.23-1.16.56-1.69.98l-2.49-1-2 3.46 2.11 1.65c-.04.32-.07.65-.07.98s.02.66.07.98l-2.11 1.65 2 3.46 2.49-1c.52.4 1.08.73 1.69.98L11 21h4l.36-2.93c.6-.23 1.16-.56 1.69-.98l2.49 1 2-3.46-2.11-1.65ZM13 15.5A3.5 3.5 0 1 1 13 8a3.5 3.5 0 0 1 0 7.5Z")},
        {QStringLiteral("sidebar_left"), QStringLiteral("M3 5c0-1.1.9-2 2-2h14c1.1 0 2 .9 2 2v14c0 1.1-.9 2-2 2H5c-1.1 0-2-.9-2-2V5Zm5 0v14h11V5H8Zm-2 0H5v14h1V5Z")},
        {QStringLiteral("sidebar_right"), QStringLiteral("M3 5c0-1.1.9-2 2-2h14c1.1 0 2 .9 2 2v14c0 1.1-.9 2-2 2H5c-1.1 0-2-.9-2-2V5Zm11 0v14h5V5h-5ZM5 5v14h9V5H5Z")},
        {QStringLiteral("console"), QStringLiteral("M20 4H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V6c0-1.1-.9-2-2-2ZM8 17l-1.4-1.4L9.17 13 6.6 10.4 8 9l4 4-4 4Zm9 0h-5v-2h5v2Z")},
        {QStringLiteral("docs"), QStringLiteral("M14 2H6c-1.1 0-2 .9-2 2v16c0 1.1.9 2 2 2h12c1.1 0 2-.9 2-2V8l-6-6Zm1 9H7V9h8v2Zm2 4H7v-2h10v2Zm-4-7V3.5L18.5 8H13Z")},
        {QStringLiteral("code"), QStringLiteral("M9.4 16.6 4.8 12l4.6-4.6L8 6l-6 6 6 6 1.4-1.4Zm5.2 0L19.2 12l-4.6-4.6L16 6l6 6-6 6-1.4-1.4Z")},
        {QStringLiteral("ai"), QStringLiteral("M19 8h-1V6h-2v2H8V6H6v2H5c-1.1 0-2 .9-2 2v9h18v-9c0-1.1-.9-2-2-2Zm-9 7H8v-2h2v2Zm6 0h-2v-2h2v2ZM8 3h8v2H8V3Z")},
        {QStringLiteral("send"), QStringLiteral("M2 21 23 12 2 3v7l15 2-15 2v7Z")},
        {QStringLiteral("save"), QStringLiteral("M17 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V7l-4-4ZM12 19a3 3 0 1 1 0-6 3 3 0 0 1 0 6ZM6 8V5h10v3H6Z")},
        {QStringLiteral("eye"), QStringLiteral("M12 4.5C7 4.5 2.73 7.6 1 12c1.73 4.4 6 7.5 11 7.5s9.27-3.1 11-7.5c-1.73-4.4-6-7.5-11-7.5Zm0 12a4.5 4.5 0 1 1 0-9 4.5 4.5 0 0 1 0 9Zm0-2a2.5 2.5 0 1 0 0-5 2.5 2.5 0 0 0 0 5Z")},
        {QStringLiteral("bell"), QStringLiteral("M12 22a2.5 2.5 0 0 0 2.45-2h-4.9A2.5 2.5 0 0 0 12 22Zm6-6v-5c0-3.07-1.63-5.64-4.5-6.32V4a1.5 1.5 0 0 0-3 0v.68C7.64 5.36 6 7.92 6 11v5l-2 2v1h16v-1l-2-2Z")},
        {QStringLiteral("search"), QStringLiteral("M9.5 3a6.5 6.5 0 0 1 5.17 10.44l4.44 4.45-1.42 1.41-4.45-4.44A6.5 6.5 0 1 1 9.5 3Zm0 2a4.5 4.5 0 1 0 0 9 4.5 4.5 0 0 0 0-9Z")},
        {QStringLiteral("filter"), QStringLiteral("M3 5h18l-7 8v6l-4 2v-8L3 5Z")},
        {QStringLiteral("import"), QStringLiteral("M5 20h14v-2H5v2ZM19 9h-4V3H9v6H5l7 7 7-7Z")},
        {QStringLiteral("export"), QStringLiteral("M5 20h14v-2H5v2Zm7-18-7 7h4v6h6V9h4l-7-7Z")},
        {QStringLiteral("check"), QStringLiteral("M9 16.2 4.8 12l-1.4 1.4L9 19 21 7l-1.4-1.4L9 16.2Z")},
        {QStringLiteral("online"), QStringLiteral("M12 2a10 10 0 1 0 0 20 10 10 0 0 0 0-20Z")},
        {QStringLiteral("activity"), QStringLiteral("M16 17h-2l-2.2-6.6L9 17H7l4-14h2l2.2 6.6L18 3h2l-4 14Z")}};
    return paths.value(name, paths.value(QStringLiteral("activity")));
}

}  // namespace

QIcon IconFactory::icon(const QString& name, const QColor& color) {
    const QString svg = QStringLiteral(
                            R"SVG(<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><path fill="%1" d="%2"/></svg>)SVG")
                            .arg(color.name(QColor::HexRgb), materialPath(name));
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    return QIcon(pixmap);
}

}  // namespace courierman::ui
