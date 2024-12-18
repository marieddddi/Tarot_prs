#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"

#define MSG_KEY 1234
#define MSG_SIZE 1024

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

void joueur_pret(struct msg_buffer message, int msgid, int client_id) {
    // Signaler que le joueur est prêt
    message.msg_type = client_id;
    strcpy(message.msg_text, "Joueur prêt");
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi du message de disponibilité");
    }
    printf("Joueur %d prêt.\n", client_id);
}

void recevoir_cartes(struct msg_buffer message, int msgid, int client_id) {
    // Recevoir les cartes
    if (msgrcv(msgid, &message, sizeof(message.msg_text), client_id, 0) == -1) {
        perror("Erreur lors de la réception des cartes");
    }
    printf("Cartes reçues par le joueur %d :\n%s\n", client_id, message.msg_text);

    //envoie cartes recues
    message.msg_type = client_id;
    strcpy (message.msg_text, "cartes recues");
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi du message de disponibilité");
    }
}

void choix_contrat_client (struct msg_buffer message, struct msg_buffer message_reponse, int msgid, int client_id) {
    // Attendre la demande de contrat
    if (msgrcv(msgid, &message, sizeof(message.msg_text), client_id, 0) == -1) {
        perror("Erreur lors de la réception de la demande de contrat");
        return EXIT_FAILURE;
    }
    printf("Message du serveur : %s\n", message.msg_text);

    // Choisir un contrat
    char contrat_choisi[MSG_SIZE];
    printf("Choisissez un contrat parmi : %s\n", message.msg_text);

    int reponse_valide =0;
    while (reponse_valide !=1) {
        // Lire la saisie utilisateur
        fgets(contrat_choisi, sizeof(contrat_choisi), stdin);
        contrat_choisi[strcspn(contrat_choisi, "\n")] = '\0';  // Supprimer le retour à la ligne

        // Utiliser "Passe" si aucun contrat n'est saisi
        if (strlen(contrat_choisi) == 0) {
            strcpy(contrat_choisi, "Passe");
        }
        if (contrat(contrat_choisi) == 3 || strstr(message.msg_text, contrat_choisi) == NULL) {
            printf("Contrat non valide.\n");
        }
        else {
            reponse_valide = 1;
        }
    }

    // Envoyer le contrat au serveur
    memset(&message_reponse, 0, sizeof(message_reponse));  // Réinitialiser le buffer
    message_reponse.msg_type = client_id;
    strcpy(message_reponse.msg_text, contrat_choisi);

    if (msgsnd(msgid, &message_reponse, sizeof(message_reponse.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi de la réponse du contrat");
        return EXIT_FAILURE;
    }
    printf("Contrat envoyé au serveur : %s\n", contrat_choisi);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <client_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int client_id = atoi(argv[1]);  // Récupère l'identifiant du client
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("Erreur lors de la connexion à la file de messages");
        return EXIT_FAILURE;
    }

    struct msg_buffer message;
    struct msg_buffer message_reponse;

    //joueur pret
    joueur_pret(message, msgid, client_id);

    //recevoir cartes
    recevoir_cartes(message, msgid, client_id);
    
    //faire les contrats
    choix_contrat_client(message, message_reponse, msgid, client_id);

    return EXIT_SUCCESS;
}
