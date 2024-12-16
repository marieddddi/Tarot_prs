// client.c
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


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <client_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int client_id = atoi(argv[1]);
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("Erreur lors de la connexion à la file de messages");
        return EXIT_FAILURE;
    }

    struct msg_buffer message;

    // Signaler que le client est prêt
    message.msg_type = client_id;
    strcpy(message.msg_text, "Joueur prêt");
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi du message de disponibilité");
        return EXIT_FAILURE;
    }
    printf("Joueur %d prêt.\n", client_id);

    // Recevoir les cartes
    if (msgrcv(msgid, &message, sizeof(message.msg_text), client_id, 0) == -1) {
        perror("Erreur lors de la réception des cartes");
        return EXIT_FAILURE;
    }
    printf("Cartes reçues par le joueur %d :\n%s\n", client_id, message.msg_text);

    // Attendre la demande de contrat
    while (msgrcv(msgid, &message, sizeof(message.msg_text), client_id, 0) == -1);
    printf("Message du serveur : %s\n", message.msg_text);

    // Choisir un contrat
    char contrat[10];
    scanf("%s", contrat);

    // Envoyer le contrat au serveur
    message.msg_type = client_id;
    strcpy(message.msg_text, contrat);
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi de la réponse du contrat");
        return EXIT_FAILURE;
    }
    printf("Contrat envoyé : %s\n", contrat);

    return EXIT_SUCCESS;
}

