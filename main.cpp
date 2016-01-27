#include <iostream>
#include <QString>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

QStringList readFileToStringList(QString fileName)
{
    QStringList strings;
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream stream(&file);
    while (true)
    {
        QString line = stream.readLine();
        if (line.isNull())
            break;
        else
            strings.append(line);
    }
    return strings;
}

QString regExpMatch(const QString &regExp, const QString &pattern, int nth)
{
    QRegularExpression re(regExp, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(pattern);
    if (match.hasMatch())
        return match.captured(nth);

    return QString();
}

QStringList regExpMatch(const QString &regExp, const QString &pattern)
{
    QRegularExpression re(regExp, QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(pattern);
    if (match.hasMatch())
        return match.capturedTexts();

    return QStringList();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Please provide a file from \"git log --stat\" as an arg\n";
        return -1;
    }
    QStringList strings = readFileToStringList(QString::fromLatin1(argv[1]));

    QString author = "";

    QMap<QString, int> strCount;

    for (QStringList::iterator it = strings.begin(); it != strings.end(); ++it)
    {
        QString str = *it;

        QString match = regExpMatch("Merge:", str, 0);
        if (!match.isEmpty())
        {
            while (it != strings.end())
            {
                it++;
                match = regExpMatch("commit [0-9a-z]{40}", *it, 0);
                if (!match.isEmpty())
                    break;
            }
        }

        QString newAuthor = regExpMatch("Author: ([^<]+)", str, 1).trimmed();
        if (!newAuthor.isEmpty())
            author = newAuthor;

//        if (!regExpMatch("Date:", str, 0).isEmpty())
//            qDebug() << str;
        if (!regExpMatch("Date:[\\s]+Sat Dec 26", str, 0).isEmpty())
            break;

        QStringList captured = regExpMatch("([^\\s]+)[\\s]+\\|[\\s]+([\\d]+)[\\s]+([\\+]*)([\\-]*)", str);
        if (!captured.isEmpty())
        {
            QString fileName = captured.at(1);
            int changedStrings = captured.at(2).toInt();
            int plus = captured.at(3).length();
            int minus = captured.at(4).length();

            int newStrings = changedStrings * plus / (plus + minus);

//            qDebug() << author << fileName << newStrings;

            bool ok = regExpMatch("\\.css$", fileName).isEmpty()
                    && regExpMatch("\\.map$", fileName).isEmpty()
                    && regExpMatch("/jsx-tpl/", fileName).isEmpty()
                    && regExpMatch("\\.idl$", fileName).isEmpty()
                    && regExpMatch("\\.rds$", fileName).isEmpty();

            if (ok)
                strCount[author] += newStrings;
        }
    }

//    qDebug() << strCount;

    QMapIterator<QString, int> i(strCount);
    while (i.hasNext())
    {
        i.next();
        std::cout << i.key().toStdString() << ": " << i.value() << std::endl;
    }

    return 0;
}

