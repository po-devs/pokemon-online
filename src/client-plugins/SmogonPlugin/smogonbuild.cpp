#include "smogonbuild.h"

smogonbuild::smogonbuild(){}

//This function causes the plugin to crash horribly for reasons beyond us.
//SmogonBuild::~SmogonBuild()
//{
//    delete item;
//    delete ability;
//    delete nature;
//    delete EVList;
//    delete move1;
//    delete move2;
//    delete move3;
//    delete move4;
//}

/*
 *  Converts the EV int array into a string so that it can be easily
 *      displayed to the user.
 */
QString smogonbuild::EVListToString()
{
    QString retString = "";
    for(int i=0;i<6;i++)
    {
        if(EVList->at(i) != 0)
        {
            char buf[10];
            switch(i)
            {
            case 0:sprintf(buf, "%d HP /", EVList->at(i));break;
            case 1:sprintf(buf, "%d Atk /", EVList->at(i));break;
            case 2:sprintf(buf, "%d Def /", EVList->at(i));break;
            case 3:sprintf(buf, "%d SpA /", EVList->at(i));break;
            case 4:sprintf(buf, "%d SpD /", EVList->at(i));break;
            case 5:sprintf(buf, "%d Spe /", EVList->at(i));break;
            }
            retString += buf;
        }
    }
    //Get rid of the rightmost extra " /"
    retString = retString.left(retString.length()-2);
    return retString;
}
