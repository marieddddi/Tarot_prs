/* serveur.c */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include "fonctions.h"

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

//fonction permettant d'envoyer un message à tous les clients
void envoyer_message(int msgid, char *message) {
    struct msg_buffer msg;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        msg.msg_type = i+1;

        // Réinitialiser le buffer avant de copier le message
        memset(msg.msg_text, 0, MSG_SIZE);
        strncpy(msg.msg_text, message, MSG_SIZE - 1);
        msg.msg_text[MSG_SIZE - 1] = '\0'; // S'assurer que le message est terminé

        if (msgsnd(msgid, &msg, strlen(msg.msg_text) + 1, 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
    }
    sleep(0.5); // Permet de donner du temps aux clients pour traiter les messages
}

//fonction permettant de recevoir un message de tous les clients indiquant qu'ils veulent jouer
void attendre_clients(int msgid) {
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
        sleep(0.5);
    }
}

//fonction permettant de distribuer les cartes
void distribuer_cartes_aux_clients(int msgid, struct paquet *jeu) {
    //on distribue le jeu dans les paquets de chaque joueur et du chien. 
    distribuer_cartes(jeu, &joueur1, &joueur2, &joueur3, &joueur4, &chien);

    struct msg_buffer message;

    //on envoie le paquet de chaque joueur à chaque client
    //le paquet est sous forme d'un texte , donc on le convertit en chaine de caractères
    for (int i = 0; i < MAX_CLIENTS; i++) {
        message.msg_type = i + 1;
        char buffer[MSG_SIZE] = "";

        for (int j = 0; j < joueurs[i]->nb_cartes; j++) {
            char carte_info[50];
            snprintf(carte_info, sizeof(carte_info), "%d %c %s \n", j + 1,
                     joueurs[i]->jeu[j].couleur, 
                     joueurs[i]->jeu[j].valeur);
            strcat(buffer, carte_info);
        }

        strncpy(message.msg_text, buffer, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        printf("\nCartes envoyées au joueur %d.\n\n", i + 1);
        printf ("%s\n", message.msg_text);

        if (msgrcv(msgid, &message, MSG_SIZE, i + 1, 0) == -1) {
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
        message.msg_type = joueur;
        strncpy(message.msg_text, contrats_possibles, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
            perror("Erreur lors de l'envoi de la demande de contrat");
            exit(EXIT_FAILURE);
        }
        printf("Demande envoyée au joueur %d avec msg_type = %d.\n", joueur, joueur);

        memset(&message_reponse, 0, sizeof(message_reponse));
        sleep(1); //pause pour que le client ait le temps de repondre

        if (msgrcv(msgid, &message_reponse, MSG_SIZE, joueur, 0) == -1) {
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
    sleep(1);
    return preneur;
}

//fonction permettant d'envoyer la jeu du client choisit 
void envoyer_jeu(int msgid, struct paquet *paquet, int preneur) {
    struct msg_buffer message;
    char buffer[MSG_SIZE] = "";
    char carte_info[50];

    for (int i = 0; i < paquet->nb_cartes; i++) {
        snprintf(carte_info, sizeof(carte_info), "%d %c %s\n", i + 1,
                 paquet->jeu[i].couleur, 
                 paquet->jeu[i].valeur);
        strcat(buffer, carte_info);
    }

    message.msg_type = preneur;
    printf ("\nEnvoi du jeu au joueur %d ...\n", preneur);
    strncpy(message.msg_text, buffer, MSG_SIZE - 1);
    message.msg_text[MSG_SIZE - 1] = '\0';

    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi du chien");
        exit(EXIT_FAILURE);
    }
    printf("Jeu envoyé au joueur %d.\n\n", preneur);
}

//fonction envoyant un paquet à tous les joueurs 
void envoyer_jeu_joueurs (int msgid, struct paquet *paquet) {
    struct msg_buffer message;
    char buffer[MSG_SIZE] = "";
    char carte_info[50];

    //on crée un buffer contenant les informations de chaque carte
    for (int i = 0; i < paquet->nb_cartes; i++) {
        snprintf(carte_info, sizeof(carte_info), "%d %c %s\n", i + 1, 
            paquet->jeu[i].couleur,
            paquet->jeu[i].valeur);
        strcat(buffer, carte_info);
    }

    //on envoie le message à chaque joueur
    for (int i = 0; i < MAX_CLIENTS; i++) {
        message.msg_type = i+1;
        strncpy(message.msg_text, buffer, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1 ) {
            perror("Erreur lors de l'envoi du paquet en cours");
            exit(EXIT_FAILURE);
        }
    }
    printf("Paquet envoyé à tous les joueurs.\n");
}

//fonction envoyant le chien à tous les joueurs
void montrer_chien(int msgid, struct paquet *chien) {
    struct msg_buffer message;
    char buffer[MSG_SIZE] = "";

    for (int i = 0; i < chien->nb_cartes; i++) {
        char carte_info[50];
        snprintf(carte_info, sizeof(carte_info), "%d %c %s %.1f\n", i + 1,
                 chien->jeu[i].couleur, 
                 chien->jeu[i].valeur, 
                 chien->jeu[i].point);
        strcat(buffer, carte_info);
    }

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        message.msg_type = i;
        strncpy(message.msg_text, buffer, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
            perror("Erreur lors de l'envoi du chien");
            exit(EXIT_FAILURE);
        }
        printf("Chien envoyé au joueur %d.\n", i);
    }
}

struct paquet *envoyer_jeu_avec_chien(int msgid, int preneur, struct paquet *chien, struct paquet *paquet) {
    struct msg_buffer message;

    // Ajout des cartes du chien au paquet
    for (int i = 0; i < chien->nb_cartes; i++) {
        paquet->jeu[paquet->nb_cartes] = chien->jeu[i];
        paquet->nb_cartes++;
    }

    // Création et envoi du message contenant le paquet
    message.msg_type = preneur;
    char buffer[MSG_SIZE] = "";

    for (int j = 0; j < paquet->nb_cartes; j++) {
        char carte_info[50];
        snprintf(carte_info, sizeof(carte_info), "%d %c %s %.1f\n", j + 1,
                 paquet->jeu[j].couleur,
                 paquet->jeu[j].valeur,
                 paquet->jeu[j].point);
        strcat(buffer, carte_info);
    }

    strncpy(message.msg_text, buffer, MSG_SIZE - 1);
    message.msg_text[MSG_SIZE - 1] = '\0';

    if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
        perror("Erreur lors de l'envoi du paquet au preneur");
        exit(EXIT_FAILURE);
    }
    printf("Paquet envoyé au preneur, avec %d cartes.\n", paquet->nb_cartes);

    return paquet;
}

void faire_chien(int msgid, struct paquet *chien, int preneur, struct paquet *paquet) {
    struct msg_buffer message;
    bool non_valide=true;
    int index = 0;
    //on affiche le nb de mess dans la file 
    //on stock le nouveau jeu fait avec afficher chien
    envoyer_jeu_avec_chien (msgid, preneur, chien, paquet);
    sleep(1);


    for (int i = 0; i < 6; i++) {
        while(non_valide){
            // Réception des indices des cartes que le preneur met dans le chien
            if (msgrcv(msgid, &message, MSG_SIZE, preneur, 0) == -1) {
                perror("Erreur lors de la réception des indices des cartes");
                exit(EXIT_FAILURE);
            }
            printf ("Indices des cartes : %s\n", message.msg_text);
            sleep(0.5);

            index = atoi(message.msg_text);
            index = index -1;
            printf("paquet: %d\n", paquet->nb_cartes);

            // Afficher la carte correspondante
            printf("Carte choisie : %c %s\n",
                paquet->jeu[index].couleur,
                paquet->jeu[index].valeur);
            if (est_atout(&paquet->jeu[index])) {
                printf("C'est un atout !\n");

                //on envoie que c'est un atout 
                message.msg_type = preneur;
                strcpy(message.msg_text, "atout");
                if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
                    perror("Erreur lors de l'envoi du paquet au preneur");
                    exit(EXIT_FAILURE);
                }
                sleep(0.5);
            }
            else{
                //on envoie que c'est bon 
                message.msg_type = preneur;
                strcpy ( message.msg_text, "bon");
                if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
                    perror("Erreur lors de l'envoi du paquet au preneur");
                    exit(EXIT_FAILURE);
                }
                sleep(0.5);
                non_valide = false;
            } 
        }

        // Ajouter la carte au paquet_preneur
        paquet_preneur.jeu[paquet_preneur.nb_cartes] = paquet->jeu[index];
        paquet_preneur.nb_cartes++;

        // Supprimer la carte du paquet
        for (int j = index; j < paquet->nb_cartes - 1; j++) {
            paquet->jeu[j] = paquet->jeu[j + 1];
        }
        paquet->nb_cartes--;

        printf("Carte ajoutée au chien. Le paquet preneur contient maintenant %d cartes.\n",
               paquet_preneur.nb_cartes);
        printf("Le paquet contient maintenant %d cartes.\n", paquet->nb_cartes);
        //on envoie le nouveau jeu 
        envoyer_jeu (msgid, paquet, preneur);
        sleep(1);
        non_valide = true;
    }

    //on affiche le contenu dans paquet_preneur 
    afficher_paquet(&paquet_preneur);
}

