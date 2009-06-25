#include "Team_Struct.h"
#include "utilities.hh"

using namespace std;

void Team::convertFromBits(const char *bitss, Uint32 rem)
{
    MegaDeserializer bits(bitss,rem);

    for (int i = 0; i < 6; i++)
    {
        Team::Pokes &pok = pokes[i];
        pok.num = bits.pop_l(9);
        for (int j = 0; j < 4; j++)
        {
            pok.moves[j] = bits.pop_l(9);
        }
        bits.mega_pop("$4#9#7#8#8#8#8#8#8#5#5#5#5#5#5#5#1#1#1", &pok.nick, &pok.item, &pok.level, &pok.hp_ev, &pok.att_ev, &pok.def_ev,
                        &pok.satt_ev, &pok.sdef_ev, &pok.speed_ev, &pok.nature, &pok.hp_dv, &pok.att_dv, &pok.def_dv, &pok.satt_dv, &pok.sdef_dv,
                        &pok.speed_dv, &pok.gender, &pok.shiney, &pok.ability);
    };
    bits.mega_pop("$5$8$8$8", &Trainer_Name, &Trainer_Info, &Trainer_Lose, &Trainer_Win);
}

smart_ptr< fast_array<char> > Team::convertToBits() const
{
    MegaSerializer bits;
    for (int i = 0; i < 6; i++)
    {
        const Team::Pokes &pok = pokes[i];
        bits.push_l(pok.num, 9);
        for (int j = 0; j < 4; j++)
        {
            bits.push_l(pok.moves[j], 9);
        }
        bits.mega_push( "$4#9#7#8#8#8#8#8#8#5#5#5#5#5#5#5#1#1#1", &(pok.nick), pok.item, pok.level, pok.hp_ev,
                        pok.att_ev, pok.def_ev, pok.satt_ev, pok.sdef_ev, pok.speed_ev, pok.nature, pok.hp_dv,
                        pok.att_dv, pok.def_dv, pok.satt_dv, pok.sdef_dv, pok.speed_dv, pok.gender, pok.shiney,
                        pok.ability);
    };

    //5 au cas où on veut agrandir Trainer_Name jusqu'à 32 caractère par la suite
    bits.mega_push("$5$8$8$8", &Trainer_Name, &Trainer_Info, &Trainer_Lose, &Trainer_Win);

    return bits.get_strdata();
}

void Team::load(const char *file_name)
{
    ifstream in(file_name, ios::binary);

    in.seekg(0, ios::end);
    int size = in.tellg();
    in.seekg(0, ios::beg);

    char of[size];

    //attention, ca marche que dans le format compressé que c'est
    in.read(of, size);
    convertFromBits(of,size*8);
}

void Team::save(const char *file_name)
{
    ofstream out(file_name, ios::binary);
    smart_ptr< fast_array<char> > of = convertToBits();
    out.write(*of, of->size);
    out.close();
}
