#ifndef KEYINFO_H
#define KEYINFO_H

#include <QObject>

class KeyInfo
{
public:
    explicit KeyInfo();

    void setId(int i) { id = i; }
    int getId() const { return id; }
    void setName(const QString &n) { name = n; }
    const QString &getName() const { return name; }
    void setSite(const QString &s) { site = s; }
    const QString &getSite() const { return site; }
    void setUsername(const QString &u) { username = u; }
    const QString &getUsername() const { return username; }
    void setPassword(const QString &p) { password = p; }
    const QString &getPassword() const { return password; }
    void setNotes(const QString &n) { notes = n; }
    const QString &getNotes() const { return notes; }

    KeyInfo &operator=(const KeyInfo &k);
    void reset();

private:
    int id;
    QString name;
    QString site;
    QString username;
    QString password;
    QString notes;
};

#endif // KEYINFO_H
