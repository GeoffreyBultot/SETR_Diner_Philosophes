#include <iostream>
#include <ctime> // time_t time()
#include <unistd.h> // entre autres : usleep
#include <semaphore.h>
#include <pthread.h>

#include "SolutionEtudiant.hpp"

#include "Header_Prof.h"

#include <sys/time.h>
#include "string.h"
#include "sys/times.h"
pthread_attr_t pthread_attr_Sol1Scheduler;
pthread_t Sol1_threadScheduler;

sem_t semSynchroThreadsPhilos;

void* sol1_Master_Scheduler(void* args);

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
	if( sem_init( &semSynchroThreadsPhilos, 0, 0) == 0)//SemSynchro semaphores pris par defaut
		std::cout << "[INFO] semaphore semSynchroThreadsPhilos initialized" << std::endl;
	else
		std::cout << "[WARNING] semaphore semSynchroThreadsPhilos not initialized " << std::endl;

	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
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

int run_thread_eat[NB_PHILOSOPHES] = {0};
pthread_mutex_t run_lock[NB_PHILOSOPHES] = {PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t run_cond = PTHREAD_COND_INITIALIZER;
struct timeval t_second[NB_PHILOSOPHES], t_first[NB_PHILOSOPHES];

void* vieDuPhilosophe(void* idPtr)
{
	int id = * ((int*)idPtr);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    actualiserEtAfficherEtatsPhilosophes(id, P_PENSE);
    int gauche = id;
    int droite = (id+1)%NB_PHILOSOPHES;
    while(1)
    {
    	int value_sema = 0;
		while(value_sema == 0)//Tant que le semaphore est pris
			sem_getvalue(&semSynchroThreadsPhilos, &value_sema);

		gettimeofday(&t_first[id],NULL);
		actualiserEtAfficherEtatsPhilosophes(id,P_FAIM);

		pthread_mutex_lock(&run_lock[id]);
		while (!run_thread_eat[id]){
			pthread_cond_wait(&run_cond, &run_lock[id]);
		}
		run_thread_eat[id] = 0;
		pthread_mutex_unlock(&run_lock[id]);
		gettimeofday(&t_second[id],NULL);
		int diff = (t_second[id].tv_sec - t_first[id].tv_sec) * 1000000 + t_second[id].tv_usec - t_first[id].tv_usec;
		pthread_mutex_lock(&mutexCout);
		std::cout <<id<<";"<< diff << std::endl;
		pthread_mutex_unlock(&mutexCout);

		sem_wait(semFourchettes[gauche]); //Acquisition fourchette gauche
		sem_wait(semFourchettes[droite]); //Acquisition fourchette droite
		actualiserEtAfficherEtatsPhilosophes(id,P_MANGE);
		randomDelay(0,DUREE_MANGE_MAX_S);
		sem_post(semFourchettes[gauche]); //Relâchement fourchette gauche
		sem_post(semFourchettes[droite]); //Relâchement fourchette droite

		actualiserEtAfficherEtatsPhilosophes(id,P_PENSE);
		randomDelay(0,DUREE_PENSE_MAX_S);
        pthread_testcancel();
    }
    return NULL;
}

void* sol1_Master_Scheduler(void* args)
{
	bool tousFaim = false;
	bool tousFiniDeManger = false;

	int i,GroupeQuiPeutManger = 0;
	int philosopheExcluSiImpair = NB_PHILOSOPHES;
	if(NB_PHILOSOPHES % 2)
	{
		philosopheExcluSiImpair = NB_PHILOSOPHES-1;
	}
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	sem_post(&semSynchroThreadsPhilos);

	while(1)
	{
		tousFaim = false;
		while(tousFaim == false){
			usleep(1);
			tousFaim = true;
			//On fait tous les philos paires ou tous les impaires selon starti
			for(int i=GroupeQuiPeutManger;i<NB_PHILOSOPHES;i+=2){
				if(i != philosopheExcluSiImpair){
					if(etatsPhilosophes[i]!=P_FAIM)
						tousFaim=false;
				}
			}
		}

		//Libère les philos qui peuvent manger (donc tous les paires ou impaires sauf l'exclu)
		for(i = GroupeQuiPeutManger ; i < NB_PHILOSOPHES; i+=2){
			pthread_mutex_lock(&run_lock[i]);
			if(i!=philosopheExcluSiImpair){
				run_thread_eat[i] = 1;
			}
			pthread_mutex_unlock(&run_lock[i]);
		}
		pthread_cond_broadcast(&run_cond);


		//std::cout<<"signal libéré"<<std::endl;
		tousFiniDeManger = false;
		while(tousFiniDeManger  == false){
			usleep(0);
			tousFiniDeManger = true;
			for(int i=GroupeQuiPeutManger;i<NB_PHILOSOPHES;i+=2){
				if(etatsPhilosophes[i] == P_MANGE)
					tousFiniDeManger = false;
				}
			}
		if(GroupeQuiPeutManger == 0){
			GroupeQuiPeutManger = 1;
		}
		else{
			GroupeQuiPeutManger = 0;
			if(NB_PHILOSOPHES%2){
				//Choix d'un philosophe qui va être exclu de la table. Le premier ou le dernier.
				if(philosopheExcluSiImpair  == 0)
					philosopheExcluSiImpair = NB_PHILOSOPHES-1;
				else
					philosopheExcluSiImpair = 0;
			}
		}
		pthread_testcancel();
	}
}

void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
	pthread_mutex_lock(&mutexEtats);
    etatsPhilosophes[idPhilosopheChangeant] = nouvelEtat;
    pthread_mutex_unlock(&mutexEtats);

    pthread_mutex_lock(&mutexCout);
    /*for (int i=0;i<NB_PHILOSOPHES;i++) {
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
	*/
    pthread_mutex_unlock(&mutexCout);
}


