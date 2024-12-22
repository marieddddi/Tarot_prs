/* client.c */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MSG_KEY 1234
#define MSG_SIZE 1024

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

void recevoir_cartes(int msgid, int client_id) {
    struct msg_buffer message;

    // Attente du message correspondant au client_id
    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception des cartes");
        exit(EXIT_FAILURE);
    }
    printf("Cartes reçues par le joueur %d :\n%s\n", client_id, message.msg_text);
}

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

    recevoir_cartes(msgid, client_id);
    return EXIT_SUCCESS;
}
