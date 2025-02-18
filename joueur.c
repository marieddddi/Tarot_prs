#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fonctions.h"
#include <sys/shm.h>
#include <errno.h>


#define MSG_KEY 1234
#define MSG_SIZE 1024
#define SHM_KEY 5678

struct msg_buffer {
    long msg_type;
    char msg_text[MSG_SIZE];
};

//envoie au serveur que le joueur est prêt
void joueur_pret(int msgid, int client_id) {
    struct msg_buffer message;
    message.msg_type = client_id+10;
    strcpy(message.msg_text, "Joueur prêt");

    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi du message de disponibilité");
    }
    printf("Joueur %d prêt.\n", client_id);
}

//fonction permettant de recevoir un message
void recevoir_message (int msgid, int client_id, int mode) {
    struct msg_buffer message;
    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception du message");
        exit(EXIT_FAILURE);
    }
    switch (mode) {
        case 1:
            printf("%s\n", message.msg_text);
            break;
        case 2:
            printf("Cartes reçues par le joueur %d :\n%s\n", client_id, message.msg_text);
            break;
        case 3:
            printf("Choisissez un contrat parmi : %s\n", message.msg_text);
            break;
        case 4:
            printf("Voici le chien :\n%s\n", message.msg_text);
            break;
        case 5:
            printf("Voici le jeu reçu : \n%s\n", message.msg_text);
            break;
        case 6:
            printf("Voici votre jeu :\n%s\n", message.msg_text);
            break;
        default:
            break;
    }
}

//fonction permettant de recevoir un message de fin indiquant que le jeu est terminé
void recevoir_message_fin (int msgid, int client_id) {
    struct msg_buffer message;
    while (strcmp(message.msg_text, "Le jeu est terminé !") != 0) {
        if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
            perror("Erreur lors de la réception du message");
            exit(EXIT_FAILURE);
        }
        printf("%s\n", message.msg_text);
    }
}

//fonction permettant de recevoir ses cartes 
void recevoir_cartes(int msgid, int client_id) {
    struct msg_buffer message;

    //on recoit les cartes
    recevoir_message(msgid,client_id,2);

    message.msg_type = client_id+10;
    strcpy(message.msg_text, "Cartes reçues");

    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Erreur lors de l'envoi de la confirmation des cartes");
        exit(EXIT_FAILURE);
    }
}

//fonction permettant de faire son contrat parmis la liste des contrats possible s(envoyé par le serveur)
int choix_contrat_client(int msgid, int client_id) {
    struct msg_buffer message;
    struct msg_buffer message_reponse;
    char contrat_choisi[MSG_SIZE];
    int reponse_valide = 0;
    char *endptr;
    int preneur;

    //on recoit les contrats disponibles
    if (msgrcv(msgid, &message, MSG_SIZE, client_id, 0) == -1) {
        perror("Erreur lors de la réception du message");
        exit(EXIT_FAILURE);
    }
    printf("Choisissez un contrat parmi : %s\n", message.msg_text);

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
    printf("Vous avez choisi le contrat : %s\n", contrat_choisi);

    message_reponse.msg_type = client_id+10;
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
    
    preneur = strtol(message.msg_text, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Erreur : le message reçu n'est pas un entier valide (%s)\n", message.msg_text);
        exit(EXIT_FAILURE);
    }
    return preneur;
}

//fonction qui affiche le chien reçu par le preneur
void montrer_chien(int msgid, int client_id) {

    //on recoit le chien et on l'affiche
    recevoir_message(msgid,client_id,4);
}

