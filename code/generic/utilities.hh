#ifndef UTILITIES_HH_INCLUDED
#define UTILITIES_HH_INCLUDED
//
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <cstdarg>

#ifndef TRACE
 #ifdef DEBUG
  #define TRACE(ARG) cout << #ARG << endl; ARG
 #else
  #define TRACE(ARG) ARG
 #endif
#endif
#ifndef MIN
 #define MIN(a, b) (a < b ? a : b)
#endif
#ifndef MAX
 #define MAX(a, b) (a > b ? a : b)
#endif

//template <class T>
//class fast_array
//{
//    public:
//        T *data;
//        size_t size;
//
//        fast_array(size_t size = 0)
//        {
//            this->size = size;
//            data = (T*) malloc(size*sizeof(T));
//        }
//
//        ~fast_array()
//        {
//            free(data);
//        }
//
//        void resize(size_t new_size);
//
//        void clear()
//        {
//            free(data);
//            data = NULL;
//            size = 0;
//        }
//
//        operator const T*() const
//          { return data; }
//
//        operator T*()
//          { return data; }
//
//        const T& operator * () const
//          { return *data; }
//
//        T& operator * ()
//          { return *data; }
//
//        const T* operator->() const
//          { return &(operator*()); }
//
//        T* operator->()
//          { return &(operator*()); }
//
//        T* operator+(const T * const &item) const
//          { return data + item.data; }
//
//        const T& operator [] (int i) const
//          { return *(data+i); }
//
//        T& operator [] (int i)
//          { return *(data+i); }
//};
template <class T>
class fast_array
{
    public:
        std::vector<T> elems;

        T *data;
        size_t size;

        fast_array(size_t size = 0)
        {
            elems.resize(size);
            this->size = size;
            data = &elems[0];
        }

        void resize(size_t new_size)
        {
            elems.resize(new_size);
            size = new_size;
            data = &elems[0];
        }

        void clear()
        {
            resize(0);
        }

        operator const T*() const
          { return data; }

        operator T*()
          { return data; }

        const T& operator * () const
          { return *data; }

        T& operator * ()
          { return *data; }

        const T* operator->() const
          { return &(operator*()); }

        T* operator->()
          { return &(operator*()); }

        T* operator+(const T * const &item) const
          { return data + item.data; }

        const T& operator [] (int i) const
          { return *(data+i); }

        T& operator [] (int i)
          { return *(data+i); }
};

template <class T>
class smart_ptr
{
    public:
        /* private */
        void destroy()
          { if (count && !(--(*count))) delete count, delete data, count = NULL; }

        T *data;
        size_t *count;

        /* public */
        smart_ptr() : count(NULL)
          { ; }

        smart_ptr(T* item) : data(item), count(new size_t(1))
          { ; }

        smart_ptr(const smart_ptr<T> &item)
          { data = item.data, count = item.count; if (count) ++(*count); }

        ~smart_ptr()
          { destroy(); }

        bool occupied () const
          { return count!=NULL; }

        size_t refcount() const
            { if (!count) return 0; else return *count; }

        operator T*() const
          { return data; }

        operator bool () const
          { return occupied(); }

        const T& operator * () const
          { return *data; }

        T& operator * ()
          { return *data; }

        const T* operator->() const
          { return &(operator*()); }

        T* operator->()
          { return &(operator*()); }

        T* operator+(const T * const &item) const
          { return data + item.data; }

        const smart_ptr<T> & operator=(T *item)
          { destroy(); count = new size_t(1); data = item; return *this;}

        const smart_ptr<T> & operator=(const smart_ptr<T> &item)
          { if (item.count == count) return *this; destroy(); if (item.count) data = item.data, count = item.count, ++(*count); return (*this); }

        /* Attention, les smart_ptr ne gèrent pas les tableaux, surtout au niveau de la libération */
        const T& operator [] (int i) const
          { return *(data+i); }

        T& operator [] (int i)
          { return *(data+i); }

        bool operator == (const smart_ptr<T> &other) const
          { return count == other.count; }

        bool operator < (const smart_ptr<T> &other) const
          { return count < other.count; }

        bool operator != (const smart_ptr<T> &other) const
          { return count != other.count; }
};

//if you want to wrap some class in a smart ptr...
template <class T>
class wrapper : public smart_ptr<T>
{
public:
    wrapper () : smart_ptr<T> (new T()) {;}
    template <class U>
    wrapper (const U &init) : smart_ptr<T> (new T(init)) {;}

    template <class U>
    operator U () const { return U(**this); }

    template <class U>
    operator U () { return U(**this); }
};

class NonCopyable
{
private:
    NonCopyable(const NonCopyable &other){;}
public:
    NonCopyable() {;}
};


