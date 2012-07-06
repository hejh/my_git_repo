#include "shell_main_window.h"
#include "onyx/ui/languages.h"
#include "onyx/screen/screen_update_watcher.h"

using namespace shell;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Shell");

    // Resource.
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(wifi_images);

    // Need a translator map.
    ui::loadTranslator(QLocale::system().name());

    // Set Font
    sys::SystemConfig conf;
    if (!conf.defaultFontFamily().isEmpty())
    {
        QApplication::setFont(conf.defaultFontFamily());
    }
    conf.close();

    shell::view::ShellMainWindow main_wnd;
    onyx::screen::watcher().addWatcher(&main_wnd);
    main_wnd.firstOpen();

    // after scan, need call main_wnd.updateAll() to update screen
    shell::data::ShellDataSource::instance().mediaScan();

    main_wnd.updateAll();
    int ret  = app.exec();
    return ret;
}
