#include "fonctions.h"  


// Initialisation de la structure d'une carte
void init_carte(struct carte *carte){
    carte->couleur = '0';
    carte->valeur[0] = '0';
    carte->valeur[1] = '\0';  // La chaîne de caractères doit être terminée par un \0
    carte->point = 0.0;
}

// Création de l'ensemble des cartes, le jeu de tarot
void creer_jeu(struct carte jeu[78]){
    int i = 0;
    int j = 0;
    char couleurs[4] = {'C', 'K', 'P', 'T'};  // K = carreau, P = pique, T = trefle, C = coeur
    char* valeurs[14] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "V", "C", "D", "R"};  // Valeurs sous forme de chaînes de caractères
    float points[14] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.5, 2.5, 3.5, 4.5};  // Points associés à chaque valeur
    char* atouts[22] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "*"}; // '*' = excuse
    float points_atouts[22] = {4.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 4.5, 4.5};  // Points associés à chaque atout
    
    // Création des cartes de couleurs
    for(i = 0; i < 4; i++){
        for(j = 0; j < 14; j++){
            jeu[i * 14 + j].couleur = couleurs[i];
            strcpy(jeu[i * 14 + j].valeur, valeurs[j]);  // Copie de la valeur dans la structure
            jeu[i * 14 + j].point = points[j];
        }
    }

    // Ajout des atouts
    for(i = 0; i < 22; i++){
        jeu[56 + i].couleur = ' ';  // Couleur vide représentée par un espace
        strcpy(jeu[56 + i].valeur, atouts[i]);  // Copie de la valeur de l'atout dans la structure
        jeu[56 + i].point = points_atouts[i];
    }
}

//creer un paquet de 78 cartes
void creer_paquet (struct paquet *p){
    p->nb_cartes = 78;
    creer_jeu(p->jeu);
}

//Savoir si la carte est un atout
bool est_atout(struct carte *carte){
    if (carte->couleur == ' '){
        return true;
    }
    return false;
}

//Savoir si la carte est une carte de couleur donnée
bool est_meme_couleur (struct carte *carte, char couleur){
    if (carte->couleur == couleur){
        return true;
    }
    return false;
}

//Savoir si le paquet possède une carte de couleur donnée
bool possede_couleur(struct paquet *p, char couleur){
    for (int i = 0; i < p->nb_cartes; i++){
        if (p->jeu[i].couleur == couleur){
            return true;
        }
    }
    printf ("Vous ne possedez pas de carte de cette couleur\n");
    return false;
}

//convertir un char en entier
int valeur_en_int(const char *valeur) {
    // Convertit une chaîne de caractères en entier
    return atoi(valeur);
}

//convertir un char en index
int valeur_en_index(const char *valeur) {
    const char *ordre[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "V", "C", "D", "R"};
    int taille = sizeof(ordre) / sizeof(ordre[0]);

    for (int i = 0; i < taille; i++) {
        if (strcmp(valeur, ordre[i]) == 0) {
            return i; // Retourne l'index correspondant à la valeur
        }
    }

    return -1; // Retourne -1 si la valeur n'est pas valide
}

//fonction pour savoir si on peut jouer une carte, si elle respecte les règles
bool accepter_carte(struct carte *carteLaplusForte, struct carte *carteActuelle, struct paquet *paquet, char couleurJouee){
    //si precedent nul accepte
    if (carteLaplusForte->couleur == 0){
        printf ("Carte precedente nulle\n");
        return true;
    }
    if (strcmp(carteActuelle->valeur, "*")==0) return true;
    //si couleur identique et que ce n'est pas un atout on accepte
    if (! est_atout(carteActuelle) && est_meme_couleur(carteActuelle, couleurJouee)){
        return true;
    }

    //si on a pas de carte de la meme couleur et que c'est un atout
    if (!est_atout(carteLaplusForte) && !possede_couleur(paquet, couleurJouee) && est_atout(carteActuelle)){
        return true;
    }

    //si on a pas de carte de la meme couleur et qu'on a pas d'atout
    if (! possede_couleur(paquet, carteLaplusForte->couleur) && ! possede_couleur(paquet, ' ')){
        return true;
    }

    // si on a un atout et que la carte actuelle est un atout, on regarde bien que la carte actuelle est plus forte
    if (est_atout(carteLaplusForte) && est_atout(carteActuelle)){
        //on affiche la carte precedente et la carte actuelle
        int valeurActuelle = valeur_en_int(carteActuelle->valeur);
        int valeurForte = valeur_en_int(carteLaplusForte->valeur);
        if (valeurActuelle > valeurForte){
            return true;
        }
        //si on peut mettre qu'un atout plus faible
        for (int i = 0; i < paquet->nb_cartes; i++){
            if (est_atout(&paquet->jeu[i]) && valeur_en_int(paquet->jeu[i].valeur) > valeurForte){
                return false;
            }
        }
        return true;
    }
    printf ("Vous ne pouvez pas jouer cette carte\n");
    return false;
}

