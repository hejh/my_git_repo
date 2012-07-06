#include <QKeyEvent>

#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/data_tags.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/ui/onyx_keyboard_dialog.h"
#include "onyx/ui/common_dialogs.h"
#include "onyx/cms/cms_utils.h"
#include "onyx/sys/sys_utils.h"

#include "onycloud/data_source.h"

#include "main_view.h"
#include "shell_factory.h"
#include "image_factory.h"
#include "shell_data_source.h"
#include "conf.h"
#include "cover_view_layout_helper.h"
#include "bookmark_model.h"


using namespace shell::data;

//This is somewhat a replacement of explorer
//the data for this view's model
namespace shell
{

namespace view
{

static OnyxSearchContext s_search_ctx;
static const QString SCRIBBLE_FILE_PATH = "/root/notes";

MainItemCoverView::MainItemCoverView(QWidget *parent)
: ui::ContentView(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    updateFonts();
}

MainItemCoverView::~MainItemCoverView()
{

}

void MainItemCoverView::updateView()
{
    update();
}

void MainItemCoverView::updateFonts()
{
    {
        title_font_ = QApplication::font();
        title_font_.setPointSize(shell::data::OnyxShellConf::instance()
                .titleFontPointSize());
    }

    {
        sub_title_font_ = QApplication::font();
        sub_title_font_.setPointSize(shell::data::OnyxShellConf::instance()
                .subTitleFontPointSize());
    }
    updateView();
}

QSize MainItemCoverView::sizeHint() const
{
    return shell::data::OnyxShellConf::instance().coverViewSize();
}

void MainItemCoverView::updateTr()
{
    updateView();
}

const QString MainItemCoverView::type()
{
    return "MainItemCoverView";
}

void MainItemCoverView::resizeEvent(QResizeEvent * event )
{
    ContentView::resizeEvent(event);
    updateView();
}

void MainItemCoverView::focusInEvent(QFocusEvent * event)
{
    return ContentView::focusInEvent(event);
}

void MainItemCoverView::focusOutEvent(QFocusEvent * event)
{
    return ContentView::focusOutEvent(event);
}

void MainItemCoverView::paintEvent(QPaintEvent *pe)
{
    ContentView::paintEvent(pe);

    QPainter painter(this);
    painter.setClipRect(pe->rect());
    OData * d = data();
    if (d)
    {
        OContent *oc = static_cast<OContent *>(d);

        bool request_metadata = false;

        QPixmap pixmap;
        if (!shell::data::ShellDataSource::instance().cover(*oc, pixmap))
        {
            QFileInfo info(oc->url());
            if (sys::isImage(info.suffix()))
            {
                request_metadata = true;
            }
        }

        if (!DataSource::instance().loadFsContent(*oc))
        {
            if (cms::couldContainMetadata(oc->url()))
            {
                request_metadata = true;
            }
        }

        if (request_metadata)
        {
            shell::data::ShellDataSource::instance().notifyMetadataRequest(oc->url(), true);
        }

        CoverViewLayoutHelper layout_helper(this, painter);
        int cover_bottom_y = layout_helper.drawCover(pixmap);

        int height_left = rect().height() - cover_bottom_y;

        QString sub_title("");
        if (isFile(*oc))
        {
            QString author = oc->author();
            if (author.compare("Unknown Author", Qt::CaseInsensitive) == 0
                    || author.isEmpty())
            {
                author.clear();
            }
            sub_title = author;
        }

        layout_helper.drawAllTitle(oc->title(), title_font_,
                sub_title, sub_title_font_, height_left);
    }
}


MainItemDetailsView::MainItemDetailsView(QWidget *parent)
: ui::ContentView(parent)
, layout_(this)
, cover_label_(0)
, title_label_(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    setRepaintOnMouseRelease(false);
    createLayout();
}

MainItemDetailsView::~MainItemDetailsView()
{

}

void MainItemDetailsView::updateView()
{
    OData * d = data();
    if (d)
    {
        OContent *oc = static_cast<OContent *>(d);
        const QPixmap & image = shell::data::ImageFactory::instance().pixmap(*oc, cms::THUMBNAIL_MIDDLE);
        cover_label_.setPixmap(image);

        int left = 0, right = 0;
        layout_.getContentsMargins(&left, 0, &right, 0);
        int w = parentWidget()->width() - left - right - image.width() - layout_.spacing() - 80;//size_label_.width();

        DataSource::instance().loadFsContent(*oc);
        QString title = oc->title();
        if (isFile(*oc))
        {
            //QFileInfo info(oc->url());
            //title = info.fileName();
            title_label_.setText(title_label_.fontMetrics().elidedText(title, Qt::ElideRight, w));

            QString author_name = oc->author();
            if (author_name.compare("Unknown Author", Qt::CaseInsensitive) == 0
                    || author_name.isEmpty())
            {
                author_name.clear();
            }
            else
            {
                author_name = QString(tr("Author: %1")).arg(oc->author());
            }
            author_label_.setText(author_name);

            QString string = tr("Last Access: %1");
            if ( qgetenv("ENABLE_EUROPE_DATE_TYPE").toInt() > 0 )
            {
                string = string.arg(oc->lastAccess().toString("dd.MM.yyyy hh:mm:ss"));
            }
            else
            {
                string = string.arg(oc->lastAccess().toString("yyyy.MM.dd hh:mm:ss"));
            }
            info_label_.setText(string);

            string = tr("Read Count: %1");
            string = string.arg(oc->value(TAG_READ_COUNT).toInt());
            status_label_.setText(string);
            size_label_.setText(ui::sizeString(oc->fsSize()));
        }
        else
        {
            title_label_.setText(title_label_.fontMetrics().elidedText(title, Qt::ElideRight, w));
            info_label_.setText("");
            status_label_.setText("");
            size_label_.setText("");
        }
    }
    else
    {
        info_label_.setText("");
        status_label_.setText("");
        size_label_.setText("");
        title_label_.setText("");
        cover_label_.setPixmap(QPixmap());
    }
}

void MainItemDetailsView::createLayout()
{
    layout_.setContentsMargins(2, 4, 2, 4);

    cover_label_.setAlignment(Qt::AlignCenter);
    updateFonts();

    // 4 rows and 3 cols
    layout_.setColumnStretch(0, 0);
    layout_.setColumnStretch(1, 500);
    layout_.setColumnStretch(2, 80);
    layout_.setSpacing(5);
    layout_.addWidget(&cover_label_,  0, 0, 3, 1, Qt::AlignVCenter|Qt::AlignLeft);
    layout_.addWidget(&title_label_,  0, 1, 1, 1, Qt::AlignCenter|Qt::AlignLeft);
    layout_.addWidget(&author_label_, 1, 1, 1, 1, Qt::AlignCenter|Qt::AlignLeft);
    layout_.addWidget(&info_label_,   2, 1, 1, 1, Qt::AlignCenter|Qt::AlignLeft);
    layout_.addWidget(&status_label_, 3, 1, 1, 1, Qt::AlignCenter|Qt::AlignLeft);
    layout_.addWidget(&size_label_,   3, 2, 1, 1, Qt::AlignCenter|Qt::AlignRight);
}

void MainItemDetailsView::updateFonts()
{
    int size = shell::data::OnyxShellConf::instance().titleFontPointSize();
    QFont font(QApplication::font());
    font.setPointSize(size);
    title_label_.setFont(font);
    title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    title_label_.setWordWrap(false);

    size = shell::data::OnyxShellConf::instance().subTitleFontPointSize();
    font.setPointSize(size);
    font.setBold(true);
    author_label_.setFont(font);
    info_label_.setFont(font);
    status_label_.setFont(font);
    size_label_.setFont(font);

    updateView();
}

void MainItemDetailsView::updateTr()
{
    updateView();
}


const QString MainItemDetailsView::type()
{
    return "MainItemDetailsView";
}


void MainItemDetailsView::resizeEvent(QResizeEvent * event )
{
    ContentView::resizeEvent(event);
    updateView();
}

void MainItemDetailsView::paintEvent(QPaintEvent *pe)
{
    ContentView::paintEvent(pe);

    QPainter painter(this);
    painter.setClipRect(pe->rect());
    OData * d = data();
    if (d)
    {
        OContent *oc = static_cast<OContent *>(d);
        qDebug() << "MainItemDetailsView::paintEvent: " << oc->url();
        if (!DataSource::instance().loadFsContent(*oc) && cms::couldContainMetadata(oc->url()))
        {
            qDebug() << "notify metadata request";
            shell::data::ShellDataSource::instance().notifyMetadataRequest(oc->url(), true);
        }
    }
}


MainItemListView::MainItemListView(QWidget *parent)
: ui::ContentView(parent)
, layout_(this)
, cover_label_(0)
, title_label_(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    setRepaintOnMouseRelease(false);
    createLayout();
}

MainItemListView::~MainItemListView()
{

}

void MainItemListView::updateView()
{
    OData * d = data();
    if (d)
    {
        OContent *oc = static_cast<OContent *>(d);
        const QPixmap & image = shell::data::ImageFactory::instance().pixmap(*oc, cms::THUMBNAIL_SMALL);
        cover_label_.setPixmap(image);

        int left = 0, right = 0;
        layout_.getContentsMargins(&left, 0, &right, 0);
        int w = parentWidget()->width() - left - right - image.width() - layout_.spacing() - 110;//size_label_.width();

        DataSource::instance().loadFsContent(*oc);
        QString title = oc->title();
        if (isFile(*oc))
        {
            //QFileInfo info(oc->url());
            //title = info.fileName();
            title_label_.setText(title_label_.fontMetrics().elidedText(title, Qt::ElideRight, w));

            size_label_.setText(ui::sizeString(oc->fsSize()));
        }
        else
        {
            title_label_.setText(title_label_.fontMetrics().elidedText(title, Qt::ElideRight, w));
            size_label_.setText("");
        }
    }
    else
    {
        size_label_.setText("");
        title_label_.setText("");
        cover_label_.setPixmap(QPixmap());
    }
}

void MainItemListView::createLayout()
{
    layout_.setContentsMargins(4, 4, 6, 4);

    cover_label_.setAlignment(Qt::AlignCenter);
    updateFonts();

    layout_.setColumnStretch(0, 0);
    layout_.setSpacing(5);
    layout_.addWidget(&cover_label_,  0, 0, 1, 1, Qt::AlignVCenter|Qt::AlignLeft);
    layout_.addWidget(&title_label_,  0, 1, 1, 11, Qt::AlignVCenter|Qt::AlignLeft);
    layout_.addWidget(&size_label_,   0, 13, 1, 1, Qt::AlignCenter|Qt::AlignRight);
}

void MainItemListView::updateFonts()
{
    int size = shell::data::OnyxShellConf::instance().titleFontPointSize();
    QFont font(QApplication::font());
    font.setPointSize(size);
    title_label_.setFont(font);
    title_label_.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    title_label_.setWordWrap(false);

    size = shell::data::OnyxShellConf::instance().subTitleFontPointSize();
    font.setPointSize(size);
    font.setBold(true);
    size_label_.setFont(font);

    updateView();
}

void MainItemListView::updateTr()
{
    updateView();
}


const QString MainItemListView::type()
{
    return "MainItemListView";
}


void MainItemListView::resizeEvent(QResizeEvent * event )
{
    ContentView::resizeEvent(event);
    updateView();
}

void MainItemListView::paintEvent(QPaintEvent *pe)
{
    ContentView::paintEvent(pe);

    QPainter painter(this);
    painter.setClipRect(pe->rect());
    OData * d = data();
    if (d)
    {
        OContent *oc = static_cast<OContent *>(d);
        if (!DataSource::instance().loadFsContent(*oc) && cms::couldContainMetadata(oc->url()))
        {
            shell::data::ShellDataSource::instance().notifyMetadataRequest(oc->url(), true);
        }
    }
}

MainView::MainView(QWidget *parent)
    : ui::CatalogView(&ShellFactory::instance() ,parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    int s = shell::data::OnyxShellConf::instance().spacing();
    setContentsMargins(0, 0, 0, 0);

    setInvisibleParent(context().rootContent());
    changeViewType(shell::data::OnyxShellConf::instance().viewType());
}

MainView::~MainView()
{
}

bool MainView::changeDirectory(OContent & root, OContent & content)
{
    root.setUrl(content.url());
    return true;
}

QString MainView::goUpFolder(OContent& content)
{
    QDir dir(content.url());
    QString t = content.url();
    dir.cdUp();
    content.setUrl(dir.absolutePath());
    return t;
}

void MainView::goUp()
{
    if (!currentInvisibleParent() || isRoot())
    {
        emit outOfRoot();
        return;
    }

    ds().clearFsNameFilters();
    OContent *old = currentInvisibleParent();
    if (ds().isFsFolder(*old))
    {
        if (!ds().isFsRootFolder(*old))
        {
            QString url = goUpFolder(*old);
            updateChildren(*currentInvisibleParent());
            selectByUrl(url);

            // need to clear here.
            return;
        }
    }

    // check if we need to change current invisible parent.
    setInvisibleParent(currentInvisibleParent()->parent());
    updateChildren(*currentInvisibleParent());

    if (isRoot())
    {
        emit toRoot();
    }
    select(old);
}

void MainView::goRoot()
{
    setInvisibleParent(context().rootContent());
    updateChildren(*currentInvisibleParent());
    emit toRoot();
}

void MainView::keyReleaseEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape || ke->key() == Qt::Key_Home)
    {
        ke->accept();
        goUp();
        return;
    }
    CatalogView::keyReleaseEvent(ke);
}

