/*finger.c reforge*/
#include "finger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utmp.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h> 
#include <stdbool.h> //per usare il tipo bool
#include <sys/stat.h> //uso stat()
#include <ctype.h> //per usare strcasestr()


struct options{
	/*struttura per le opzione del comando finger 0->False 1->True */
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

void printUserInfo(struct UserInfo *userInfo) {

    printf("Utente: %s\n", userInfo->utente);
    printf("Directory: %s\n", userInfo->directory);
    printf("Shell: %s\n", userInfo->shell);
    printf("Nome completo: %s\n", userInfo->nomeCompleto);
    printf("Numero stanza: %s\n", userInfo->numeroStanza);
    printf("Telefono lavoro: %s\n", userInfo->telefonoLavoro);
    printf("Telefono casa: %s\n", userInfo->telefonoCasa);
    printf("TTY: %s\n", userInfo->tty);
    printf("Ultimo accesso: %s\n", userInfo->lastLogin);
    printf("Idle ore: %d\n", userInfo->idleOre);
    printf("Idle minuti: %d\n", userInfo->idleMin);
}

void longFinger(struct UserInfo *user){
	printf("Login    : %-30s     Name      : %s\n",user->utente, user->nomeCompleto);
	printf("Directory: %-30s     Shell     : %s\n",user->directory, user->shell);
	printf("Office   : %s, %-30s Home Phone: %s\n",user->numeroStanza, user->telefonoLavoro, user->telefonoCasa);

	if (user->tty == NULL){
		printf("Never logged in.\n");
	}else{
		printf("On since %.*s on %s from %s\n\t %d hours %d minutes idle\n",
			(int)strcspn(user->lastLogin, "\n"),user->lastLogin, user->tty, user->tty, user->idleOre, user->idleMin);
			//(int)strcspn(user->lastLogin, "\n") serve per rimuovere uno "\n" di troppo 
	}

}

void shortFinger(struct UserInfo *user){
	printf("%-10s %-10s %-5s %d:%-5d %-10.*s %s\n", 
		user->utente, user->nomeCompleto, user->tty,user->idleOre,user->idleMin,(int)strcspn(user->lastLogin,"\n"),user->lastLogin,user->numeroStanza);
}

//gestione degli errori
void getErrors(int errorCode){
	switch(errorCode){
		case 1:
			printf("comando non riconosciuto\nfinger [-lmps] [nomeUtente]\n");
			break;
		case 2:
			printf("Errore nell'apertura del file\n");
			break;
		case 3:
			printf("Errore nell'allocazione della memoria\n");
			break;
		case 4:
			printf("finger: Utente non trovato\n");
			
	}
	exit(EXIT_FAILURE);
	
}

//controllo per la ricerca tralasciando il case sensitive
int strCaseSense(char *gecos, const char *username) {
    char *token;
    
    // Estraiamo il primo token da gecos
    token = strtok(gecos, " ");
    
    // Confrontiamo il nome e il cognome (ignorando il case)
    while(token != NULL) {
        // Se il token corrisponde al nome o al cognome, restituisci 1
        if(strcasecmp(token, username) == 0) {
            return 1;
        }
        
        // Passiamo al prossimo token (cognome)
        token = strtok(NULL, ",");
    }
    
    return 0; // Nessuna corrispondenza trovata
}

//funzione per la gestione delle opzioni
void getOptions(int argc, char *argv[], struct options *opts){


	int numberOfUsers = 0;

	for (int i=1;i<argc;i++){
	
		if (argv[i][0] == '-'){
		
			/*quali opzioni si sta utilizzando?*/
			for (int j=1;argv[i][j]!='\0';j++){
			
				if(argv[i][j]=='l'){
					opts->opt_l = 1;
				}
				if(argv[i][j]=='m'){
					opts->opt_m = 1;
				}
				if(argv[i][j]=='p'){
					opts->opt_p = 1;
				}
				if(argv[i][j]=='s'){
					opts->opt_s = 1;
				}
				if(argv[i][j]!='l' && argv[i][j]!='m' && argv[i][j]!='p' && argv[i][j]!='s' ){
					getErrors(1); //comando non riconosciuto
				}
			}
			
		}
		else{
			numberOfUsers++;
		}
				
	}
}

void checkPlanningFile(char *username,char *filename){

	char *filepath;
	filepath = malloc((strlen("/home/")+strlen(username) + strlen(filename) + 2 ) * sizeof(char));
	sprintf(filepath,"/home/%s/%s",username,filename);

	if (access(filepath,F_OK) != -1) {

		FILE *file = fopen(filepath,"r");
		if (file == NULL){
			printf("Impossibile aprire il file.\n");
			free(filepath);
		}
		printf("%s:\n",filename);
		char c;
		while ((c = fgetc(file)) != EOF){ //EOF indica la fine del file (libreria C)
			putchar(c);
		}
		fclose(file);


	}else{
		printf("No %s\n", filename);
	}

	free(filepath);

}

void mailBox(char *username){

	struct stat file_stat;

	char *filepath = malloc((strlen("/var/mail/") + strlen(username) + 1) * sizeof(char));
	sprintf(filepath, "/var/mail/%s", username);

	// Ottenere le informazioni sul file
    if ( stat(filepath, &file_stat) == 0 ) {
        // Verifica se il file esiste
        if (file_stat.st_size == 0) {
            printf("No mail.\n");
        } else {

        	char last_read_str[30];
        	time_t last_read_time = file_stat.st_atime;
        	strftime(last_read_str, sizeof(last_read_str), "%a %b %d %H:%M:%S %Y", localtime(&last_read_time));
            
            // Verifica se ci sono nuove mail non lette
            if (file_stat.st_atime < file_stat.st_mtime) {

            	char last_recv_str[30];
        		time_t last_recv_time = file_stat.st_mtime;
        		strftime(last_recv_str, sizeof(last_recv_str), "%a %b %d %H:%M:%S %Y", localtime(&last_recv_time));

                printf("New mail received %s\n	Unread since %s\n", last_recv_str, last_read_str);

            } else {
                // Formatta la data di ultima lettura della mailbox
                printf("Mail last read %s\n", last_read_str);
            }
        }
    } else {
        // File non trovato
        printf("No mail.\n");
    }
}

void getUserInfo(const char *username, struct options *opts) {

	struct passwd *pw;
	//struct utmp *ut;
	struct UserInfo *user = malloc(sizeof(struct UserInfo));
	bool userFound = false;

	char *token;

	setpwent();

	while ((pw = getpwent()) != NULL){



		//cerca l'utente nel file passwd (lo cerca anche per nome/cognome se l'opzione m è attiva)

		char gecos[strlen(pw->pw_gecos) + 1];
		strcpy(gecos, pw->pw_gecos);

		if (strcmp(pw->pw_name,username) == 0 || (strCaseSense(gecos,username) == 1 && opts->opt_m == 0)){


			//recupera tutte le informazioni necessarie nel file passwd
			user->utente = pw->pw_name;
			user->directory = pw->pw_dir;
			user->shell = pw->pw_shell;

			token = strtok(pw->pw_gecos,",");

			if (token != NULL){
				user->nomeCompleto = token;
				token = strtok(NULL,",");
			}
			if (token != NULL){
				user->numeroStanza = token;
				token = strtok(NULL,",");
			}
			if (token != NULL){
				user->telefonoLavoro = token;
				token = strtok(NULL,",");
			}
			if (token != NULL){
				user->telefonoCasa = token;
				token = strtok(NULL,",");
			}


			//recupera tutte le informazioni necessarie nel file utmp

			struct utmp *ut;

			time_t current_time = time(NULL); // Ottiene il tempo corrente in secondi
			char aux[UT_NAMESIZE + 1]; // UT_NAMESIZE è definito in <utmp.h> come la lunghezza massima di un nome utente

			setutent(); // Apre il file utmp per la scansione

			
			while ((ut = getutent()) != NULL) { // Scansione del file utmp
			    strncpy(aux, ut->ut_user, UT_NAMESIZE); // Copia il nome utente in aux
			    aux[UT_NAMESIZE] = '\0'; // Assicura che la stringa sia terminata correttamente

			    if (strcmp(aux, username) == 0) { // Se il nome utente corrente corrisponde a quello cercato
			        user->tty = ut->ut_line; // Assegna il terminale associato all'utente
			        time_t ltime = ut->ut_time; // Ottiene il tempo dell'ultimo accesso dell'utente

			        user->lastLogin = ctime(&ltime); // Converte il tempo dell'ultimo accesso in una stringa leggibile
			        time_t idle_seconds = current_time - ut->ut_time; // Calcola il tempo trascorso dall'ultimo accesso in secondi
			        int idle_hours = idle_seconds / 3600; // Calcola il numero di ore di inattività
			        int idle_minutes = (idle_seconds % 3600) / 60; // Calcola il numero di minuti di inattività
			        user->idleOre = idle_hours; // Assegna il numero di ore di inattività all'utente
			        user->idleMin = idle_minutes; // Assegna il numero di minuti di inattività all'utente

			        break; // Poiché abbiamo trovato il nome utente cercato, possiamo uscire dal ciclo
			    }
			}

			endutent(); // Chiude il file utmp dopo la scansione


			if (opts->opt_l == 1){
				longFinger(user);
			}
			else if (opts->opt_s == 1){
				shortFinger(user);
			}
			else{
				longFinger(user);
			}
			
			if (opts->opt_s == 0){
				mailBox(user->utente);
				if (opts->opt_p == 0) {
					checkPlanningFile(user->utente,".plan");
					checkPlanningFile(user->utente,".project");
					checkPlanningFile(user->utente,".pgpkey");
				}

				printf("\n");
			}

			userFound = true;

		}

	}
	endpwent();
	
	if(!userFound){		
		printf("L'utenete %s non è stato trovato\n", username);
	}
	free(user);
}

int main(int argc, char *argv[]) {

	/*0=false, 1=true*/
	struct options opts;
	opts.opt_l = 0;
	opts.opt_m = 0;
	opts.opt_p = 0;
	opts.opt_s = 0;
	

	//funzione per gestire le opzioni passata negli argomenti & contare il numero di utenti
	getOptions(argc, argv, &opts);
	
	//mostra opzioni selezionata
	//printf("Options:\nl: %d\nm: %d\np: %d\ns: %d\n", opts.opt_l,opts.opt_m,opts.opt_p,opts.opt_s);
	


	//for per prende ogni nome utente dagli argomenti	
	for (int i=1;i<argc;i++){

		if (argv[i][0] != '-'){
			getUserInfo(argv[i], &opts);
		}
	}


	return 0;
}
