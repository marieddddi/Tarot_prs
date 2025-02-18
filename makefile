CC = gcc
CFLAGS = -Wall -g

# Fichiers sources
SRCS_SERVER = serveur.c fonctions.c
SRCS_JOUEUR = joueur.c fonctions.c
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_JOUEUR = $(SRCS_JOUEUR:.c=.o)

# Liste des exécutables
EXES = serveur joueur

# Cible par défaut
all: $(EXES)

# Règle pour l'exécutable serveur
serveur: $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o serveur $(OBJS_SERVER)

# Règle pour l'exécutable joueur
joueur: $(OBJS_JOUEUR)
	$(CC) $(CFLAGS) -o joueur $(OBJS_JOUEUR)

# Règle générique pour compiler les fichiers .o du serveur
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Cible pour nettoyer les fichiers générés
clean:
	rm -f $(OBJS_SERVER) $(OBJS_JOUEUR) $(EXES)
