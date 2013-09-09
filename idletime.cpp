#include "main.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

/// Az xprintidle futtatása
/// @param cmd A parancs string
/// @param __ex Ha értéke hamis, akkor ha nem sikerül a parancsot elindítani, ill. a kimenete nem értelmezhető, akkor
///             nem dobja fel a hiba üzenet ablakot, hanem csak visszatér nullával.
/// @return Az xprintidle által kiírt értékkl, vagyis a ezred másodperc értékkel tér vissza. Hiba esetén 0, vagy negatív.
static int xprintidle(QString cmd = QString(), bool __ex = true)
{
    if (cmd.isEmpty()) cmd = "xprintidle";
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(cmd);
    int to = 1000;
    bool ok;
    ok = proc.waitForStarted(to);
    if (!ok) {
        QString msg = QObject::trUtf8("Az \"%1\" program idítása sikertelen.").arg(cmd);
        if (__ex == false) {
            DS << msg << endl;
            return 0;
        }
        message(sCrit, msg + sSugg);
        return -1;
    }
    ok = proc.waitForFinished(to);
    if (!ok) {
        message(sCrit, QObject::trUtf8("\"%1\" program időtullépés?").arg(cmd) + sSugg);
        return -1;
    }
    QString out = proc.readAll();
    out = out.simplified();
    int it = out.toInt(&ok);
    if (!ok) {
        QString msg = QObject::trUtf8("xprintidle program Kimenete nem értelmezhető:<br><i>\n");
        msg += out + "</i>";
        msg += sSugg;
        if (__ex == false) {
            DS << msg << endl;
            return 0;
        }
        message(sCrit, msg);
        return -1;
    }
    it /= 1000;   // ms => s
    if (!it) it = 0;    // a nulla is hibát jelez
    DS << __PRETTY_FUNCTION__ << " return : " << it << endl;
    return it;
}

static int getDisplay()
{
    const char *pe = getenv("DISPLAY");
    bool ok;
    int i;
    if (pe == NULL) {
        message(sCrit, QObject::trUtf8("Nincs DISPLAY környezeti változó!") + sSugg);
        return -1;
    }
    QString ed = pe;
    i = ed.indexOf(QChar('.')); // havan '.' akkor onnantól töröljük
    if (i > 1) ed = ed.mid(0, i);
    if (ed.size() < 2 || ed[0] != QChar(':') || ((i = ed.mid(1).toInt(&ok)), !ok)) {
        message(sCrit, QObject::trUtf8("A DISPLAY környezeti vátozó értéke nem értelmezhető : \"%1\"").arg(pe) + sSugg);
        return -1;
    }
    DS << __PRETTY_FUNCTION__ << " return : " << i << endl;
    return i;
}

static QList<int> otherDisplays(int myDisplay, bool& ok)
{
    static  QDir *pTmpDir = NULL;
    if (pTmpDir == NULL) {
        pTmpDir = new QDir("/tmp");
        QStringList fl;
        fl << QString(".X[0-9]-lock");
        fl << QString(".X[0-9][0-9]-lock");
        pTmpDir->setNameFilters(fl);  // Ez ugysem változik
        pTmpDir->setFilter(QDir::Files | QDir::Hidden | QDir::System);
    }
    QStringList lfl = pTmpDir->entryList();
    QList<int>  r;
    foreach (QString lf, lfl) {
        QString n = lf.mid(2,2);    // fajl névben a sorszám a 3. karaktertől max 2 karakter
        if (n[1] == QChar('-')) n = n.mid(0,1); // csak egy számjegy van
        int dn = n.toInt(&ok);
        if (!ok) {
            message(sCrit, QObject::trUtf8("Program hiba.") + sSugg);
            ok = false;
            return r;
        }
        if (dn == myDisplay) continue;    // Mienk, már kérdeztük
        r << dn;
    }
    ok = true;
    return r;
}

/// Lekérdezi a tétlenségi időt
/// Az értéket másodpercben adja vissza
/// @param Ha a lekérdezés részeredménye kisebb mint _min, akkor azonnal visszatér a részeredménnyel.
int getIdleTime(int _min)
{
    DS << __PRETTY_FUNCTION__ << " _min : " << _min << endl;
    bool ok;
    int minIdleTime = INT_MAX;
    // Virtuális konzolokon mért idők
    QDateTime   now = QDateTime::currentDateTime();
    static  QDir *pDevDir = NULL;
    if (pDevDir == NULL) {
        pDevDir = new QDir("/dev");
        QStringList fl;
        fl << QString("tty[1-7]");
        pDevDir->setNameFilters(fl);
        pDevDir->setFilter(QDir::AllEntries | QDir::System);
    }
    QFileInfoList fil = pDevDir->entryInfoList();
    foreach (QFileInfo fi, fil) {
        int it = (int)fi.lastModified().secsTo(now);
        DS << "Console : " << fi.fileName() << " idle time " << it << endl;
        minIdleTime = qMin(minIdleTime, it);
        if (minIdleTime < _min) {
            DS << __PRETTY_FUNCTION__ << " return : " << minIdleTime << " / " << fi.fileName() << endl;
            return minIdleTime;
        }
    }
    // Az X, Saját display:
    int xidle = xprintidle();
    if (xidle < 0) return -1;
    minIdleTime = qMin(minIdleTime, xidle);
    if (minIdleTime < _min) {
        DS << __PRETTY_FUNCTION__ << " return : " << minIdleTime << " / X" << endl;
        return minIdleTime;
    }
    static int display = -1;    // static mert nem vátozik
    if (display == -1) {
        display = getDisplay();
        // Megvan hanyadik X display a mienk
    }
    QList<int> dnl = otherDisplays(display, ok);
    if (!ok) return -1;
    foreach (int dn, dnl) {
        // Ha nem a mienk, akkor az egy terminál esetén csak a root-é lehet.
        // Mert a kiosk kivételével minden root-ként fut, csak a mi példányuk a kioskuser-é,
        // és abbol több nem lehet.
        // A sudoers -ben a kioskuser-nek jelszó nélkül engedélyzni kell az xprintidle hívását, pl.:
        // kioskuser	 ALL=(root) NOPASSWD: /usr/bin/xprintidle
        QString cmd = QString("sudo DISPLAY=:%1 xprintidle").arg(dn);
        xidle = xprintidle(cmd, false);
        if (xidle < 0) return -1;
        minIdleTime = qMin(minIdleTime, xidle);
        if (minIdleTime < _min) {
            DS << __PRETTY_FUNCTION__ << " return : " << minIdleTime << " / X:" << dn << endl;
            return minIdleTime;
        }
    }
    DS << __PRETTY_FUNCTION__ << " return : " << minIdleTime << endl;
    return minIdleTime;
}
