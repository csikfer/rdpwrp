#ifndef PARSER_H
#define PARSER_H

#include <QIODevice>
#include <QByteArray>

extern QString             yyLastError; ///< Parser utolsó hiba string.
extern QString             yyLastLine;  ///< Az utoljára feldolgozott teljes sor.
extern int                 yyLineNo;    ///< Az eddig beolvasott sorok száma.
extern QString             yyLine;      ///< A sorból még hátra lévő (feldolgozatlan)rész.

extern int parseConfig(QIODevice *_in);
extern int parseCommand(QByteArray& _in, QByteArray& _out);

void yyprint(const QByteArray& _o);
inline void yyprint(const QString& _o)  { yyprint(_o.toUtf8()); }
inline void yyprint(const char * _o)    { yyprint(QString(_o)); }


#endif // PARSER_H