void MainView::onItemActivated(ui::ContentView* item, int)
{
    if (item == 0 || item->data() == 0)
    {
        return;
    }

    QApplication::processEvents();

    ds().clearFsNameFilters();
    OContent * d = static_cast<OContent *>(item->data());
    if (ds().isFsFolder(*d))
    {
        changeViewType(shell::data::OnyxShellConf::instance().viewType());
        if (ds().isFsRootFolder(*d))
        {
            setInvisibleParent(d);
        }
        changeDirectory(*currentInvisibleParent(), *d);
        updateChildren(*currentInvisibleParent());
        setFocusTo(0, 0);
    }
    else if (d->isBranch())
    {
        associateEmptyData();
        setInvisibleParent(d);
        updateChildren(*d);
        setFocusTo(0, 0);
    }
    else if (d->contentType() == NODE_TYPE_GOUP)
    {
        goUp();
    }
    else
    {
        emit itemActivated(this, item, 0);
    }
}

bool MainView::isRoot()
{
    return (currentInvisibleParent() == currentRoot());
}

void MainView::resizeEvent(QResizeEvent * event )
{
    if (isInFs())
    {
        if (shell::data::OnyxShellConf::instance().viewType() == ui::THUMBNAIL_VIEW)
        {
            setPreferItemSize(shell::data::OnyxShellConf::instance().coverViewSize());
        }
        else if (shell::data::OnyxShellConf::instance().viewType() == ui::DETAILS_VIEW)
        {
            setPreferItemSize(shell::data::OnyxShellConf::instance().detailsViewSize());
        }
        else if (shell::data::OnyxShellConf::instance().viewType() == ui::LIST_VIEW)
        {
            setPreferItemSize(shell::data::OnyxShellConf::instance().listViewSize());
        }
    }
    else
    {
        setPreferItemSize(shell::data::OnyxShellConf::instance().coverViewSize());
    }
}

