
/* client.c */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"
#include <errno.h>

#define MSG_KEY 1234
#define MSG_SIZE 1024

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

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

void joueur_pret(int msgid, int client_id) {
    struct msg_buffer message;
    message.msg_type = client_id;
    strcpy(message.msg_text, "Joueur prêt");

    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi du message de disponibilité");
    }
    printf("Joueur %d prêt.\n", client_id);
}

void recevoir_message (int msgid, int client_id) {
    struct msg_buffer message;
    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception du message");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", message.msg_text);
    sleep(0.5);
}

void recevoir_cartes(int msgid, int client_id) {
    struct msg_buffer message;

    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception des cartes");
        exit(EXIT_FAILURE);
    }
    printf("Cartes reçues par le joueur %d :\n%s\n", client_id, message.msg_text);

    message.msg_type = client_id;
    strcpy(message.msg_text, "Cartes reçues");

    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi de la confirmation des cartes");
        exit(EXIT_FAILURE);
    }
}

int choix_contrat_client(int msgid, int client_id) {
    struct msg_buffer message;
    struct msg_buffer message_reponse;

    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception de la demande de contrat");
        exit(EXIT_FAILURE);
    }

    char contrat_choisi[MSG_SIZE];
    printf("Choisissez un contrat parmi : %s\n", message.msg_text);

    int reponse_valide = 0;
    while (!reponse_valide) {
        fgets(contrat_choisi, sizeof(contrat_choisi), stdin);
        contrat_choisi[strcspn(contrat_choisi, "\n")] = '\0';

        if (strlen(contrat_choisi) == 0) {
            strcpy(contrat_choisi, "Passe");
        }

        if (contrat(contrat_choisi) == 3 || strstr(message.msg_text, contrat_choisi) == NULL) {
            printf("Contrat non valide.\n");
        } else {
            reponse_valide = 1;
        }
    }

    message_reponse.msg_type = client_id;
    strncpy(message_reponse.msg_text, contrat_choisi, MSG_SIZE - 1);
    message_reponse.msg_text[MSG_SIZE - 1] = '\0';

    if (msgsnd(msgid, &message_reponse, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi de la réponse du contrat");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception du preneur");
        exit(EXIT_FAILURE);
    }
    char *endptr;
    int preneur = strtol(message.msg_text, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Erreur : le message reçu n'est pas un entier valide (%s)\n", message.msg_text);
        exit(EXIT_FAILURE);
    }
    return preneur;
}

void montrer_chien(int msgid, int client_id) {
    struct msg_buffer message;

    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception du chien");
        exit(EXIT_FAILURE);
    }
    printf("Voici le chien :\n%s\n", message.msg_text);
}


void faire_chien_client(int msgid, int preneur) {
    struct msg_buffer message_reponse;
    bool ok = false;

    //on affihce le nb de mess dans la file
    afficher_nombre_messages (msgid);

     if (msgrcv(msgid, &message_reponse, MSG_SIZE, preneur, 0) == -1) {
            perror("Erreur lors de la réception du message");
            exit(EXIT_FAILURE);
        }
        printf("Voici le jeu reçu : %s\n", message_reponse.msg_text);


    for (int i = 0; i < 6; i++) {
        while (!ok) {
            int carte = 0;
            // Demander à l'utilisateur de choisir une carte
            while ( carte <= 0 || carte > 24-i) {
                printf("Choisissez une carte à mettre dans le chien (1-%d) : ", 24-i);
                scanf("%d", &carte);
            }

            // Préparer le message avec l'index de la carte choisie
            message_reponse.msg_type = preneur;
            snprintf(message_reponse.msg_text, MSG_SIZE, "%d", carte);  // Convertir l'index en chaîne

            // Envoi de l'index de la carte choisie au serveur
            if (msgsnd(msgid, &message_reponse, strlen(message_reponse.msg_text) + 1, 0) == -1) {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }
            sleep(0.5);

            //on recupere le mess pour savoir si la carte ets bonne ou non
            struct msg_buffer message_reponse2;
            if (msgrcv(msgid, &message_reponse2, MSG_SIZE, preneur , 0) == -1) {
                perror("Erreur lors de la réception du message");
                exit(EXIT_FAILURE);
            }
            sleep(0.5);
            printf("Voici le message reçu : %s\n", message_reponse2.msg_text);
            if (strcmp (message_reponse2.msg_text, "bon") == 0) {
                ok = true;
            }
            else {
                printf("La carte %d n'est pas bonne\n", carte);
            }
            printf("Vous avez choisi la carte %d\n", carte);
        }
        ok = false;
        //on affiche le nouveau jeu 
        if (msgrcv(msgid, &message_reponse, MSG_SIZE, preneur, 0) == -1) {
            perror("Erreur lors de la réception du message");
            exit(EXIT_FAILURE);
        }
        sleep(0.5);
        printf("Voici le jeu reçu : %s\n", message_reponse.msg_text);
    }
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

    joueur_pret(msgid, client_id);
    recevoir_cartes(msgid, client_id);
    int preneur = choix_contrat_client(msgid, client_id);
    montrer_chien(msgid, client_id);
    if (client_id == preneur) faire_chien_client(msgid, preneur);
    recevoir_message (msgid, client_id);
    return EXIT_SUCCESS;

}
