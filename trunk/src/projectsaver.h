#ifndef PROJECTSAVER_H
#define PROJECTSAVER_H

#include <QObject>


class QXmlStreamWriter;
class ProjectSaver : public QObject
{
    Q_OBJECT
public:
    explicit ProjectSaver(QObject *parent = 0);
    bool save(const QString &dir);
signals:
    
public slots:
private:
    void beginWritePage();
    void writeBlock();
    QString copyFile(const QString &source, const QString &destDir);
private:
    QXmlStreamWriter * stream;

};

#endif // PROJECTSAVER_H
