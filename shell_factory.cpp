#include "shell_factory.h"
#include "main_view.h"
#include "recent_reading_view.h"
#include "path_bar.h"
#include "tab_bar.h"
#include "about_dialog.h"

namespace shell
{

namespace view
{

ShellFactory::ShellFactory()
{
}

ShellFactory::ShellFactory(ShellFactory &ref)
{
}

ShellFactory::~ShellFactory()
{
}

ContentView * ShellFactory::createView(QWidget *parent, const QString &type)
{
    if (type.compare(MainItemCoverView::type(), Qt::CaseInsensitive) == 0)
    {
        return new MainItemCoverView(parent);
    }
    else if (type.compare(MainItemDetailsView::type(), Qt::CaseInsensitive) == 0)
    {
        return new MainItemDetailsView(parent);
    }
    else if (type.compare(MainItemListView::type(), Qt::CaseInsensitive) == 0)
    {
        return new MainItemListView(parent);
    }
    else if (type.compare(RecentReadingItemView::type(), Qt::CaseInsensitive) == 0)
    {
        return new RecentReadingItemView(parent);
    }
    else if (type.compare(PathBarItemView::type(), Qt::CaseInsensitive) == 0)
    {
        return new PathBarItemView(parent);
    }
    else if (type.compare(TabBarItemView::type(), Qt::CaseInsensitive) == 0)
    {
        return new TabBarItemView(parent);
    }
    else if (type.compare(AboutInfoView::type(), Qt::CaseInsensitive) == 0)
    {
        return new AboutInfoView(parent);
    }
    return new MainItemCoverView(parent);
}

}

}