template <class T>
std::string toString(T item)
{
    std::stringstream temp;
    temp << item;
    return temp.str();
}

template <class T>
std::string padd(int num, T item)
{
    std::string z;
    z.assign(num, '0');
    z+= toString(item);
    return z.substr(z.length()-num);
}

template <class A, class B>
A ConvertTo(const B &item)
{
    std::stringstream str;
    str << item;
    A ret;
    str >> ret;
    return ret;
}

smart_ptr<std::string> find_line(const char* path, int num, bool binary = false);
smart_ptr<std::string> find_line(std::ifstream &file, int num);
smart_ptr< fast_array<char> > get_file_content(const char *path, bool binary = false);

template <>
inline int ConvertTo(const std::string &item)
{
    return atoi(item.c_str());
}

template <>
inline int ConvertTo(char const * const & item)
{
    return atoi(item);
}

template <>
inline long ConvertTo(const std::string &item)
{
    return atol(item.c_str());
}

template <>
inline long ConvertTo(char const * const & item)
{
    return atol(item);
}

template <>
inline size_t ConvertTo(const std::string &item)
{
    return atol(item.c_str());
}

template <>
inline size_t ConvertTo(char const * const & item)
{
    return atol(item);
}

/* Sérialise au bit près, pour économiser de l'espace sur de petits morceaux.
   Pour de grands morceaux, autant compresser */
class BitsSerializer
{
    public:
        /** private **/
        /* consider it private or don't complain */
        smart_ptr< fast_array<char> > bits;
        size_t count;

        /* Ces fonctions n'agrandissent pas la string (optimisation), et donc
           risque de SEGFAULT!!!*/
        /* C'est pourquoi vous devez les considérer comme private!!! */
        void push_bit(bool bit) {bit==0?((*bits)[count/8])&=~(1 << (count%8)):((*bits)[count/8]|=(1 << count%8)); count++;}
        void push_ch(char item);

        /** public **/
        BitsSerializer() : bits (new fast_array<char>() ), count(0){}
        void reset() {count = 0; bits->clear();}

        /* Fonctions sécurisées */
        void push_cc(const char* item, size_t char_len);
        void push_str(const std::string &item);
        void push_l(long item, unsigned char bits_len);

        size_t get_blen() const {return count;}
        size_t get_clen() const {return bits->size;}

        /* A la fin du serialize */
        void get_cdata (char *data, size_t *bits_len=NULL) const {memcpy(data, *bits, get_clen()); if (bits_len) *bits_len = count;}
        const char * get_ccdata(size_t *bits_len=NULL) const {if (bits_len) *bits_len = count; return *bits;}
        smart_ptr< fast_array<char> > get_strdata (size_t *bits_len=NULL) const {if (bits_len) *bits_len = count; return bits;}
};

/* attention
   La durée de vie du const char* passé en paramètre doit être plus longue que celle de la classe! */
class BitsDeserializer
{
    public:
        /** private **/
        const char * bits;
        char count;
        mutable size_t remaining_bitlen;
        char pop_ch();
        bool pop_bit();
        bool check_bitlen(size_t bitlen) const;

        /** public **/
        BitsDeserializer() : count(0), remaining_bitlen(0){}
        BitsDeserializer(const char* item, size_t remaining_bitlen) {init (item, remaining_bitlen);}
        BitsDeserializer(const std::string &item) {init(item);}
        void init(const char * item, size_t remaining_bitlen) {bits = item; count = 0; this->remaining_bitlen = remaining_bitlen;}
        void init(const std::string &item) {bits = item.data(); count=0; this->remaining_bitlen = item.size()*8;}

        bool pop_c(char *item, size_t char_len);
        template <class long_int>
        bool pop_l(long_int &l, unsigned char bits_len);
        bool pop_str(std::string &str, size_t char_len);
};

//Encapsule un Serializer pour le rendre plus pratique
class MegaSerializer
{
    public:
        std::string scheme;
        BitsSerializer slave;

        void push_cc(const char* item, size_t char_len, unsigned char bbl);
        void push_str(const std::string &item, unsigned char bbl);
        void push_l(long item, unsigned char bits_len);
        void mega_push(const char *scheme, ...);

        /* A la fin du serialize */
        void get_cdata (char *data, size_t *bits_len=NULL) const { slave.get_cdata(data, bits_len); }
        const char * get_ccdata(size_t *bits_len=NULL) const { return slave.get_ccdata(bits_len); }
        smart_ptr< fast_array<char> > get_strdata (size_t *bits_len=NULL) const { return slave.get_strdata(bits_len); }
};

class MegaDeserializer
{
    public:
        BitsDeserializer slave;

