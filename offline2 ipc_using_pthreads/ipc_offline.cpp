#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include <unistd.h>
#include <bits/stdc++.h>

using namespace std;
//semaphores
sem_t kiosk_empty;
sem_t kiosk_full;
sem_t kiosk_mutex;

//constants to be replaced by file i/o
#define M 2
#define POISSON 1
int gid_count = 0;


class Passenger{
    bool vip;
    int id;

    public:
        Passenger(bool v, int i){vip = v; id = i;}
        bool is_vip(){ return vip; }
        int getID(){ return id; }
};


//Waiting Lines
queue<Passenger> kiosks;

void * SentToAirport(void * arg)
{
    Passenger pass(((Passenger *)arg)->is_vip(), ((Passenger *)arg)->getID());
    cout << "Passenger " << pass.getID() <<" arrived at airport!" << endl;
    sem_wait(&kiosk_empty);
    sem_wait(&kiosk_mutex);

    //push into kiosks here
    cout << "Passenger " << pass.getID() <<" pushed" << endl;
    kiosks.push(pass);

    sem_post(&kiosk_mutex);
    sem_post(&kiosk_full);
}


void * PassengerProducer(void * arg)
{
    int i = 13;
    while(i--)
    {
        sleep(POISSON);
        pthread_t temp;
        Passenger * pass = new Passenger(rand()%2, ++gid_count);
        pthread_create(&temp, NULL, SentToAirport, pass);
    }
}



void * KioskHandler(void * arg)
{
    while(1)
    {
        sem_wait(&kiosk_full);
        sem_wait(&kiosk_mutex);

        //pop from kiosks to push into random belt
        sleep(2);
        Passenger p = kiosks.front();
        kiosks.pop();
        cout << "Passenger " << p.getID() << " done at kiosk!\n";

        sem_post(&kiosk_mutex);
        sem_post(&kiosk_empty);
    }
}



int main()
{
    srand(time(NULL));
    pthread_t pass_gen_thread;
    pthread_t kiosk_thread;

    //init_semaphore();
    sem_init(&kiosk_empty, 0, M);
    sem_init(&kiosk_full, 0, 0);
    sem_init(&kiosk_mutex, 0, 1);

	char * dummy_msg = "";
	pthread_create(&pass_gen_thread, NULL, PassengerProducer, (void*)dummy_msg);
	pthread_create(&kiosk_thread, NULL, KioskHandler, (void*)dummy_msg);

	pthread_exit(NULL);
    return 0;
}

