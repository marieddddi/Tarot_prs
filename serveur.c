/* serveur.c */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"

#define MSG_KEY 1234
#define MAX_CLIENTS 4
#define MSG_SIZE 1024

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

struct paquet chien, j1, j2, j3, j4, paquet_preneur, paquet_adversaires;
struct paquet *joueurs[] = {&j1, &j2, &j3, &j4};


void afficher_nombre_messages(int msgid) {
    struct msqid_ds buf;

    // Récupérer les informations sur la file de messages
    if (msgctl(msgid, IPC_STAT, &buf) == -1) {
        perror("Erreur lors de la récupération des informations de la file de messages");
        exit(EXIT_FAILURE);
    }

    // Afficher le nombre de messages
    printf("Nombre de messages dans la file : %ld\n", buf.msg_qnum);
}


void attendre_clients(int msgid) {
    struct msg_buffer message;
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (msgrcv(msgid, &message, MSG_SIZE, i, 0) == -1) {
            perror("Erreur lors de la réception du message d'un client");
            exit(EXIT_FAILURE);
        }
        printf("Client %d prêt : %s\n", i, message.msg_text);
    }
}

void distribuer_cartes_aux_clients(int msgid, struct paquet *jeu) {
    distribuer_cartes(jeu, &j1, &j2, &j3, &j4, &chien);

    struct msg_buffer message;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        message.msg_type = i + 1;
        char buffer[MSG_SIZE] = "";

        for (int j = 0; j < joueurs[i]->nb_cartes; j++) {
            char carte_info[50];
            snprintf(carte_info, sizeof(carte_info), "%d %c %s %.1f\n", j + 1,
                     joueurs[i]->jeu[j].couleur, 
                     joueurs[i]->jeu[j].valeur, 
                     joueurs[i]->jeu[j].point);
            strcat(buffer, carte_info);
        }

        strncpy(message.msg_text, buffer, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        printf("Cartes envoyées au joueur %d.\n", i + 1);

        if (msgrcv(msgid, &message, MSG_SIZE, i + 1, 0) == -1) {
            perror("Erreur lors de la réception de la confirmation de réception des cartes");
            exit(EXIT_FAILURE);
        }
    }
}

