#include <iostream>
#include <ctime> // time_t time()
#include <unistd.h> // entre autres : usleep
#include <semaphore.h>
#include <pthread.h>

#include "SolutionEtudiant.hpp"

#include "Header_Prof.h"

#ifdef SOLUTION_ETUDIANT


void ViePhilosopheSolution1(int id);
void ViePhilosopheSolution2(int id);


#ifdef SOLUTION_1
pthread_attr_t pthread_attr_Sol1Scheduler;
pthread_t Sol1_threadScheduler;
pthread_mutex_t mutex_Eat_Philosophes[NB_PHILOSOPHES];
pthread_mutexattr_t mutexattr_Eat_Philosophes[NB_PHILOSOPHES];
sem_t semSynchroThreadsPhilos;

void* sol1_Master_Scheduler(void* args);

#endif

sem_t sem_t_fourchettes[NB_PHILOSOPHES-1];
pthread_attr_t pthread_attr_philosophes[NB_PHILOSOPHES];

pthread_mutexattr_t pthread_attr_Cout;
pthread_mutexattr_t pthread_attr_Etats;
int * t_idx_philos;

void initialisation()
{
	sched_param schedparam;
	int PID = getpid();
	threadsPhilosophes = (pthread_t*)malloc(NB_PHILOSOPHES * sizeof(pthread_t));
	semFourchettes = (sem_t**)malloc(NB_PHILOSOPHES * sizeof(sem_t*));
	etatsPhilosophes = (char*)malloc(NB_PHILOSOPHES * sizeof(char));
	t_idx_philos = (int*)malloc(NB_PHILOSOPHES *sizeof(int));

	/*HIGH PRIORITY AND SCHED FIFO*/
	sched_getparam(PID,&schedparam);
	schedparam.sched_priority = 1;
	sched_setscheduler(PID,SCHED_FIFO,&schedparam);

	mutexCout = PTHREAD_MUTEX_INITIALIZER;
	mutexEtats = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mutexCout, &pthread_attr_Cout);
	pthread_mutex_init(&mutexEtats, &pthread_attr_Etats);

	instantDebut = time(NULL);

	/*Create synchro threads philos*/
#ifdef SOLUTION_1
	if( sem_init( &semSynchroThreadsPhilos, 0, 0) == 0)//SemSynchro semaphores pris par defaut
		std::cout << "[INFO] semaphore semSynchroThreadsPhilos initialized" << std::endl;
	else
		std::cout << "[WARNING] semaphore semSynchroThreadsPhilos not initialized " << std::endl;
#endif

	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		/*Init mutex eat philos*/
		mutex_Eat_Philosophes[i] = PTHREAD_MUTEX_INITIALIZER;
		if( pthread_mutex_init(&mutex_Eat_Philosophes[i], &mutexattr_Eat_Philosophes[i]) == 0)//Fourchettes lachees par defaut
			std::cout << "[INFO] semaphore semSynchroThreadsPhilos initialized" << std::endl;
		else
			std::cout << "[WARNING] semaphore semSynchroThreadsPhilos not initialized " << std::endl;

		/*Init sem forks philos*/
		semFourchettes[i] = (sem_t*)malloc(sizeof(sem_t));
		if( sem_init( semFourchettes[i], 0, 1) == 0)//Fourchettes lachees par defaut
			std::cout << "[INFO] semaphore Fourchette " <<i<<" initialized" << std::endl;
		else
			std::cout << "[WARNING] semaphore Fourchette " <<i<<" not initialized " << std::endl;
		t_idx_philos[i] = i;

		/*Create threads philosophes*/
		if(pthread_create( &threadsPhilosophes[i], &(pthread_attr_philosophes[i]), vieDuPhilosophe, &(t_idx_philos[i])) == 0)
			std::cout<<"[INFO] Thread philosophe " <<i<< " created"<<std::endl;
		else
			std::cout<<"[WARNING] Thread philosophe " <<i<< " not created"<<std::endl;
	}

#ifdef SOLUTION_1
	/*create scheduler thread*/
	if( pthread_create(&Sol1_threadScheduler, &pthread_attr_Sol1Scheduler, sol1_Master_Scheduler, NULL)==0)
		std::cout << "[INFO] Thread Sol1_threadScheduler created" << std::endl;
	else
		std::cout << "[WARNING] Thread Sol1_threadScheduler not created " << std::endl;

	bool TousPense = false;

	while(TousPense ==false)
	{
		TousPense =true;
		for(int i = 0;i<NB_PHILOSOPHES;i++)
		{
			if(etatsPhilosophes[i] != P_PENSE)
			{
				TousPense  = false;
			}
		}

	}
	sem_post(&semSynchroThreadsPhilos);


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
}


void* vieDuPhilosophe(void* idPtr)
{
	int id = * ((int*)idPtr);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    actualiserEtAfficherEtatsPhilosophes(id, P_PENSE);

    while(1)
    {
#ifdef SOLUTION_1
    	ViePhilosopheSolution1(id);
#endif
#ifdef SOLUTION_2
    	ViePhilosopheSolution2(id);
#endif
        pthread_testcancel();
    }
    return NULL;
}

