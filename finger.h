#ifndef FINGERDIY_H
#define FINGERDIY_H


struct options;
struct UserInfo;
int check_write_permission_tty(const char *path);
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
time_t get_idle_time(const char *tty);

#endif /* FINGERDIY_H */

