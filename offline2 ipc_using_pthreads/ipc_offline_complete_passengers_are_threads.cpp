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
sem_t skiosk_empty;
sem_t skiosk_full;

//locks aka mutex aka bin_sem
sem_t kiosk_lock;
sem_t io_lock;
sem_t belt_lock;
sem_t board_lock;
sem_t skiosk_lock;

pthread_mutex_t lr_count_lock;
pthread_mutex_t rl_count_lock;
pthread_mutex_t channel_lock;
pthread_mutex_t left_side_empty;

//constants to be replaced by file i/o
#define POISSON 1
int M = 2, N = 3, P = 3;
int w = 2, x = 2, y = 2, z = 2;

//counters and id's
int gid_count = 0, kiosk_id = 0;
auto START_TIME = high_resolution_clock::now();
int lr_count = 0, rl_count = 0;


int get_sys_time()
{
    auto END_TIME = high_resolution_clock::now();
    return duration_cast<seconds>(END_TIME - START_TIME).count();
}


class Passenger{
    int id;
    bool vip, board_pass = true;

    public:
        Passenger(bool v, int i){vip = v; id = i;}
        bool is_vip(){ return vip; }
        bool has_pass(){ return board_pass; }
        int getID(){ return id; }
        void loose_board_pass(){ board_pass = false; }
        void get_another_pass() { board_pass = true; }
};


//Waiting Lines
queue<Passenger*> kiosks;
queue<Passenger*> * belts;
queue<Passenger*> board_officer;
queue<Passenger*> skiosk;


