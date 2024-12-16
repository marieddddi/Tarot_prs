CC = gcc
CFLAGS = -Wall -g

# Fichiers sources
SRCS_SERVER = serveur.c fonctions.c
SRCS_CLIENT = client.c fonctions.c
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)

# Liste des exécutables
EXES = serveur client

# Cible par défaut
all: $(EXES)

# Règle pour l'exécutable serveur
serveur: $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o serveur $(OBJS_SERVER)

# Règle pour l'exécutable client
client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o client $(OBJS_CLIENT)

# Règle générique pour compiler les fichiers .o du serveur
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Cible pour nettoyer les fichiers générés
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(EXES)
