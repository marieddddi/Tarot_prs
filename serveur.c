#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "fonctions.h"
#include <pthread.h>

#define MSG_KEY 1234
#define MAX_CLIENTS 4
#define MSG_SIZE 1024
#define SHM_KEY 5678

//structure du message utilisé pour la boite aux lettres
struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

//paquets
struct paquet chien, joueur1, joueur2, joueur3, joueur4, paquet_preneur, paquet_adversaires;
struct paquet *joueurs[] = {&joueur1, &joueur2, &joueur3, &joueur4};
char *contrat_final = 0;

//score des joueurs sous forme de tableau
float scoreJoueurs[] = {0.0, 0.0, 0.0, 0.0};

//mutex du score
pthread_mutex_t mutex_scores = PTHREAD_MUTEX_INITIALIZER;

//fonction pour envoyer un message à un joueur
void envoyer_un_message(int msgid, int joueur, char *contenuMessage) {
    struct msg_buffer message;

    message.msg_type = joueur;
    strcpy(message.msg_text, contenuMessage);
    if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
        perror("Erreur lors de l'envoi du message au joueur");
        exit(EXIT_FAILURE);
    }
}

//fonction permettant d'envoyer un message à tous les clients
void envoyer_message(int msgid, char *message) {
    for (int i = 1; i < MAX_CLIENTS+1; i++) {
        envoyer_un_message(msgid,i,message);    
    }
}

//fonction permettant de recevoir un message de tous les joueurs indiquant qu'ils veulent jouer
//cette fonction lance aussi 4 terminaux pour les 4 joueurs
void attendre_clients(int msgid) {

    const char *command = "gnome-terminal";
    const char *client_command = "./client";

    // Boucle pour créer 4 terminaux avec les arguments 1 à 4
    for (int i = 1; i <= 4; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Processus enfant
            char arg[2];
            snprintf(arg, sizeof(arg), "%d", i); // Convertit i en chaîne
            char *args[] = {(char *)command, "--", (char *)client_command, arg, NULL};
            execvp(command, args);
            perror("execvp failed"); // S'affiche en cas d'échec
            exit(EXIT_FAILURE); // Quitte si execvp échoue
        } else if (pid < 0) { // En cas d'erreur de fork
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

    }

    int client_pret = 0;
    struct msg_buffer message;
    //tant qu'on n'a pas reçu le message de tous les clients, ce message a 5 comme type
    while (client_pret < MAX_CLIENTS) {
        if (msgrcv(msgid, &message, MSG_SIZE, 5, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        if (strcmp(message.msg_text, "jouer") == 0) {
            client_pret++;
        }
    }
}


// Fonction permettant d'envoyer le jeu au client choisi
void envoyer_jeu(int msgid, struct paquet *paquet, int preneur, int param) {
    struct msg_buffer message;
   

    // Couleurs ANSI pour les cartes
    const char *rouge = "\033[31m"; // Rouge pour ♥ et ♦
    const char *reset = "\033[0m";  // Réinitialisation des couleurs

    const char *symbole;
    const char *couleur;

    char buffer[1024] = "";  // Buffer
    char temp[100];
    if (param == 0) {
        for (int i = 0; i < paquet->nb_cartes; i++) {
            //ON associe les couleurs et les symboles
            switch (paquet->jeu[i].couleur) {
                case 'C': symbole = "♥"; couleur = rouge; break;
                case 'K': symbole = "♦"; couleur = rouge; break;
                case 'T': symbole = "♣"; couleur = reset; break;
                case 'P': symbole = "♠"; couleur = reset; break;
                default: symbole = " "; couleur = reset; break;
            }

            snprintf(temp, sizeof(temp), "%2d: %s%-2s%s %s%s  \n", i + 1, couleur, paquet->jeu[i].valeur, reset, symbole, reset);
            strncat(buffer, temp, sizeof(buffer) - strlen(buffer) - 1);
        }
    } else {
        for (int i = 0; i < paquet->nb_cartes; i++) {
            //ON associe les couleurs et les symboles
            switch (paquet->jeu[i].couleur) {
                case 'C': symbole = "♥"; couleur = rouge; break;
                case 'K': symbole = "♦"; couleur = rouge; break;
                case 'T': symbole = "♣"; couleur = reset; break;
                case 'P': symbole = "♠"; couleur = reset; break;
                default: symbole = " "; couleur = reset; break;
            }
            // Calcule le joueur recevant cette carte (ordre circulaire)
            int joueur_actuel = ((param - 1 + i) % MAX_CLIENTS) + 1;
            snprintf(temp, sizeof(temp), "%2d: %s%-2s%s %s%s (joueur %d) \n",i+1, couleur, paquet->jeu[i].valeur, reset, symbole, reset, joueur_actuel);
            strncat(buffer, temp, sizeof(buffer) - strlen(buffer) - 1);
        
        }
    }

    // Vérifier si le buffer dépasse la taille
    if (strlen(buffer) >= sizeof(message.msg_text)) {
        fprintf(stderr, "Erreur : message trop grand pour être envoyé.\n");
        exit(EXIT_FAILURE);
    }

    // Préparer et envoyer le message
    message.msg_type = preneur;
    strncpy(message.msg_text, buffer, sizeof(message.msg_text) - 1);
    message.msg_text[sizeof(message.msg_text) - 1] = '\0';

    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi du jeu");
        exit(EXIT_FAILURE);
    }
}


    

//fonction envoyant un paquet à tous les joueurs 
void envoyer_jeu_joueurs (int msgid, struct paquet *paquet, int param) {
    //on envoie le message à chaque joueur
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Calcule l'index du joueur suivant de manière circulaire
        envoyer_jeu(msgid, paquet, i+1, param);
    }
    printf("Paquet envoyé à tous les joueurs.\n");
}

