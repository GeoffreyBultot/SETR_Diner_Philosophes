#include <iostream>
#include <ctime> // time_t time()
#include <unistd.h> // entre autres : usleep
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "SolutionEtudiant.hpp"

#include "Header_Prof.h"

void ViePhilosopheSolution2(int id);

pthread_mutex_t mutex_Eat_Philosophes[NB_PHILOSOPHES];
pthread_mutexattr_t mutexattr_Eat_Philosophes[NB_PHILOSOPHES];
sem_t semSynchroThreadsPhilos;


pthread_attr_t pthread_attr_philosophes[NB_PHILOSOPHES];

pthread_mutexattr_t pthread_attr_Cout;
pthread_mutexattr_t pthread_attr_Etats;
int * t_idx_philos;
int excluded = NB_PHILOSOPHES;

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
	schedparam.sched_priority = 80;
	std::cout<<sched_setscheduler(PID,SCHED_FIFO,&schedparam)<<std::endl;
	printf(" %s\n", strerror(errno));
	mutexCout = PTHREAD_MUTEX_INITIALIZER;
	mutexEtats = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mutexCout, &pthread_attr_Cout);
	pthread_mutex_init(&mutexEtats, &pthread_attr_Etats);

	instantDebut = time(NULL);
	if(NB_PHILOSOPHES%2)
		excluded = 0;
	/*Create synchro threads philos*/
	if( sem_init( &semSynchroThreadsPhilos, 0, 0) == 0)//SemSynchro semaphores pris par defaut
		std::cout << "[INFO] semaphore semSynchroThreadsPhilos initialized" << std::endl;
	else
		std::cout << "[WARNING] semaphore semSynchroThreadsPhilos not initialized " << std::endl;


	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		/*Init mutex eat philos*/
		mutex_Eat_Philosophes[i] = PTHREAD_MUTEX_INITIALIZER;
		if( pthread_mutex_init(&mutex_Eat_Philosophes[i], &mutexattr_Eat_Philosophes[i]) == 0)//Fourchettes lachees par defaut
			std::cout << "[INFO] semaphore semSynchroThreadsPhilos initialized" << std::endl;
		else
			std::cout << "[WARNING] semaphore semSynchroThreadsPhilos not initialized " << std::endl;

	}
	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		/*Init sem forks philos*/
		semFourchettes[i] = (sem_t*)malloc(sizeof(sem_t));
		if( sem_init( semFourchettes[i], 0, 1) == 0)//Fourchettes lachees par defaut
			std::cout << "[INFO] semaphore Fourchette " <<i<<" initialized" << std::endl;
		else
			std::cout << "[WARNING] semaphore Fourchette " <<i<<" not initialized " << std::endl;
		t_idx_philos[i] = i;
	}

	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		/*Create threads philosophes*/
		if(pthread_create( &threadsPhilosophes[i], &(pthread_attr_philosophes[i]), &vieDuPhilosophe, &(t_idx_philos[i])) == 0)
			std::cout<<"[INFO] Thread philosophe " <<i<< " created"<<std::endl;
		else
			std::cout<<"[WARNING] Thread philosophe " <<i<< " not created"<<std::endl;
	}

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

void* vieDuPhilosophe(void* idPtr)
{
	int id = * ((int*)idPtr);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    actualiserEtAfficherEtatsPhilosophes(id, P_PENSE);
    int policy;
    int max_prio_for_policy;
    pthread_attr_init(&pthread_attr_philosophes[id]);
	pthread_attr_getschedpolicy(&pthread_attr_philosophes[id], &policy);
	max_prio_for_policy = 50;// sched_get_priority_max(policy);
	pthread_setschedprio(pthread_self(), max_prio_for_policy);
	//std::cout<<pthread_setschedprio(pthread_self(), max_prio_for_policy)<<std::endl;
	//printf(" %s\n", strerror(errno));

    while(1)
    {
    	ViePhilosopheSolution2(id);
        pthread_testcancel();
    }
    return NULL;
}


void ViePhilosopheSolution2(int id)
{
	int value_sema = 0;
	//std::cout<<"attente de synchro"<<std::endl;
	while(value_sema == 0)//Tant que le semaphore est pris
		sem_getvalue(&semSynchroThreadsPhilos, &value_sema);
	//std::cout<<"TheadSynchronisé"<<std::endl;
	actualiserEtAfficherEtatsPhilosophes(id,P_FAIM);
	sem_wait(semFourchettes[id]); //Acquisition fourchette gauche
	sem_wait(semFourchettes[ (id+1)%NB_PHILOSOPHES]); //Acquisition fourchette droite

	actualiserEtAfficherEtatsPhilosophes(id,P_MANGE);
	randomDelay(0,DUREE_MANGE_MAX_S);

	sem_post(semFourchettes[id]); //Relâchement fourchette gauche
	sem_post(semFourchettes[(id+1)%NB_PHILOSOPHES]); //Relâchement fourchette droite
	actualiserEtAfficherEtatsPhilosophes(id,P_PENSE);

	randomDelay(0,DUREE_PENSE_MAX_S);
}


char getPhiloState(int idPhilo)
{
	char stateToReturn = 0 ;
	pthread_mutex_lock(&mutexEtats);
	stateToReturn = etatsPhilosophes[idPhilo];
	pthread_mutex_unlock(&mutexEtats);
	return stateToReturn;
}