void retirer_carte(struct paquet *paquet, int index) {
    // Supprimer la carte du paquet
    //si c'est l'excuse, on la met directement dans le paquet du joueur 

    for (int j = index; j < paquet->nb_cartes - 1;j++) { // On boucle sur les cartes restantes
        paquet->jeu[j] = paquet->jeu[j + 1];
    }
    paquet->nb_cartes--;
}

void ajouter_carte(struct paquet *paquetAjout, struct carte *carteAjoutee) {
    // Ajouter la carte au paquet
    paquetAjout->jeu[paquetAjout->nb_cartes] = *carteAjoutee;
    paquetAjout->nb_cartes++;
}

// Fonction pour mettre à jour l'ordre des joueurs
void mettreAJourOrdreJoueurs(int ordre_joueurs[], int JoueurQuiPrend) {
    int nouvel_ordre[MAX_CLIENTS];
    int index = 0;

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

void jouer_un_tour(int msgid, struct paquet *paquet_adversaires, struct paquet *paquet_preneur, int preneur, int ordre[MAX_CLIENTS]) {
    struct msg_buffer message;
    char *aToi = "a toi";
    int index = 0;

    while (joueur1.nb_cartes>17){
        bool carte_valide = false;
        struct carte carteLaPlusForte = { 0, {0,0}, 0.0 };
        char couleurJouee = ' ';
        int joueurQuiPrendEnsuite = 0;
        int premierJoueur = 0;

        struct paquet paquet_en_cours = {0};


        for (int i = 0; i < MAX_CLIENTS; i++) {
            int joueur = ordre[i];
            printf("Tour du joueur %d\n", joueur);

            message.msg_type = joueur;
            strncpy(message.msg_text, aToi, MSG_SIZE - 1);
            message.msg_text[MSG_SIZE - 1] = '\0'; 

            if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
                perror("Pb envoie");
                exit(EXIT_FAILURE);
            }
            sleep(1);
            memset (&message, 0, sizeof(message));
            if (msgrcv(msgid, &message, sizeof(message.msg_text), joueur, 0) == -1) {
                perror("Erreur lors de la réception de l'accusé de réception du joueur");
                exit(EXIT_FAILURE);
            }
            if (strcmp(message.msg_text, "pret") != 0) {
                printf ("mess: %s\n", message.msg_text);
                fprintf(stderr, "Le joueur %d n'a pas confirmé qu'il est prêt.\n", joueur);
                exit(EXIT_FAILURE);
            }

            sleep(1);
            envoyer_jeu(msgid, joueurs[joueur - 1], joueur);
            sleep(1);

            while (!carte_valide) {
                if (msgrcv(msgid, &message, MSG_SIZE, joueur, 0) == -1) {
                    perror("Erreur lors de la réception de la carte");
                    exit(EXIT_FAILURE);
                }
                sleep(1);
                index = atoi(message.msg_text) - 1;

                carte_valide = accepter_carte(&carteLaPlusForte, &joueurs[joueur - 1]->jeu[index], joueurs[joueur - 1], couleurJouee);
                strcpy(message.msg_text, carte_valide ? "valide" : "non_valide");
                message.msg_type = joueur;
                if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
                    perror("Erreur lors de l'envoi de la validation");
                    exit(EXIT_FAILURE);
                }
            }

            struct carte carte_jouee = joueurs[joueur - 1]->jeu[index];
            ajouter_carte(&paquet_en_cours, &carte_jouee);
            //on affiche le paquet en cours 
            printf("paquet en cours: \n");
            afficher_paquet (&paquet_en_cours);
            retirer_carte(joueurs[joueur - 1], index);

            if (strcmp(carte_jouee.valeur, "*")==0 && joueur != preneur) {
                ajouter_carte(paquet_adversaires, &carte_jouee);
            } 
            if (strcmp(carte_jouee.valeur,"*")==0 && joueur == preneur) {
                ajouter_carte(paquet_preneur, &carte_jouee);
            }
            if (qui_a_la_plus_forte_carte(&carteLaPlusForte, &carte_jouee, couleurJouee) == 1 && strcmp(carte_jouee.valeur,"*")!=0) {
                joueurQuiPrendEnsuite = joueur;
                carteLaPlusForte = carte_jouee;
                //on modifie la couleur jouee, c'est celle du premier joueur qui a joué
                if (premierJoueur == 0) {
                    couleurJouee = carte_jouee.couleur;
                    premierJoueur = 1;
                }
            }
            printf("envoie du jeu aux joueurs\n");
            envoyer_jeu_joueurs(msgid, &paquet_en_cours);
            carte_valide = false;
        }

        if (joueurQuiPrendEnsuite == preneur) {
            for (int i = 0; i < paquet_en_cours.nb_cartes; i++) {
                //on ajoute pas l'excuse une seconde fois
                if (strcmp(paquet_en_cours.jeu[i].valeur,"*")!=0)
                ajouter_carte(paquet_preneur, &paquet_en_cours.jeu[i]);
            }
        } else {
            for (int i = 0; i < paquet_en_cours.nb_cartes; i++) {
                if (strcmp(paquet_en_cours.jeu[i].valeur,"*")!=0)
                ajouter_carte(paquet_adversaires, &paquet_en_cours.jeu[i]);
            }
        }
        printf("joueur qui prend ensuite : %d\n", joueurQuiPrendEnsuite);
        
        mettreAJourOrdreJoueurs(ordre, joueurQuiPrendEnsuite);
        printf ("ordre des joueurs : \n");
        for (int i = 0; i < MAX_CLIENTS; i++) {
            printf("%d ", ordre[i]);
        }
    }
}