void MainView::updateChildren(OContent& oc)
{
    ds().updateChildren(oc);
    onNewContent(oc);
    if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_BOOKS) ||
            currentInvisibleParent()->name() == ds().name(NODE_TYPE_MUSICS))
    {
        changeViewType(ui::LIST_VIEW, false);
    }
    else if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_IMAGES))
    {
        changeViewType(ui::THUMBNAIL_VIEW, false);
    }
}

void MainView::onPagebarClicked(const int p, const int value)
{
    gotoPage(value);
}

void MainView::fileSystemChanged(int type, bool mount)
{
    if (mount)
    {
        changeViewType(shell::data::OnyxShellConf::instance().viewType());
        QString name = ds().name(type);
        QVector<OContentPtr > &children = currentRoot()->children();
        foreach(OContentPtr c, children)
        {
            if (c->name() == name)
            {
                setInvisibleParent(c.data());
                updateChildren(*c);
                selectFirst();
                return;
            }
        }
    }
    else
    {
        // At first, we change current path
        if (type == shell::data::NODE_TYPE_SD)
        {
            changePath(shell::data::OnyxShellConf::instance().fsSdRootFolder());
        }
        else if (type == shell::data::NODE_TYPE_LIBRARY ||
                 type == shell::data::NODE_TYPE_USB)
        {
            changePath(shell::data::OnyxShellConf::instance().fsLibraryRootFolder());
        }

        // change current root content.
        setInvisibleParent(context().rootContent());

        // TODO: able to reduce screen refresh.
        QApplication::processEvents();
        emit toRoot();
    }
}

