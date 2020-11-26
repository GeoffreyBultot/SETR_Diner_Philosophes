//
//  Header_Prof.h
//  Semaphores_DinerDesPhilosophes
//
//  Created by Gwendal Le Vaillant on 16/09/2016.
//  Copyright © 2016 Gwendal Le Vaillant. All rights reserved.
//

#ifndef Header_Prof_h
#define Header_Prof_h



// Pour choisir quelle version compiler
#define SOLUTION_ETUDIANT
//#define SOLUTION_BASE
#define SOLUTION_1
//#define SOLUTION_2
//#define SOLUTION_3



// États stockés bêtements dans des types char
#define P_PENSE 'P'
#define P_FAIM  'F'
#define P_MANGE 'M'

#define NB_PHILOSOPHES 5

#define DUREE_MANGE_MAX_S   1.5 // en secondes
#define DUREE_PENSE_MAX_S   5.5 // idem





// ----- Variables Globales -----
// Déclarées dans Main_Prof.cpp mais utilisables dans n'importe
// quel fichier source
// ------------------------------

// Stockage de l'état des philosophes et mutex de protection nécessaire pour ce tableau
extern char* etatsPhilosophes;
extern pthread_mutex_t mutexEtats;

// Tâches exécutant la vie de chaque philosophe
extern pthread_t* threadsPhilosophes;

// Acquérir une fourchette équivaut à faire l'acquisition du sémaphore correspondant, tout simplement !
// La fouchette d'indice i est à la gauche du philosophe d'indice i
extern sem_t** semFourchettes;


// Protection de l'écriture dans la console (contre les accès simultanés)
extern pthread_mutex_t mutexCout;

// Mesure globale de temps (pour l'affichage), à initialiser au lancement des threads
extern time_t instantDebut;




// ----- Fonctions -----

// [[ vide, à compléter ]]
// /!\ Ouverture des sémaphores : avec les flags : O_CREAT | O_EXCL
void initialisation();


// Fonction à compléter pour simuler plusieurs choses :
// - le fait de prendre les fourchettes (acquérir des sémaphores associés aux fourchettes)
// - le fait de manger (simulé par un "sleep")
// - le fait de penser (simulé par un "sleep")
//
// Rappel : un philosophe passe toujours dans les états A FAIM, puis MANGE, puis PENSE, puis revient
// à l'état A FAIM, etc. (d'où la boucle while(1))
//
//
// Cette fonction implémentera la vie d'un philosophe "bien sage" : il obéira aux consignes
// d'un ordonnanceur par exemple en ajoutant des mutex ou sémaphores, etc... Ou il pourra communiquer avec
// les autres lorsque c'est nécessaire (selon la version du programme demandé). Vous devrez rajouter tout
// ce qui manque pour la synchronisation entre threads, etc.
//
// [ À compléter dans le fichier source élève ]
void* vieDuPhilosophe(void* idPtr);


// Fonction à appeler systématiquement lorsqu'un philosophe change d'état (on ne fera pas le changement
// d'état directement dans vieDuPhilosophe() )
// Et :
// - Affiche proprement l'état de tous les philosophes sur 1 ligne
// - Montre bien quel philosophe vient de subir un changement
//
// [ À compléter (rajouter les protections contre les accès concurrents) ]
void actualiserEtAfficherEtatsPhilosophes(int idPhilosophe, char nouvelEtat);


// Tout doit être nettoyé proprement ! Threads, mutex, etc...
// Et surtout sémaphores qui sinon restent actifs au niveau du S.E.
void terminerProgramme();




#endif /* Header_Prof_h */
