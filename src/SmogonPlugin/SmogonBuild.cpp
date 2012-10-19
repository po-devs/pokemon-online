#include "SmogonBuild.h"

SmogonBuild::SmogonBuild(){}

SmogonBuild::~SmogonBuild()
{
    delete item;
    delete ability;
    delete nature;
    delete EVList;
    delete move1;
    delete move2;
    delete move3;
    delete move4;
}

void SmogonBuild::printBuild()
{
    SmogonBuild build = *this;
    //Print the build name
    printf("buildName : %s\n",build.buildName.toAscii().data());
    //print the items
    foreach(QString item, *build.item)
    {
        printf("<Item : %s>", item.toAscii().data());
    }
    printf("\n");
    //print the abilities
    foreach(QString ability, *build.ability)
    {
        printf("<Ability : %s>", ability.toAscii().data());
    }
    printf("\n");
    foreach(QString nature, *build.nature)
    {
        printf("<Nature : %s>", nature.toAscii().data());
    }
    printf("\n");
    foreach(int ev, *build.EVList)
    {
        printf("<EV : %d>",ev);
    }
    printf("\n");
    foreach(QString move, *build.move1)
    {
        printf("<Move1 : %s>",move.toAscii().data());
    }
    printf("\n");
    foreach(QString move, *build.move2)
    {
        printf("<Move2 : %s>",move.toAscii().data());
    }
    printf("\n");
    foreach(QString move, *build.move3)
    {
        printf("<Move3 : %s>",move.toAscii().data());
    }
    printf("\n");
    foreach(QString move, *build.move4)
    {
        printf("<Move4 : %s>",move.toAscii().data());
    }
    printf("\n");

    printf("Description: %s", build.description.toAscii().data());
}
