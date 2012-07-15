#include "sqlstatsbattleplugin.h"
#include "../PokemonInfo/battlestructs.h"

SqlStatsBattlePlugin::SqlStatsBattlePlugin(SqlStatsPlugin *master): master(master) {
    master->battles++;
}

SqlStatsBattlePlugin::~SqlStatsBattlePlugin() {
    master->battles--;
}

QHash<QString, BattlePlugin::Hook> SqlStatsBattlePlugin::getHooks() {
    QHash<QString, Hook> ret;
    ret.insert("battleStarting(BattleInterface&)", (Hook)(&SqlStatsBattlePlugin::battleStarting));
    return ret;
}

int SqlStatsBattlePlugin::battleStarting(BattleInterface &b)
{
    //qDebug() << "Battle Starting. Stats " << this;

    if (b.clauses() & ChallengeInfo::ChallengeCup) {
        qDebug() << "CC " << this;
        return -1;
    }

    QString tier = b.tier();

    if (tier.length() == 0) {
        tier = QString("Mixed Tiers Gen %1").arg(b.gen().num);
    }

    if (!master->createTable(tier))
        return -1;

    /*
    ааа, тупое говно
    for (int player = 0; player<2; player++) {
        TeamBattle team = b.team(player);
        for (int slot = 0; slot<6; slot++) {// number of pokes in the team?
            QSqlTableModel *model = new QSqlTableModel(0, master->db);
            model->setTable("`" + tier + "`"); // why the fuck should I escape shit?

            PokeBattle &pokemon = team.poke(slot);
            QList<int> moves;
            for (int i = 0; i<4; i++)
                moves.push_back(pokemon.move(i));
            qSort(moves);
            QList<int> ev = pokemon.evs();
            QList<int> iv = pokemon.dvs();
            // don't read the next few lines
            model->setFilter(
                          "  `pokemon` = " + QString::number(pokemon.num().pokenum)
                        + " AND `form`    = " + QString::number(pokemon.num().subnum)
                        + " AND `level`   = " + QString::number(pokemon.level())
                        + " AND `nature`  = " + QString::number(pokemon.nature())
                        + " AND `ability` = " + QString::number(pokemon.ability())
                        + " AND `gender`  = " + QString::number(pokemon.gender())

                        + " AND `move1`   = " + QString::number(moves[0])
                        + " AND `move2`   = " + QString::number(moves[1])
                        + " AND `move3`   = " + QString::number(moves[2])
                        + " AND `move4`   = " + QString::number(moves[4])

                        + " AND `ev_hp`   = " + QString::number(ev[Hp])
                        + " AND `ev_atk`  = " + QString::number(ev[Attack])
                        + " AND `ev_def`  = " + QString::number(ev[Defense])
                        + " AND `ev_satk` = " + QString::number(ev[SpAttack])
                        + " AND `ev_sdef` = " + QString::number(ev[SpDefense])
                        + " AND `ev_spd`  = " + QString::number(ev[Speed])

                        + " AND `iv_hp`   = " + QString::number(iv[Hp])
                        + " AND `iv_atk`  = " + QString::number(iv[Attack])
                        + " AND `iv_def`  = " + QString::number(iv[Defense])
                        + " AND `iv_satk` = " + QString::number(iv[SpAttack])
                        + " AND `iv_sdef` = " + QString::number(iv[SpDefense])
                        + " AND `iv_spd`  = " + QString::number(iv[Speed])
            );

            model->select();
            qDebug() << "query returned" << model->rowCount() << "rows";
            if (model->rowCount()) {
                qDebug() << "updating existing entry ";
                QSqlRecord record = model->record(0);
                record.setValue("usage", record.value("usage").toInt() +1 );
                // if lead ...
                model->setRecord( 0, record );
            }
            else {
                qDebug() << "creating new entry";
                QSqlRecord record = model->record();

                model->removeColumn( model->fieldIndex("id") );

                record.setValue("pokemon", pokemon.num().pokenum);
                record.setValue("form",    pokemon.num().subnum);
                record.setValue("level",   pokemon.level());
                record.setValue("nature",  pokemon.nature());
                record.setValue("ability", pokemon.ability());
                record.setValue("gender",  pokemon.gender());

                record.setValue("move1",   moves[0]);
                record.setValue("move2",   moves[1]);
                record.setValue("move3",   moves[2]);
                record.setValue("move4",   moves[4]);

                record.setValue("ev_hp",   ev[Hp]);
                record.setValue("ev_atk",  ev[Attack]);
                record.setValue("ev_def",  ev[Defense]);
                record.setValue("ev_satk", ev[SpAttack]);
                record.setValue("ev_sdef", ev[SpDefense]);
                record.setValue("ev_spd",  ev[Speed]);

                record.setValue("iv_hp",   iv[Hp]);
                record.setValue("iv_atk",  iv[Attack]);
                record.setValue("iv_def",  iv[Defense]);
                record.setValue("iv_satk", iv[SpAttack]);
                record.setValue("iv_sdef", iv[SpDefense]);
                record.setValue("iv_spd",  iv[Speed]);

                record.setValue("usage",   1);
                // if ...
                record.setValue("lead",    0);

                if (!model->insertRecord(-1, record))
                    qDebug() << "Failed to insert new entry:" << master->db.lastError().text();

                qDebug() << "new record:" << record;
            }
            model->submitAll();
            qDebug() << model->query().lastQuery();// << master->db.lastError().text();
        }
    }
    qDebug() << master->db.lastError().text();
*/

    for (int player = 0; player<2; player++) {
        TeamBattle team = b.team(player);
        for (int slot = 0; slot<6; slot++) {
            PokeBattle &pokemon = team.poke(slot);

            QList<int> moves;
            for (int i = 0; i<4; i++)
                moves.push_back(pokemon.move(i));
            qSort(moves);
            QList<int> ev = pokemon.evs();
            QList<int> iv = pokemon.dvs();

            QByteArray idArray;
            idArray += pokemon.num().toPokeRef();
            idArray += pokemon.level();
            idArray += pokemon.nature();
            idArray += pokemon.ability();
            idArray += pokemon.gender();
            for (int i=0; i<4; i++)
                idArray += moves[i];
            for (int i=0; i<6; i++)
                idArray += ev[i];
            for (int i=0; i<6; i++)
                idArray += iv[i];
            quint16 id = qChecksum(idArray.data(), idArray.length());
            // so, is it really unique enough?

            qDebug() << id;

            QSqlQuery query(master->db);
            query.prepare(
                          "INSERT OR IGNORE INTO `" +tier+ "` ("
                            "`id`, "
                            "`pokemon`, `form`, `level`, `nature`, `ability`, `gender`, "
                            "`move1`, `move2`, `move3`, `move4`, "
                            "`ev_hp`, `ev_atk`, `ev_def`, `ev_satk`, `ev_sdef`, `ev_spd`, "
                            "`iv_hp`, `iv_atk`, `iv_def`, `iv_satk`, `iv_sdef`, `iv_spd`, "
                            "`usage`, `lead_usage` "
                          ") "

                        "VALUES ("
                          ":id, "
                          ":pokemon, :form, :level, :nature, :ability, :gender, "
                          ":move1, :move2, :move3, :move4, "
                          ":ev_hp, :ev_atk, :ev_def, :ev_satk, :ev_sdef, :ev_spd, "
                          ":iv_hp, :iv_atk, :iv_def, :iv_satk, :iv_sdef, :iv_spd, "
                          "0, 0"
                        "); "
                          //"ON DUPLICATE KEY IGNORE; "
                        );
            query.bindValue(":id",      id);
            query.bindValue(":pokemon", pokemon.num().pokenum);
            query.bindValue(":form",    pokemon.num().subnum);
            query.bindValue(":level",   pokemon.level());
            query.bindValue(":nature",  pokemon.nature());
            query.bindValue(":ability", pokemon.ability());
            query.bindValue(":gender",  pokemon.gender());

            query.bindValue(":move1",   moves[0]);
            query.bindValue(":move2",   moves[1]);
            query.bindValue(":move3",   moves[2]);
            query.bindValue(":move4",   moves[3]);

            query.bindValue(":ev_hp",   ev[Hp]);
            query.bindValue(":ev_atk",  ev[Attack]);
            query.bindValue(":ev_def",  ev[Defense]);
            query.bindValue(":ev_satk", ev[SpAttack]);
            query.bindValue(":ev_sdef", ev[SpDefense]);
            query.bindValue(":ev_spd",  ev[Speed]);

            query.bindValue(":iv_hp",   iv[Hp]);
            query.bindValue(":iv_atk",  iv[Attack]);
            query.bindValue(":iv_def",  iv[Defense]);
            query.bindValue(":iv_satk", iv[SpAttack]);
            query.bindValue(":iv_sdef", iv[SpDefense]);
            query.bindValue(":iv_spd",  iv[Speed]);

            if (!query.exec()) {
                qDebug() << "failed to execute query" << query.executedQuery() << "Error:" << query.lastError().text();
                return -1;
            }

            // if(sqlite) ...
            QSqlQuery incQuery(master->db);
            if (!incQuery.exec("UPDATE `"+tier+"` set usage = usage +1 where id = " +QString::number(id)+ ";")) {
                qDebug() << "failed to execute query" << incQuery.executedQuery() << "Error:" << incQuery.lastError().text();
                return -1;
            }
            //lead


        }
    }

    //qDebug() << "End Battle Starting Stats " << this;
    return -1;
}