//fonction permettant de distribuer les cartes
void distribuer_cartes_aux_clients(int msgid, struct paquet *jeu) {
    //on distribue le jeu dans les paquets de chaque joueur et du chien. 
    distribuer_cartes(jeu, &joueur1, &joueur2, &joueur3, &joueur4, &chien);

    struct msg_buffer message;

    //on envoie le paquet de chaque joueur à chaque client
    //le paquet est sous forme d'un texte , donc on le convertit en chaine de caractères
    for (int i = 0; i < MAX_CLIENTS; i++) {
        envoyer_jeu(msgid,joueurs[i],i+1,0);

        if (msgrcv(msgid, &message, MSG_SIZE, i+10 + 1, 0) == -1) {
            perror("Erreur lors de la réception de la confirmation de réception des cartes");
            exit(EXIT_FAILURE);
        }
    }
}

//fonction permettant de connaitre le niveau du contrat choisi
void niveau_contrat(char *choix_contrat, char *contrat_precedent, char *resultat) {
    memset(resultat, 0, 100); //initialisation de la chaine de caractères

    float niveau = contrat(choix_contrat);

    if (niveau == 0) {
        strncpy(resultat, contrat_precedent, 99);
    } else if (niveau == 1) {
        strncpy(resultat, "Passe, Garde", 99);
    } else {
        strncpy(resultat, "Passe", 99);
    }

    resultat[99] = '\0';
    printf("Contrats possibles: %s\n", resultat);
}