void terminerProgramme()
{
    int i;
    int sem_value;
    sem_destroy(&semSynchroThreadsPhilos);


	pthread_cancel(Sol1_threadScheduler);
    std::cout<< "Sol1_threadScheduler to join"<<std::endl;
	pthread_join(Sol1_threadScheduler, NULL);
    std::cout<< "Sol1_threadScheduler joined"<<std::endl;

    for(i=0;i<NB_PHILOSOPHES;i++)
    {
    	if(pthread_cancel(threadsPhilosophes[i]) == 0)
			std::cout << "[INFO] thread philosophe " << i << " canceled successfull"<<std::endl;
		else
			std::cout << "[WARNING] thread philosophe " << i << " error during cancel() "<<std::endl;
    }

    for(i = 0 ; i < NB_PHILOSOPHES; i++)
	{
		//pthread_mutex_lock(&run_lock[i]);
		run_thread_eat[i] = 1;
		//pthread_mutex_unlock(&run_lock[i]);
	}
	pthread_cond_broadcast(&run_cond);

	for(i=0;i<NB_PHILOSOPHES;i++)
	{
		sem_getvalue(semFourchettes[i], &sem_value);
		if( sem_value == 0)
		{
			std::cout << "[INFO] Semaphore fourchette " << i << "not posted" << std::endl;
			sem_post(semFourchettes[i]);
		}
		if(sem_destroy(semFourchettes[i]) == 0)
			std::cout << "[INFO] semaphore fourchette " << i << " close correctly"<<std::endl;
		else
			std::cout << "[WARNING] semaphore fourchette " << i << " error during close() "<<std::endl;
	}
	for(i=0;i<NB_PHILOSOPHES;i++)
    {
    	if(pthread_join(threadsPhilosophes[i], NULL) == 0)
    		std::cout << "[INFO] thread philosophe " << i << " join() correctly"<<std::endl;
    	else
    		std::cout << "[WARNING] thread philosophe " << i << " error during join() "<<std::endl;

    }
    free(threadsPhilosophes);
    free(semFourchettes);
    free(etatsPhilosophes);
    free(t_idx_philos);
}
