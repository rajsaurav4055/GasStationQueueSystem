#include <iostream>
#include <queue>
#include <math.h>
#include <random>
using namespace std;

class Event{
    friend  bool operator<(const Event& e1, const Event& e2);
    friend bool operator==(const Event& e1 , const Event& e2);
    public:
        enum EvtType {arrival, departure1, departure2, departure3};
        Event(){}
        Event(EvtType type, double etime):_type(type), _etime(etime){}
        Event(EvtType type, double etime, long ID):_type(type), _etime(etime), _ID(ID){}
        Event(EvtType type, double etime, long ID, string serverChosen):_type(type), _etime(etime), _ID(ID),_serverChosen(serverChosen){}
        EvtType get_type() {return _type;}
        double get_time() {return _etime;}
        double get_ID() {return _ID;}
        void set_ID(int ID) {_ID=ID;}
        string get_serverChosen(){return _serverChosen;}
    protected:
        EvtType _type;
        double _etime;
        long _ID;
        string _serverChosen;

};

//global variables
double clocktime, meanInterArrivalTime, meanServiceTime1, meanServiceTime2, meanServiceTime3, lastEventTime1, lastEventTime2, lastEventTime3, totalBusy1, totalBusy2, totalBusy3, maxQueueLength12,maxQueueLength1, maxQueueLength2, maxQueueLength3, sumResponseTime;

long numberOfCustomers, queueLength12, queueLength1, queueLength2, queueLength3, numberInService1, numberInService2, numberInService3, totalCustomers, numberOfDepartures1, numberOfDepartures2, numberOfDepartures3;

priority_queue <Event> futureEventList;

queue <Event> customers12, customers1, customers2, customers3;

long serialID=0;
int numberOfCustomersArrived = 0;

bool operator <(const Event& e1, const Event& e2){
    return e2._etime < e1._etime;
}

bool operator ==(const Event& e1, const Event& e2){
    return e2._etime == e1._etime;
}


string serverChosen(){
    return ((rand() > RAND_MAX*0.53) ? "s2":"s1");
}

double unif(){
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    return dist(mt);
}

double exponential(double mean) {
    return -mean*log(unif());
}

void initialization(){
    //initialize global variables
    clocktime = 0;
    queueLength12=0;
    queueLength1=0;
    queueLength2=0;
    queueLength3 = 0;
    numberInService1=0;
    numberInService2=0;
    numberInService3=0;
    lastEventTime1 = 0;
    totalBusy1 = 0;
    lastEventTime2 = 0;
    totalBusy2 = 0;
    totalBusy3 = 0;
    lastEventTime3 = 0;
    maxQueueLength12=0;
    maxQueueLength1=0;
    maxQueueLength2=0;
    maxQueueLength3=0;
    sumResponseTime=0;
    numberOfDepartures1=0;
    numberOfDepartures2=0;

    Event evt(Event::arrival,exponential(meanInterArrivalTime), serialID,serverChosen());
    futureEventList.push(evt);
}


void scheduleDeparture1(long RID)
{
    double serviceTime;
    serviceTime = exponential(meanServiceTime1);
    Event depart1(Event::departure1,clocktime+serviceTime, RID);
    futureEventList.push(depart1);
    numberInService1=1;
    queueLength1--;
}

void scheduleDeparture2(long RID)
{
    double serviceTime;
    serviceTime = exponential(meanServiceTime2);
    Event depart2(Event::departure2,clocktime+serviceTime, RID);
    futureEventList.push(depart2);
    numberInService2=1;
    queueLength2--;
}

void scheduleDeparture3(long RID){
    double serviceTime;
    serviceTime= exponential(meanServiceTime3);
    Event depart3(Event::departure3,clocktime+serviceTime, RID);
    futureEventList.push(depart3);
    numberInService3=1;
    queueLength3--;
    
}

void processArrival(Event evt){

    numberOfCustomersArrived++;
    queueLength12++; //increament number waiting for either going to s1 or s2 
    customers12.push(evt); //customers in queue 1
    //if server1 is idle, fetch the event, do statistics, and put into service.
    if(evt.get_serverChosen() == "s1"){
        queueLength1++;
        queueLength12--;
        customers1.push(evt);
    }else if(evt.get_serverChosen()== "s2"){
        queueLength2++;
        queueLength12--;
        customers2.push(evt);
    }
    if(evt.get_serverChosen() == "s1" && numberInService1==0){
        scheduleDeparture1(evt.get_ID());
        lastEventTime1 = clocktime; //added here instead of out of the if statement
    }else if(evt.get_serverChosen() == "s2" && numberInService2 == 0){
        scheduleDeparture2(evt.get_ID());
        lastEventTime2 = clocktime; //added here instead 
    }else if(evt.get_serverChosen() == "s1" && numberInService1==1){
        totalBusy1+=(clocktime - lastEventTime1);
    }else if(evt.get_serverChosen() == "s2" && numberInService2==1){
        totalBusy2+=(clocktime - lastEventTime2);
    }

    //adjust max queue length statistics
    if(maxQueueLength12 <queueLength12) maxQueueLength12 = queueLength12;
    if(maxQueueLength1 <queueLength1) maxQueueLength1 = queueLength1;
    if(maxQueueLength2 <queueLength2) maxQueueLength2 = queueLength2;

    //schedule the next arrival
    serialID++;
    Event next_arrival(Event::arrival,clocktime+ exponential(meanInterArrivalTime), serialID, serverChosen());
    futureEventList.push(next_arrival);
    
}

