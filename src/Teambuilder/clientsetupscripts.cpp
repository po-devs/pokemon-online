#include "client.h"
#include "channel.h"
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>

Q_DECLARE_METATYPE(Analyzer*)
typedef Analyzer* T;

static QScriptValue analyzerTo(QScriptEngine *e, const T& r) {
    return e->newQObject(r);
}

static void analyzerFrom(const QScriptValue&s, T&r) {
    r = dynamic_cast<T>(s.toQObject());
}

Q_DECLARE_METATYPE(Channel*)
typedef Channel* U;

static QScriptValue channelTo(QScriptEngine *e, const U& r) {
    return e->newQObject(r);
}

static void channelFrom(const QScriptValue&s, U&r) {
    r = dynamic_cast<U>(s.toQObject());
}

typedef QHash<qint32, QString> hash32string;
Q_DECLARE_METATYPE(hash32string)

static QScriptValue hash32stringTo(QScriptEngine *e, const hash32string& r) {
    QScriptValue v = e->newObject();

    QHashIterator<qint32, QString> it(r);

    while (it.hasNext()) {
        it.next();

        v.setProperty(it.key(), it.value());
    }

    return v;
}

static void hash32stringFrom(const QScriptValue &v, hash32string &r) {
    r.clear();

    QScriptValueIterator it(v);

    while (it.hasNext()) {
        it.next();

        r.insert(it.name().toInt(), it.value().toString());
    }
}

Q_DECLARE_METATYPE(UserInfo)

static QScriptValue userInfoTo(QScriptEngine *e, const UserInfo& info) {
    QScriptValue v = e->newObject();
    v.setProperty("name", info.name);
    v.setProperty("flags", info.flags);
    v.setProperty("ip", info.ip);
    v.setProperty("auth", info.auth);
    v.setProperty("date", info.date);
    return v;
}
static void userInfoFrom(const QScriptValue &v, UserInfo& info) {
    info.name = v.property("name").toString();
    info.flags = v.property("flags").toInt32();
    info.name = v.property("ip").toString();
    info.auth = v.property("auth").toInt32();
    info.date = v.property("date").toString();
}

Q_DECLARE_METATYPE(QColor)
Q_DECLARE_METATYPE(PlayerInfo)
Q_DECLARE_METATYPE(QStringList)

static QScriptValue playerInfoTo(QScriptEngine *e, const PlayerInfo& info) {
    QScriptValue v = e->newObject();
	v.setProperty("id", info.id);
	v.setProperty("name", info.name);
	v.setProperty("info", info.info);
	v.setProperty("auth", info.auth);
	v.setProperty("flags", info.flags.data);
	v.setProperty("avatar", info.avatar);
	v.setProperty("color", e->toScriptValue(info.color));
	return v;
}

static void playerInfoFrom(const QScriptValue &v, PlayerInfo& info) {
    info.id = v.property("id").toInt32();
	info.name = v.property("name").toString();
	info.info = v.property("info").toString();
	info.auth = v.property("auth").toInt32();
	info.flags.data = v.property("flags").toUInt32();
	info.avatar = v.property("avatar").toUInt32();
	info.color = qscriptvalue_cast<QColor>(v.property("color"));
}

Q_DECLARE_METATYPE(FindBattleData)

static QScriptValue findBattleDataTo(QScriptEngine *e, const FindBattleData& data) {
    QScriptValue v = e->newObject();
    v.setProperty("rated", data.rated);
    v.setProperty("sameTier", data.sameTier);
    v.setProperty("ranged", data.ranged);
    v.setProperty("range", data.range);
    v.setProperty("teams", data.teams);
    return v;
}

static void findBattleDataFrom(const QScriptValue &v, FindBattleData& data) {
    data.rated = v.property("rated").toBool();
    data.sameTier = v.property("sameTier").toBool();
    data.ranged = v.property("ranged").toBool();
    data.range = v.property("range").toUInt16();
    data.teams = quint8(v.property("teams").toUInt16()); // there is no toUInt8(), so use this workaround
}

void Client::registerMetaTypes(QScriptEngine *e)
{
    qScriptRegisterMetaType<T>(e, &analyzerTo, &analyzerFrom);
    qScriptRegisterMetaType<U>(e, &channelTo, &channelFrom);
    qScriptRegisterMetaType<hash32string>(e, &hash32stringTo, &hash32stringFrom);
    qScriptRegisterMetaType<UserInfo>(e, &userInfoTo, &userInfoFrom);
    qScriptRegisterMetaType<PlayerInfo>(e, &playerInfoTo, &playerInfoFrom);
    qScriptRegisterMetaType<FindBattleData>(e, &findBattleDataTo, &findBattleDataFrom);
    qScriptRegisterSequenceMetaType<QStringList>(e);
}
