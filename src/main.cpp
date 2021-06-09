#include "MainWindow.h"

#include <Random.h>

#include <fmt/core.h>

#include <QApplication>

#include <time.h>

int main(int argc, char *argv[])
{
    auto seed = static_cast<unsigned long>(time(nullptr));
    seed = 42;
    Random::Seed(seed);

    fmt::print("Seed: {}\n", seed);

    // Tril::ChromeTracing::AddTraceWindow("Trilobytes", 1000000, std::chrono::steady_clock::now() + std::chrono::seconds(60));

    QApplication a(argc, argv);
    MainWindow w;
    w.setGeometry(0, 0, 1920, 1080);
    w.show();
    return a.exec();
}