void processDeparture1(Event evt)
{
    Event finished1=customers1.front();
    customers1.pop();
    if(queueLength1 > 0 && customers1.front().get_serverChosen() == "s1") scheduleDeparture1(customers1.front().get_ID());
    else numberInService1 =0;
    totalBusy1+= (clocktime - lastEventTime1);
    numberOfDepartures1++;
    lastEventTime1= clocktime;

    //push at server 2 customers, increase queue length and schedule departure,
    // same as process arrival, except no next arrival known
    queueLength3++;
    customers3.push(finished1);
    if(numberInService3==0){
        scheduleDeparture3(finished1.get_ID());
        lastEventTime3= clocktime;
    }
    else totalBusy3+=(clocktime - lastEventTime3);
    //adjust max queue length statistics
    if(maxQueueLength3 <queueLength3) maxQueueLength3 = queueLength3;
}

void processDeparture2(Event evt)
{
    Event finished2=customers2.front();
    customers2.pop();
    if(queueLength2>0 && customers2.front().get_serverChosen() == "s2") scheduleDeparture2(customers2.front().get_ID());
    else numberInService2 =0;
    double Response = (clocktime -finished2.get_time());
    sumResponseTime += Response;
    totalBusy2+= (clocktime -lastEventTime2);
    numberOfDepartures2++;
    lastEventTime2= clocktime;
}

void processDeparture3(Event evt)
{
    Event finished3=customers3.front();
    customers3.pop();
    if(queueLength3>0) scheduleDeparture3(customers3.front().get_ID());
    else numberInService3 =0;
    double Response = (clocktime -finished3.get_time());
    sumResponseTime += Response;
    totalBusy3+= (clocktime -lastEventTime3);
    numberOfDepartures3++;
    lastEventTime3= clocktime;
}


void reportGeneration(){
    double RHO1 = totalBusy1/ clocktime;
    double RHO2 = totalBusy2/ clocktime;
    double RHO3 = totalBusy3/ clocktime;
    double AVGR = sumResponseTime/ totalCustomers;

    cout<<"3 SERVER QUEUE SIMULATION - Circle K checkout counters"<<'\n'<< endl;
    cout<<'\t'<<"MEAN INTERARRIVAL TIME: "<<meanInterArrivalTime<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 1: "<<meanServiceTime1<<" minutes"<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 2: "<<meanServiceTime2<<" minutes"<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 3: "<<meanServiceTime3<<" minutes"<<endl;
    cout<<'\t'<<"NUMBER OF CUSTOMERS ARRIVED "<<numberOfCustomersArrived<<endl;
    cout<<'\t'<<"NUMBER OF DEPARTURES: "<< numberOfCustomersArrived <<endl;
    cout<<'\t'<<"SERVER 1 UTILIZATION: "<<RHO1<<endl;
    cout<<'\t'<<"SERVER 2 UTILIZATION: "<<RHO2<<endl;
    cout<<'\t'<<"SERVER 3 UTILIZATION: "<<RHO3<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 1: "<<maxQueueLength1<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 2: "<<maxQueueLength2<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 3: "<<maxQueueLength3<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 12: "<<maxQueueLength12<<endl;
    cout<<'\t'<<"AVERAGE RESPONSE TIME: "<<AVGR<< "minutes"<<endl;
    cout<<'\t'<<"SIMULATION RUN LENGTH: "<< clocktime << "minutes"<< endl;

}


int main(int argc , char* argv[]){
    unsigned seed ;  
    srand(time(0));
    meanInterArrivalTime = 2.67;
    meanServiceTime1= 3.1; 
    meanServiceTime2= 2.68; 
    meanServiceTime3= 2.55;
    totalCustomers=100;
    initialization();
    while( (numberOfCustomersArrived) < totalCustomers){

        Event evt= futureEventList.top();
        futureEventList.pop();
        clocktime = evt.get_time();
        if(evt.get_type() == Event::arrival){
            processArrival(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " arrived at " <<evt.get_serverChosen()<< endl;
        }
        else if(evt.get_type() == Event::departure1) {
            processDeparture1(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 1" << endl;
        }
        else if(evt.get_type() == Event::departure2){
            processDeparture2(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 2" << endl;
        }else{
            processDeparture3(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 3" << endl;
        }
    }
    reportGeneration();
    cout<< "End of Simulation !" << endl;
}