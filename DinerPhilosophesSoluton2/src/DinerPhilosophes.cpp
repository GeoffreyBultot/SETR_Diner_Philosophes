// Attention : Ce fichier ne doit pas être modifié


#include <iostream>
#include <ctime> // time_t time()
#include <unistd.h> // entre autres : usleep
#include <semaphore.h>

#include <signal.h> // gestion des signaux (pour nous : fermeture du programme)

#include "Header_Prof.h"



// Déclaration effective des globales
char* etatsPhilosophes = NULL;
pthread_mutex_t mutexEtats;
pthread_t* threadsPhilosophes = NULL;
sem_t** semFourchettes = NULL;
pthread_mutex_t mutexCout;
time_t instantDebut;

// Prototypes
void generalSignalHandler(int);





int main(int argc, const char * argv[]) {

    // Infos utiles
    std::cout << "Diner des Philosophes. PID = " << getpid() << std::endl; // !!! breakpoint avec commande !!!
#ifdef SOLUTION_1
    std::cout << "Solution 1 : ordonnanceur maître en scrutation périodique" << std::endl;
#endif
#ifdef SOLUTION_2
    std::cout << "Solution 2 : ordonnanceur asynchrone, 1 seule fonction d'ordonnancement appelée depuis chaque tâche" << std::endl;
#endif


    // - - - - - Mise en place du handler de signal - - - - -
    /// Le signal SIGTERM est le signal par défaut envoyé par la commande kill
    /// Pour arrêter ce programme proprement (même s'il est actuellement en cours de débug dans Eclipse et donc contrôlé par GDB),
    /// il suffira d'exécuter la commande 'kill PID_DE_VOTRE_PROCESSUS' dans un terminal

    // Création et initialisation de la structure POSIX
    struct sigaction signalAction;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;
    // Fonction appelée lors de la signalisation
    signalAction.sa_handler = &generalSignalHandler;
    // Interception de SIGTERM uniquement
    if (sigaction(SIGTERM, &signalAction, NULL) == -1) // si erreur
        std::cerr << "Impossible d'intercepter SIGTERM !" << std::endl;



    // - - - - - Fonction à compléter - - - - -
    initialisation();

    // - - - - - Attente infinie - - - - -
    while(1)
        sleep(60);

    return 0;
}




// Pourrait intercepter et gérer tous les signaux, selon la configuration
void generalSignalHandler(int signal)
{
    if (signal == SIGTERM)
    {
        std::cout << std::endl << "Exécution de terminerProgramme() en cours..." << std::endl;
        terminerProgramme();
        std::cout << "Fin." << std::endl;
        exit(0);
    }
    else
        std::cerr << "Signal non-géré" << std::endl;
}