void MainView::changePath(const QString &path)
{
    currentInvisibleParent()->setUrl(path);
    updateChildren(*currentInvisibleParent());
    selectFirst();
}

void MainView::changeSortOrder()
{
    Qt::SortOrder order = shell::data::OnyxShellConf::instance().sortOrder();
    if (order == Qt::AscendingOrder)
    {
        order = Qt::DescendingOrder;
    }
    else
    {
        order = Qt::AscendingOrder;
    }
    shell::data::OnyxShellConf::instance().setSortOrder(order);

    onNewContent(*currentInvisibleParent());
}


ui::OnyxSearchDialog & MainView::searchDialog()
{
    if (!search_dialog_)
    {
        search_dialog_.reset(new OnyxSearchDialog(0, s_search_ctx));
        connect(search_dialog_.get(), SIGNAL(search(OnyxSearchContext&)), this, SLOT(startSearch()));
        onyx::screen::watcher().addWatcher(search_dialog_.get());
    }
    return *search_dialog_;
}

void MainView::search()
{
    s_search_ctx.setSearchAll(true);
    searchDialog().showNormal();
}

void MainView::startSearch()
{
    QString pattern = s_search_ctx.pattern();
    if (!pattern.startsWith("*"))
    {
        pattern.prepend("*");
    }
    if (!pattern.endsWith("*"))
    {
        pattern.append("*");
    }
    QStringList filters(pattern);
    ds().setFsNameFilters(filters);
    updateChildren(*currentInvisibleParent());
    searchDialog().setVisible(false);
}

