/**
 * @file SolutionEtudiant.cpp
 * @brief
 * @details
 *
 * - Company       	 		: ISIB
 * - Project 	     		: Diner des philosophes
 * - Creation 				: 2020
 * - Author		 			: Bultot Geoffrey (gbultot@etu.he2b.be)
 *   When | Who | What
 *   ------- | ----- | -----
 *   This file contain the resoltution of "dining philosophers problem" :
 *   					- philosophe gesture for solution 2
*/

/***************************************************************************
* Includes Directives
***************************************************************************/

#include <iostream>		//
#include <ctime> 		// time_t time()
#include <unistd.h> 	// entre autres : usleep
#include <semaphore.h>	// Utilisation sémaphores
#include <pthread.h>	// Utilisation pthread
#include <errno.h>		// Affichage erreurs objets posix
#include <string.h>		// entre autres : strerror
#include "SolutionEtudiant.hpp"
#include "Header_Prof.h"
#include <sys/time.h>	// gettimeOfTheDay

/***************************************************************************
* Variables declarations
***************************************************************************/
/*Sémaphore de synchronisation*/
sem_t semSynchroPensePhilos;

/*Mutexs pour protéger les variables:
 * Groupe qui peut manger
 * Cptr_pense*/
pthread_mutex_t mutex_GroupeQuiPeutManger;
pthread_mutex_t mutex_cptrPense;

/*Attributs (au besoin)*/
pthread_attr_t pthread_attr_philosophes[NB_PHILOSOPHES];
pthread_mutexattr_t pthread_mutex_attr_GroupeQuiPeutManger;
pthread_mutexattr_t pthread_mutex_attr_cptrPense;
pthread_mutexattr_t pthread_attr_Cout;
pthread_mutexattr_t pthread_attr_Etats;

/*Tableau qui contient l'id des philosophes*/
/*Ce tableau est alloué dynamiquement dans la fonction initialisation et est situé sur le tas.
 * Le thread d'un philosophe prenant en paramètre un pointeur, il fallait lui passer une variable
 * qui a un emplacement mémoire fixe et non un indie de boucle*/
int * t_idx_philos;
/*Philosophe qui sera exclu une fois sur deux si le nombre de philosophes est impair*/
int philosopheExcluSiImpair = NB_PHILOSOPHES;
/*Groupe qui peut manger : les paires (0) ou les impaires (1)*/
int groupeQuiPeutManger = 1;
/*Nombre de philosophes qui vont penser*/
int cptr_pense = 0;
/*Utilisé comme variable condition : permet de savoir si un philosophe peut manger*/
bool run_thread_eat[NB_PHILOSOPHES] = {false};
/*Pour l'acquisition des temps caractéristiques*/
struct timeval t_second[NB_PHILOSOPHES], t_first[NB_PHILOSOPHES];

