%{
#include "dialog.h"
#include "control.h"
#include "parser.h"
#include <QStack>

QString             yyLastError;
QString             yyLastLine;
int                 yyLineNo;
QString             yyLine;

static QStack<QChar> yyUnGetChars;
static QIODevice   *yyFile = NULL;
static QIODevice   *yyOutF = NULL;

static int yyparse();

int parseConfig(QIODevice *_in)
{
    yyFile = _in;
    yyLastError.clear();
    yyLine.clear();
    yyUnGetChars.clear();
    yyUnGetChars << QChar(' ') << QChar('@');   // Config parser
    yyLineNo = 0;
    int r = yyparse();
    yyFile = NULL;
    return r;
}

int parseCommand(QByteArray& _in, QByteArray& _out)
{
    yyFile = new QBuffer(&_in);
    yyFile->open(QIODevice::ReadOnly);
    yyFile->seek(0);
    _out.clear();
    yyOutF = new QBuffer(&_out);;
    yyOutF->open(QIODevice::WriteOnly);
    yyLastError.clear();
    yyLine.clear();
    yyUnGetChars.clear();
    yyUnGetChars << QChar(' ') << QChar('!');   // Command parser
    yyLineNo = 0;
    cCntrl::_ok();
    int r = yyparse();
    delete yyFile;
    delete yyOutF;
    yyOutF = yyFile = NULL;
    return r;
}

void yyprint(const QByteArray& _o)
{
    yyOutF->write(_o);
}

static int yyerror(QString em)
{
    yyLastError = em;
    return -1;
}

static int yyerror(const char * em)
{
    return yyerror(QString::fromUtf8(em));
}

static QChar yyget()
{
    if (yyUnGetChars.size() > 0) return yyUnGetChars.pop();
    while (yyLine.isEmpty()) {
        ++yyLineNo;
        yyLastLine = yyLine  = QString::fromUtf8(yyFile->readLine());
        DS << "yyget() line#" << yyLastLine << " : \"" << yyLastLine << "\"" << endl;
        if (yyLine.isEmpty() && yyFile->atEnd()) return 0;
    }
    QChar c = yyLine.at(0);
    yyLine = yyLine.remove(0,1);
    return c;
}

static inline void yyunget(const QChar& _c) {
    yyUnGetChars.push(_c);
}

static int yylex(void);

%}

%union {
    void *              u;
    qlonglong           i;
    bool                b;
    QString *           s;
    QStringList *      sl;
    QStringPair *      sp;
    QStringPairList * spl;
    cAppButton *       ap;
    cAppButtonList *  apl;
}

%token      DOMAIN_T RDP_T OFF_T HELP_T COMMAND_T APP_T
%token      IDLE_T DIALOG_T TIME_T MIN_T RUN_T WEB_T KIOSK_T
%token      RESTART_T NO_T MODE_T CLEAN_T INFO_T BROWSER_T
%token      MASTER_T USER_T COMMANDS_T CHROME_T FIREFOX_T QUPZILLA_T

%token      PING_T SET_T OUT_T

%token <i>  INTEGER_V
%token <s>  STRING_V NAME_V

%type  <i>  int int_z
%type  <s>  str cmd icon
%type <sp>  server domain strpair
%type <spl> servers
%type  <ap> ap
%type <apl> apl
/*
%type <sl>  strs
*/
%%

main    : '@' configs
        | '!' cmds
        ;
/* Config parser */
configs : config
        | config configs
        ;
config  : DOMAIN_T domain '{' servers '}'   { mainDialog::addDomain($2, $4); }
        | RDP_T COMMAND_T str ';'           { mainDialog::setRdpCmd($3); }
        | OFF_T COMMAND_T str ';'           { mainDialog::setOffCmd($3); }
        | RESTART_T COMMAND_T str ';'       { mainDialog::setResCmd($3); }
        | HELP_T COMMAND_T str ';'          { mainDialog::setHelpCmd($3); }
        | COMMAND_T str cmd icon int_z ';'  { mainDialog::addCommand($2, $3, $4, $5); }
        | BROWSER_T COMMAND_T str ';'       { mainDialog::setBrowserCmd($3); }
        | BROWSER_T COMMANDS_T '{' apl '}'  { mainDialog::setBrowserCmds($4); }
        | KIOSK_T MODE_T ';'                { isKiosk = true; }
        | MASTER_T USER_T str str ';'       { mainDialog::setMaster($3, $4); }
        | cmdcfgs
        ;