void MainView::remove()
{
    QString path = selectedUrl();
    QString selected_name = selectedContent()->name();

    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU, onyx::screen::ScreenCommand::WAIT_ALL);
    QApplication::processEvents();

    if (selected_name == ShellDataSource::instance().name(NODE_TYPE_SCRIBBLE)
            || selected_name == ShellDataSource::instance().name(NODE_TYPE_WEBSITE))
    {
        path = selectedContent()->title();
    }

    ui::DeleteFileDialog dialog(path, this);
    int ret = dialog.exec();
    if (ret != QMessageBox::Yes)
    {
        updateAll();
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::DW, true, onyx::screen::ScreenCommand::WAIT_ALL);
        return;
    }
    update();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);

    if (selected_name == ShellDataSource::instance().name(NODE_TYPE_WEBSITE))
    {
        BookmarkModel bookmarks;
        if (bookmarks.loadBookmarks())
        {
            if (bookmarks.deleteBookmark(selectedContent()->title(), selectedContent()->url()))
            {
                bookmarks.saveBookmarks();
            }
        }

        QString rm_command = QString("rm -rf \"/usr/share/gui_shell/sites/%1\"")
                .arg(TAG_USER_DEFINED_PREFIX+selectedContent()->title());
        // TODO: handle the return value or deploy qt file io to remove
        system(rm_command.toUtf8().constData());
    }
    else if (selected_name == ShellDataSource::instance().name(NODE_TYPE_SCRIBBLE))
    {
        ShellDataSource::instance().mdb().removeNote(selectedContent()->title());
    }
    else if (isInShortcuts())
    {
        ShellDataSource::instance().mdb().unlinkBySource(path);
    }
    else
    {
        ds().remove(path);
    }
    updateAll();
    selectFirst();
}

void MainView::renameScribble()
{
    QString name = selectedContent()->title();
    QString result;
    ui::OnyxKeyboardDialog dialog(this, tr("Rename"));
    onyx::screen::watcher().addWatcher(&dialog);
    dialog.popup(name);
    result = dialog.inputText();
    if (result.isEmpty() || result == name)
    {
        QApplication::processEvents();
        update();
        onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GU);
        return;
    }

    QDir dir(SCRIBBLE_FILE_PATH);
    QString oldname = name + ".sketch";
    QString newname =  result + ".sketch";
    if (!dir.rename(oldname, newname))
    {
        ErrorDialog dialog(tr("Rename failed."));
        dialog.exec();
    }
    else
    {
        ContentManager & db = ds().mdb();
        ScopedDB<ContentManager> lock(db);
        cms::Notes notes;
        db.allNotes(notes);

        NoteInfo noteInfo;
        noteInfo.mutable_name() = result;
        noteInfo.mutable_thumbnail() = selectedContent()->cover().toImage();
        db.addNoteIndex(noteInfo);
        db.removeNote(name);

        updateAll();
        selectByUrl(dir.absoluteFilePath(result));
        selectByTitle(name);
    }

    QApplication::processEvents();
    update();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GU);
}