//fonction demandant au joueur de choisir son contrat
int demande_contrat(int msgid, int ordre_joueurs[], int nb_joueurs) {
    char contrats_joueurs[MAX_CLIENTS][100]; 
    char nouveaux_contrats[100];
    int joueur;
    int preneur = 0;
    int val_en_cours;
    int niveau_max = 0;
    struct msg_buffer message;
    struct msg_buffer message_reponse;
    char contrats_possibles[100] = "Passe, Petite, Garde";

    memset(contrats_joueurs, 0, sizeof(contrats_joueurs)); //initialisation de la chaine de caractères
    memset(&message_reponse, 0, sizeof(message_reponse)); //initialisation de la structure
    memset(&message, 0, sizeof(message)); //initialisation de la structure
    //Au départ, le joueur a tous les contrats possibles
    strcpy(message_reponse.msg_text, "Passe");

    //choix du contrat pour chaque joueur. On commence par le premier joueur
    for (int i = 0; i < nb_joueurs; i++) {
        joueur = ordre_joueurs[i];
        //char nouveaux_contrats[100];
        //on etablit le niveau du contrat choisi. Au départ, c'est le contrat passe => le plus faible niveau
        niveau_contrat(message_reponse.msg_text, contrats_possibles, nouveaux_contrats);
        //on met à jour la chaine de caractères contenant les contrats possibles
        strncpy(contrats_possibles, nouveaux_contrats, 99);
        contrats_possibles[99] = '\0';
       
       //on prepare le message envoyant les contrats possibles aux clients
        envoyer_un_message(msgid,joueur,contrats_possibles);
        printf("Demande envoyée au joueur %d avec msg_type = %d.\n", joueur, joueur);

        memset(&message_reponse, 0, sizeof(message_reponse));

        if (msgrcv(msgid, &message_reponse, MSG_SIZE, joueur+10, 0) == -1) {
            perror("Erreur lors de la réception de la réponse du contrat");
            break;
        }
        printf ("Réponse du joueur %d : %s\n", joueur, message_reponse.msg_text);

        //on met à jour la chaine de caractères contenant le contrat choisi par le joueur
        strncpy(contrats_joueurs[joueur], message_reponse.msg_text, 99);
        contrats_joueurs[joueur][99] = '\0';
    }

    //on determine le niveau de contrat le plus fort cad celui qui prend
    for (int i = 1; i < nb_joueurs + 1; i++) {
        val_en_cours = contrat(contrats_joueurs[i]);
        if (val_en_cours > niveau_max) {
            preneur = i;
            niveau_max = val_en_cours;
        }
    }

    //si on n'a pas de preneur, on redistribue les cartes et on redemande les contrats
    if (preneur == 0) {
        printf("Aucun preneur trouvé.\n");
    } else {
        printf("Le preneur est le joueur %d avec un contrat de niveau %d\n", preneur, niveau_max);
        //on stock le contrat 
        contrat_final = malloc(strlen(contrats_joueurs[preneur]) + 1);
        //on stock le contrat dans contrat_final
        strcpy(contrat_final, contrats_joueurs[preneur]);
    }

    //on envoie à chaque joueur celui qui prend. Si c'est 0, le client devra rechoisir un contrat avec ses nouvelles cartes
    for (int i = 1; i < nb_joueurs + 1; i++) {
        memset(&message, 0, sizeof(message));
        message.msg_type = i;
        snprintf(message.msg_text, sizeof(message.msg_text), "%d", preneur);
        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
            perror("Erreur lors de l'envoi du preneur");
            exit(EXIT_FAILURE);
        }
    }
    return preneur;
}


//fonction permettant d'envoyer le jeu du preneur avec le chien en plus
struct paquet *envoyer_jeu_avec_chien(int msgid, int preneur, struct paquet *chien, struct paquet *paquet) {

    // Ajout des cartes du chien au paquet
    for (int i = 0; i < chien->nb_cartes; i++) {
        paquet->jeu[paquet->nb_cartes] = chien->jeu[i];
        paquet->nb_cartes++;
    }

    // envoie du paquet contenant ses 18 cartes et les 6 cartes du chien
    envoyer_jeu(msgid,paquet,preneur,0);
    
    printf("\nPaquet envoyé au preneur, avec %d cartes.\n\n", paquet->nb_cartes);

    return paquet;
}

