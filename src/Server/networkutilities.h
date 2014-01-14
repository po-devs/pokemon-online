#ifndef NETWORKUTILITIES_H
#define NETWORKUTILITIES_H

#include <Utilities/coreclasses.h>

template <typename ...Params>
QByteArray makeZipPacket(int command, Params&&... params) {
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);

    out.pack(uchar(command), std::forward<Params>(params)...);

    QByteArray cp = qCompress(tosend);

    QByteArray ret(6+cp.length(), Qt::Uninitialized);

    const int l = ret.length()-4;
    ret[0] = l >> (3*8);
    ret[1] = l >> (2*8);
    ret[2] = l >> 8;
    ret[3] = l;
    ret[4] = '\0'; /* ZipCommand == 0 */
    ret[5] = '\0'; /* 0 = Single command, 1 would be multiple packets */

    ret.replace(6, cp.length(), cp);
    return ret;
}

template <typename ...Params>
QByteArray makePacket(int command, Params&&... params) {
    QByteArray ret(4, Qt::Uninitialized);
    DataStream out(&ret, QIODevice::Append);

    out.pack(uchar(command), std::forward<Params>(params)...);

    const int l = ret.length()-4;
    ret[0] = l >> (3*8);
    ret[1] = l >> (2*8);
    ret[2] = l >> 8;
    ret[3] = l;

    return ret;
}


#endif // NETWORKUTILITIES_H
