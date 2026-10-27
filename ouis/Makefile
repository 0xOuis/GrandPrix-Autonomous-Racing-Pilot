CC     = gcc
CFLAGS = -ansi -pedantic -Wall -Wextra

# Fichiers du projet complet (décommenter quand les autres parties seront prêtes)
# SRCS = main.c parser.c physics.c strategy.c follow_line.c
# EXEC = pilote

# Pour l'instant : juste les tests de physics
TEST_SRCS = test_physics.c physics.c
TEST_EXEC = test_physics

# Note : follow_line.c doit être dans ce dossier (copié depuis l'archive GDC)
# Décommenter les lignes suivantes quand vous l'aurez copié :
# TEST_SRCS += follow_line.c
# CFLAGS    += -I.

test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_SRCS) common.h physics.h
	$(CC) $(CFLAGS) -o $(TEST_EXEC) $(TEST_SRCS)

clean:
	rm -f $(TEST_EXEC) pilote *.o

.PHONY: test clean
