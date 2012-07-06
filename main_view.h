#ifndef ONYX_SHELL_MAIN_VIEW_H_
#define ONYX_SHELL_MAIN_VIEW_H_


#include "onyx/ui/catalog_view.h"
#include "onyx/ui/factory.h"
#include "shell_data_source.h"
#include "onyx/ui/onyx_search_dialog.h"

using namespace ui;

namespace shell
{

namespace view
{

/// Represents each single item in main view.
class MainItemCoverView : public ui::ContentView
{
    Q_OBJECT

public:
    MainItemCoverView(QWidget *parent);
    virtual ~MainItemCoverView();

public:
    void updateView();
    void updateFonts();
    void updateTr();
    static const QString type();
    virtual QSize sizeHint () const;

protected:
    void resizeEvent(QResizeEvent * event );
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);

    void paintEvent(QPaintEvent *pe);

private:
    QFont title_font_;
    QFont sub_title_font_;
};

/// Represents each single item in main view.
class MainItemDetailsView : public ui::ContentView
{
    Q_OBJECT

public:
    MainItemDetailsView(QWidget *parent);
    virtual ~MainItemDetailsView();

public:
    void updateView();
    void updateFonts();
    void updateTr();
    static const QString type();

protected:
    void resizeEvent(QResizeEvent * event );
    void paintEvent(QPaintEvent *pe);

private:
    void createLayout();

private:
    QGridLayout layout_;
    QLabel cover_label_;
    QLabel title_label_;
    QLabel info_label_;
    QLabel status_label_;
    QLabel size_label_;
    QLabel author_label_;
};

/// Represents each single item in main view.
class MainItemListView : public ui::ContentView
{
    Q_OBJECT

public:
    MainItemListView(QWidget *parent);
    virtual ~MainItemListView();

public:
    void updateView();
    void updateFonts();
    void updateTr();
    static const QString type();

protected:
    void resizeEvent(QResizeEvent * event );
    void paintEvent(QPaintEvent *pe);

private:
    void createLayout();

private:
    QGridLayout layout_;
    QLabel cover_label_;
    QLabel title_label_;
    QLabel size_label_;
};


class MainView : public CatalogView
{
    Q_OBJECT

public:
    MainView(QWidget* parent = 0);
    virtual ~MainView();

public slots:
    void goUp();
    void onPagebarClicked(const int, const int);
    void fileSystemChanged(int, bool);
    void changePath(const QString &path);
    void goRoot();

    void changeSortOrder();
    void search();
    void remove();
    void rename(bool is_scribble_selected);
    void renameScribble();
    void changeViewType(ViewType type, bool keep = true);

    void updateAll(bool keep_selected = false);
    void updateFonts();
    void updateTr();

    QString currentType();
    bool isFileSelected();
    bool isFolderSelected();
    bool isGoUpSelected();
    bool isInShortcuts();

    bool isInFs();
    bool isWebsiteSelected();
    bool isInPrivateStorage();
    bool isInRecentReading();
    bool isInWebsites();
    bool isNoteFileSelected();
    bool isScribbleFileSelected();

    bool isInSD();
    QDir currentFsUrl();
    QString selectedUrl();

    OContent * selectedContent();
    void onCoverReady(const QString & path, const QPixmap &);
    void onMetadataReady(const QString & path);

    bool pasteToSelected();
    bool paste();

    bool selectFirst();
    void addShortcut(const QString &);

    //void updateMediaInfo(int type);

Q_SIGNALS:
    void outOfRoot();
    void toRoot();
    void locationChanged(const QString & path, const QString &root, const QString &root_title);
    void toArchive(const QString & path, const QString &root, const QString &root_title);

protected:
    void keyReleaseEvent(QKeyEvent *);
    virtual void onItemActivated(ContentView* item, int user_data);
    void resizeEvent(QResizeEvent * event );

private Q_SLOTS:
    void onNewContent(OContent &);
    void onUpdateCover(OContent &);
    void onError(OContent&, QNetworkReply::NetworkError);
    void onUpdateChildren(OContent &);

    void myUpdateData(OContent& oc);
    void updateChildren(OContent& oc);

    void startSearch();
    bool selectByUrl(const QString &url);
    bool selectByTitle(const QString &title);

private:
    void setInvisibleParent(OContent *);
    OContent * currentInvisibleParent();
    OContent * currentRoot();
    ContentContext & context();

    bool isRoot();

    bool changeDirectory(OContent & root, OContent & content);
    QString goUpFolder(OContent& content);

    ui::OnyxSearchDialog & searchDialog();

    shell::data::ShellDataSource & ds();

private:
    scoped_ptr<OnyxSearchDialog> search_dialog_;
    ODatas my_data_;
    bool in_file_system_;
};


}

}

#endif // ONYX_SHELL_MAIN_VIEW_H_