/*Mutex et conditions pour les attentes passives (faim) des philosophes*/
pthread_mutexattr_t mutexattr_run_thread_eat[NB_PHILOSOPHES];
pthread_mutex_t mutex_run_thread_eat[NB_PHILOSOPHES] = {PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t run_cond = PTHREAD_COND_INITIALIZER;


/***************************************************************************
* privates functions declarations
***************************************************************************/
/*Fonctions permettant d'écrire et de lire des variables en les protégeant avec des mutex */
void setPhiloState(int idPhilo, char statePhilo);
void afficheStatePhilos(int idPhilosopheChangeant);
void setGroupeQuiPeutManger(int groupe);
int getGroupeQuiPeutManger();


/***************************************************************************
* Functions
***************************************************************************/
/*@brief cette fonction permet d'initiliser le Diner des philosophes
 * @return void*/
void initialisation()
{
	sched_param schedparam;
	int PID = getpid();
	/*Allocations dynamiques de tableaux sur le tas*/
	threadsPhilosophes 	= (pthread_t*)malloc(NB_PHILOSOPHES * sizeof(pthread_t));
	semFourchettes 		= (sem_t**)malloc(NB_PHILOSOPHES * sizeof(sem_t*));
	etatsPhilosophes 	= (char*)malloc(NB_PHILOSOPHES * sizeof(char));
	t_idx_philos 		= (int*)malloc(NB_PHILOSOPHES *sizeof(int));

	/*HIGH PRIORITY AND SCHED FIFO*/
	sched_getparam(PID,&schedparam);
	schedparam.sched_priority = 80;

	/*Initialisation des mutexs*/
	mutexCout = PTHREAD_MUTEX_INITIALIZER;
	mutexEtats = PTHREAD_MUTEX_INITIALIZER;
	mutex_GroupeQuiPeutManger = PTHREAD_MUTEX_INITIALIZER;
	mutex_cptrPense = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_init(&mutex_GroupeQuiPeutManger, &pthread_mutex_attr_GroupeQuiPeutManger);
	pthread_mutex_init(&mutex_cptrPense, &pthread_mutex_attr_cptrPense);
	pthread_mutex_init(&mutexCout, &pthread_attr_Cout);
	pthread_mutex_init(&mutexEtats, &pthread_attr_Etats);
	/*Si on a un nombre de philosophes impairs, le premier qui sera exclu sera le 0
	 * Sinon, ce sera le N philosophe (donc aucun)*/
	if(NB_PHILOSOPHES%2)
		philosopheExcluSiImpair = 0;
	else
		philosopheExcluSiImpair = NB_PHILOSOPHES;

	/*semaphore permettant d'arrêter les philosophes dans leur état PENSE
	 * Le sémaphore est pris par défaut et relâché après initialisation*/
	if( sem_init( &semSynchroPensePhilos, 0, 0) == 0)
		std::cout << "[INFO] semaphore semSynchroThreadsPhilos initialized" << std::endl;
	else
		std::cout << "[WARNING] semaphore semSynchroThreadsPhilos not initialized " << std::endl;

	/*Initilise les mutexs pour la variable condition*/
	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		if( pthread_mutex_init(&mutex_run_thread_eat[i], &mutexattr_run_thread_eat[i]) == 0)//Fourchettes lachees par defaut
			std::cout << "[INFO] mutex run thread" << i << "initialized" << std::endl;
		else
			std::cout << "[WARNING] mutex run thread" << i << "not initialized " << std::endl;
		setPhiloState(i,P_PENSE);
	}

	/*Initialise les sémaphore des fourchettes*/
	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		/*allocation dynamique de chaque fourchette*/
		semFourchettes[i] = (sem_t*)malloc(sizeof(sem_t));
		if( sem_init( semFourchettes[i], 0, 1) == 0)//Fourchettes lachees par defaut
			std::cout << "[INFO] semaphore Fourchette " <<i<<" initialized" << std::endl;
		else
			std::cout << "[WARNING] semaphore Fourchette " <<i<<" not initialized " << std::endl;
	}

	/*Personne n'a le droit de manger au départ*/
	for(int i = 0;i<NB_PHILOSOPHES;i++){
		pthread_mutex_lock(&mutex_run_thread_eat[i]);
		run_thread_eat[i] = false;
		pthread_mutex_unlock(&mutex_run_thread_eat[i]);
	}

	/*Crée les threads représentant les philosophes*/
	for(int i=0;i<NB_PHILOSOPHES;i++)
	{
		/*Sauvegarde de l'id du philosophe*/
		t_idx_philos[i] = i;
		/*Création du thread avec son id stocké dans t_idx_philos (sur le tas)*/
		if(pthread_create( &threadsPhilosophes[i], &(pthread_attr_philosophes[i]), &vieDuPhilosophe, &(t_idx_philos[i])) == 0)
			std::cout<<"[INFO] Thread philosophe " <<i<< " created"<<std::endl;
		else
			std::cout<<"[WARNING] Thread philosophe " <<i<< " not created"<<std::endl;
	}
	/*Pour l'affichage du temps*/
	instantDebut = time(NULL);

	/*Libère les threads*/
	sem_post(&semSynchroPensePhilos);
}

/*@brief fonction qui crée un délais aléatoire entre f_min_sec et f_max_sec
 *@param[in] f_max_Sec
 *@param[in] f_min_Sec
 *@return void */
void randomDelay(float f_min_Sec, float f_max_Sec)
{
	int random;

	srand(time(NULL));
	random = (rand())%((int)f_max_Sec*1000000);
	if( (float)random < (f_min_Sec*1000000))
		random = int(f_min_Sec*1000000);
	else if((float)random > (f_max_Sec*1000000))
		random = int(f_max_Sec*1000000);
	usleep(random);
}

/*@brief thread représentat la vie d'un philosophe
 * @param[in] idPtr
 * @return void* */
