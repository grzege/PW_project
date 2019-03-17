#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <ctype.h>
#include <time.h>

#define ID 2 //identyfikator danego głodomora, z przedziału zamkniętego od 0 do 4
#define MAX 5 //liczba glodomorów

static struct sembuf buf;

void V(int semid, int semnum){//podniesienie semafora
 buf.sem_num = semnum;
 buf.sem_op = 1;
 buf.sem_flg = 0;
 if (semop(semid, &buf, 1) == -1){
    perror("Blad przy podnoszeniu semafora.\n");
    exit(1);
   }
}

void P(int semid, int semnum){//opuszczenie semafora
  buf.sem_num = semnum;
  buf.sem_op = -1;
  buf.sem_flg = 0;
   if (semop(semid, &buf, 1) == -1){
    perror("Blad przy opuszczaniu semafora.\n");
    exit(1);
   }
 }
int losuj(){//generator liczb losowych z zakresu od 1 do 10
  time_t czas;
  int start = time(&czas);
  srand(start);
  return (rand()%10)+1;
}
void jedz()//funkcja symuluje jedzenie posiłku
{
  sleep(losuj());
}
void mysl()//funkcja symuluje odpoczynek pomiędzy posiłkami
{
  sleep(losuj()+5);
}
int minimum(int waga[])//funkcja odpowiada za zwrócenie indeksu glodomora o najwyższym priorytecie (tego który zjadł najmniej)
{
  int M=0;
  for(int i=M+1;i<MAX;i++)if(waga[i]<waga[M])M=i;
  return M;
}

int main()
{
int sem_widelec;//semafor widelcowy, 1 semafor na każdy z 5 widelców
int sem_talerzy; //semafor talerzowy, 1 semafor odpowiada za każdy z 4 talerzy
int sem_prior; //semafor priorytetowy, 1 semefor odpowiada za priorytet każdego z 5 glodomorów
int shm_waga; //segment pamięci współdzielonej
int *waga; //, kazdy z 5 odpowiada za wagę posiłków zjedzonych przez odpowiadającego mu głodomora
//tworzenie i przyłączenie pamięci współdzielonej - odpowiada za wagę posiłków zjedzonych przez poszczególnych głodomorów
shm_waga=shmget(34170,MAX*sizeof(int),IPC_CREAT|0600);
if(shm_waga==-1)
{
  perror("Blad utworzenia segmentu pamieci wspoldzielonej.");
  exit(1);
}
waga=(int*)shmat(shm_waga,NULL,0);
if(waga==NULL)
{
  perror("Blad przylaczenia segmentu pamieci wspoldzielone.");
  exit(1);
}
//tworzenie semaforów
sem_widelec=semget(45281,MAX,IPC_CREAT|0600);
if(sem_widelec == -1)
{
  perror("Blad utworzenia semafora widelcowego.");
  exit(1);
}
sem_talerzy=semget(56392,1,IPC_CREAT|0600);
if(sem_talerzy == -1)
{
  perror("Blad utworzenia semafora talerzowego.");
  exit(1);
}
sem_prior=semget(67403,MAX,IPC_CREAT|0600);
if(sem_prior == -1)
{
  perror("Blad utworzenia semafora priorytetowego.");
  exit(1);
}
//nadawanie wartości semaforom i pamięci współdzielonej
if(semctl(sem_talerzy,0,SETVAL,(int)MAX-1)==-1)
{
  perror("Blad nadania wartosci semaforowi talerzowemu.");
  exit(1);
}
for(int i=0;i<MAX;i++)
{
  waga[i]=0;
  if(semctl(sem_widelec,i,SETVAL,(int)1)==-1)
  {
    perror("Blad nadania wartosci semaforowi widelcowemu.");
    exit(1);
  }
  if(semctl(sem_prior,i,SETVAL,(int)0)==-1)
  {
    perror("Blad nadania wartosci semaforowi widelcowemu.");
    exit(1);
  }
}
//pętla właściwa programu, personifikacja głodomora
while(1)
{
  printf("Nr %d mysli|%d\n",ID,waga[ID]);
  mysl(); //procedura myslenia
  printf("Nr %d jest glodny|%d\n",ID,waga[ID]);
  if(ID!=minimum(waga))P(sem_prior,ID); //jeśli proces nie ma najmniejszego priorytetu to opuszczamy semafor priorytetu (zmniejszamy priorytet)
  P(sem_talerzy,0); //wez talerz
  P(sem_widelec,ID); //podnies lewy widelec
  P(sem_widelec,(ID+MAX-1)%MAX); //podnies prawy widelec
  printf("Nr %d je|%d\n\n",ID,waga[ID]);
  waga[ID]+=losuj(); //dodanie wagi posilku do sumy wag zjedzonych
  V(sem_prior,minimum(waga)); //odblokuj proces glodomora, ktory zjadł najmniej (zwieksz priorytet tego procesu)
  jedz(); //procedura jedzenia
  V(sem_widelec,ID); //odloz lewy widelec
  V(sem_widelec,(ID+MAX-1)%MAX); //odloz prawy widelec
  V(sem_talerzy,0); //odloz talerz
}

return 0;
}
