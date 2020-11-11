#include <iostream>
#include <ctime> // time_t time()
#include <unistd.h> // entre autres : usleep
#include <semaphore.h>
#include <pthread.h>

#include "SolutionEtudiant.hpp"

#include "Header_Prof.h"

#ifdef SOLUTION_ETUDIANT


typedef enum
{
	E_Faim = 0,
	E_Mange = 1,
	E_Pense = 2
}T_statePhilo;

//int [N_PHILOSOPHES];


sem_t sem_t_fourchettes[NB_PHILOSOPHES-1];
pthread_attr_t pthread_attr_philosophes[NB_PHILOSOPHES];
pthread_mutexattr_t pthread_attr_Cout;
pthread_mutexattr_t pthread_attr_Etats;

//pthread_t pthread_philosophes[NB_PHILOSOPHES];

void initialisation()
{
	//semFourchettes = malloc(NB_PHILOSOPHES * sizeof(sem_t*));
	threadsPhilosophes = (pthread_t*)malloc(NB_PHILOSOPHES * sizeof(pthread_t));
	semFourchettes = (sem_t**)malloc(NB_PHILOSOPHES * sizeof(sem_t*));
	etatsPhilosophes = (char*)malloc(NB_PHILOSOPHES * sizeof(char));
	#ifdef SOLUTION_1
	mutexCout = PTHREAD_MUTEX_INITIALIZER;
	mutexEtats = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mutexCout, &pthread_attr_Cout);
	pthread_mutex_init(&mutexEtats, &pthread_attr_Etats);
	instantDebut = time(NULL);
	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		semFourchettes[i] = (sem_t*)malloc(sizeof(sem_t));
		//sem
		//sem_init( semFourchettes[i], 0, 1);
		sem_init( semFourchettes[i], 0, 1);
		pthread_create( &threadsPhilosophes[i], &(pthread_attr_philosophes[i]), vieDuPhilosophe, &i);

		usleep(10);
	}


	#endif

	#ifdef SOLUTION_2


	#endif
	#ifdef SOLUTION_3


	#endif

}


void randomDelay(float f_min_Sec, float f_max_Sec)
{
	int random;

	srand(time(NULL));
	random = (rand())%((int)f_max_Sec*1000000);
	if( (float)random < (f_min_Sec*1000000))
		random = int(f_min_Sec*1000000);
	usleep(random);

/*	if(random > f_max_Sec)
		random = f_max_Sec;

*/
}


void* vieDuPhilosophe(void* idPtr)
{
    int id = * ((int*)idPtr);
    T_statePhilo statePhilo = E_Pense;
    // Configuration du thread : il sera annulable à partir de n'importe quel point de préemption
    // (appel bloquant, appel système, etc...)
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    while(1)
    {
        //std::cout << "thread : " << id << std::endl;

        switch(statePhilo)
        {
    	case E_Faim:


    		actualiserEtAfficherEtatsPhilosophes(id,'F');
    		sem_wait(semFourchettes[id]); //Acquisition fourchette gauche
    		//usleep(10);
    		sem_wait(semFourchettes[ (id+1)%NB_PHILOSOPHES]); //Acquisition fourchette droite
    		sem_post(semFourchettes[id]); //Relâchement fourchette gauche
    		sem_post(semFourchettes[(id+1)%NB_PHILOSOPHES]); //Relâchement fourchette droite
    		statePhilo = E_Mange;
    		break;

    	case E_Mange:
    		actualiserEtAfficherEtatsPhilosophes(id,'M');
    		randomDelay(0,DUREE_MANGE_MAX_S);
    		statePhilo = E_Pense;
    		break;
    	case E_Pense:
    		actualiserEtAfficherEtatsPhilosophes(id,'P');
    		randomDelay(0,DUREE_PENSE_MAX_S);
    		statePhilo = E_Faim;
    		break;
        }





        // ***** À implémenter : *****
        // - structure permettant le contrôle du philosphe
        // - prise/relâchement des fourchettes de gauche et de droite, au bon moment
        // - ordres de changement d'état et d'actualisation de l'affichage dans la foulée
        //     (grâce à : void actualiserEtAfficherEtatsPhilosophes(int, char))
        // - simulation des actions "manger" et "penser" par des appels à usleep(...)
        
        
        
        
        pthread_testcancel();
        //TODO pthread_testcancel(); // point où l'annulation du thread est permise
    }
    
    return NULL;
}




void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
    // ***** À IMPLÉMENTER : PROTECTION ÉTATS *****
    // ***** À IMPLÉMENTER : PROTECTION CONSOLE *****
    

	pthread_mutex_lock(&mutexEtats);
    etatsPhilosophes[idPhilosopheChangeant] = nouvelEtat;
    pthread_mutex_unlock(&mutexEtats);

    pthread_mutex_lock(&mutexCout);
    //std::cout << "oui" <<std::endl;
    
    for (int i=0;i<NB_PHILOSOPHES;i++) {
        if (i==idPhilosopheChangeant)
            std::cout << "*";
        else
            std::cout << " ";
        std::cout << etatsPhilosophes[i];
        if (i==idPhilosopheChangeant)
            std::cout << "* ";
        else
            std::cout << "  ";
    }
    std::cout << "          (t=" << difftime(time(NULL), instantDebut) << ")" << std::endl;

    pthread_mutex_unlock(&mutexCout);
}




void terminerProgramme()
{
    int i;
    for(i=0;i<NB_PHILOSOPHES;i++)
    {
    	pthread_cancel(threadsPhilosophes[i]);
    	//std::cout << pthread_philosophes[i] <<std::endl;
    	pthread_join(threadsPhilosophes[i], NULL);
    	sem_close(semFourchettes[i]);
    	//sem_unlink(semFourchettes[i]);
    }
    free(threadsPhilosophes);
    free(semFourchettes);
}


#endif