void* vieDuPhilosophe(void* idPtr)
{
	int id = * ((int*)idPtr);
	int gauche = id;
	int droite = (id+1)%NB_PHILOSOPHES;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while(1)
    {
    	int value_sema = 0;
		while(value_sema == 0)//Tant que le semaphore est pris
			sem_getvalue(&semSynchroPensePhilos, &value_sema);
		//gettimeofday(&t_first[id],NULL);
		actualiserEtAfficherEtatsPhilosophes(id,P_FAIM);
		//gettimeofday(&t_second[id],NULL);
		//int diff = (t_second[id].tv_sec - t_first[id].tv_sec) * 1000000 + t_second[id].tv_usec - t_first[id].tv_usec;
		pthread_mutex_lock(&mutexCout);
		//std::cout <<id<<";"<< diff << std::endl;
		pthread_mutex_unlock(&mutexCout);
		sem_wait(semFourchettes[gauche]); //Acquisition fourchette gauche
		sem_wait(semFourchettes[droite]); //Acquisition fourchette droite
		actualiserEtAfficherEtatsPhilosophes(id,P_MANGE);
		randomDelay(0,DUREE_MANGE_MAX_S);
		sem_post(semFourchettes[gauche]); //Relâchement fourchette gauche
		sem_post(semFourchettes[droite]); //Relâchement fourchette droite
		actualiserEtAfficherEtatsPhilosophes(id,P_PENSE);
		randomDelay(0,DUREE_PENSE_MAX_S);
    }
    return NULL;
}

/*@brief cette fonction permet de récupérer l'état d'un philosophe avec son id (mutex protection)
 * @param[in] idPhilo
 * return char
 * */
char getPhiloState(int idPhilo)
{
	char stateToReturn = 0 ;
	pthread_mutex_lock(&mutexEtats);
	stateToReturn = etatsPhilosophes[idPhilo];
	pthread_mutex_unlock(&mutexEtats);
	return stateToReturn;
}

/*@brief cette fonction permet mettre à jour l'état d'un philosophe (mutex protection)
 * @param[in] idPhilo
 * return char
 * */
void setPhiloState(int idPhilo, char statePhilo)
{
	pthread_mutex_lock(&mutexEtats);
	etatsPhilosophes[idPhilo] = statePhilo;
	pthread_mutex_unlock(&mutexEtats);
}

/*@brief cette fonction permet mettre à jour le groupe qui peut manger (mutex protection)
 * @param[in] groupe
 * return void
 * */
void setGroupeQuiPeutManger(int groupe)
{
	pthread_mutex_lock(&mutex_GroupeQuiPeutManger);
	groupeQuiPeutManger = groupe;
	pthread_mutex_unlock(&mutex_GroupeQuiPeutManger);
}

/*@brief cette fonction permet de savoir quel groupe peut manger (mutex protection)
 * return int
 * */
int getGroupeQuiPeutManger()
{
	int ret;
	pthread_mutex_lock(&mutex_GroupeQuiPeutManger);
	ret = groupeQuiPeutManger;
	pthread_mutex_unlock(&mutex_GroupeQuiPeutManger);
	return ret;
}

/*@brief cette fonction permet d'afficher l'état des philosophes à l'intant t avec mutex protection sur std::cout
 * @param[in] idPhilosopheChangeant
 * return void
 * */
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

/*@brief cette fonction appelée par chaque philosophe permet de pendre des décisions sur le blocage ou non de ceux-ci.
 * La fonction actualise et affiche égelement l'état des philosophes dans la console;
 * @param[in] idPhilosopheChangeant
 * @param[in] nouvelEtat
 * return void
 * */
