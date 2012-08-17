#include "scanner.h"
#include "utils.h"
#include <QFileInfo>


class XSaneScannerFE : public ScannerBase
{
public:
    XSaneScannerFE(const QString &PLL, QObject *parent = 0) : ScannerBase(parent)
    {

        addParameter("-s");
        addParameter("-n");
        addParameter("-N");
        setProgramName("XSane");
        setPreloadLibrary(PLL);
        addEnvironmentVar("LD_PRELOAD=" + PLL);

    }

    void exec()
    {
        waitFor();
        execInternal("xsane");
    }
};


ScannerBase::ScannerBase(QObject *parent) :
    QObject(parent), scanProcess(this)
{
    environment.append(QProcess::systemEnvironment());
}

void ScannerBase::addParameter(const QString &s)
{
    parameters.append(s);
}

void ScannerBase::addEnvironmentVar(const QString &s)
{
    environment.append(s);
}

void ScannerBase::setOutputFile(const QString &s)
{
    outputFile = s;
}

QString ScannerBase::programName()
{
    return pName;
}

void ScannerBase::waitFor()
{
    scanProcess.terminate();
    scanProcess.waitForFinished(10000);
}

void ScannerBase::execInternal(const QString &s)
{
    scanProcess.setEnvironment(environment);
    QStringList sl;
    sl.append(parameters);
    sl.append(outputFile);
    connect(&scanProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    scanProcess.start(s, sl);
}

void ScannerBase::setProgramName(const QString &s)
{
    pName = s;
}

void ScannerBase::setPreloadLibrary(const QString &s)
{
    preloadLibrary = s;
}

QString ScannerFactory::findPreloadLibrary()
{
    QFileInfo lib;
    lib.setFile("/usr/local/lib/yagf/libxspreload.so");
    if (!lib.exists())
        lib.setFile("/usr/lib/yagf/libxspreload.so");
    if (!lib.exists())
        lib.setFile("/usr/lib64/yagf/libxspreload.so");
    if (!lib.exists())
        lib.setFile("/usr/local/lib64/yagf/libxspreload.so");
    if (lib.exists())
        return lib.filePath();
    else return "";
}

void ScannerFactory::findFEs()
{
    if (findProgram("xsane"))
        fes << "xsane";
}




ScannerFactory::ScannerFactory()
{
    preloadPath = findPreloadLibrary();
    findFEs();
}


ScannerBase *ScannerFactory::createScannerFE(const QString &name)
{
    if (name == "xsane")
        return new XSaneScannerFE(preloadPath);
    return NULL;
}


void ScannerBase::finished(int, QProcess::ExitStatus)
{
    emit processFinished();
}
