#ifndef FINGERDIY_H
#define FINGERDIY_H

struct options {
    _Bool opt_l;
    _Bool opt_m;
    _Bool opt_p;
    _Bool opt_s;
};

struct UserInfo {
    char *utente;
    char *directory;
    char *shell;
    char *nomeCompleto;
    char *numeroStanza;
    char *telefonoLavoro;
    char *telefonoCasa;
    char *tty;
    char *lastLogin;
    int idleOre;
    int idleMin;
};

void longFinger(struct UserInfo *user);
void shortFinger(struct UserInfo *user);
char* phoneFormat(char *number);
int getOptions(int argc, char *argv[], struct options *opts);
int strCaseSense(const char *gecosOriginal, const char *username);
void checkPlanningFile(char *username, char *filename);
void mailBox(char *username);
void initializeUserInfo(struct UserInfo *user);
void fingerDIY(int argc, char *argv[], struct options *opts);
char** get_logged_in_users(int *user_count);

#endif /* FINGERDIY_H */