void actualiserEtAfficherEtatsPhilosophes(int idPhilosopheChangeant, char nouvelEtat)
{
	switch(nouvelEtat)
	{
	/*___________________________________________________________________*/
	/*                           LE PHILO A FAIM                         */
	/*___________________________________________________________________*/
	case P_FAIM:
		{
			/*Mise à jour de l'*/
			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);

			//LE PHILO A FAIM DONC ON LE FAIT ATTENDRE SON TOUR !
			/*Regarde si tout le groupe a faim*/
			bool tousLeGroupeAFaim = true;
			for(int i = getGroupeQuiPeutManger() ; i<NB_PHILOSOPHES;i+=2){
				if(i!=philosopheExcluSiImpair){
					if(getPhiloState(i) != P_FAIM){
					tousLeGroupeAFaim = false;
				}}
			}
			/*Si tout le groupe a faim, mise à jour des variables conditionnelles*/
			if(tousLeGroupeAFaim){
				for(int i = getGroupeQuiPeutManger() ; i<NB_PHILOSOPHES;i+=2){
					if(i!=philosopheExcluSiImpair){
						pthread_mutex_lock(&mutex_run_thread_eat[i]);
						run_thread_eat[i] = true;
						pthread_mutex_unlock(&mutex_run_thread_eat[i]);
					}
				}
				pthread_cond_broadcast(&run_cond);
			}

			pthread_mutex_lock(&mutex_run_thread_eat[idPhilosopheChangeant]);
			while (!run_thread_eat[idPhilosopheChangeant])
			{
				pthread_cond_wait(&run_cond, &mutex_run_thread_eat[idPhilosopheChangeant]);
			}
			run_thread_eat[idPhilosopheChangeant] = false;
			pthread_mutex_unlock(&mutex_run_thread_eat[idPhilosopheChangeant]);
		}
		break;
	/*___________________________________________________________________*/
	/*                            LE PHILO MANGE                         */
	/*___________________________________________________________________*/
	case P_MANGE:
		{
			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);
		}
		break;

	/*___________________________________________________________________*/
	/*                              LE PHILO PENSE                       */
	/*___________________________________________________________________*/
	case P_PENSE:
		{
			setPhiloState(idPhilosopheChangeant,nouvelEtat);
			afficheStatePhilos(idPhilosopheChangeant);
			/*incrément de philosophes qui viennent penser*/
			pthread_mutex_lock(&mutex_cptrPense);
			cptr_pense++;
			/*Si assez de philosophes pensent (comme 2 groupes : NB_PHILOSOPHES\2*/
			if(cptr_pense == (NB_PHILOSOPHES/2) ){
				cptr_pense = 0;
				/**/
				if(idPhilosopheChangeant%2 == 0)
					setGroupeQuiPeutManger(1);
				else
					setGroupeQuiPeutManger(0);

				if(NB_PHILOSOPHES%2 && idPhilosopheChangeant%2 == 0){
					if(philosopheExcluSiImpair == NB_PHILOSOPHES-1)
						philosopheExcluSiImpair = 0;
					else if(philosopheExcluSiImpair == 0)
						philosopheExcluSiImpair = NB_PHILOSOPHES-1;
				}
			}
			pthread_mutex_unlock(&mutex_cptrPense);
			pthread_cond_broadcast(&run_cond);
		}
		break;
	default:
		{
			exit(EXIT_FAILURE);
		}
		break;
	}
}

/*@brief fonction appelée lors de la réception d'un SIGINT ou SIGTERM;
 * Détruit tous les objets POSIX, libération des allocations dynamiques et termine les threads
 * @return void
 * */
void terminerProgramme()
{
    int i;
    int sem_value;
    sem_destroy(&semSynchroPensePhilos);
    sem_getvalue(&semSynchroPensePhilos, &sem_value);

	if( sem_value == 0)
	{
		std::cout << "[INFO] Semaphore synchro not posted value = " << sem_value << std::endl;
		std::cout<<"post return : " <<sem_post(&semSynchroPensePhilos);
		sem_getvalue(&semSynchroPensePhilos, &sem_value);
		std::cout <<"  new value : "<< sem_value<<std::endl;
	}

	if(sem_destroy(&semSynchroPensePhilos) != -1)
		std::cout << "[INFO] semaphore synchro destroyed correctly"<<std::endl;
	else
		std::cout << "[WARNING] semaphore synchro error during destroy() " << strerror(errno) <<std::endl;


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

	for(i=0;i<NB_PHILOSOPHES;i++)
    {

    	if(pthread_cancel(threadsPhilosophes[i]) == 0)
			std::cout << "[INFO] thread philosophe " << i << " canceled successfull"<<std::endl;
		else
			std::cout << "[WARNING] thread philosophe " << i << " error during cancel() "<< strerror(errno) <<std::endl;
    }
	for(i=0;i<NB_PHILOSOPHES;i++){
    	if(pthread_join(threadsPhilosophes[i], NULL) == 0)
    		std::cout << "[INFO] thread philosophe " << i << " join() correctly"<<std::endl;
    	else
    		std::cout << "[WARNING] thread philosophe " << i << " error during join() "<< strerror(errno) <<std::endl;
    }
	for(i=0;i<NB_PHILOSOPHES;i++){
		pthread_mutex_destroy(&mutex_run_thread_eat[i]);
	}
	pthread_cond_destroy(&run_cond);
	free(threadsPhilosophes);
    free(semFourchettes);
    free(etatsPhilosophes);
    free(t_idx_philos);
}
