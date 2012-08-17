#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QProcess>
#include <QStringList>


class ScannerBase : public QObject
{
    Q_OBJECT
public:
    explicit ScannerBase(QObject *parent = 0);
    void addParameter(const QString &s);
    void addEnvironmentVar(const QString &s);
    void setOutputFile(const QString &s);
    QString programName();
    virtual void exec() = 0;
signals:
    void processFinished();
public slots:
private slots:
    void finished( int, QProcess::ExitStatus );
protected:
    void waitFor();
    void execInternal(const QString &s);
    void setProgramName(const QString &s);
    void setPreloadLibrary(const QString &s);

private:
    QProcess scanProcess;
    QStringList parameters;
    QStringList environment;
    QString outputFile;
    QString pName;
    QString preloadLibrary;
};

class ScannerFactory {
public:
    ScannerFactory();
    QStringList frontEnds();
    ScannerBase * createScannerFE(const QString &name);
private:
    QString findPreloadLibrary();
    void findFEs();
private:
    QString preloadPath;
    QStringList fes;
};


#endif // SCANNERBASE_H
