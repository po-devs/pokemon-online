#ifndef DATA_H
#define DATA_H

struct Data {
    struct PosValue {
        int pos;
        QColor value;
        QString desc;

        PosValue(int pos=0, QColor color = QColor(), const QString &desc = QString()) : pos(pos), value(color), desc(desc) {}
    };

    QString stylesheet;
    QList<PosValue> colors;

    QVector<int> findColor(const QColor &c);
};

#endif // DATA_H