//Le preneur peut faire son chien, en envoyant une carte à la fois. Le serveur va la valider ou non
void faire_chien_client(int msgid, int preneur) {
    struct msg_buffer message_reponse;
    bool ok = false;
    int carte;
    struct msg_buffer message_reponse2;

    //on recoit le jeu
    recevoir_message(msgid,preneur,5);

    for (int i = 0; i < 6; i++) {
        while (!ok) {
            carte = 0;
            // Demande au joueur de choisir une carte
            while ( carte <= 0 || carte > 24-i) {
                printf("Choisissez une carte à mettre dans le chien (1-%d) : ", 24-i);
                scanf("%d", &carte);
            }

            // Préparer le message avec l'index de la carte choisie
            message_reponse.msg_type = preneur+10;
            snprintf(message_reponse.msg_text, MSG_SIZE, "%d", carte);  // Convertir l'index en chaîne

            // Envoi de l'index de la carte choisie au serveur
            if (msgsnd(msgid, &message_reponse, strlen(message_reponse.msg_text) + 1, 0) == -1) {
                perror("Erreur lors de l'envoi du message");
                exit(EXIT_FAILURE);
            }

            //on recupere le mess pour savoir si la carte est bonne ou non
            if (msgrcv(msgid, &message_reponse2, MSG_SIZE, preneur , 0) == -1) {
                perror("Erreur lors de la réception du message");
                exit(EXIT_FAILURE);
            }
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
        recevoir_message(msgid,preneur,5);
    }
}

//Fonction principale, qui gère un tour de jeu
void faire_un_tour(int msgid, int joueur_id) {
    struct msg_buffer message_reponse;
    bool carte_valide;
    bool permissionJouer;
    int carte_choisie;

    for(int i = 18; i > 0 ; i--) { //on doit faire 18 tours car on a tous 18 cartes
        carte_valide = false;
        permissionJouer = false;
        carte_choisie = 0;
        //memset (&message_reponse, 0, sizeof(message_reponse));

        while (!permissionJouer) {
            if (msgrcv(msgid, &message_reponse, sizeof(message_reponse.msg_text), joueur_id, 0 ) == -1) {
                perror("Erreur lors de la réception du message");
                exit(EXIT_FAILURE);
            }
            if (strcmp(message_reponse.msg_text, "a toi") == 0) {
                printf("C'est à toi de jouer !\n");

                // Envoyer une confirmation au serveur
                message_reponse.msg_type = joueur_id+10;
                strcpy(message_reponse.msg_text, "pret");
                if (msgsnd(msgid, &message_reponse, sizeof(message_reponse.msg_text), 0) == -1) {
                    perror("Erreur lors de l'envoi de la confirmation");
                    exit(EXIT_FAILURE);
                }

                permissionJouer = true;
            }
            else printf ("Jeu en cours: %s\n", message_reponse.msg_text);
        }
        recevoir_message(msgid,joueur_id,6);

        while (!carte_valide) {
            carte_choisie = 0;
            while (carte_choisie <= 0 || carte_choisie > i) {
                printf("Choisissez une carte à jouer (1-%d) : ", i); // Ajustez 24 à la taille réelle si nécessaire
                scanf("%d", &carte_choisie);
            }

            message_reponse.msg_type = joueur_id+10;
            snprintf(message_reponse.msg_text, MSG_SIZE, "%d", carte_choisie);

            if (msgsnd(msgid, &message_reponse, strlen(message_reponse.msg_text) + 1, 0) == -1) {
                perror("Erreur lors de l'envoi de l'index de la carte");
                exit(EXIT_FAILURE);
            }

            if (msgrcv(msgid, &message_reponse, MSG_SIZE, joueur_id, 0) == -1) {
                perror("Erreur lors de la réception de la validation");
                exit(EXIT_FAILURE);
            }

            if (strcmp(message_reponse.msg_text, "valide") == 0) {
                carte_valide = true;
                printf("Carte jouée avec succès.\n");
            } else {
                printf("Carte invalide. Réessayez.\n");
            }
        }

        printf("Jeu mis à jour :\n");
        recevoir_message(msgid,joueur_id,1);
    }
}

//affiche les scores
void afficher_scores(float *scores) {
    printf("Scores des joueurs :\n");
    for (int i = 0; i < 4; i++) {
        printf("Joueur %d : %.2f\n", i + 1, scores[i]);
    }
}

//Joue uen partie, on fait 18 tours
void jouer_partie (int client_id, int msgid) {
    int preneur = 0;
    
    //jeu
    while (preneur == 0){
        recevoir_cartes(msgid, client_id);
        preneur = choix_contrat_client(msgid, client_id);
    }
    montrer_chien(msgid, client_id);
    if (client_id == preneur) faire_chien_client(msgid, preneur);

    faire_un_tour(msgid, client_id);
    
    //on recoit un message de fin
    recevoir_message_fin (msgid, client_id);   
}



int main(int argc, char *argv[]) {
    int choix;
    struct msg_buffer message;
    int client_id;


    if (argc != 2) {
        printf("Usage: %s <client_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Connexion à la mémoire partagée pour les scores
    int shmid = shmget(SHM_KEY, 4 * sizeof(float), 0666);
    if (shmid == -1) {
        perror("Erreur lors de la connexion à la mémoire partagée");
        //on affiche le message d'erreur
        return EXIT_FAILURE;
    }

    float *scores = (float *)shmat(shmid, NULL, 0);
    if (scores == (void *)-1) {
        perror("Erreur lors de l'attachement à la mémoire partagée");
        return EXIT_FAILURE;
    }

    client_id = atoi(argv[1]);
    /* on affiche un menu demandant au joueur de choisir une action:
    1- joueur une partie 
    2- afficher les scores
    3- afficher les règles
    4- quitter le jeu */
    printf("Bienvenue dans le jeu de tarot !\n");

    while (1) {
        int msgid = msgget(MSG_KEY, 0666);
        if (msgid == -1) {
            perror("Erreur lors de la connexion à la file de messages");
            printf("val msgid : %d\n", msgid);
            return EXIT_FAILURE;
        }

        printf("Que voulez-vous faire ? \n");
        printf("1- Jouer une partie\n");
        printf("2- Afficher les scores\n");
        printf("3- Afficher les règles du jeu\n");
        printf("4- Quitter le jeu\n");
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide. Veuillez entrer un numéro.\n");
            while (getchar() != '\n'); // Vide le buffer
            continue; // Recommence la boucle
        }

        while (getchar() != '\n'); // Vide les caractères restants dans le buffer
        switch (choix) {
            case 1:
                //on joue une partie, on envoie d'abord au serveur qu'on veut jouer une partie
                memset(&message, 0, sizeof(message));
                message.msg_type = 5;
                strcpy (message.msg_text, "jouer");
                if (msgsnd (msgid, &message, sizeof(message.msg_text), 0) == -1) {
                    perror("Erreur lors de l'envoi du message");
                    return EXIT_FAILURE;
                }
                jouer_partie(client_id, msgid);
                msgctl(msgid, IPC_RMID, NULL);
                break;
            case 2:
                afficher_scores(scores);
                break;
            case 3:
                //ouvrir et afficher le fichier regles.txt
                FILE *fichier = fopen("regles.txt", "r");
                if (fichier == NULL) {
                    perror("Erreur lors de l'ouverture du fichier");
                    return EXIT_FAILURE;
                }
                char ligne[100];
                while (fgets(ligne, sizeof(ligne), fichier) != NULL) {
                    printf("%s", ligne);
                }
                fclose(fichier);
                break;
            case 4:
                printf("Au revoir !\n");
                return EXIT_SUCCESS;
            default:
                printf("Choix non valide. Veuillez réessayer.\n");
        }
    }

    return EXIT_SUCCESS;

}