void * PassengerLifeCycle(void * arg);
void * PassengerProducer(void * arg)
{
    srand(time(NULL)); int i = 5;
    while(i--)
    {
        sleep(POISSON);
        pthread_t passenger_t;
        bool status = false;
        if(rand()%2 == 0) status = true;
        Passenger * pass = new Passenger(status, ++gid_count);
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

void * startKioskOp(Passenger * pass, string status)
{
    //kiosk pop
    sem_wait(&kiosk_full);
    sem_wait(&kiosk_lock);

    kiosk_id++;
    if(kiosk_id > M) kiosk_id = 1;

    Passenger * p = kiosks.front();

    sem_wait(&io_lock);
    cout << "Passenger " << p->getID() << status << " has started self-check in at kiosk " << kiosk_id << " at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sleep(w);
    kiosks.pop();

    sem_wait(&io_lock);
    cout << "Passenger " << p->getID() << status << " has finished check in at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sem_post(&kiosk_lock);
    sem_post(&kiosk_empty);
}


void sendToSecurity(Passenger * pass, int belt_no)
{
    sem_wait(&belt_sem_emp[belt_no]);
    sem_wait(&belt_lock);

    belts[belt_no].push(pass);

    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() << " has started waiting for security check in belt " << belt_no << " at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sem_post(&belt_lock);
    sem_post(&belt_sem_full[belt_no]);
}

void startSecurityCheck(int belt_no)
{
    sem_wait(&belt_sem_full[belt_no]);
    sem_wait(&belt_lock);

    Passenger * p = belts[belt_no].front();

    sem_wait(&io_lock);
    cout << "Passenger " << p->getID() << " has started the security check at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sleep(x);
    belts[belt_no].pop();

    sem_wait(&io_lock);
    cout << "Passenger " << p->getID() << " has crossed the security check at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sem_post(&belt_lock);
    sem_post(&belt_sem_emp[belt_no]);
}

void sendToBoarding(Passenger * pass, string status)
{
    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() << status <<" has started waiting to be boarded at time " << get_sys_time() << endl;
    if(rand()%2 == 0) pass->loose_board_pass();
    sem_post(&io_lock);

    sem_wait(&boarding_empty);
    sem_wait(&board_lock);

    //gone in to be checked by officer
    board_officer.push(pass);

    sem_post(&board_lock);
    sem_post(&boarding_full);
}

void check_and_board(string status)
{
    sem_wait(&boarding_full);
    sem_wait(&board_lock);

    Passenger * pass = board_officer.front();

    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() << status <<" has started boarding the plane at time  " << get_sys_time() << endl;
    sem_post(&io_lock);

    sleep(y);

    board_officer.pop();

    sem_wait(&io_lock);
    if(pass->has_pass())
        cout << "Passenger " << pass->getID() << status <<" has boarded the plane at time  " << get_sys_time() << endl;
    else
        cout << "Passenger " << pass->getID() << status <<" has \tfailed to show the boarding pass at time  " << get_sys_time() << endl;
    sem_post(&io_lock);


    sem_post(&board_lock);
    sem_post(&boarding_empty);
}

void LeftToRightVIP(Passenger * pass, string status)
{
    pthread_mutex_lock(&lr_count_lock);
    lr_count++;
    if(lr_count == 1){
        pthread_mutex_lock(&left_side_empty);
        pthread_mutex_lock(&channel_lock);
    }
    pthread_mutex_unlock(&lr_count_lock);

    //passenger passed
    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() << status <<" has passed the VIP Channel(LR) at time  " << get_sys_time() << endl;
    sem_post(&io_lock);

    pthread_mutex_lock(&lr_count_lock);
    lr_count--;
    if(lr_count == 0){
        pthread_mutex_unlock(&channel_lock);
        pthread_mutex_unlock(&left_side_empty);
    }
    pthread_mutex_unlock(&lr_count_lock);
}

void RightToLeftVIP(Passenger * pass, string status)
{
    //block right-people when left-people exists
    pthread_mutex_lock(&left_side_empty);
    pthread_mutex_unlock(&left_side_empty);

    pthread_mutex_lock(&rl_count_lock);
    rl_count++;
    if(rl_count == 1){
        pthread_mutex_lock(&channel_lock);
    }
    pthread_mutex_unlock(&rl_count_lock);

    //passenger passed
    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() << status <<" has passed the VIP Channel(RL) at time  " << get_sys_time() << endl;
    sem_post(&io_lock);

    pthread_mutex_lock(&rl_count_lock);
    rl_count--;
    if(rl_count == 0){
        pthread_mutex_unlock(&channel_lock);
    }
    pthread_mutex_unlock(&rl_count_lock);
}

void sendToSpecialKiosk(Passenger * pass)
{
    sem_wait(&skiosk_empty);
    sem_wait(&skiosk_lock);

    //push into kiosks here
    skiosk.push(pass);

    sem_post(&skiosk_lock);
    sem_post(&skiosk_full);
}

void startSpecialKioskOp(string status)
{
    //kiosk pop
    sem_wait(&skiosk_full);
    sem_wait(&skiosk_lock);

    Passenger * p = skiosk.front();

    sem_wait(&io_lock);
    cout << "Passenger " << p->getID() << status << " has started self-check in at the special kiosk at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sleep(w);
    skiosk.pop();

    sem_wait(&io_lock);
    p->get_another_pass();
    cout << "Passenger " << p->getID() << status << " has finished check in special kiosk at time " << get_sys_time() << endl;
    sem_post(&io_lock);

    sem_post(&skiosk_lock);
    sem_post(&skiosk_empty);
}

void * PassengerLifeCycle(void * arg)
{
    srand(time(NULL));
    string status = "";
    Passenger * pass = (Passenger *) arg;
    if(pass->is_vip()) status = " (VIP)";

    sem_wait(&io_lock);
    cout << "Passenger " << pass->getID() << status <<" has arrived at airport at time " << get_sys_time() << endl;
    sem_post(&io_lock);


    //kiosk op
    sendToKiosk(pass);
    startKioskOp(pass, status);

    //belt op
    if(!pass->is_vip())
    {
        int belt_no = rand()%N;
        sendToSecurity(pass, belt_no);
        startSecurityCheck(belt_no);
    }else{
        LeftToRightVIP(pass, status);
    }

    //boarding
    while(1)
    {
        sendToBoarding(pass, status);
        check_and_board(status);
        if(pass->has_pass()) break;

        //else use VIP Channel to go RL
        RightToLeftVIP(pass, status);

        //special kiosk
        sendToSpecialKiosk(pass);
        startSpecialKioskOp(status);

        //then use VIP Channel to go LR
        LeftToRightVIP(pass, status);
    }
}

void init_sem()
{
    //io_lock
    sem_init(&io_lock, 0, 1);

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
    sem_init(&board_lock, 0, 1);
    sem_init(&boarding_empty, 0, 1);
    sem_init(&boarding_full, 0, 0);

    //special kiosk
    sem_init(&skiosk_lock, 0, 1);
    sem_init(&skiosk_empty, 0, 1);
    sem_init(&skiosk_full, 0, 0);

    //vip channel locks
    pthread_mutex_init(&lr_count_lock, NULL);
    pthread_mutex_init(&rl_count_lock, NULL);
    pthread_mutex_init(&channel_lock, NULL);
    pthread_mutex_init(&left_side_empty, NULL);

}

void init_pthreads()
{
	char * dummy_msg = "";
    pthread_t producer_t;
	pthread_create(&producer_t, NULL, PassengerProducer, (void*)dummy_msg);

}


int main()
{
    belts = new queue<Passenger*>[N];

    init_sem();
    init_pthreads();

	pthread_exit(NULL);
    return 0;
}
