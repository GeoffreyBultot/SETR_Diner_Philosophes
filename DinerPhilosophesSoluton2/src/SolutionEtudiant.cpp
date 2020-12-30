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
	max_prio_for_policy = 50;//+id;// sched_get_priority_max(policy);
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
	actualiserEtAfficherEtatsPhilosophes(id,P_MANGE);
	sem_wait(semFourchettes[id]); //Acquisition fourchette gauche
	sem_wait(semFourchettes[ (id+1)%NB_PHILOSOPHES]); //Acquisition fourchette droite
	//std::cout<<"philo "<<id<<" take forks "<<id<< " and " << (id+1)%NB_PHILOSOPHES<<std::endl;
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

void afficheStatePhilos(int idPhilosopheChangeant)
{
	pthread_mutex_lock(&mutexCout);
	for (int i=0;i<NB_PHILOSOPHES;i++) {
		if (i==idPhilosopheChangeant)
			std::cout << "*";
		else
			std::cout << " ";
		std::cout << getPhiloState(i);
		if (i==idPhilosopheChangeant)
			std::cout << "* ";
		else
			std::cout << "  ";
	}
	std::cout << "          (t=" << difftime(time(NULL), instantDebut) << ")" << std::endl;

	pthread_mutex_unlock(&mutexCout);
}



bool wantToEat[NB_PHILOSOPHES] = {false};
int groupeQuiPeutManger = 0;

void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
	int starti=idPhilosopheChangeant%2;
	//std::cout<<"excluded = "<<excluded;

	switch(nouvelEtat)
	{
		/*______________________________________________________________________________*/
		/*								LE PHILO A FAIM									*/
		/*______________________________________________________________________________*/
	case P_FAIM://Si il a faim on va le faire attendre qu'il puisse manger
		{
			//Dans ce cas c'est à son tour de manger donc on va le mettre en "mange"
			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);
			//Premier cas, est-ce que c'est à son tour de manger ??
			while( (excluded == idPhilosopheChangeant) )
			{
				//std::cout<<idPhilosopheChangeant;
				//usleep(10000);
			}
			while( (idPhilosopheChangeant%2 != groupeQuiPeutManger))
			{

			}

			//L'idée ici c'est de dire que le philo veut manger et qu'il attend les autrees
			wantToEat[idPhilosopheChangeant] = true;

			//On attend les autres philos qui doivent manger avec lui
			bool SonGroupeVeutManger = false;

			//tant que tout le groupe n'est pas prêt
			while(SonGroupeVeutManger == false){
				usleep(1);
				SonGroupeVeutManger = true;
				for(int i = starti; i< NB_PHILOSOPHES;i+=2){//Check la liste du philo
					if(i!=excluded){
						if(getPhiloState(i) == false) {
							SonGroupeVeutManger = false;
						}
					}
				}
			}



			//std::cout<<"philo numero "<<idPhilosopheChangeant<<" attend que son groupe mange";

			//std::cout<<"philo numero "<<idPhilosopheChangeant<<" va manger avec ses amis";
		}
		break;
		/*______________________________________________________________________________*/
		/*								LE PHILO MANGE									*/
		/*______________________________________________________________________________*/
	case P_MANGE:
		{
			//std::cout<<"je suis dans mange" << idPhilosopheChangeant<<std::endl;

			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);
		}
		break;

		/*______________________________________________________________________________*/
		/*								LE PHILO PENSE									*/
		/*______________________________________________________________________________*/

	case P_PENSE:
		{
			bool auMoinsUnMange = false;
			wantToEat[idPhilosopheChangeant] = false;

			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);
			for(int i = 0; i< NB_PHILOSOPHES;i++){//Check la liste du philo
					if(getPhiloState(i) == P_MANGE){
						auMoinsUnMange  = true;
					}
			}

			if( (auMoinsUnMange == false) ){
				//printf("%d\n",groupeQuiPeutManger);
				if(groupeQuiPeutManger == 0)
					groupeQuiPeutManger = 1;
				else
					groupeQuiPeutManger = 0;

				if(NB_PHILOSOPHES%2){
					if(idPhilosopheChangeant == NB_PHILOSOPHES-1)
						excluded = NB_PHILOSOPHES-1;
					else if(idPhilosopheChangeant == 0)
						excluded = 0;
				}
			}//std::cout<<"philo : "<<idPhilosopheChangeant<< "new exluded : "<<excluded<<std::endl;


		}
		break;
	default:
		{
			exit(EXIT_FAILURE);
		}
		break;
	}
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
