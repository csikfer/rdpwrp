%{
#include "dialog.h"
#include <QStack>

QString             yyLastError;
QString             yyLastLine;
int                 yyLineNo;
QString             yyLine;
static QStack<QChar> yyUnGetChars;
static QFile       *yyFile = NULL;

static int yyparse();

int parseConfig(QFile *_in)
{
    yyFile = _in;;
    yyLastError.clear();
    yyLine.clear();
    yyUnGetChars.clear();
    yyLineNo = 0;
    int r = yyparse();
    yyFile = NULL;
    return r;
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
        yyLastLine = yyLine  = yyFile->readLine();
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
    void *          u;
    qlonglong       i;
    bool            b;
    QString *       s;
    QStringList *  sl;
}

%token      DOMAIN_T RDP_T OFF_T HELP_T COMMAND_T APP_T
%token      IDLE_T DIALOG_T TIME_T MIN_T RUN_T WEB_T

%token <i>  INTEGER_V
%token <s>  STRING_V NAME_V

%type  <i>  int int_z
%type  <s>  str cmd icon
%type <sl>  strs

%%

conf    : configs
        ;
configs : config
        | config configs
        ;
config  : DOMAIN_T str '{' strs '}'         { Dialog::addDomain($2, $4); }
        | RDP_T COMMAND_T str ';'           { Dialog::setRdpCmd($3); }
        | OFF_T COMMAND_T str ';'           { Dialog::setOffCmd($3); }
        | HELP_T COMMAND_T str ';'          { Dialog::setHelpCmd($3); }
        | IDLE_T TIME_T int ';'             { idleTime  = $3; }
        | IDLE_T DIALOG_T TIME_T int ';'    { idleDialogTime  = $4; }
        | MIN_T RUN_T TIME_T int ';'        { minProgTime = $4; }
        | COMMAND_T str cmd icon int_z ';'  { Dialog::addCommand($2, $3, $4, $5); }
        ;
str     : NAME_V        { $$ = $1; }
        | STRING_V      { $$ = $1; }
        ;
strs    : str           { $$ = new QStringList(*$1); delete $1; }
        | strs  str     { $$ = $1; *$$ << *$2; delete $2; }
        ;
int     : INTEGER_V     { $$ = $1; }
        ;
int_z   : int           { $$ = $1; }
        |               { $$ = -1; }
        ;
cmd     : str           { $$ = $1; }
        | HELP_T        { $$ = new QString(Dialog::getRdpCmd()); }
        | OFF_T         { $$ = new QString(Dialog::getOffCmd()); }
        ;
icon    : str           { $$ = $1; }
        | HELP_T        { $$ = new QString(":/images/help.ico"); }
        | WEB_T         { $$ = new QString(":/images/web.ico"); }
        | OFF_T         { $$ = new QString(":/images/off.ico"); }
        | APP_T         { $$ = new QString(":/images/app.ico"); }
        |               { $$ = new QString(":/images/app.ico"); }
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
    static const char cToken[] = "{};,";
    static const struct token {
        const char *name;
        int         value;
    } sToken[] = {
        TOK(DOMAIN) TOK(RDP) TOK(OFF) TOK(HELP) TOK(COMMAND) TOK(APP)
        TOK(IDLE) TOK(DIALOG) TOK(TIME) TOK(MIN) TOK(RUN) TOK(WEB)
        { NULL, 0 }
    };
    bool ok;
    // DBGFN();
    yylval.u = NULL;
    QChar     c;
    // Elvalaszto karakterek és kommentek atlepese
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
