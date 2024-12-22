/* serveur.c */
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

void envoyer_preneur(int msgid, int preneur) {
    struct msg_buffer message;
    memset(&message, 0, sizeof(message));
    message.msg_type = preneur;  // Utiliser le numéro du preneur comme type de message
    strncpy(message.msg_text, "C'est toi qui prends le chien", MSG_SIZE - 1);
    message.msg_text[MSG_SIZE - 1] = '\0';

    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi du message au preneur");
        exit(EXIT_FAILURE);
    }
    printf("Message envoyé au preneur %d : %s\n", preneur, message.msg_text);
}

int main() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("Erreur lors de la création de la file de messages");
        exit(EXIT_FAILURE);
    }

    // Envoyer les messages aux deux preneurs
    envoyer_preneur(msgid, 1);
    envoyer_preneur(msgid, 2);

    // Ne pas supprimer immédiatement la file de messages pour permettre aux clients de lire
    printf("Serveur terminé. Attendez que les clients lisent les messages.\n");
    return 0;
}