void* sol1_Master_Scheduler(void* args)
{
	bool tousFaim = false;
	bool tousFiniDeManger = false;

	int i,starti = 0;
	int philosopheExcluSiImpair = NB_PHILOSOPHES;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);


	for(i =  0; i <NB_PHILOSOPHES ; i++){
		pthread_mutex_lock(&mutex_Eat_Philosophes[i]);
		std::cout<<"mutex locked : " << i << std::endl;
	}

	sem_post(&semSynchroThreadsPhilos);
	while(1)
	{
		std::cout << "check si faim"<<std::endl;
		//Attendre que tous les philosophes aient faim.
		tousFaim = false;
		while(tousFaim == false){
			tousFaim = true;
			//On fait tous les philos paires ou tous les impaires selon starti
			for(int i=starti;i<NB_PHILOSOPHES;i+=2)
			{
				if(i != philosopheExcluSiImpair)
				{
					if(etatsPhilosophes[i]!=P_FAIM)
						tousFaim=false;
					}
				}
			}

		//Tous les philosophes ont faim
		//std::cout<<"[DEBUG] TOUS FAIM "<<std::endl;
		for(i =  starti ; i <NB_PHILOSOPHES ; i+=2){
			if(i!=philosopheExcluSiImpair){
				pthread_mutex_unlock(&mutex_Eat_Philosophes[i]);
				//std::cout << "mutex unlock : " << i << std::endl;
			}
		}


		//Tous les philosophes vont manger
		//std::cout<<"[DEBUG] Ils VONT MANGER"<<std::endl;

		tousFiniDeManger = false;
		while(tousFiniDeManger  == false){
			tousFiniDeManger = true;
			for(int i=starti;i<NB_PHILOSOPHES;i+=2){
				if(i != philosopheExcluSiImpair){
					if(etatsPhilosophes[i]==P_MANGE)
						tousFiniDeManger = false;
					}
				}
			}


		//std::cout<<"[DEBUG] FINI DE MANGER"<<std::endl;
		for(i =  starti; i <NB_PHILOSOPHES; i+=2){
			if(i!=philosopheExcluSiImpair){
				pthread_mutex_lock(&mutex_Eat_Philosophes[i]);
				//std::cout << "mutex Lock : " << i << std::endl;
			}
		}

		if(starti == 0)
		{
			starti = 1;
			//std::cout<<"C EST AU TOUT DES IMPAIRES"<<std::endl;
		}
		else
		{
			starti = 0;
			//std::cout<<"C EST AU TOUT DES PAIRES"<<std::endl;if(NB_PHILOSOPHES % 2) //si on a un nombre impaire de philosophes
			if(NB_PHILOSOPHES%2)
			{//Choix d'un philosophe qui va être exclu de la table. Le premier ou le dernier.
				if(philosopheExcluSiImpair  == 0)
					philosopheExcluSiImpair = NB_PHILOSOPHES-1;
				else
					philosopheExcluSiImpair = 0;
				std :: cout << "excluded = " << philosopheExcluSiImpair <<std::endl;
			}
		}

		pthread_testcancel();

	}
}

void ViePhilosopheSolution1(int id)
{
	int value_sema = 0;
	while(value_sema == 0)//Tant que le semaphore est pris
		sem_getvalue(&semSynchroThreadsPhilos, &value_sema);

	actualiserEtAfficherEtatsPhilosophes(id,P_FAIM);
	pthread_mutex_lock(&mutex_Eat_Philosophes[id]);
	sem_wait(semFourchettes[id]); //Acquisition fourchette gauche
	sem_wait(semFourchettes[ (id+1)%NB_PHILOSOPHES]); //Acquisition fourchette droite

	//std::cout<<"[debug] pilo : "<< id <<" mange" << std::endl;
	actualiserEtAfficherEtatsPhilosophes(id,P_MANGE);

	randomDelay(0,DUREE_MANGE_MAX_S);
	pthread_mutex_unlock(&mutex_Eat_Philosophes[id]);

	sem_post(semFourchettes[id]); //Relâchement fourchette gauche
	sem_post(semFourchettes[(id+1)%NB_PHILOSOPHES]); //Relâchement fourchette droite

	actualiserEtAfficherEtatsPhilosophes(id,P_PENSE);
	randomDelay(0,DUREE_PENSE_MAX_S);
}

void ViePhilosopheSolution2(int id)
{

}


void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
	pthread_mutex_lock(&mutexEtats);
    etatsPhilosophes[idPhilosopheChangeant] = nouvelEtat;
    pthread_mutex_unlock(&mutexEtats);

    pthread_mutex_lock(&mutexCout);
    
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
    int sem_value;
    sem_close(&semSynchroThreadsPhilos);


	pthread_cancel(Sol1_threadScheduler);
    std::cout<< "Sol1_threadScheduler to join"<<std::endl;
	pthread_join(Sol1_threadScheduler, NULL);
    std::cout<< "Sol1_threadScheduler joined"<<std::endl;

    for(i=0;i<NB_PHILOSOPHES;i++)
    {
    	pthread_mutex_destroy(&mutex_Eat_Philosophes[i]);
    	if(pthread_cancel(threadsPhilosophes[i]) == 0)
			std::cout << "[INFO] thread philosophe " << i << " canceled successfull"<<std::endl;
		else
			std::cout << "[WARNING] thread philosophe " << i << " error during cancel() "<<std::endl;

    	if(pthread_join(threadsPhilosophes[i], NULL) == 0)
    		std::cout << "[INFO] thread philosophe " << i << " join() correctly"<<std::endl;
    	else
    		std::cout << "[WARNING] thread philosophe " << i << " error during join() "<<std::endl;
    	sem_getvalue(&sem_t_fourchettes[i], &sem_value);
    	if( sem_value == 0)
    	{
    		std::cout << "[INFO] Semaphore fourchette " << i << "not posted" << std::endl;
    		sem_post(&sem_t_fourchettes[i]);
    	}

    	if(sem_close(semFourchettes[i]) == 0)
    		std::cout << "[INFO] semaphore fourchette " << i << " close correctly"<<std::endl;
    	else
    		std::cout << "[WARNING] semaphore fourchette " << i << " error during close() "<<std::endl;
    }
    free(threadsPhilosophes);
    free(semFourchettes);
    free(etatsPhilosophes);
    free(t_idx_philos);
}


#endif