void MainView::rename(bool is_scribble_selected)
{
    if(is_scribble_selected)
    {
        renameScribble();
        return;
    }

    QFileInfo info(selectedContent()->url());
    QString name = info.fileName();
    QString result;
    ui::OnyxKeyboardDialog dialog(this, tr("Rename"));
    onyx::screen::watcher().addWatcher(&dialog);
    dialog.popup(name);
    result = dialog.inputText();
    if (result.isEmpty() || result == name)
    {
        QApplication::processEvents();
        update();
        onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GU);
        return;
    }

    QDir dir = info.absoluteDir();
    if (!dir.rename(name, result))
    {
        ErrorDialog dialog(tr("Rename failed."));
        dialog.exec();
    }
    else
    {
        updateAll();
        selectByUrl(dir.absoluteFilePath(result));
    }

    QApplication::processEvents();
    update();
    onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GU);
}

bool MainView::selectByUrl(const QString &url)
{
    foreach(OContentPtr c, currentInvisibleParent()->children())
    {
        if (c->url() == url)
        {
            select(c.data());
            return true;
        }
    }
    return false;
}

bool MainView::selectByTitle(const QString &title)
{
    foreach(OContentPtr c, currentInvisibleParent()->children())
    {
        if (c->title() == title)
        {
            select(c.data());
            return true;
        }
    }
    return false;
}

bool MainView::selectFirst()
{
    if (currentInvisibleParent()->children().size() > 0)
    {
        select(currentInvisibleParent()->children().front().data());
        onyx::screen::watcher().enqueue(0, onyx::screen::ScreenProxy::GC);
        return true;
    }
    return false;
}

void MainView::changeViewType(ViewType type, bool keep)
{
    if (keep)
    {
        shell::data::OnyxShellConf::instance().setViewType(type);
    }

    // remember current item.
    OContent * selected = selectedContent();
    if (type == ui::THUMBNAIL_VIEW)
    {
        setSubItemType(MainItemCoverView::type());
        setPreferItemSize(shell::data::OnyxShellConf::instance().coverViewSize());
    }
    else if (type == ui::DETAILS_VIEW)
    {
        setSubItemType(MainItemDetailsView::type());
        setPreferItemSize(shell::data::OnyxShellConf::instance().detailsViewSize());
    }
    else if (type == ui::LIST_VIEW)
    {
        setSubItemType(MainItemListView::type());
        setPreferItemSize(shell::data::OnyxShellConf::instance().listViewSize());
    }
    onNewContent(*currentInvisibleParent());
    select(selected);
}

void MainView::setInvisibleParent(OContent *c)
{
    context().setCurrentContent(c);
}

OContent * MainView::currentInvisibleParent()
{
    return context().currentContent();
}

OContent * MainView::currentRoot()
{
    return context().rootContent();
}

ContentContext & MainView::context()
{
    return ds().desktopContext();
}

void MainView::myUpdateData(OContent& oc)
{
    if (ds().isFsFolder(oc) || ds().isCollection(oc))
    {
        foreach (OContentPtr item_p, oc.children())
        {
            bool load_succ = DataSource::instance().loadFsContent(*item_p);
            if (!load_succ)
            {
                qDebug() << "Note: " << item_p->url() << " load file info failed.";
            }
        }

        ds().sort(oc.children(), shell::data::OnyxShellConf::instance().sortOrder(), shell::data::OnyxShellConf::instance().sortBy());
    }

    data().clear();
    for(int i = 0; i < oc.children().size(); ++i)
    {
        data().push_back(oc.children().at(i).data());
    }

    /*
    if (ds().isFsFolder(oc))
    {
        data().push_front(ds().goUpContent());
    }
    */
    setData(data(), true);
}