void mettre_a_jour_scores(float *scores, int joueur, float valeur) {
    scores[joueur] += valeur;
    printf("Score mis à jour : Joueur %d -> %.2f\n", joueur + 1, scores[joueur]);
}

void score_final_joueurs (struct paquet *paquet_preneur, char *contrat_final, int preneur ) {
    float scorePreneur = score_final(paquet_preneur, contrat_final);
    printf("valeur du prenuer: %d\n", preneur);
    //on affiche le score de chacun des joueurs et on les stocks
    if (scorePreneur >0.0){
        //score des adversaires negatif
        float score_adversaires = -scorePreneur;
        float score_preneur = scorePreneur*3;
        for (int i = 1; i < MAX_CLIENTS+1; i++) {
            if (i==preneur){
                scoreJoueurs[i-1] = score_preneur;
                printf("vous avez un score1 de %f\n", score_preneur);
            }
            else{
                scoreJoueurs[i-1] = score_adversaires;
                printf("vous avez un score2 de %f\n", score_adversaires);
            }
        }
    }
    else{
        //score des adversaires positif
        float score_adversaires = -scorePreneur;
        float score_preneur = scorePreneur*3;
        for (int i = 1; i < MAX_CLIENTS+1; i++) {
            if (i==preneur){
                scoreJoueurs[i-1] = score_preneur;
                printf("vous avez un score11 de %f\n", score_preneur);
            }
            else{
                scoreJoueurs[i-1] = score_adversaires;
                printf("vous avez un score21 de %f\n", score_adversaires);
            }
        }
    }
}