cmdcfgs : IDLE_T TIME_T int ';'             { idleTime  = $3; }
        | IDLE_T DIALOG_T TIME_T int ';'    { idleDialogTime  = $4; }
        | MIN_T RUN_T TIME_T int ';'        { minProgTime = $4; }
        | NO_T KIOSK_T MODE_T ';'           { isKiosk = false; }
        ;
str     : NAME_V        { $$ = $1; }
        | STRING_V      { $$ = $1; }
        ;
/*
strs    : str           { $$ = new QStringList(*$1); delete $1; }
        | strs  str     { $$ = $1; *$$ << *$2; delete $2; }
        ;
*/
int     : INTEGER_V     { $$ = $1; }
        ;
int_z   : int           { $$ = $1; }
        |               { $$ = -1; }
        ;
cmd     : str           { $$ = $1; }
        | HELP_T        { $$ = new QString(mainDialog::getRdpCmd()); }
        | OFF_T         { $$ = new QString(mainDialog::getOffCmd()); }
        | RESTART_T     { $$ = new QString(mainDialog::getResCmd()); }
        ;
icon    : str           { $$ = $1; }
        | HELP_T        { $$ = new QString(":/images/help.ico"); }
        | WEB_T         { $$ = new QString(":/images/web.ico"); }
        | INFO_T        { $$ = new QString(":/images/info.ico"); }
        | OFF_T         { $$ = new QString(":/images/off.ico"); }
        | RESTART_T     { $$ = new QString(":/images/reboot.ico"); }
        | CLEAN_T       { $$ = new QString(":/images/clean.ico"); }
        | APP_T         { $$ = new QString(":/images/app.ico"); }
        |               { $$ = new QString(":/images/app.ico"); }
        | CHROME_T      { $$ = new QString(":/images/chrome.ico"); }
        | FIREFOX_T     { $$ = new QString(":/images/firefox.ico"); }
        | QUPZILLA_T    { $$ = new QString(":/images/qupzilla.ico"); }
        ;
servers : server            { *($$ = new QStringPairList()) << *$1; delete $1; }
        | servers server    { *($$ = $1)                    << *$2; delete $2; }
        ;
server  : domain ';'        { $$ = $1; }
        ;
domain  : str               { $$ = new QStringPair(*$1, QString()); delete $1; }
        | strpair           { $$ = $1; }
        ;
strpair : str str           { $$ = new QStringPair(*$1, *$2); delete $1; delete $2; }
        ;
apl     : ap ';'            { *($$ = new cAppButtonList()) << *$1; delete $1; }
        | apl ap ';'        { *($$ = $1)                   << *$2; delete $2; }
        ;
ap      : str cmd icon      { $$ = new cAppButton($3, $1, $2); }
        ;
/* Command parser */
cmds    : command
        | command ';' cmds
        ;
command : PING_T            { ; }
        | OFF_T             { cCntrl::_command(mainDialog::getOffCmd()); }
        | RESTART_T         { cCntrl::_command(mainDialog::getResCmd()); }
        | COMMAND_T str     { cCntrl::command($2); }
        | RUN_T             { cCntrl::getRun(); }
        | SET_T TIME_T OUT_T int    { cCntrl::setCmdTo($4); }
        | cmdcfgs
        ;
%%

