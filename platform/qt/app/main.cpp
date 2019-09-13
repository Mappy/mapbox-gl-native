#include "mapwindow.hpp"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QMapboxGLSettings settings;
    settings.setCacheDatabasePath("/tmp/mbgl-cache.db");
    settings.setCacheDatabaseMaximumSize(20 * 1024 * 1024);
	settings.setAccessToken("pk.eyJ1IjoibWFwcHlpb3MiLCJhIjoiY2pqaWh5OGZuMTJ2MzN2cm1heHpmZmVjbCJ9.zT4TP13qJsNthxAUuYMYmg");

    MapWindow window(settings);

    window.resize(800, 600);
    window.show();

    if (argc == 2 && QString("--test") == argv[1]) {
        window.selfTest();
    }

    return app.exec();
}
