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

    int client_id = atoi(argv[1]);  // Récupère l'identifiant du client
    int msgid = msgget(MSG_KEY, 0666);
    if (msgid == -1) {
        perror("Erreur lors de la connexion à la file de messages");
        return EXIT_FAILURE;
    }

    struct msg_buffer message;
    struct msg_buffer message_reponse;

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

//envoie cartes recues
    message.msg_type = client_id;
    strcpy (message.msg_text, "cartes recues");
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi du message de disponibilité");
        return EXIT_FAILURE;
    }

    // Attendre la demande de contrat
    if (msgrcv(msgid, &message, sizeof(message.msg_text), client_id, 0) == -1) {
        perror("Erreur lors de la réception de la demande de contrat");
        return EXIT_FAILURE;
    }
    printf("Message du serveur : %s\n", message.msg_text);

    // Choisir un contrat
    char contrat[MSG_SIZE];
    printf("Choisissez un contrat parmi : %s\n", message.msg_text);

    // Lire la saisie utilisateur
    fgets(contrat, sizeof(contrat), stdin);
    contrat[strcspn(contrat, "\n")] = '\0';  // Supprimer le retour à la ligne

    // Utiliser "Passe" si aucun contrat n'est saisi
    if (strlen(contrat) == 0) {
        strcpy(contrat, "Passe");
    }

    // Envoyer le contrat au serveur
    memset(&message_reponse, 0, sizeof(message_reponse));  // Réinitialiser le buffer
    message_reponse.msg_type = client_id;
    strcpy(message_reponse.msg_text, contrat);

    if (msgsnd(msgid, &message_reponse, sizeof(message_reponse.msg_text), 0) == -1) {
        perror("Erreur lors de l'envoi de la réponse du contrat");
        return EXIT_FAILURE;
    }
    printf("Contrat envoyé au serveur : %s\n", contrat);

    return EXIT_SUCCESS;
}
