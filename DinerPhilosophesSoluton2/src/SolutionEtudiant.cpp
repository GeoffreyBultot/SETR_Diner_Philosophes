#include <iostream>
#include <ctime> // time_t time()
#include <unistd.h> // entre autres : usleep
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "SolutionEtudiant.hpp"

#include "Header_Prof.h"

pthread_mutex_t mutex_Eat_Philosophes[NB_PHILOSOPHES];
pthread_mutexattr_t mutexattr_Eat_Philosophes[NB_PHILOSOPHES];
sem_t semSynchroThreadsPhilos;


pthread_attr_t pthread_attr_philosophes[NB_PHILOSOPHES];

pthread_mutexattr_t pthread_attr_Cout;
pthread_mutexattr_t pthread_attr_Etats;
int * t_idx_philos;
int excluded = NB_PHILOSOPHES;


int run_thread_eat[NB_PHILOSOPHES] = {0};
pthread_mutex_t run_lock[NB_PHILOSOPHES] = {PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t run_cond = PTHREAD_COND_INITIALIZER;



void setPhiloState(int idPhilo, char statePhilo);
void afficheStatePhilos(int idPhilosopheChangeant);

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
		setPhiloState(i,P_PENSE);
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

	instantDebut = time(NULL);
	/*for(int i = 0;i<NB_PHILOSOPHES;i++){
		setPhiloState(i,P_FAIM);
		afficheStatePhilos(i);
	}*/

	for(int i = 1;i<NB_PHILOSOPHES;i+=2){
		pthread_mutex_lock(&run_lock[i]);
		run_thread_eat[i] = 1;
		pthread_mutex_unlock(&run_lock[i]);
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
    while(1)
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
        pthread_testcancel();
    }
    return NULL;
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
int groupeQuiPeutManger = 1;

void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
	switch(nouvelEtat)
	{
		/*______________________________________________________________________________*/
		/*								LE PHILO A FAIM									*/
		/*______________________________________________________________________________*/
	case P_FAIM:
		{
			//Si il a faim on va le faire attendre qu'il puisse manger

			//Dans ce cas c'est à son tour de manger donc on va le mettre en "mange"
			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);

			//LE PHILO A FAIM DONC ON LE FAIT ATTENDRE SON TOUR !

			pthread_mutex_lock(&run_lock[idPhilosopheChangeant]);
			while (!run_thread_eat[idPhilosopheChangeant])
			{
				pthread_cond_wait(&run_cond, &run_lock[idPhilosopheChangeant]);
			}
			run_thread_eat[idPhilosopheChangeant] = 0;
			pthread_mutex_unlock(&run_lock[idPhilosopheChangeant]);
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
					if( (getPhiloState(i) == P_MANGE) ){
						auMoinsUnMange  = true;
					}
			}


			if( (auMoinsUnMange == false) ){
				if(groupeQuiPeutManger == 0)
					groupeQuiPeutManger = 1;
				else
					groupeQuiPeutManger = 0;

				if(NB_PHILOSOPHES%2 && idPhilosopheChangeant%2 == 0){
					if(excluded == NB_PHILOSOPHES-1)
						excluded = 0;
					else if(excluded == 0)
						excluded = NB_PHILOSOPHES-1;
				}
				//std::cout<<"le groupe qui peut manger mtn est le "<<groupeQuiPeutManger<<std::endl;
				//Attendre ceux qui doivent manger
				bool ilsOntFaim = false;
				while(ilsOntFaim == false){
					ilsOntFaim = true;
					for(int i = groupeQuiPeutManger ; i < NB_PHILOSOPHES ; i+=2){
						if(i!=excluded){
							if(getPhiloState(i) != P_FAIM){
								ilsOntFaim = false;
							}
						}
					}
				}
				for(int i = groupeQuiPeutManger ; i < NB_PHILOSOPHES; i+=2){
					pthread_mutex_lock(&run_lock[i]);
					if(i!=excluded){
						run_thread_eat[i] = 1;
					}
					pthread_mutex_unlock(&run_lock[i]);
				}
				pthread_cond_broadcast(&run_cond);
			}
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
