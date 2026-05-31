#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QStandardPaths>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setOrganizationName(QStringLiteral("CourierManTests"));
    QCoreApplication::setApplicationName(QStringLiteral("CourierMan"));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