// when new content is available, we find a content view to display it.
void MainView::onNewContent ( OContent& oc )
{
    // When children have been changed.
    if (currentInvisibleParent() == &oc)
    {
        myUpdateData(oc);

        if (ds().isFsFolder(oc))
        {
            qDebug() << "New location " << oc.url();
            if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_SD))
            {
                emit locationChanged(oc.url(), shell::data::OnyxShellConf::instance().fsSdRootFolder(),
                        ds().title(NODE_TYPE_SD));
            }
            else if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_LIBRARY))
            {
                emit locationChanged(oc.url(), shell::data::OnyxShellConf::instance().fsLibraryRootFolder(),
                        ds().title(NODE_TYPE_LIBRARY));
            }
            else if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_PRIVATE_STORAGE))
            {
                emit locationChanged(oc.url(), data::OnyxShellConf::instance().privateStorageFolder(),
                        ds().title(NODE_TYPE_PRIVATE_STORAGE));
            }
        }
        else
        {
            if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_BOOKS))
            {
                emit toArchive(oc.url(), OnyxShellConf::instance().booksFolder(),
                        ds().title(NODE_TYPE_BOOKS));
            }
            else if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_IMAGES))
            {
                emit toArchive(oc.url(), OnyxShellConf::instance().picturesFolder(),
                        ds().title(NODE_TYPE_IMAGES));
            }
            else if (currentInvisibleParent()->name() == ds().name(NODE_TYPE_MUSICS))
            {
                emit toArchive(oc.url(), OnyxShellConf::instance().musicFolder(),
                        ds().title(NODE_TYPE_MUSICS));
            }
        }
    }
}

/// The children list is fetched one by one, so the size could be changed.
void MainView::onUpdateChildren(OContent& oc)
{
    if (currentInvisibleParent() != &oc)
    {
        qDebug("ignore update children.");
        return;
    }

    bool dirty = false;
    if (oc.children().size() != paginator().size())
    {
        myUpdateData(oc);

        // If not matched, we need to check it's increased or decreased.
        if (oc.children().size() > paginator().size())
        {
            paginator().reset(paginator().first_visible(), paginator().items_per_page(), children().size());
        }
        else
        {
            resetPaginator();
        }
        dirty = true;
    }
    associateData();
    if (dirty)
    {
        broadcastPositionSignal();
        onyx::screen::watcher().enqueue(parentWidget(), onyx::screen::ScreenProxy::GU);
    }
    else
    {
        onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GU);
    }
}

// Not necessary to lock, as the pointer is different.
void MainView::onUpdateCover ( OContent& oc )
{
    foreach( ContentView *p, visibleSubItems())
    {
        if (p->data() == &oc)
        {
            p->updateData(&oc, true);
            onyx::screen::watcher().enqueue(p, onyx::screen::ScreenProxy::GU);
            return;
        }
    }
}

void MainView::onCoverReady(const QString &path, const QPixmap &pixmap)
{
    foreach(ContentView *p, visibleSubItems())
    {
        OContent * c = static_cast<OContent *>(p->data());
        if (c && c->url() == path)
        {
            // don't need to setCover now, updateView will read it from cms database.
            // c->setCover(pixmap);
            p->updateData(c, true);
            onyx::screen::watcher().enqueue(p, onyx::screen::ScreenProxy::GU);
            return;
        }
    }

    qDebug() << " No content found " << path;
}

void MainView::onMetadataReady(const QString &path)
{
    foreach(ContentView *p, visibleSubItems())
    {
        OContent * c = static_cast<OContent *>(p->data());
        if (c && c->url() == path)
        {
            // don't need to setCover now, updateView will read it from cms database.
            // c->setCover(pixmap);
            p->updateData(c, true);
            onyx::screen::watcher().enqueue(p, onyx::screen::ScreenProxy::GU);
            return;
        }
    }

    qDebug() << " No content found " << path;
}

bool MainView::pasteToSelected()
{
    QFileInfo info(ds().sourceFile());
    QString target;
    ds().paste(QDir(selectedUrl()), target);
    updateAll();
    QApplication::processEvents();
    selectByUrl(target);
    return true;
}

bool MainView::paste()
{
    QFileInfo info(ds().sourceFile());
    QString target;
    ds().paste(currentFsUrl(), target);
    updateAll(false);
    QApplication::processEvents();
    selectByUrl(target);
    return true;
}

void MainView::onError(OContent&, QNetworkReply::NetworkError error)
{
    int e = 0;
}

shell::data::ShellDataSource & MainView::ds()
{
    return shell::data::ShellDataSource::instance();
}

void MainView::updateAll(bool keep_selected)
{
    QString url;
    QString title;
    if (keep_selected && selectedContent())
    {
        url = selectedContent()->url();
        title = selectedContent()->title();
    }

    updateChildren(*currentInvisibleParent());

    if (keep_selected)
    {
        if (!url.isEmpty())
        {
            selectByUrl(url);
        }
        else
        {
            selectByTitle(title);
        }
    }
}