int main() {
    
    int preneur;
    struct msg_buffer messageRecu;
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
        int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
        if (msgid == -1) {
            perror("Erreur lors de la création de la file de messages");
            exit(EXIT_FAILURE);
        }
        printf("msgid: %d\n", msgid);
        printf("creation ok \n");
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

        printf("Le preneur est le joueur %d.\n", preneur);

        printf("Montrons le chien...\n");
        envoyer_jeu_joueurs(msgid, &chien);

        faire_chien (msgid, &chien, preneur, joueurs[preneur-1]);

        //une fois le chien fait, on envoie à chaque que le chien est fait
        envoyer_message (msgid, message);

        //au debut, c'est le joueur 1 qui joue, ensuite l'ordre sera fait par celui qui prendra le tour
        //on fait un tour
        jouer_un_tour(msgid, &paquet_adversaires, &paquet_preneur, preneur, ordre_joueurs);
        
        //le jeu est terminé, on envoie à chaque joueur que le jeu est terminé
        //scores
        score_final_joueurs(&paquet_preneur, contrat_final, preneur);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            mettre_a_jour_scores (scores, i, scoreJoueurs[i]);
        }

        envoyer_message (msgid, message2);
        sleep(2);
        printf("La partie est terminée.\n");
        if (joueurQuiCommence < 4){
            joueurQuiCommence++;
            printf ("joueur %d commence la partie\n", joueurQuiCommence);
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