/*finger.c reforge*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <utmp.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h> 
#include <stdbool.h>
#include <sys/stat.h> 

#include "fingerDIY.h"

int headerShortPrint = 0; // 0 -> no stampata, 1 -> stampata

/*Print versione lunga (-l)*/
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
/*print versione corta (-s)*/
void shortFinger(struct UserInfo *user){

	if (headerShortPrint == 0){
		// Stampa header per l'output ridotto
		headerShortPrint = 1;
		printf("Login\t Name\t\t Tty\t idle\t Login Time\t\t Office Phone\t Office\n");
	} 

	printf("%s\t", user->utente);
	printf("%s\t", user->nomeCompleto);
	if (user->tty == NULL){
		printf("*\t");
		printf("*\t");
		printf("No Logins\t");
	}
	else{
		printf("%s\t", user->tty);	
		printf("%d:%d\t", user->idleOre,user->idleMin);
		printf("%.*s\t", (int)strcspn(user->lastLogin, "\n"),user->lastLogin);
	}	
	printf("%s ", user->telefonoLavoro);
	printf("%s\n", user->numeroStanza);
}


/*formatta il numero di telefono come descritto nella documentazione del comando finger*/
char* phoneFormat(char *number){

	int len = strlen(number); // ottiene la lunghezza del numero di telefono 

	// controlla se la stringa contiene solo caratteri numerici
	for (int i=0; number[i] != '\0';i++){
		if (number[i] < '0' || number[i] > '9'){
			return number;
		}
	}

	 //buffer per il numero di telefono
	char pbuf[15];
	memset(pbuf, '\0', sizeof(pbuf));
	char *p = pbuf;


	// switch sulla lunghezza del numero di telefono
	switch (len) {
		case 11:
			*p++ = '+'; // Aggiunge il prefisso internazionale
			*p++ = *number++; // Aggiunge il primo numero
			*p++ = '-'; // Aggiunge il trattino
		case 10:
			*p++ = *number++; // Aggiunge i primi tre numeri
			*p++ = *number++;
			*p++ = *number++;
			*p++ = '-'; // Aggiunge il trattino
		case 7:
			*p++ = *number++; // Aggiunge i successivi tre numeri
			*p++ = *number++;
			*p++ = *number++;
			break;
		case 5:
		case 4:
			*p++ = 'x'; // Aggiunge 'x'
			*p++ = *number++; // Aggiunge il primo numero
			break;
		default:
			return number; // Ritorna l'input originale se la lunghezza non è gestita

	}
	// Se la lunghezza non è 4, aggiunge un trattino e altri tre numeri
	if (len != 4) {
		*p++ = '-';
		*p++ = *number++;
	}
	*p++ = *number++;
	*p++ = *number++;
	*p++ = *number++;
	*p = '\0'; // Termina la stringa con il carattere nullo

	// Ritorna una copia della stringa formattata
	return strdup(pbuf);

}

//andrà alla ricerca delle opzione (argomenti che iniziano con il carattere '-')
int getOptions(int argc, char *argv[], struct options *opts){


	int numberOfUsers = 0; // contatore degli utenti

	// For che attraversa tutti gli argomenti della linea di comando
	for (int i=1;i<argc;i++){
		
		// Controlla se l'argomento è un opzione (inizia con '-')
		if (argv[i][0] == '-'){
		
			// for per indentificare se l'opzione richiesta
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
					// se l'opzione non è riconosciuta, stampa un messaggio di errore
					printf("comando non riconosciuto\nfinger [-lmps] [nomeUtente]\n");
					exit(1);
				}
			}
			
		}
		else{
			// Conta il numero di utenti
			numberOfUsers++;
		}
				
	}
	// ritorna il numero di utenti trovati negli argomenti
	return numberOfUsers;
}


// Confronta il nome e il cognome con un username ignorando la differenza tra maiuscole e minuscole
int strCaseSense(const char *gecosOriginal, const char *username) {
	// Duplica la stringa gecosOriginal per modificarla
	char *gecos = strdup(gecosOriginal);
	char *nome = NULL;
	char *cognome = NULL;

	//prende il primo campo della stringa gecos (ovvero il nome e il cognome)
    char *token = strtok(gecos, ",");
	if (token == NULL){
		free(gecos); //libera la memoria allocata prima di uscire
	    return 0;
	}

	// Trova la posizione dello spazio nella stringa token
    char *space_pos = strchr(token, ' ');
    if (space_pos != NULL) {
        // Se c'è uno spazio, separa nome e cognome
        *space_pos = '\0';
        nome = token;
        cognome = space_pos + 1;
    } else {
        // Se non c'è spazio, hai solo il nome o il cognome
        nome = token;
        cognome = token;
    }

    //se c'è corrispondenza ritorna 1
    if (strcasecmp(nome,username) == 0 || strcasecmp(cognome,username) == 0) {
    	free(gecos);
    	return 1;
    }

	// Nessuna corrispondenza trovata, ritorna 0
    free(gecos);    
    return 0;
}