//fonction retirant une carte d'un paquet
void retirer_carte(struct paquet *paquet, int index) {
    // Supprimer la carte du paquet
    //si c'est l'excuse, on la met directement dans le paquet du joueur 

    for (int j = index; j < paquet->nb_cartes-1; j++) { // On boucle sur les cartes restantes
        paquet->jeu[j] = paquet->jeu[j+1];
    }
    paquet->nb_cartes--;
}

//fonction permettant au preneur de faire son chien cad d'enlever 6 cartes selon les règles établies (voir CR)
void faire_chien(int msgid, struct paquet *chien, int preneur, struct paquet *paquet) {
    struct msg_buffer message;
    bool non_valide=true;
    int index = 0;
    //on stock le nouveau jeu fait avec afficher chien
    envoyer_jeu_avec_chien (msgid, preneur, chien, paquet);

    //le preneur doit enlever 6 cartes de son paquet qui en contient maintenant 24 avec les 6 du chien
    for (int i = 0; i < 6; i++) {
        //tant que la carte n'est pas valide on re demande
        while(non_valide){
            // Réception des indices des cartes que le preneur met dans le chien
            if (msgrcv(msgid, &message, MSG_SIZE, preneur+10, 0) == -1) {
                perror("Erreur lors de la réception des indices des cartes");
                exit(EXIT_FAILURE);
            }
            printf ("\nIndices de la carte : %s\n", message.msg_text);

            index = atoi(message.msg_text);
            index = index -1;
            printf("Le paquet contient maintenant %d cartes \n", paquet->nb_cartes);

            // Afficher la carte correspondante
            printf("Carte choisie : %c %s\n\n",
                paquet->jeu[index].couleur,
                paquet->jeu[index].valeur);

                //si c'est un atout, on refuse
            if (est_atout(&paquet->jeu[index])) {
                printf("\nC'est un atout, impossible !\n");

                //on envoie que c'est un atout 
                envoyer_un_message(msgid,preneur,"atout");
            }
            else{
                //on envoie que c'est bon 
                envoyer_un_message(msgid,preneur,"bon");
                non_valide = false;
            } 
        }

        // On ajoute la carte au paquet_preneur
        paquet_preneur.jeu[paquet_preneur.nb_cartes] = paquet->jeu[index];
        paquet_preneur.nb_cartes++;

        //On supprime la carte du paquet
        retirer_carte(paquet,index);

        printf("\nCarte ajoutée au chien. Le paquet preneur contient maintenant %d cartes.\n",
               paquet_preneur.nb_cartes);
        printf("Le paquet contient maintenant %d cartes.\n\n", paquet->nb_cartes);
        //on envoie le nouveau jeu 
        envoyer_jeu (msgid, paquet, preneur,0);
        non_valide = true;
    }

    //on affiche le contenu dans paquet_preneur 
    printf("\nContenu du paquet preneur:\n");
    afficher_paquet(&paquet_preneur);
}

//fonction ajoutant une carte au paquet
void ajouter_carte(struct paquet *paquetAjout, struct carte *carteAjoutee) {
    // Ajouter la carte au paquet
    paquetAjout->jeu[paquetAjout->nb_cartes] = *carteAjoutee;
    paquetAjout->nb_cartes++;
}

// Fonction pour mettre à jour l'ordre des joueurs
void mettreAJourOrdreJoueurs(int ordre_joueurs[], int JoueurQuiPrend) {
    int nouvel_ordre[MAX_CLIENTS];

    // Trouver la position actuelle du joueur qui prend
    int position = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (ordre_joueurs[i] == JoueurQuiPrend) {
            position = i;
            break;
        }
    }

    if (position == -1) {
        printf("Erreur : JoueurQuiPrend non trouvé dans l'ordre actuel\n");
        return;
    }

    // Remplir le nouvel ordre à partir de la position trouvée
    for (int i = 0; i < MAX_CLIENTS; i++) {
        nouvel_ordre[i] = ordre_joueurs[(position + i) % MAX_CLIENTS];
    }

    // Copier le nouvel ordre dans ordre_joueurs
    for (int i = 0; i < MAX_CLIENTS; i++) {
        ordre_joueurs[i] = nouvel_ordre[i];
    }
}

