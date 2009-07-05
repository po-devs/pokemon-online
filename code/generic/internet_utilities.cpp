#include <iostream>
#include <string>
#include "intersock.hpp"
#include "utilities.hh"
#include "internet_utilities.hh"

using namespace std;
using namespace interface;


namespace _508

{

/** @name  calc_length
    @brief calcule la longueur du message à recevoir
    @param data  le début du message
    @param l_l  la longueur en nombre de caractères de la longueur du message
**/
size_t calc_length(const char *data, uint8_t l_l)
{
    size_t res = 0;
    while (l_l > 0)
    {
        res *= 256;
        res += (uint8_t)*data;
        --l_l;
        ++data;
    }
    return res;
}

/*  Receive a message from &sock and append it to recv, stopping if finished is true.
    sock should be set to NonBlocking beforehand.

    Return:
        - sf::Socket::NotReady if was interrupted by finished
        - sf::Socket::Error or sf::Socket::Disconnected if such was the error
        - sf::Socket::Done if everything went well
*/
Socket::Status recv_body(TCPSocket &sock, string &recv, const bool &finished)
{
    /* three steps: getting the length of the length of the message,
       then the length of the message,
       then the message
    */

    char buffer[500];

    size_t received;

    TRACE(/* Step one : length of length */)
    TRACE(while (1))
    {
        if (finished)
            return Socket::NotReady;

        Socket::Status s = sock->Receive(buffer, 1, received);

        if (s == Socket::Error) {
            TRACE ( (void) "Error"; )
            TRACE (return s;)
        }

        if (s == Socket::Done) {
            TRACE ( (void) "Done"; )
            TRACE(break;)
        }

        SDL_Delay(10);
    }

    TRACE(if (received != 1)) {
        TRACE(return Socket::Error;)
    }

    /* length of length :) */
    TRACE(uint8_t l_l = buffer[0];)

    // cursor, where are we located in the buffer
    int i=0;

    /* I know about the warning, but if someone changes the size of the buffer to
        something less than 255, it won't be useless anymore */
    if (l_l >= sizeof(buffer))
            return Socket::Error;

    //Step two : length
    TRACE(while (1))
    {
        if (finished)
            return Socket::NotReady;

        Socket::Status s = sock->Receive(buffer+i, l_l - i, received);

        if (s == Socket::Error) {
            TRACE( (void) "Error"; )
            TRACE( return s; )
        }

        if (s == Socket::Done)
        {
            TRACE( (void) "Done"; )
            TRACE(i += received;) /*moving cursor*/
            if (i == l_l)
                break;
            else
                continue;
        }

        SDL_Delay(10);
    }

    //length of the message
    TRACE(size_t length = calc_length(buffer, l_l);)
    TRACE(recv.reserve(recv.length()+length);)

    //Step three: Yata!
    TRACE(while (1))
    {
        if (finished)
            return Socket::NotReady;

        size_t l = MIN(sizeof(buffer), length);

        if (l > 0) {
            Socket::Status s = sock->Receive(buffer, l, received);

            if (s == Socket::Error) {
                TRACE( (void) "Error"; )
                TRACE(return s;)
            }

            if (s == Socket::Done)
            {
                TRACE( (void) "Done"; )
                TRACE(recv.append(buffer, received);)
                length -= received; /*remaining length to go*/

                if (length == 0)
                    break;
                else
                    continue;
            }

            SDL_Delay(10);
        }

        break;
    }

    TRACE(return Socket::Done;)
}

/*  Send a formmatted message stopping if finished is suddenly set to true.
    sock should be set to NonBlocking beforehand.

    Returns:
        - Socket::NotReady if was interrupted by finished
        - Socket::Error or sf::Socket::Disconnected if such was the error
        - Socket::Done if everything went well
*/
Socket::Status send_body(TCPSocket &sock, const string &send, const bool &finished)
{
    return send_body(sock, send.data(), send.length(), finished);
}

Socket::Status send_body(TCPSocket &sock, const char* send, size_t length, const bool &finished)
{
    string toSend;

    /* on prépare toSend */
    format_string(toSend, send, length);

    cout << "Sending body " << send << endl;
    cout << "Length of message: " << toSend.length() << endl;
    cout << "Character 1 : " << int (toSend[1]) << endl;

    /* on envoie la sauce */
    while (1)
    {
        if (finished)
            return Socket::NotReady;

        TRACE(Socket::Status s = sock->Send(toSend.data(), toSend.length());)

        if (s == Socket::NotReady) {
            SDL_Delay(10);
            continue;
        }

        TRACE(return s;)
    }
}



/** @name  calc_l_l
    @brief calcule la longueur de la longueur du message à envoyer
    @param length  la longueur du message à envoyer
**/
uint8_t calc_l_l(size_t length)
{
    uint8_t ret = 0;

    while (length > 0)
    {
        length = length >> 8; //divise par 256
        ++ret;
    }

    if (ret == 0)
        ret = 1;

    return ret;
}

/** @name  format_length
    @brief rentre la longueur du message à envoyer dans la string donnée à partir de la position 1
    @param length la longueur du message à envoyer
    @param l_l    la longueur de la longueur du message à envoyer
    @param buffer la string dans laquelle on écrit

    suppose que la string est déjà assez grande
**/
void format_length(size_t length, uint8_t l_l, std::string &buffer)
{
    char *temp = &buffer[0] + l_l;

    if (length == 0)
    {
        *temp = 0;
    } else
    {
        while (length > 0)
        {
            *temp = length & 255; //modulo 256
            --temp;
            length = length >> 8; //divise par 256
        }
    }
}

/* formatte la première string à partir de la deuxième.

   Ne pas passer deux fois le même objet ou vous aurez de mauvais résultats...
*/
void format_string(std::string &formatted, const std::string &base)
{
    format_string (formatted, base.data(), base.length());
}

void format_string(std::string &formatted, const char *base, size_t length)
{
    uint8_t l_l = calc_l_l(length);

    formatted.reserve(l_l + 1 + length);
    formatted.resize(1+l_l); //to clear previous data and allow enough space for format_length

    //formatting
    formatted[0] = l_l;
    format_length(length, l_l, formatted);
    formatted.append(base, length);
}


}//namespace _508
