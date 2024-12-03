
#include <stdio.h>
#include <string.h>

// Structure d'une carte
struct carte {
    char couleur;   // Couleur : 'C' (Cœur), 'D' (Carreau), 'T' (Trèfle), 'P' (Pique), 'A' (Atout), 'E' (Excuse)
    char valeur[3]; // Valeur : "1" à "10", "V" (Valet), "C" (Cavalier), "D" (Dame), "R" (Roi), ou atouts/excuse
    float points;   // Points de la carte
};

// Structure d'un paquet
struct paquet {
    struct carte jeu[78]; // Jeu de cartes (78 au total)
    int nb_cartes;        // Nombre de cartes dans le paquet
};

// Initialisation d'une carte vide
void init_carte(struct carte *carte) {
    carte->couleur = '0';
    carte->valeur[0] = '0';
    carte->valeur[1] = '\0'; // La chaîne doit être terminée par '\0'
    carte->points = 0.0;
}

// Création du jeu de tarot
void creer_jeu(struct carte jeu[78]) {
    // Couleurs classiques du tarot
    char couleurs[4] = {'C', 'D', 'T', 'P'}; // Cœur, Carreau, Trèfle, Pique
    int index = 0;

    // Points des cartes classiques
    float points_classiques[14] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.5, 2.5, 3.5, 4.5};

    // Création des cartes classiques
    for (int i = 0; i < 4; i++) { // Pour chaque couleur
        for (int valeur = 1; valeur <= 14; valeur++) { // Valeurs de 1 à 14
            jeu[index].couleur = couleurs[i];
            if (valeur <= 10) {
                snprintf(jeu[index].valeur, sizeof(jeu[index].valeur), "%d", valeur);
            } else {
                // Utilisation des lettres pour les figures
                switch (valeur) {
                    case 11: strcpy(jeu[index].valeur, "V"); break; // Valet
                    case 12: strcpy(jeu[index].valeur, "C"); break; // Cavalier
                    case 13: strcpy(jeu[index].valeur, "D"); break; // Dame
                    case 14: strcpy(jeu[index].valeur, "R"); break; // Roi
                }
            }
            jeu[index].points = points_classiques[valeur - 1];
            index++;
        }
    }

    // Création des atouts (valeurs de 1 à 21)
    for (int valeur = 1; valeur <= 21; valeur++) {
        jeu[index].couleur = 'A'; // 'A' pour Atout
        snprintf(jeu[index].valeur, sizeof(jeu[index].valeur), "%d", valeur);
        jeu[index].points = (valeur == 1 || valeur == 21) ? 4.5 : 0.5; // Petit et 21 valent 4.5 points
        index++;
    }

    // Création de l'excuse
    jeu[index].couleur = 'E'; // 'E' pour Excuse
    strcpy(jeu[index].valeur, "X");
    jeu[index].points = 4.5;
}

// Création d'un paquet de cartes
void creer_paquet(struct paquet *p) {
    p->nb_cartes = 78;
    creer_jeu(p->jeu);
}

// Suppression d'une carte d'un paquet
void supprimer_carte(struct paquet *p, int index) {
    if (index < 0 || index >= p->nb_cartes) return; // Index invalide
    for (int i = index; i < p->nb_cartes - 1; i++) {
        p->jeu[i] = p->jeu[i + 1];
    }
    p->nb_cartes--;
}

// Ajout d'une carte à un paquet
void ajouter_carte(struct paquet *p, struct carte c) {
    if (p->nb_cartes >= 78) return; // Paquet plein
    p->jeu[p->nb_cartes] = c;
    p->nb_cartes++;
}

// Couper un paquet de cartes
void couper_paquet(struct paquet *p, int position) {
    if (position <= 0 || position >= p->nb_cartes) return; // Position invalide
    struct carte temp[78];
    int index = 0;

    for (int i = position; i < p->nb_cartes; i++) {
        temp[index++] = p->jeu[i];
    }
    for (int i = 0; i < position; i++) {
        temp[index++] = p->jeu[i];
    }
    memcpy(p->jeu, temp, sizeof(temp));
}

// Calculer le score d'un paquet
float calculer_score(struct paquet *p) {
    float score = 0.0;
    for (int i = 0; i < p->nb_cartes; i++) {
        score += p->jeu[i].points;
    }
    return score;
}

// remporter un pli