//fonction permettant de jouer une partie
void jouer_un_tour(int msgid, struct paquet *paquet_adversaires, struct paquet *paquet_preneur, int preneur, int ordre[MAX_CLIENTS]) {
    struct msg_buffer message;
    char *aToi = "a toi";
    int index = 0;
    bool carte_valide;
    int joueur;
    char couleurJouee;
    int joueurQuiPrendEnsuite ;
    int premierJoueur;
    int joueurDepart;

    while (joueur1.nb_cartes>0){
        carte_valide = false; //au depart la carte n'est pas valide
        struct carte carteLaPlusForte = { 0, {0,0}, 0.0 }; //la carte la plus forte est une carte à 0 car personne n'a joué
        couleurJouee = ' ';
        joueurQuiPrendEnsuite = 0;
        premierJoueur = 0;
        struct paquet paquet_en_cours = {0};

        //on demande a chaque joueur de jouer une carte
        for (int i = 0; i < MAX_CLIENTS; i++) {
            joueur = ordre[i];
            joueurDepart = ordre[0];
            printf("\nTour du joueur %d\n\n", joueur);

            //on envoie au joueur que c'est à lui de jouer
            envoyer_un_message(msgid,joueur,aToi);

            //le joueur doit envoyer qu'il est prêt à jouer une carte
            memset (&message, 0, sizeof(message));
            if (msgrcv(msgid, &message, sizeof(message.msg_text), joueur+10, 0) == -1) {
                perror("Erreur lors de la réception de l'accusé de réception du joueur");
                exit(EXIT_FAILURE);
            }
            if (strcmp(message.msg_text, "pret") != 0) {
                printf ("mess: %s\n", message.msg_text);
                fprintf(stderr, "Le joueur %d n'a pas confirmé qu'il est prêt.\n", joueur);
                exit(EXIT_FAILURE);
            }

            //on envoie le jeu du joueur pour qu'il puisse choisir sa carte
            envoyer_jeu(msgid, joueurs[joueur-1], joueur,0);

            while (!carte_valide) {
                if (msgrcv(msgid, &message, MSG_SIZE, joueur+10, 0) == -1) {
                    perror("Erreur lors de la réception de la carte");
                    exit(EXIT_FAILURE);
                }
                index = atoi(message.msg_text)-1;

                //on vérifie si la carte est valide
                carte_valide = accepter_carte(&carteLaPlusForte, &joueurs[joueur - 1]->jeu[index], joueurs[joueur - 1], couleurJouee);
                strcpy(message.msg_text, carte_valide ? "valide" : "non_valide");
                message.msg_type = joueur;
                if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
                    perror("Erreur lors de l'envoi de la validation");
                    exit(EXIT_FAILURE);
                }
            }

            //si la carte est valide, on l'ajoute au paquet en cours
            struct carte carte_jouee = joueurs[joueur - 1]->jeu[index];
            ajouter_carte(&paquet_en_cours, &carte_jouee);

            //on affiche le paquet en cours 
            printf("\nPaquet en cours: \n");
            afficher_paquet (&paquet_en_cours);
            printf("\n\n");

            //on retire la carte du paquet du joueur
            retirer_carte(joueurs[joueur - 1], index);

            //Si le joueur a joué l'excuse, on l'ajoute dans son paquet et cette carte n'est pas la plus forte d'office
            if (strcmp(carte_jouee.valeur, "*")==0 && joueur != preneur) {
                ajouter_carte(paquet_adversaires, &carte_jouee);
            } 
            if (strcmp(carte_jouee.valeur,"*")==0 && joueur == preneur) {
                ajouter_carte(paquet_preneur, &carte_jouee);
            }

            //on regarde quelle carte est la plus forte
            if (qui_a_la_plus_forte_carte(&carteLaPlusForte, &carte_jouee, couleurJouee) == 1 && strcmp(carte_jouee.valeur,"*")!=0) {
                joueurQuiPrendEnsuite = joueur;
                carteLaPlusForte = carte_jouee;
                //on modifie la couleur jouee, c'est celle du premier joueur qui a joué
                if (premierJoueur == 0) {
                    couleurJouee = carte_jouee.couleur;
                    printf("couleur: %d\n", couleurJouee);
                    premierJoueur = joueur;
                }
            }
            printf("Envoie du jeu aux joueurs\n\n");
            envoyer_jeu_joueurs(msgid, &paquet_en_cours,joueurDepart);
            carte_valide = false;
        }

        //Cas où le joueur qui prend est le preneur 
        if (joueurQuiPrendEnsuite == preneur) {
            for (int i = 0; i < paquet_en_cours.nb_cartes; i++) {
                //on ajoute pas l'excuse une seconde fois
                if (strcmp(paquet_en_cours.jeu[i].valeur,"*")!=0)
                ajouter_carte(paquet_preneur, &paquet_en_cours.jeu[i]);
            }
        } else { //Cas où le joueur qui prend est un autre joueur
            for (int i = 0; i < paquet_en_cours.nb_cartes; i++) {
                if (strcmp(paquet_en_cours.jeu[i].valeur,"*")!=0)
                ajouter_carte(paquet_adversaires, &paquet_en_cours.jeu[i]);
            }
        }
        printf("Joueur qui prend ensuite : %d\n\n", joueurQuiPrendEnsuite);
        
        //On met à jour l'ordre des joueurs pour la suite
        mettreAJourOrdreJoueurs(ordre, joueurQuiPrendEnsuite);
        printf ("Ordre des joueurs : \n");
        for (int i = 0; i < MAX_CLIENTS; i++) {
            printf("%d ", ordre[i]);
        }
    }
}

