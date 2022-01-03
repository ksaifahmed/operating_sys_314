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
sem_t * belt_sem_full;
sem_t * belt_sem_emp;
sem_t boarding_empty;
sem_t boarding_full;

sem_t kiosk_lock;
sem_t io_lock;
sem_t belt_lock;
sem_t board_lock;

//constants to be replaced by file i/o
#define POISSON 1
int M = 2, N = 3, P = 3;
int w = 2, x = 2, y = 2, z = 2;
int gid_count = 0, kiosk_id = 0;
auto START_TIME = high_resolution_clock::now();


int get_sys_time()
{
    auto END_TIME = high_resolution_clock::now();
    return duration_cast<seconds>(END_TIME - START_TIME).count();
}

class Passenger{
    int id;
    bool vip, pass = true;

    public:
        Passenger(bool v, int i){vip = v; id = i;}
        bool is_vip(){ return vip; }
        bool has_pass(){ return pass; }
        int getID(){ return id; }
        void set_pass(bool b){ pass = b; }

};


//Waiting Lines
queue<Passenger*> kiosks;
queue<Passenger*> * belts;
queue<Passenger*> board_officer;


void * PassengerLifeCycle(void * arg);
void * PassengerProducer(void * arg)
{
    while(1)
    {
        sleep(POISSON);
        pthread_t passenger_t;
        Passenger * pass = new Passenger(rand()%2, ++gid_count);
        pthread_create(&passenger_t, NULL, PassengerLifeCycle, pass);
    }
}

void sendToKiosk(Passenger * pass)
{
    sem_wait(&kiosk_empty);
    sem_wait(&kiosk_lock);

    //push into kiosks here
    kiosks.push(pass);

    sem_post(&kiosk_lock);
    sem_post(&kiosk_full);
}

void * startKioskOp(Passenger * pass)
{
    //kiosk pop
    sem_wait(&kiosk_full);
    sem_wait(&kiosk_lock);

    kiosk_id++;
    if(kiosk_id > M) kiosk_id = 1;

    Passenger * p = kiosks.front();
    cout << "Passenger " << p->getID() << " has started self-check in at kiosk " << kiosk_id << " at time " << get_sys_time() << endl;
    sleep(w);
    kiosks.pop();
    cout << "Passenger " << p->getID() << " has finished check in at time " << get_sys_time() << endl;

    sem_post(&kiosk_lock);
    sem_post(&kiosk_empty);
}


void sendToSecurity(Passenger * pass, int belt_no)
{
    sem_wait(&belt_sem_emp[belt_no]);
    sem_wait(&belt_lock);

    belts[belt_no].push(pass);
    cout << "Passenger " << pass->getID() << " has started waiting for security check in belt " << belt_no << " at time " << get_sys_time() << endl;

    sem_post(&belt_lock);
    sem_post(&belt_sem_full[belt_no]);
}

void startSecurityCheck(int belt_no)
{
    sem_wait(&belt_sem_full[belt_no]);
    sem_wait(&belt_lock);

    Passenger * p = belts[belt_no].front();
    cout << "Passenger " << p->getID() << " has started the security check at time " << get_sys_time() << endl;
    sleep(x);
    belts[belt_no].pop();
    cout << "Passenger " << p->getID() << " has crossed the security check at time " << get_sys_time() << endl;

    sem_post(&belt_lock);
    sem_post(&belt_sem_emp[belt_no]);
}

void sendToBoarding(Passenger * pass)
{
    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() <<" has started waiting to be boarded at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sem_wait(&boarding_empty);
    sem_wait(&board_lock);

    //gone in to be checked by officer
    board_officer.push(pass);

    sem_post(&board_lock);
    sem_post(&boarding_full);
}

void check_and_board()
{
    sem_wait(&boarding_full);
    sem_wait(&board_lock);

    Passenger * pass = board_officer.front();
    cout << "Passenger " << pass->getID() <<" has started boarding the plane at time  " << get_sys_time() << endl;
    sleep(y);
    board_officer.pop();
    cout << "Passenger " << pass->getID() <<" has boarded the plane at time  " << get_sys_time() << endl;

    sem_post(&board_lock);
    sem_post(&boarding_empty);
}

void * PassengerLifeCycle(void * arg)
{
    Passenger * pass = (Passenger *) arg;
    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() <<" has arrived at airport at time " << get_sys_time() << endl;
    sem_post(&io_lock);


    //kiosk op
    sendToKiosk(pass);
    startKioskOp(pass);

    //belt op
    int belt_no = rand()%N;
    sendToSecurity(pass, belt_no);
    startSecurityCheck(belt_no);

    //boarding
    sendToBoarding(pass);
    check_and_board();
}

void init_sem()
{
    //kiosk_sem
    sem_init(&kiosk_empty, 0, M);
    sem_init(&kiosk_full, 0, 0);
    sem_init(&kiosk_lock, 0, 1);

    //belt_sem
    sem_init(&belt_lock, 0, 1);
    belt_sem_emp = new sem_t[N];
    belt_sem_full = new sem_t[N];
    for(int i=0; i<N; i++){
        sem_init(&belt_sem_emp[i], 0, P);
        sem_init(&belt_sem_full[i], 0, 0);
    }

    //boarding sem
    sem_init(&boarding_empty, 0, 1);
    sem_init(&boarding_full, 0, 0);

}

void init_pthreads()
{
	char * dummy_msg = "";
    pthread_t producer_t;
	pthread_create(&producer_t, NULL, PassengerProducer, (void*)dummy_msg);

}


int main()
{
    srand(time(NULL));
    belts = new queue<Passenger*>[N];

    init_sem();
    init_pthreads();

	pthread_exit(NULL);
    return 0;
}

