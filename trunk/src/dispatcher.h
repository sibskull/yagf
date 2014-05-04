#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QObject>
#include <QStringList>

class PageCollection;
class MainForm;
class Dispatcher : public QObject
{
    Q_OBJECT
public:
    explicit Dispatcher(PageCollection * pages, MainForm * gui, QObject *parent = 0);
    void loadFiles(const QStringList &files);
signals:
    void _appendPages(const QStringList &files);
public slots:

private:
    PageCollection * mPages;
    MainForm * mGui;
    
};

#endif // DISPATCHER_H