//fonction qui met à jour les scores d'un joueur
//L'ajout du mutex permet de garantir qu'un client ne lise pas un score en cours de modification
void mettre_a_jour_scores(float *scores, int joueur, float valeur) {
    pthread_mutex_lock(&mutex_scores);  // Verrouille l'accès aux scores
    scores[joueur] += valeur;
    printf("Score mis à jour : Joueur %d -> %.2f\n", joueur + 1, scores[joueur]);
    pthread_mutex_unlock(&mutex_scores);  // Déverrouille
}

//fonction qui met à jour les scores des joueurs 
void score_final_joueurs (struct paquet *paquet_preneur, char *contrat_final, int preneur ) {
    float score_adversaires ;
    float score_preneur;
    int i;
    float scorePreneur = score_final(paquet_preneur, contrat_final);
    
    //on affiche le score de chacun des joueurs et on les stocks
    if (scorePreneur >0.0){
        //score des adversaires negatif
        score_adversaires = -scorePreneur;
        score_preneur = scorePreneur*3;
        for (i = 1; i < MAX_CLIENTS+1; i++) {
            if (i==preneur){
                scoreJoueurs[i-1] = score_preneur;
            }
            else{
                scoreJoueurs[i-1] = score_adversaires;
            }
        }
    }
    else{
        //score des adversaires positif
        score_adversaires = -scorePreneur;
        score_preneur = scorePreneur*3;
        for (i = 1; i < MAX_CLIENTS+1; i++) {
            if (i==preneur){
                scoreJoueurs[i-1] = score_preneur;
            }
            else{
                scoreJoueurs[i-1] = score_adversaires;
            }
        }
    }
}