void setPhiloState(int idPhilo, char statePhilo)
{
	pthread_mutex_lock(&mutexEtats);
	etatsPhilosophes[idPhilo] = statePhilo;
	//std::cout<<"philo"<<idPhilo<<statePhilo<<std::endl;
	pthread_mutex_unlock(&mutexEtats);
}


bool wantToEat[NB_PHILOSOPHES] = {false};

void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
	int starti=0;
	int i_otherGroup = 0;
	//std::cout<<"excluded = "<<excluded;
	if(idPhilosopheChangeant%2){ //L'id est impair
		i_otherGroup = 0;
		starti=1;
	}
	else	{
		i_otherGroup = 1;
		starti=0;
	}
	if(nouvelEtat == P_MANGE)
	{

		while(excluded == idPhilosopheChangeant){
			usleep(1);};
		wantToEat[idPhilosopheChangeant] = true;
		//On va attendre que ceux de son groupe aient envie de manger
		//Et que ceux du groupe avant ne mangent plus
		bool AutreGroupeAFiniDeManger = false;
		bool SonGroupeVeutManger = false;
		//usleep(100);
		//std::cout<<"philo "<<idPhilosopheChangeant<<" veut manger"<<std::endl;
		while(SonGroupeVeutManger == false){
			usleep(1);
			SonGroupeVeutManger = true;
			for(int i = starti; i< NB_PHILOSOPHES;i+=2){//Check la liste du philo
				if(i!=excluded){
					if(wantToEat[i] == false) {
						SonGroupeVeutManger = false;
					}
				}
			}
		}
		std::cout<<"tout le groupe du philo "<<idPhilosopheChangeant<<" veut manger"<<std::endl;

		bool tousMangent = false;
		setPhiloState(idPhilosopheChangeant,nouvelEtat);
		while(tousMangent == false){
			usleep(1);
			tousMangent = true;
			for(int i = starti; i< NB_PHILOSOPHES;i+=2){//Check la liste du philo
				if(i!=excluded){
					if(getPhiloState(i) != P_MANGE){
						tousMangent = false;
					}
				}

			}
		}

		while(AutreGroupeAFiniDeManger == false){
			usleep(1);
			AutreGroupeAFiniDeManger = true;
				for(int i = i_otherGroup; i< NB_PHILOSOPHES;i+=2){//Check la liste du philo
					if(i!=excluded){
						if(getPhiloState(i) == P_MANGE ){
							AutreGroupeAFiniDeManger = false;
						}
					}
				}
			}
		//std::cout<<"autre groupe que le philo "<<idPhilosopheChangeant<<" fini de manger"<<std::endl;

		if(idPhilosopheChangeant == excluded){
			if(NB_PHILOSOPHES%2){
				if(idPhilosopheChangeant == NB_PHILOSOPHES-1)
					excluded = 0;
				else
					excluded = NB_PHILOSOPHES-1;
			}
		}
	}
	else
	{
		wantToEat[idPhilosopheChangeant] = false;
		setPhiloState(idPhilosopheChangeant,nouvelEtat);
	}


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
    sem_destroy(&semSynchroThreadsPhilos);
    sem_getvalue(&semSynchroThreadsPhilos, &sem_value);
	if( sem_value == 0)
	{
		std::cout << "[INFO] Semaphore synchro not posted value = " << sem_value << std::endl;
		std::cout<<"post return : " <<sem_post(&semSynchroThreadsPhilos);
		sem_getvalue(&semSynchroThreadsPhilos, &sem_value);
		std::cout <<"  new value : "<< sem_value<<std::endl;
	}

	if(sem_destroy(&semSynchroThreadsPhilos) != -1)
		std::cout << "[INFO] semaphore synchro destroyed correctly"<<std::endl;
	else
		std::cout << "[WARNING] semaphore synchro error during destroy() " << strerror(errno) <<std::endl;

	for(i=0;i<NB_PHILOSOPHES;i++){
		pthread_mutex_destroy(&mutex_Eat_Philosophes[i]);
	}
	for(i=0;i<NB_PHILOSOPHES;i++){

		sem_getvalue(semFourchettes[i], &sem_value);
		if( sem_value == 0)
		{
			std::cout << "[INFO] Semaphore fourchette " << i << " not posted value = " << sem_value;
			std::cout<<"post return : " <<sem_post(semFourchettes[i]);
			sem_getvalue(semFourchettes[i], &sem_value);
			std::cout <<"  new value : "<< sem_value<<std::endl;
		}

		if(sem_destroy(semFourchettes[i]) != -1)
			std::cout << "[INFO] semaphore fourchette " << i << " destroyed correctly"<<std::endl;
		else
			std::cout << "[WARNING] semaphore fourchette " << i << " error during destroy() " << strerror(errno) <<std::endl;

	}
	for(i=0;i<NB_PHILOSOPHES;i++){

	}
	for(i=0;i<NB_PHILOSOPHES;i++)
    {

    	if(pthread_cancel(threadsPhilosophes[i]) == 0)
			std::cout << "[INFO] thread philosophe " << i << " canceled successfull"<<std::endl;
		else
			std::cout << "[WARNING] thread philosophe " << i << " error during cancel() "<< strerror(errno) <<std::endl;

    	if(pthread_join(threadsPhilosophes[i], NULL) == 0)
    		std::cout << "[INFO] thread philosophe " << i << " join() correctly"<<std::endl;
    	else
    		std::cout << "[WARNING] thread philosophe " << i << " error during join() "<< strerror(errno) <<std::endl;
    }
    free(threadsPhilosophes);
    free(semFourchettes);
    free(etatsPhilosophes);
    free(t_idx_philos);
}