//fonction pour savoir qui a la plus forte carte
int qui_a_la_plus_forte_carte(struct carte *carteLaPlusForte, struct carte *carteActuelle, char couleurJouee) {
    //On convertit les chars en entier
    int valeurActuelle = valeur_en_index(carteActuelle->valeur);
    int valeurForte = valeur_en_index(carteLaPlusForte->valeur);

    int valeurActuelleAtout = valeur_en_int(carteActuelle->valeur);
    int valeurForteAtout = valeur_en_int(carteLaPlusForte->valeur);

    printf ("valeurActuelle : %d, valeurForte : %d\n", valeurActuelle , valeurForte);
    
    //si precedent nul accepte
    if (carteLaPlusForte->couleur == 0) {
        return 1;
    }
    if (strcmp(carteActuelle->valeur,"*")==0){
        return 0;
    }
    //si c'est la bonne couleur, on regarde qui a la plus forte carte (selon la valeur)
    if (carteActuelle->couleur == couleurJouee &&
        !est_atout(carteLaPlusForte) && 
        !est_atout(carteActuelle) &&
        valeurActuelle > valeurForte &&
        carteLaPlusForte->couleur==couleurJouee) {
        return 1;
    }

    if (carteActuelle->couleur== couleurJouee &&
        est_atout(carteLaPlusForte) && 
        est_atout(carteActuelle) &&
        valeurActuelleAtout > valeurForteAtout &&
        carteLaPlusForte->couleur==couleurJouee) {
        return 1;
    }

    if (!est_atout(carteLaPlusForte) && !est_atout(carteActuelle) && carteActuelle->couleur==couleurJouee && valeurActuelle <valeurForte && carteLaPlusForte->couleur==couleurJouee){
        return 0;
    }
    //si on joue un atout, on gagne
    if (est_atout(carteActuelle) && !est_atout(carteLaPlusForte)) {
        return 1;
    }
    return 0;
}

//calcul des points d'un paquet
float calculer_points(struct paquet *paquet){
    float points = 0.0;
    for (int i = 0; i < paquet->nb_cartes; i++){
        points += paquet->jeu[i].point;
        printf ("points: %f", points);
    }
    return points;
}

//definir le plus grand contrat: si que 0 on refait un tour !
float contrat (char *choix_contrat){
    if (strcmp(choix_contrat, "Passe") == 0){
        return 0;
    }
    else if (strcmp(choix_contrat, "Petite") == 0){
        return 1;
    }
    else if (strcmp(choix_contrat, "Garde") == 0){
        return 2;
    }
    return 3; //3 = erreur 
}

//score que l'on a fait, on doit voir si on a des bouts dans notre jeu -> a utiliser a la fin de la partie car on peut recuperer des bouts (1)
float score(struct paquet *paquet){
    int nb_bout = 0;
    float nb_points = 0.0;
    for (int i = 0; i < paquet->nb_cartes; i++){
        if ((strcmp(paquet->jeu[i].valeur,"1")==0 && paquet->jeu[i].couleur==' ') || (strcmp(paquet->jeu[i].valeur,"21")==0 && paquet->jeu[i].couleur==' ') || (strcmp(paquet->jeu[i].valeur,"*")==0 && paquet->jeu[i].couleur==' ')){
            nb_bout += 1;
        }
    }
    
    //on definit le nombre de points a atteindre
    switch (nb_bout)
    {
    case 1:
        nb_points = 51.0;
        break;
    case 2:
        nb_points = 41.0;
        break;
    case 3:
        nb_points = 36.0;
        break;
    
    default:
        nb_points = 56.0;
        break;
    }
    return (calculer_points(paquet) - nb_points);
}

//score final de la partie
float score_final(struct paquet *paquet, char *choix_contrat){
    return (25.0 + score(paquet)) * contrat(choix_contrat);
}

// Fonction qui permet de distribuer les cartes
void distribuer_cartes(struct paquet *jeu, struct paquet *j1, struct paquet *j2, struct paquet *j3, struct paquet *j4, struct paquet *chien) {
    // Mélange du jeu
    srand(time(NULL));  // Initialisation du générateur de nombres aléatoires pour distribuer les cartes aléatoirement
    for (int i = 0; i < jeu->nb_cartes; i++) {
        int j = rand() % jeu->nb_cartes;  // Sélection aléatoire d'une carte
        struct carte temp = jeu->jeu[i];  // Echange des cartes
        jeu->jeu[i] = jeu->jeu[j];
        jeu->jeu[j] = temp;
    }

    // Initialisation des paquets des joueurs et du chien
    j1->nb_cartes = 0;
    j2->nb_cartes = 0;
    j3->nb_cartes = 0;
    j4->nb_cartes = 0;
    chien->nb_cartes = 0;

    // Distribution des cartes
    for (int i = 0; i < jeu->nb_cartes; i++) {
        if (i < 18) {  // 18 cartes pour le joueur 1
            j1->jeu[j1->nb_cartes++] = jeu->jeu[i];
        }
        else if (i < 36) {  // 18 cartes pour le joueur 2
            j2->jeu[j2->nb_cartes++] = jeu->jeu[i];
        }
        else if (i < 54) {  // 18 cartes pour le joueur 3
            j3->jeu[j3->nb_cartes++] = jeu->jeu[i];
        }
        else if (i < 72) {  // 18 cartes pour le joueur 4
            j4->jeu[j4->nb_cartes++] = jeu->jeu[i];
        }
        else {  // Les cartes restantes pour le chien
            chien->jeu[chien->nb_cartes++] = jeu->jeu[i];
        }
    }
}

//fonction pour afficher le paquet
void afficher_paquet(struct paquet *paquet) {
    for (int i = 0; i < paquet->nb_cartes; i++) {
        printf("%c %s \n", paquet->jeu[i].couleur, paquet->jeu[i].valeur);
    }
}

