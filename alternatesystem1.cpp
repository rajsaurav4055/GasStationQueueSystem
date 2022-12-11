#include <iostream>
#include <queue>
#include <math.h>
#include <random>
using namespace std;

class Event{
    friend  bool operator<(const Event& e1, const Event& e2);
    friend bool operator==(const Event& e1 , const Event& e2);
    public:
        enum EvtType {arrival, departure1, departure2, departure3, departure4};
        Event(){}
        Event(EvtType type, double etime):_type(type), _etime(etime){}
        Event(EvtType type, double etime, long ID):_type(type), _etime(etime), _ID(ID){}
        Event(EvtType type, double etime, long ID, string serverChosen):_type(type), _etime(etime), _ID(ID),_serverChosen(serverChosen){}
        EvtType get_type() {return _type;}
        double get_time() {return _etime;}
        double get_ID() {return _ID;}
        void set_ID(int ID) {_ID=ID;}
        void set_serverChosen(string s){_serverChosen=s;}
        string get_serverChosen(){return _serverChosen;}
    protected:
        EvtType _type;
        double _etime;
        long _ID;
        string _serverChosen;

};

//global variables
double clocktime, meanInterArrivalTime, meanServiceTime1, meanServiceTime2, meanServiceTime3, meanServiceTime4, lastEventTime1, lastEventTime2, lastEventTime3, lastEventTime4, totalBusy1, totalBusy2, totalBusy3, totalBusy4, maxQueueLength12, maxQueueLength3, maxQueueLength4, sumResponseTime;

long numberOfCustomers, queueLength12, queueLength3, queueLength4, numberInService1, numberInService2, numberInService3, numberInService4, totalCustomers, numberOfDepartures1, numberOfDepartures2, numberOfDepartures3, numberOfDepartures4;

priority_queue <Event> futureEventList;

queue <Event> customers12, customers3, customers4;

long serialID=0;
int numberOfCustomersArrived = 0;

bool operator <(const Event& e1, const Event& e2){
    return e2._etime < e1._etime;
}

bool operator ==(const Event& e1, const Event& e2){
    return e2._etime == e1._etime;
}


// string serverChosen(){
//     return ((rand() > RAND_MAX*0.53) ? "s2":"s1");
// }

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
    queueLength3 = 0;
    queueLength4 = 0;
    numberInService1=0;
    numberInService2=0;
    numberInService3=0;
    numberInService4=0;
    lastEventTime1 = 0;
    totalBusy1 = 0;
    lastEventTime2 = 0;
    totalBusy2 = 0;
    totalBusy3 = 0;
    lastEventTime3 = 0;
    totalBusy4=0;
    lastEventTime4=0;
    maxQueueLength12=0;
    maxQueueLength3=0;
    maxQueueLength4=0;
    sumResponseTime=0;
    numberOfDepartures1=0;
    numberOfDepartures2=0;
    numberOfDepartures3=0;
    numberOfDepartures4=0;

    Event evt(Event::arrival,exponential(meanInterArrivalTime), serialID,"server 1");
    futureEventList.push(evt);
}


void scheduleDeparture1(long RID)
{
    double serviceTime;
    serviceTime = exponential(meanServiceTime1);
    Event depart1(Event::departure1,clocktime+serviceTime, RID);
    futureEventList.push(depart1);
    numberInService1=1;
    queueLength12--;
}

void scheduleDeparture2(long RID)
{
    double serviceTime;
    serviceTime = exponential(meanServiceTime2);
    Event depart2(Event::departure2,clocktime+serviceTime, RID);
    futureEventList.push(depart2);
    numberInService2=1;
    queueLength12--;
}

void scheduleDeparture3(long RID){
    double serviceTime;
    serviceTime= exponential(meanServiceTime3);
    Event depart3(Event::departure3,clocktime+serviceTime, RID);
    futureEventList.push(depart3);
    numberInService3=1;
    queueLength3--;
    
}

void scheduleDeparture4(long RID){
    double serviceTime;
    serviceTime = exponential(meanServiceTime4);
    Event depart4(Event::departure4,clocktime+serviceTime, RID);
    futureEventList.push(depart4);
    numberInService4=1;
    queueLength4--;
}

void processArrival(Event evt){
    numberOfCustomersArrived++;
    queueLength12++; //increament number waiting for either going to s1 or s2 
    customers12.push(evt); //customers in queue 1
    //if server1 is idle, fetch the event, do statistics, and put into service.
    if(numberInService1==0){
        // cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " arrived at server 1"<< endl;
        // evt.set_serverChosen("server 1");
        scheduleDeparture1(evt.get_ID());
        lastEventTime1 = clocktime; //added here instead of out of the if statement
    }else if(numberInService2==0){
        // cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " arrived at server 2"<< endl;
        // evt.set_serverChosen("server 2");
        scheduleDeparture2(evt.get_ID());
        lastEventTime2 = clocktime; //added here instead 
    }else{
        totalBusy1+=(clocktime - lastEventTime1);
        totalBusy2+=(clocktime - lastEventTime2);
    }

    //adjust max queue length statistics
    if(maxQueueLength12 < queueLength12) maxQueueLength12 = queueLength12;

    //schedule the next arrival
    serialID++;
    Event next_arrival(Event::arrival,clocktime+ exponential(meanInterArrivalTime), serialID);
    futureEventList.push(next_arrival);
    
}