// Funzione per controllare e leggere il file di pianificazione dell'utente
void checkPlanningFile(char *username,char *filename){

	// Alloca memoria per il percorso completo del file
	char *filepath = malloc((strlen("/home/")+strlen(username) + strlen(filename) + 2 ) * sizeof(char));	
	if (filepath == NULL){
		perror("[-] malloc:checkPlanningFile()\n");
		exit(1);
	}
	// Costruisce il percorso del file
	sprintf(filepath,"/home/%s/%s",username,filename);

	// Controlla se il file esiste
	if (access(filepath,F_OK) != -1) {
		// Prova ad aprire il file in lettura
		FILE *file = fopen(filepath,"r");
		if (file == NULL){
			// Gestisce l'errore di apertura del file
			printf("[-] Impossibile aprire il file.\n");
			free(filepath); // Libera la memoria allocata
		}
		printf("%s:\n",filename); // Stampa il nome del file
		
		// Legge e stampa il contenuto del file carattere per carattere
		char c;
		while ((c = fgetc(file)) != EOF){
			putchar(c);
		}
		fclose(file);
	}else{
		// Il file non esiste
		printf("No %s\n", filename);
	}

	free(filepath); //libera memoria allocata

}

// Funzione per verificare la casella di posta dell'utente
void mailBox(char *username){

	struct stat file_stat;

	// Alloca memoria per il percorso del file di posta
	char *filepath = malloc((strlen("/var/mail/") + strlen(username) + 1) * sizeof(char));
	if (filepath == NULL){
		perror("[-] malloc:mailBox()\n");
		exit(1);
	}

	// Construisce il percorso del file di posta
	sprintf(filepath, "/var/mail/%s", username);

	/* Ottenere le informazioni sul file.

		Se la data di ultima lettura (st_atime) è precedente alla data di ultima modifica (st_mtime), viene stampato un messaggio indicante che ci sono nuove mail non lette.
		Altrimenti, viene stampata la data di ultima lettura della casella di posta.
	*/
    if ( stat(filepath, &file_stat) == 0 ) {
        // Verifica se il file esiste
        if (file_stat.st_size == 0) {
            printf("No mail.\n");
        } else {
        	// Formatta la data di ultima lettura della mailbox
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
                // data di ultima lettura della mailbox
                printf("Mail last read %s\n", last_read_str);
            }
        }
    } else {
        // File non trovato
        printf("No mail.\n");
    }

    free(filepath);
}
/*Inizializza tutti i campi della struttura UserInfo a NULL o a valori vuoti*/
void initializeUserInfo(struct UserInfo *user) {
    user->utente = NULL;
    user->directory = NULL;
    user->shell = NULL;
    user->nomeCompleto = NULL;
    user->numeroStanza = NULL;
    user->telefonoLavoro = NULL;
    user->telefonoCasa = NULL;
    user->tty = NULL;
    user->lastLogin = NULL;
    user->idleOre = 0;
    user->idleMin = 0;
}

