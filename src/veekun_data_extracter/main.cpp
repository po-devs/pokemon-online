#include <QtCore>
#include <QtXml>

int main(int argc, char *argv[])
{
    QDomDocument doc("doc");
    QFile in("raw_dump.txt");
    in.open(QIODevice::ReadOnly);

    QString msg;
    int line, col;
    doc.setContent(&in,&msg,&line,&col);
    if (msg.length() > 0) {
        qDebug() << msg << ": line " << line << ", col " << col;
    }

    QList<QFile*> files;

    QDomElement el = doc.firstChildElement("table");
    if (el.isNull()) {
        qDebug() << "table not found";
    }

    QDomElement header = el.firstChildElement("tr");
    if (header.isNull()) {
        qDebug() << "tr not found";
    }

    QDomNodeList headers = header.childNodes();

    for (int i = 0; i < headers.size(); i++) {
        QDomNode n = headers.at(i);
        QFile *f = new QFile("5G/"+n.toElement().text()+".txt");
        f->open(QIODevice::WriteOnly);
        f->write("\n"); //#0 move
        files.push_back(f);
    }

    QDomElement next = header.nextSiblingElement("tr");
    while (!next.isNull()) {
        QDomNodeList data = next.childNodes();

        for (int i = 0; i < data.size(); i++) {
            QDomNode n = data.at(i);
            files[i]->write(n.toElement().text().toUtf8() + "\n");
        }

        next = next.nextSiblingElement("tr");
    }

    foreach(QFile *f, files) {
        delete f;
    }

    qDebug() << "Finished!";
    QCoreApplication a(argc, argv);
    a.exec();
}