void MainView::updateFonts()
{
    ui::ViewType type = shell::data::OnyxShellConf::instance().viewType();
    if (!this->isInFs())
    {
        type = ui::THUMBNAIL_VIEW;
    }
    QVector<ContentView *> & all = visibleSubItems();
    foreach(ContentView *p, all)
    {
        if (type == ui::THUMBNAIL_VIEW)
        {
            static_cast<MainItemCoverView *>(p)->updateFonts();
        }
        else if (type == ui::DETAILS_VIEW)
        {
            static_cast<MainItemDetailsView *>(p)->updateFonts();
        }
        else if (type == ui::LIST_VIEW)
        {
            static_cast<MainItemListView *>(p)->updateFonts();
        }
    }
    update();
}

void MainView::updateTr()
{
    ds().updateDesktopTr();

    ui::ViewType type = shell::data::OnyxShellConf::instance().viewType();
    QVector<ContentView *> & all = visibleSubItems();
    foreach(ContentView *p, all)
    {
        if (type == ui::THUMBNAIL_VIEW)
        {
            static_cast<MainItemCoverView *>(p)->updateTr();
        }
        else if (type == ui::DETAILS_VIEW)
        {
            static_cast<MainItemDetailsView *>(p)->updateTr();
        }
        else if (type == ui::LIST_VIEW)
        {
            static_cast<MainItemListView *>(p)->updateTr();
        }
    }
}

QString MainView::currentType()
{
    return currentInvisibleParent()->name();
}

OContent * MainView::selectedContent()
{
    ContentView * p = focusItem();
    if (p == 0 || p->data() == 0)
    {
        return 0;
    }
    OContent *c = static_cast<OContent *>(p->data());
    return c;
}

bool MainView::isFileSelected()
{
    OContent * c = selectedContent();
    if (c)
    {
        return isFile(*c);
    }
    return false;
}

bool MainView::isFolderSelected()
{
    OContent * c = selectedContent();
    if (c)
    {
        return isFolder(*c);
    }
    return false;
}

bool MainView::isGoUpSelected()
{
    OContent * c = selectedContent();
    if (c)
    {
        return (c->name() == ds().name(NODE_TYPE_GOUP));
    }
    return false;
}

bool MainView::isInSD()
{
    return (currentType() == ds().name(NODE_TYPE_SD));
}

bool MainView::isScribbleFileSelected()
{
    OContent * c = selectedContent();
    if (c)
    {
        return (c->name() == ds().name(NODE_TYPE_SCRIBBLE));
    }
    return false;
}

bool MainView::isNoteFileSelected()
{
    OContent * c = selectedContent();
    if (c)
    {
        return(c->name() == ds().name(NODE_TYPE_WRITEPAD));
    }
    return false;
}

bool MainView::isWebsiteSelected()
{
    OContent * c = selectedContent();
    if (c)
    {
        return c->name() == ds().name(NODE_TYPE_WEBSITE);
    }
    return false;
}

bool MainView::isInFs()
{
    return (currentType() == ds().name(NODE_TYPE_SD) ||
            currentType() == ds().name(NODE_TYPE_LIBRARY) ||
            currentType() == ds().name(NODE_TYPE_BOOKS) ||
            currentType() == ds().name(NODE_TYPE_IMAGES) ||
            currentType() == ds().name(NODE_TYPE_MUSICS) ||
            isInPrivateStorage());
}

bool MainView::isInPrivateStorage()
{
    return currentType() == ds().name(NODE_TYPE_PRIVATE_STORAGE);
}

bool MainView::isInRecentReading()
{
    return (currentType() == ds().name(NODE_TYPE_RECENT_DOCS));
}

bool MainView::isInWebsites()
{
    return currentType() == ds().name(NODE_TYPE_WEBSITES);
}

QDir MainView::currentFsUrl()
{
    if (this->isInFs())
    {
        return currentInvisibleParent()->url();
    }
    return QString();
}

QString MainView::selectedUrl()
{
    OContent * c = selectedContent();
    if (c)
    {
        return c->url();
    }
    return QString();
}

void MainView::addShortcut(const QString &url)
{
    if (url.isEmpty())
    {
        return;
    }

    cms::ContentManager & db = ds().mdb();
    int count = db.links(url);
    QFileInfo info(url);
    QString target = info.fileName();
    QString postfix("(1%)");
    if (count > 0)
    {
        target += postfix.arg(count);
    }
    db.link(url, target);
    return;
}

bool MainView::isInShortcuts()
{
    return currentType() == ds().name(NODE_TYPE_SHORTCUTS);
}

}

}