void processDeparture1(Event evt)
{
    Event finished1=customers12.front();
    customers12.pop();
    if(queueLength12 > 0){
        scheduleDeparture1(customers12.front().get_ID());
    } 
    else{
        numberInService1 =0;
    } 
    
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
    if(maxQueueLength3 < queueLength3) maxQueueLength3 = queueLength3;
}

void processDeparture2(Event evt)
{
    Event finished2=customers12.front();
    customers12.pop();
    if(queueLength12 > 0) {
        scheduleDeparture2(customers12.front().get_ID());
    }
    else {
        numberInService2 =0;
    }
    // double Response = (clocktime -finished2.get_time());
    // sumResponseTime += Response;
    totalBusy2+= (clocktime -lastEventTime2);
    numberOfDepartures2++;
    lastEventTime2= clocktime;

    //push at server 4 customers, increase queue length and schedule departure,
    //same as process arrival, except no next arrival known
    queueLength4++;
    customers4.push(finished2);
    if(numberInService4==0){
        scheduleDeparture4(finished2.get_ID());
        lastEventTime4=clocktime;
    }
    else  totalBusy4+=(clocktime-lastEventTime4);
    //adjust max queue length statistics
    if(maxQueueLength4 < queueLength4) maxQueueLength4 = queueLength4;
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

void processDeparture4(Event evt){
    Event finished4=customers4.front();
    customers4.pop();
    if(queueLength4>0) scheduleDeparture4(customers4.front().get_ID());
    else numberInService4 =0;
    double Response = (clocktime -finished4.get_time());
    sumResponseTime += Response;
    totalBusy4+= (clocktime -lastEventTime4);
    numberOfDepartures4++;
    lastEventTime4= clocktime;
}


void reportGeneration(){
    double RHO1 = totalBusy1/ clocktime;
    double RHO2 = totalBusy2/ clocktime;
    double RHO3 = totalBusy3/ clocktime;
    double RHO4 = totalBusy4/ clocktime;
    double AVGR = sumResponseTime/ totalCustomers;

    cout<<"3 SERVER QUEUE SIMULATION - Circle K checkout counters"<<'\n'<< endl;
    cout<<'\t'<<"MEAN INTERARRIVAL TIME: "<<meanInterArrivalTime<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 1: "<<meanServiceTime1<<" minutes"<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 2: "<<meanServiceTime2<<" minutes"<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 3: "<<meanServiceTime3<<" minutes"<<endl;
    cout<<'\t'<<"MEAN SERVICE TIME for server 4: "<<meanServiceTime4<<" minutes"<<endl;
    cout<<'\t'<<"NUMBER OF CUSTOMERS ARRIVED "<<numberOfCustomersArrived<<endl;
    cout<<'\t'<<"NUMBER OF DEPARTURES: "<< numberOfCustomersArrived <<endl;
    cout<<'\t'<<"SERVER 1 UTILIZATION: "<<RHO1<<endl;
    cout<<'\t'<<"SERVER 2 UTILIZATION: "<<RHO2<<endl;
    cout<<'\t'<<"SERVER 3 UTILIZATION: "<<RHO3<<endl;
    cout<<'\t'<<"SERVER 4 UTILIZATION: "<<RHO4<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 1: "<<maxQueueLength12<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 2: "<<maxQueueLength3<<endl;
    cout<<'\t'<<"MAXIMUM QUEUE LENGTH FOR WAITING LINE 3: "<<maxQueueLength4<<endl;
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
    meanServiceTime4= 2.5;
    totalCustomers=100;
    initialization();
    while( (numberOfCustomersArrived) < totalCustomers){

        Event evt= futureEventList.top();
        futureEventList.pop();
        clocktime = evt.get_time();
        if(evt.get_type() == Event::arrival){
            processArrival(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " arrived to the system"<< endl;
        }
        else if(evt.get_type() == Event::departure1) {
            processDeparture1(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 1" << endl;
        }
        else if(evt.get_type() == Event::departure2){
            processDeparture2(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 2" << endl;
        }else if(evt.get_type() == Event::departure3){
            processDeparture3(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 3" << endl;
        }else{
            processDeparture4(evt);
            cout<< "Time: " << clocktime <<" :\t customer " << evt.get_ID() << " finished from server 4" << endl;
        }
    }
    reportGeneration();
    cout<< "End of Simulation !" << endl;
}