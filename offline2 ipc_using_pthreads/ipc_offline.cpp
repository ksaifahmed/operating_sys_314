#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include <unistd.h>
#include <chrono>
#include <bits/stdc++.h>

using namespace std;
using namespace chrono;
//semaphores
sem_t kiosk_empty;
sem_t kiosk_full;
sem_t bin_sem;
sem_t * belt_sem_full;
sem_t * belt_sem_emp;

//constants to be replaced by file i/o
#define POISSON 1
int M = 2, N = 3, P = 3;
int w = 2, x = 2, y = 2, z = 2;
int gid_count = 0;
auto START_TIME = high_resolution_clock::now();


int get_sys_time()
{
    auto END_TIME = high_resolution_clock::now();
    return duration_cast<seconds>(END_TIME - START_TIME).count();
}

class Passenger{
    bool vip;
    int id, s_time = 0;

    public:
        Passenger(bool v, int i){vip = v; id = i;}
        bool is_vip(){ return vip; }
        int getID(){ return id; }
        int get_stime(){ return s_time; }
        void inc_stime(int t){ s_time += t; }
};


//Waiting Lines
queue<Passenger> kiosks;
queue<Passenger> * belts;

void * SentToAirport(void * arg)
{
    Passenger pass(((Passenger *)arg)->is_vip(), ((Passenger *)arg)->getID());
    sem_wait(&bin_sem);
    cout << "Passenger " << pass.getID() <<" has arrived at airport at time " << get_sys_time() << endl;
    sem_post(&bin_sem);

    sem_wait(&kiosk_empty);
    sem_wait(&bin_sem);

    //push into kiosks here
    kiosks.push(pass);

    sem_post(&bin_sem);
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
    int kiosk_id = 0;
    while(1)
    {
        kiosk_id++;
        if(kiosk_id > M) kiosk_id = 1;
        int belt_no = rand()%N;

        //kiosk pop
        sem_wait(&kiosk_full);
        sem_wait(&bin_sem);

        Passenger p = kiosks.front();
        cout << "Passenger " << p.getID() << " has started self-check in at kiosk " << kiosk_id << " at time " << get_sys_time() << endl;
        sleep(w);
        kiosks.pop();
        cout << "Passenger " << p.getID() << " has finished check in at time " << get_sys_time() << endl;

        sem_post(&bin_sem);
        sem_post(&kiosk_empty);


        //push into random belt
        sem_wait(&belt_sem_emp[belt_no]);
        sem_wait(&bin_sem);

        belts[belt_no].push(p);
        cout << "Passenger " << p.getID() << " has started waiting for security check in belt " << belt_no << " at time " << get_sys_time() << endl;

        sem_post(&bin_sem);
        sem_post(&belt_sem_full[belt_no]);

    }
}


void * SecurityCheck(void * arg)
{
    while(1)
    {
        int * belt_id = (int *) arg;
        sem_wait(&belt_sem_full[*belt_id]);
        sem_wait(&bin_sem);

        Passenger p = belts[*belt_id].front();
        cout << "Passenger " << p.getID() << " has started the security check at time " << get_sys_time() << endl;
        sleep(x);
        belts[*belt_id].pop();
        cout << "Passenger " << p.getID() << " has crossed the security check at time " << get_sys_time() << endl;

        sem_post(&bin_sem);
        sem_post(&belt_sem_emp[*belt_id]);
    }
}



void init_sem()
{
    //kiosk_sem
    sem_init(&kiosk_empty, 0, M);
    sem_init(&kiosk_full, 0, 0);
    sem_init(&bin_sem, 0, 1);

    //belt_sem
    belt_sem_emp = new sem_t[N];
    belt_sem_full = new sem_t[N];
    for(int i=0; i<N; i++){
        sem_init(&belt_sem_emp[i], 0, P);
        sem_init(&belt_sem_full[i], 0, 0);
    }

}

void init_pthreads()
{
	char * dummy_msg = "";
    pthread_t pass_gen_thread;
    pthread_t kiosk_thread;
    pthread_t belt_threads[N];

	pthread_create(&pass_gen_thread, NULL, PassengerProducer, (void*)dummy_msg);
	pthread_create(&kiosk_thread, NULL, KioskHandler, (void*)dummy_msg);

	for(int i=0; i<N; i++){
        int * belt_id = new int;
        *belt_id = i;
        pthread_create(&belt_threads[i], NULL, SecurityCheck, (void *)belt_id);
	}

}


int main()
{
    srand(time(NULL));
    belts = new queue<Passenger>[N];

    init_sem();
    init_pthreads();

	pthread_exit(NULL);
    return 0;
}