int main() {
    int preneur;
    int ordre_joueurs[MAX_CLIENTS] ;
    struct paquet jeu;
    char *message2 = "Le jeu est terminé !";
    char *message = "Le chien est fait ! Commencons à jouer !";
    int joueurQuiCommence =1;

    // Création de la mémoire partagée pour les scores
    int shmid = shmget(SHM_KEY, MAX_CLIENTS * sizeof(float), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur lors de la création de la mémoire partagée");
        exit(EXIT_FAILURE);
    }

    //On s'attache à la mémoire partagée
    float *scores = (float *)shmat(shmid, NULL, 0);
    if (scores == (void *)-1) {
        perror("Erreur lors de l'attachement à la mémoire partagée");
        exit(EXIT_FAILURE);
    }

    // Initialisation des scores
    for (int i = 0; i < MAX_CLIENTS; i++) {
        scores[i] = 0.0;
    }

    // Initialisation de l'ordre des joueurs
    ordre_joueurs[0] = 1;
    ordre_joueurs[1] = 2;
    ordre_joueurs[2] = 3;
    ordre_joueurs[3] = 4;

    while(1){
        //On crée une nouvelle boite aux lettres à chaque partie
        int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
        if (msgid == -1) {
            perror("Erreur lors de la création de la file de messages");
            exit(EXIT_FAILURE);
        }

        //on attend un message indiquant "jouer"
        printf("Attente des joueurs...\n");
        attendre_clients(msgid);

        memset(&paquet_preneur, 0, sizeof(paquet_preneur));
        memset(&paquet_adversaires, 0, sizeof(paquet_adversaires));

        //on init le nb de cartes de paquet_prneeur;
        paquet_preneur.nb_cartes = 0;
        preneur = 0;

        //jeu
        creer_paquet(&jeu);

        printf("Tous les joueurs sont prêts. Distribution des cartes...\n");
        while (preneur == 0){
            distribuer_cartes_aux_clients(msgid, &jeu);
            preneur = demande_contrat(msgid, ordre_joueurs, MAX_CLIENTS);
        }

        printf("Le preneur est le joueur %d.\n\n", preneur);

        printf("Montrons le chien...\n\n");
        envoyer_jeu_joueurs(msgid, &chien,0);

        //Le preneur fait son chien
        faire_chien (msgid, &chien, preneur, joueurs[preneur-1]);

        //une fois le chien fait, on envoie à chaque joueur que le chien est fait
        envoyer_message (msgid, message);

        //au debut, c'est le joueur à la posiiton 0 dans l'ordre qui joue, ensuite l'ordre sera fait par celui qui prendra le tour
        //on fait un tour
        jouer_un_tour(msgid, &paquet_adversaires, &paquet_preneur, preneur, ordre_joueurs);
        
        //le jeu est terminé, on envoie à chaque joueur que le jeu est terminé
        //scores
        score_final_joueurs(&paquet_preneur, contrat_final, preneur);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            mettre_a_jour_scores (scores, i, scoreJoueurs[i]);
        }

        envoyer_message (msgid, message2);
        printf("La partie est terminée.\n");

        //On met à jour l'ordre des joueurs pour la prochaine partie. C'ets le joueur suivant qui commencera. 
        if (joueurQuiCommence < 4){
            joueurQuiCommence++;
            printf ("Le joueur %d commencera la prochaine partie\n\n", joueurQuiCommence);
            mettreAJourOrdreJoueurs(ordre_joueurs,joueurQuiCommence);
            printf ("Ordre des joueurs : \n");
            for (int i = 0; i < MAX_CLIENTS; i++) {
                printf("%d ", ordre_joueurs[i]);
            }
        }
        else{
            joueurQuiCommence=1;
            mettreAJourOrdreJoueurs(ordre_joueurs,joueurQuiCommence);
        }

        //On supprime la boite aux lettres
        msgctl(msgid, IPC_RMID, NULL);
    }

    // Détachement et suppression de la mémoire partagée
    if (shmdt(scores) == -1) {
        perror("Erreur lors du détachement de la mémoire partagée");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Erreur lors de la suppression de la mémoire partagée");
    }
    return 0;
}