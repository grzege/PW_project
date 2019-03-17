#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define P 6//P miejsc w wagoniku
#define N 20 //N pasazerow
//N>P
int wagonik=0; //obecne obciazenie wagonika
int zielone=1;//zmienna realizujaca zielone swiatlo dla wagonika, zapala sie gdy wagonik rusza i gasnie gdy wyruszy

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;//mutex odpowiada za wylacznosc sekcji krytycznej
pthread_mutex_t mutex2=PTHREAD_MUTEX_INITIALIZER;//mutex odpowiada za wylacznosc sekcji krytycznej
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;//zmienna warunkowa - realizuje wstrzymanie watkow-pasazerow, ktorzy wsiada do wagonika, az do momentu powrotu
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;//zmienna warunkowa - realizuje oczekiwanie na zapelnienie sie wagonika pasazerami

void *kolejka() //funkcja watku realizujacego wagonik kolejki
{
  while(1)
  {
    pthread_cond_wait(&cond2,&mutex2);
    printf("\nKolejka rusza.\n\n");
    sleep(1);//czas jazdy
    wagonik-=P;
    zielone++;
    pthread_mutex_unlock(&mutex2);
    pthread_cond_broadcast(&cond);
  }
}

void *pasazer(void *nr)//funkcja watku realizujacego pasazera kolejki
{
  int numer=*(int*)nr;
  while(1)
  {
    pthread_mutex_lock(&mutex);
    sleep(1);//czas zajmowania miejsca przez pasażera
    if(wagonik<P)
    {
      wagonik+=1;
      printf("P%d, ",numer);
      pthread_cond_wait(&cond,&mutex);

    }
    else if(zielone)
    {
      --zielone;
      pthread_cond_signal(&cond2);
    }
    pthread_mutex_unlock(&mutex);
  }
}

int main()
{
  int nr[N];//numer porzadkowy kazdego z N pasazerow
  pthread_t pth[N],pth2;

  for(int i=0;i<N;i++)
  {
    nr[i]=i;
    if(!i)
    {
      if(pthread_create(&pth2,NULL,kolejka,NULL))
      {
        printf("Blad tworzenia wątku.\n");
        exit(1);
      }
    }
    if(pthread_create(&pth[i],NULL,pasazer,&nr[i]))
    {
      printf("Blad tworzenia wątku.\n");
      exit(1);
    }
  }
  for(int i=0;i<N;i++)
  {
    if(pthread_join(pth[i],NULL))
    {
      printf("Blad oczekiwania na zakonczenie watku.\n");
      exit(1);
    }
  }
  if(pthread_join(pth2,NULL))
  {
    printf("Blad oczekiwania na zakonczenie watku.\n");
    exit(1);
  }
  return 0;
}
