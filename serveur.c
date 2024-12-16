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
    struct paquet *chien_ptr = &chien;
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

        strcpy(message.msg_text, buffer);

        if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        printf("Cartes envoyées au joueur %d.\n", i + 1);
    }
}

char *niveau_contrat(char *choix_contrat) {
    // Augmenter la taille allouée pour éviter les débordements
    char *contrats_possibles = malloc(100);  // Allouer plus de mémoire
    if (contrats_possibles == NULL) {
        perror("Erreur d'allocation mémoire");
        exit(EXIT_FAILURE);
    }

    printf("Choix du contrat: %s\n", choix_contrat);
    float niveau = contrat(choix_contrat);
    printf("Niveau du contrat: %f\n", niveau);

    if (niveau == 0) strcpy(contrats_possibles, "Passe, Petite, Garde");
    else if (niveau == 1) strcpy(contrats_possibles, "Garde");
    else strcpy(contrats_possibles, "Pas de contrat possible");

    printf("Contrats possibles: %s\n", contrats_possibles);
    return contrats_possibles;
}


void demande_contrat(int msgid, int ordre_joueurs[], int nb_joueurs) {
    struct msg_buffer message;
    struct msg_buffer message_reponse;
    strcpy(message.msg_text, "Passe");

    for (int i = 0; i < nb_joueurs; i++) {
        int joueur = ordre_joueurs[i];

        // Préparer et envoyer la demande
        message.msg_type = joueur;
        char *contrats_possibles = niveau_contrat(message.msg_text);
        strcpy(message.msg_text, contrats_possibles);
        printf ("Contrats possibles dans le message : %s\n", message.msg_text);

        if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
            perror("Erreur lors de l'envoi de la demande de contrat");
            exit(EXIT_FAILURE);
        }
        printf("Demande envoyée au joueur %d.\n", joueur);

        // Attendre la réponse du joueur
        while (msgrcv(msgid, &message_reponse, sizeof(message_reponse.msg_text), joueur, 0) == -1);
        printf("Contrat reçu du joueur %d : %s\n", joueur, message_reponse.msg_text);
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

    //on définit l'ordre des joueurs. On commence par le joueur 1, puis le joueur 2, etc.
    //Si on fait une autre partie, on commencera par le joueur 2, puis le joueur 3, etc.
    int ordre_joueurs[MAX_CLIENTS] = {1, 2, 3, 4};

    //on demande au joueur 1 de choisir un contrat puis au joueur 2, etc. en adaptant les contrats possibles
    //use fonction contrat
    demande_contrat(msgid, ordre_joueurs, MAX_CLIENTS);
    


    // Suppression de la file de messages 
    msgctl(msgid, IPC_RMID, NULL);
    printf("Serveur terminé.\n");
    return 0;
}