/*
	funzione per eseguire il comando finger do-it-yourself
	prende come parametri la lunghezza del vettore degli argomenti, il vettore degli argomenti e la struct
	con le opzioni selezionate
*/
void fingerDIY(int argc, char *argv[], struct options *opts){
	struct passwd *pw;

	// Apre il file passwd per la scansione
	setpwent();

	// Ciclo per esplorare tutte le entry del file passwd
	while ((pw = getpwent()) != NULL){
		
		struct UserInfo *user = malloc(sizeof(struct UserInfo));
		if (user == NULL){
			perror("[-] malloc:fingerDIY()\n");
			exit(1);
		}

		// Inizializza tutti i campi della struttura UserInfo a NULL o a valori vuoti
        initializeUserInfo(user);

		bool userFound = false;
		
		char gecos[strlen(pw->pw_gecos) + 1];
		strcpy(gecos, pw->pw_gecos);


		// Ciclo per scorrere il vettore degli argomenti
		for (int i=0;i<argc && userFound == false;i++){
			
			// Verifica se il nome utente (o nome/cognome se l'opzione -m è 0) è tra gli argomenti
			if (argv[i][0] != '-' && ((strcmp(pw->pw_name, argv[i]) == 0) || (strCaseSense(gecos, argv[i]) == 1 && opts->opt_m == 0))){	
				userFound = true; //se è entrato nella if allora abbiamo trovato un utente
 			}

		}

		// Recupera tutte le informazioni necessarie dal file passwd se l'utente è stato trovato
		if (userFound) {
			
			user->utente = pw->pw_name;
			user->directory = pw->pw_dir;
			user->shell = pw->pw_shell;

			// Tokenizza il campo gecos per estrarre nome, stanza, telefoni
			char *token;
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
				user->telefonoLavoro = phoneFormat(token);
				token = strtok(NULL,",");
			}
			if (token != NULL){
				user->telefonoCasa = phoneFormat(token);
				token = strtok(NULL,",");
			}

			//Recupera tutte le informazioni necessarie nel file utmp
			struct utmp *ut;
			time_t current_time = time(NULL); // Ottiene il tempo corrente in secondi
			char aux[UT_NAMESIZE + 1]; // UT_NAMESIZE è definito in <utmp.h> come la lunghezza massima di un nome utente

			setutent(); // Apre il file utmp per la scansione
			
			while ((ut = getutent()) != NULL) { // Scansione del file utmp

			    strncpy(aux, ut->ut_user, UT_NAMESIZE); // Copia il nome utente in aux
			    aux[UT_NAMESIZE] = '\0'; // Assicura che la stringa sia terminata correttamente

			    if (strcmp(aux, pw->pw_name) == 0) { // Se il nome utente corrente corrisponde a quello cercato
			        user->tty = ut->ut_line; // Assegna il terminale associato all'utente
			        time_t ltime = ut->ut_time; // Ottiene il tempo dell'ultimo accesso dell'utente
			        user->lastLogin = ctime(&ltime); // Converte il tempo dell'ultimo accesso in una stringa leggibile
			        
			        // Calcola il tempo di attivatà dell'utente
			        time_t idle_seconds = current_time - ut->ut_time; // Calcola il tempo trascorso dall'ultimo accesso in secondi
			        user->idleOre = idle_seconds / 3600; // Calcola il numero di ore di inattività
			        user->idleMin = (idle_seconds % 3600) / 60; // Calcola il numero di minuti di inattività

			        break; // Poiché abbiamo trovato il nome utente cercato, possiamo uscire dal ciclo
			    }
			}

			endutent(); // Chiude il file utmp dopo la scansione

			// Determina quale funzione chiamare in base alle opzioni specificate
			if (opts->opt_l == 1){
				longFinger(user);
			}
			else if (opts->opt_s == 1){
				shortFinger(user);
			}
			else{
				longFinger(user); // Chiamata di default
			}
			
			// Esegue ulteriori controlli solo se opt_s è 0
			if (opts->opt_s == 0){
				mailBox(user->utente); // Verifica la casella di posta dell'utente
				if (opts->opt_p == 0) {
					// Verifica la presenza di file di pianificazione specifici
					checkPlanningFile(user->utente,".plan");
					checkPlanningFile(user->utente,".project");
					checkPlanningFile(user->utente,".pgpkey");
				}

				printf("\n");
			}
		}

		free(user); //libera memoria
		//fine while	
	}
	endpwent(); // Chiude il file passwd dopo la scansione
}

// Funzione per ottenere gli utenti attualmente loggati
char** get_logged_in_users(int *user_count) {
    struct utmp *n;
    setutent(); // Apre il file utmp e imposta la posizione all'inizio
    n = getutent(); //Legge la prima voce dal file utmp

    char **users = NULL;
    int count = 0;

    while (n) {
    	// Controlla se l'utente è in uno stato di processo utente
        if (n->ut_type == USER_PROCESS) {
            users = realloc(users, sizeof(char*) * (count + 1));// Rialloca la memoria per inserire un altro utente
            users[count] = strndup(n->ut_user, UT_NAMESIZE); // Alloca e copia il nome utente
            count++; // Incrementa il contatore degli utenti trovati
        }
        n = getutent(); // Ottiene la prossima entry nel file utmp
    }
    endutent(); // Chiude il file utmp

    *user_count = count; // Assegna al puntatore user_count il numero totale di utenti trovati
    return users; // Restituisce il gli utenti loggati nel sistema
}

int main(int argc, char *argv[]) {

	/*0=false, 1=true*/
	struct options opts;
	opts.opt_l = 0;
	opts.opt_m = 0;
	opts.opt_p = 0;
	opts.opt_s = 0;
	

	//funzione per gestire le opzioni passata negli argomenti & contare il numero di utenti
	if(getOptions(argc, argv, &opts) != 0){
		
		fingerDIY(argc-1, argv+1, &opts);
	}
	else{

		int userLoggedIn = 0;
		char **new_argv = get_logged_in_users(&userLoggedIn);

		fingerDIY(userLoggedIn, new_argv, &opts);
	}
	
	//mostra opzioni selezionata
	//printf("Options:\nl: %d\nm: %d\np: %d\ns: %d\n", opts.opt_l,opts.opt_m,opts.opt_p,opts.opt_s);

	return 0;
}
