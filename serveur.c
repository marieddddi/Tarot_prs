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

// Structure d'un message
struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

void attendre_clients(int msgid) {
    struct msg_buffer message;
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (msgrcv(msgid, &message, sizeof(message.msg_text), i, 0) == -1) {
            perror("Erreur lors de la réception du message d'un client");
            exit(EXIT_FAILURE);
        }
        printf("Client %d prêt : %s\n", i, message.msg_text);
    }
}

void distribuer_cartes_aux_clients(int msgid, struct paquet *jeu) {
    struct paquet j1, j2, j3, j4, chien;
    distribuer_cartes(jeu, &j1, &j2, &j3, &j4, &chien);

    struct paquet *joueurs[] = {&j1, &j2, &j3, &j4};
    struct msg_buffer message;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        message.msg_type = i + 1; // Type de message correspondant au client
        char buffer[MSG_SIZE] = "";

        for (int j = 0; j < joueurs[i]->nb_cartes; j++) {
            char carte_info[50];
            snprintf(carte_info, sizeof(carte_info), "%c %s %.1f\n", 
                     joueurs[i]->jeu[j].couleur, 
                     joueurs[i]->jeu[j].valeur, 
                     joueurs[i]->jeu[j].point);
            strcat(buffer, carte_info);
        }

        strncpy(message.msg_text, buffer, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0'; // Assurer une terminaison sécurisée

        if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        printf("Cartes envoyées au joueur %d.\n", i + 1);
    }
}

void niveau_contrat(char *choix_contrat, char *contrat_precedent, char *resultat) {
    // Nettoyer le buffer de résultat
    memset(resultat, 0, 100);

    printf("Choix du contrat: %s\n", choix_contrat);
    float niveau = contrat(choix_contrat); // Fonction utilisateur "contrat"
    printf("Niveau du contrat: %f\n", niveau);

    if (niveau == 0) {
        strncpy(resultat, contrat_precedent, 99);
    } else if (niveau == 1) {
        strncpy(resultat, "Passe, Garde", 99);
    } else {
        strncpy(resultat, "Passe", 99);
    }

    resultat[99] = '\0'; // Assurer une terminaison sécurisée
    printf("Contrats possibles: %s\n", resultat);
}

void demande_contrat(int msgid, int ordre_joueurs[], int nb_joueurs) {
    struct msg_buffer message;
    struct msg_buffer message_reponse;

    strcpy(message_reponse.msg_text, "Passe");  // Valeur par défaut

    char contrats_possibles[100] = "Passe, Petite, Garde";  // Contrats initiaux

    for (int i = 0; i < nb_joueurs; i++) {
        int joueur = ordre_joueurs[i];

        // Calculer les contrats possibles
        char nouveaux_contrats[100];
        niveau_contrat(message_reponse.msg_text, contrats_possibles, nouveaux_contrats);
        strncpy(contrats_possibles, nouveaux_contrats, 99);
        contrats_possibles[99] = '\0';

        // Préparer et envoyer la demande de contrat
        memset(&message, 0, sizeof(message));
        message.msg_type = joueur;
        strncpy(message.msg_text, contrats_possibles, MSG_SIZE - 1);
        message.msg_text[MSG_SIZE - 1] = '\0';

        printf("Contrats possibles dans le message : %s\n", message.msg_text);

        if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
            perror("Erreur lors de l'envoi de la demande de contrat");
            exit(EXIT_FAILURE);
        }
        printf("Demande envoyée au joueur %d avec msg_type = %d.\n", joueur, joueur);

        memset(&message_reponse, 0, sizeof(message_reponse));

        // Attendre une réponse valide
        while (1) {
            if (msgrcv(msgid, &message_reponse, sizeof(message_reponse.msg_text), joueur, 0) == -1) {
                perror("Erreur lors de la réception de la réponse du contrat");
                continue;
            }

            if (strlen(message_reponse.msg_text) == 0) {
                printf("Réponse vide reçue du joueur %d, attente d'une réponse valide...\n", joueur);
                continue;
            }

            printf("Réponse du joueur %d : %s\n", joueur, message_reponse.msg_text);
            break;
        }
    }
}

int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("Erreur lors de la création de la file de messages");
        exit(EXIT_FAILURE);
    }

    struct paquet jeu;
    creer_paquet(&jeu);

    printf("Attente des joueurs...\n");
    attendre_clients(msgid);

    printf("Tous les joueurs sont prêts. Distribution des cartes...\n");
    distribuer_cartes_aux_clients(msgid, &jeu);

    int ordre_joueurs[MAX_CLIENTS] = {1, 2, 3, 4};

    demande_contrat(msgid, ordre_joueurs, MAX_CLIENTS);

    // Suppression de la file de messages
    msgctl(msgid, IPC_RMID, NULL);
    printf("Serveur terminé.\n");
    return 0;
}