static inline bool isXDigit(QChar __c) {
    int cc;
    if (__c.isDigit())  return true;
    if (!__c.isLetter()) return false;
    cc = __c.toLatin1();
    return (cc >= 'A' && cc <= 'F') || (cc >= 'a' && cc <= 'f');
}
static inline int digit2num(QChar c) { return c.toLatin1() - '0'; }
static inline int xdigit2num(QChar c) { return c.isDigit() ? digit2num(c) : (c.toUpper().toLatin1() - 'A' + 10); }
static inline bool isOctal(QChar __c) {
    if (!__c.isDigit()) return false;
    int c = __c.toLatin1();
    return c < '8';
}

static QString *yygetstr()
{
    QString *ps = new QString;
    QChar c;
    while (QChar('\"') != (c = yyget())) {
        if (c == QChar('\\')) switch ((c = yyget()).toLatin1()) {
            case '\\':              break;
            case 'e':  c = QChar::fromLatin1('\x1b');  break;
            case 'n':  c = QChar::fromLatin1('\n');    break;
            case 't':  c = QChar::fromLatin1('\t');    break;
            case 'r':  c = QChar::fromLatin1('\r');    break;
            case 'x':  {
                int n = 0;
                for (char i = 0; i < 2; i++) {
                    if (!isXDigit(c = yyget())) { yyunget(c); break; }
                    n *= 16;
                    n = xdigit2num(c);
                }
                c = QChar(n);
                break;
            }
            case '0':   case '1':   case '2':   case '3':
            case '4':   case '5':   case '6':   case '7': {
                int n = 0;
                for (char i = 0; i < 3; i++) {
                    if (!isOctal(c = yyget())) { yyunget(c); break; }
                    n *= 8;
                    n = digit2num(c);
                }
                c = QChar(n);
                break;
            }
        }
        else if (c.isNull()) {
            delete ps;
            yyerror("EOF in text literal.");
            break;
        }
        *ps += c;
    }
    return ps;
}

#define TOK(t)  { #t, t##_T },
static int yylex(void)
{
    static const char cToken[] = "@!{};,";
    static const struct token {
        const char *name;
        int         value;
    } sToken[] = {
        TOK(DOMAIN) TOK(RDP) TOK(OFF) TOK(HELP) TOK(COMMAND) TOK(APP)
        TOK(IDLE) TOK(DIALOG) TOK(TIME) TOK(MIN) TOK(RUN) TOK(WEB) TOK(KIOSK)
        TOK(RESTART) TOK(NO) TOK(MODE) TOK(CLEAN) TOK(INFO) TOK(BROWSER)
        TOK(MASTER) TOK(USER) TOK(COMMANDS) TOK(CHROME) TOK(FIREFOX) TOK(QUPZILLA)
        TOK(PING) TOK(SET) TOK(OUT)
        { NULL, 0 }
    };
    bool ok;
    // DBGFN();
    yylval.u = NULL;
    QChar     c;
    // Elvalaszto karakterek és kommentek (#) tlepese
    // Fajl vege eseten vege
    while((c = yyget()).isNull() || c.isSpace() || c == QChar('\n') || c == QChar('#')) {
        if (c.isNull()) return 0;   // EOF, vége
        if (c == QChar('#')) yyLine.clear();
    }
    // VALUE INT
    if (c.isDigit()) {
        QString sn = c;
        while (((c = yyget()).isDigit())) {
            sn += c;
        }
        yyunget(c);
        yylval.i = sn.toLongLong(&ok, 10);
        if (!ok) yyerror("Pprogram error");
        return INTEGER_V;
    }
    // VALUE STRING tipusu
    if (c == QChar('\"')) {
        yylval.s = yygetstr();
        return STRING_V;
    }
    // Egybetus tokenek
    if (strchr(cToken, c.toLatin1())) {
        int ct = c.toLatin1();
        return ct;
    }
    QString *sp = new QString();
    while (c.isLetterOrNumber() || c == QChar('_')) {
        *sp += c;
        c = yyget();
    }
    if (!c.isNull()) yyunget(c);
    if (sp->isEmpty()) yyerror("Invalid character");
    for (const struct token *p = sToken; p->name; p++) {
        if (p->name == *sp) {
            delete sp;
            return p->value;
        }
    }
    yylval.s = sp;
    return NAME_V;
}