void niveau_contrat(char *choix_contrat, char *contrat_precedent, char *resultat) {
    memset(resultat, 0, 100);

    printf("Choix du contrat: %s\n", choix_contrat);
    float niveau = contrat(choix_contrat);
    printf("Niveau du contrat: %f\n", niveau);

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

int demande_contrat(int msgid, int ordre_joueurs[], int nb_joueurs) {
    char contrats_joueurs[MAX_CLIENTS][100]; 
    memset(contrats_joueurs, 0, sizeof(contrats_joueurs));

    struct msg_buffer message;
    struct msg_buffer message_reponse;
    memset(&message_reponse, 0, sizeof(message_reponse));

    strcpy(message_reponse.msg_text, "Passe");

    char contrats_possibles[100] = "Passe, Petite, Garde";

    for (int i = 0; i < nb_joueurs; i++) {
        int joueur = ordre_joueurs[i];

        char nouveaux_contrats[100];
        niveau_contrat(message_reponse.msg_text, contrats_possibles, nouveaux_contrats);
        strncpy(contrats_possibles, nouveaux_contrats, 99);
        contrats_possibles[99] = '\0';

        memset(&message, 0, sizeof(message));
        message.msg_type = joueur;
        strncpy(message.msg_text, contrats_possibles, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        printf("Contrats possibles dans le message : %s\n", message.msg_text);

        if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
            perror("Erreur lors de l'envoi de la demande de contrat");
            exit(EXIT_FAILURE);
        }
        printf("Demande envoyée au joueur %d avec msg_type = %d.\n", joueur, joueur);

        memset(&message_reponse, 0, sizeof(message_reponse));
        sleep(1);

        if (msgrcv(msgid, &message_reponse, MSG_SIZE, joueur, 0) == -1) {
            perror("Erreur lors de la réception de la réponse du contrat");
            break;
        }
        printf("Réponse du joueur %d : %s\n", joueur, message_reponse.msg_text);

        strncpy(contrats_joueurs[joueur], message_reponse.msg_text, 99);
        contrats_joueurs[joueur][99] = '\0';
    }

    for (int i = 1; i < nb_joueurs + 1; i++) {
        printf("Contrat du joueur %d : %s\n", i, contrats_joueurs[i]);
    }

    int preneur = 0;
    int niveau_max = 0;
    for (int i = 1; i < nb_joueurs + 1; i++) {
        int val_en_cours = contrat(contrats_joueurs[i]);
        printf("Contrat du joueur %d : %d\n", i, val_en_cours);
        if (val_en_cours > niveau_max) {
            preneur = i;
            niveau_max = val_en_cours;
        }
    }

    if (preneur == 0) {
        printf("Aucun preneur trouvé.\n");
    } else {
        printf("Le preneur est le joueur %d avec un contrat de niveau %d\n", preneur, niveau_max);
    }

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

void envoyer_jeu_avec_chien(int msgid, int preneur, struct paquet *chien, struct paquet *paquet) {
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
    afficher_nombre_messages (msgid);

    if (msgsnd(msgid, &message, strlen(message.msg_text) + 1, 0) == -1) {
        perror("Erreur lors de l'envoi du paquet au preneur");
        exit(EXIT_FAILURE);
    }
    printf("Paquet envoyé au preneur, avec %d cartes.\n", paquet->nb_cartes);
}

void faire_chien(int msgid, struct paquet *chien, int preneur, struct paquet *paquet) {
    struct msg_buffer message;
    //on affiche le nb de mess dans la file 
    afficher_nombre_messages (msgid);

    for (int i = 0; i < 6; i++) {
        // Réception des indices des cartes que le preneur met dans le chien
        if (msgrcv(msgid, &message, MSG_SIZE, preneur, 0) == -1) {
            perror("Erreur lors de la réception des indices des cartes");
            exit(EXIT_FAILURE);
        }
        printf ("Indices des cartes : %s\n", message.msg_text);

        int index = atoi(message.msg_text);
        index = index -1;
        printf("paquet: %d\n", paquet->nb_cartes);

        // Afficher la carte correspondante
        printf("Carte choisie : %c %s\n",
               paquet->jeu[index].couleur,
               paquet->jeu[index].valeur);

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
        envoyer_jeu_avec_chien (msgid, preneur, &chien, paquet);
    }

    // Afficher le nombre final de messages dans la file
    afficher_nombre_messages(msgid);
    //on affiche le contenu dans paquet_preneur 
    afficher_paquet(&paquet_preneur);


}


int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    //on init le nb de cartes de paquet_prneeur;
    paquet_preneur.nb_cartes = 0;

    if (msgid == -1) {
        perror("Erreur lors de la création de la file de messages");
        exit(EXIT_FAILURE);
    }
    printf("File de messages créée avec la clé %d\n", MSG_KEY);

    struct paquet jeu;
    creer_paquet(&jeu);

    printf("Attente des joueurs...\n");
    attendre_clients(msgid);

    printf("Tous les joueurs sont prêts. Distribution des cartes...\n");
    distribuer_cartes_aux_clients(msgid, &jeu);

    int ordre_joueurs[MAX_CLIENTS] = {1, 2, 3, 4};
    int preneur = demande_contrat(msgid, ordre_joueurs, MAX_CLIENTS);

    printf("Le preneur est le joueur %d.\n", preneur);

    printf("Montrons le chien...\n");
    montrer_chien(msgid, &chien);
    afficher_nombre_messages (msgid);

    printf("ici");

    envoyer_jeu_avec_chien (msgid, preneur, &chien, joueurs[preneur -1]);
    sleep(1);
    afficher_nombre_messages (msgid);
    faire_chien (msgid, &chien, preneur, joueurs[preneur-1]);
                    
   // msgctl(msgid, IPC_RMID, NULL);
    printf("Serveur terminé.\n");
    return 0;
}