        MegaDeserializer() {};
        MegaDeserializer(const char* item, size_t remaining_bitlen) : slave (item, remaining_bitlen) {}
        MegaDeserializer(const std::string &item) :slave(item) {}
        void init(const char * item, size_t remaining_bitlen) { slave.init(item, remaining_bitlen); }
        void init(const std::string &item) { slave.init(item); }

        bool pop_c(char *item, unsigned char bbl);
        template <class long_int>
        bool pop_l(long_int &l, unsigned char bbl);
        bool pop_str(std::string &str, unsigned char bbl);
        bool mega_pop(const char *scheme, ...);
};

/* Pour mettre +sieurs fichiers dans un seul */
class FileArchiver
{
    public:
        struct entete
        {
            std::string nom;
            size_t pos;
            size_t len;
            entete(std::string nom, size_t pos, size_t len) : nom(nom), pos(pos), len(len) {}
        };

        std::ofstream out;
        std::list<entete> entetes;
        size_t cur_pos;

        /* Lance un string en exception si impossible à ouvrir, prépare le fichier pour être archivé */
        FileArchiver(const char *path);

        /* ajouter un fichier à une archive en cours, lance un string en exception en cas de pb */
        void add_file(const char *path);
        //ajoute les entêtes à la fin
        void finish();
};

/* ouvre un fichier sauvé avec file_archiver */
/* lance un string en exception en cas de pb */
smart_ptr< fast_array<char> > OpenFile(const char *archive, const char *path);


/////////////////////////////////////////////////////////////////
///                 TPP et INL                                ///
/////////////////////////////////////////////////////////////////

template <class long_int>
bool BitsDeserializer::pop_l(long_int &l, unsigned char bits_len)
{
    if(!check_bitlen(bits_len))
        return false;

    long result = 0;

    if (bits_len % 8 == 0 && count == 0)
    {
        for (bits_len = bits_len - 8; bits_len > 0; bits_len -= 8)
        {
            result |= (*bits) << bits_len;
            bits++;
        }
        result |= (*bits) << bits_len;
        bits++;
    } else
    {
        for (bits_len = bits_len - 1; bits_len > 0; bits_len--)
        {
            result |= pop_bit() << bits_len;
        }
        result |= pop_bit() << bits_len;
    }

    l = result;

    return true;
}

inline bool BitsDeserializer::pop_bit()
{
    bool res =  *bits & (1 << ((count)%8));
    count = (count + 1)%8;
    if (count == 0)
        bits++;
    return res;
}

inline char BitsDeserializer::pop_ch()
{
    //nous ne faisons pas l'optimisation count == 0, elle est déjà faite dans pop_c
    char ch = 0;
    for (int i = 7; i >= 0; i--)
    {
        //il doit surement y avoir une manière plus simple!
        ch |= ((bits[(count+7-i)/8] & (1 << ((count + 7 - i)%8)))!=0) << i;
    }
    bits++;

    return ch;
}

inline bool BitsDeserializer::check_bitlen(size_t bitlen) const
{
    if (remaining_bitlen < bitlen) {
        return false;
    }

    remaining_bitlen -= bitlen;

    return true;
}

inline void BitsSerializer::push_str(const std::string &item)
{
    push_cc(item.data(), item.length());
}

inline bool BitsDeserializer::pop_str(std::string &str, size_t ch_len)
{
    str.resize(ch_len);
    return pop_c(const_cast<char*>(str.data()), ch_len);
}

inline void MegaSerializer::push_cc(const char *item, size_t char_len, unsigned char bbl)
{
    slave.push_l(char_len, bbl);
    slave.push_cc(item, char_len);
    scheme += '%';
    scheme += bbl;
}

inline void MegaSerializer::push_str(const std::string &item, unsigned char bbl)
{
    slave.push_l(item.length(), bbl);
    slave.push_str(item);
    scheme += '$';
    scheme += bbl;
}

inline void MegaSerializer::push_l(long item, unsigned char bbl)
{
    slave.push_l(item, bbl);
    scheme += '#';
    scheme += bbl;
}

inline bool MegaDeserializer::pop_c(char *item, unsigned char bbl)
{
    long l;
    if (!pop_l(l, bbl)) return false;
    return slave.pop_c(item, l);
}

template <class long_int>
inline bool MegaDeserializer::pop_l(long_int &l, unsigned char bbl)
{
    return slave.pop_l(l,bbl);
}

inline bool MegaDeserializer::pop_str(std::string &item, unsigned char bbl)
{
    long l;
    if (!pop_l(l, bbl)) return false;
    return slave.pop_str(item, l);
}

//# = 35, % = 37
#endif // UTILITIES_HH_INCLUDED
