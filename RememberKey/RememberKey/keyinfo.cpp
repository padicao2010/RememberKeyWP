#include "keyinfo.h"

KeyInfo::KeyInfo():
    id(-1), name(""), site(""), username(""), password(""), notes("")
{

}

KeyInfo &KeyInfo::operator=(const KeyInfo &k)
{
    this->id = k.id;
    this->name = k.name;
    this->site = k.site;
    this->username = k.username;
    this->password = k.password;
    this->notes = k.notes;
    return *this;
}

void KeyInfo::reset()
{
    id = -1;
    name = "";
    site = "";
    username = "";
    password = "";
    notes = "";
}

