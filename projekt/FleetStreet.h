#include <iostream>
#include <mutex>
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <ncurses.h>
#include <algorithm>

constexpr int maxNoOfClients = 8;
constexpr int waitingRoomCapacity = 3;
constexpr int bakeryCapacity = 4;
constexpr int razorsCapacity = 3;

class FleetStreet
{
private:

public:
    bool stop, amIDead, amIFull, razorSignal;
    int myName, myName2, myPos, myPos2, meat, meatPies, uniqueID, money, bloodiedRazors;

    std::vector<int> clientsIDs;				// active clients ID's with index corresponding to index in thread vector
	std::vector<int> priorityList;				// list of priorities to decide what Mrs Lovett has to do next
	std::vector<int> barberShopStatus;			// 0-barber's chair, 1-3 lounge
	std::vector<int> bakeryStatus;				// 0 serving pies, 1-3 bakery queue
	std::vector<int> razorsStatus;				// razor status: 0 clean free, 1 clean taken, -1 bloodied free, -2 being cleaned

    FleetStreet();
    void menuInit();
    void barberFunction();
    void bakerFunction();
    void butFirstSirIThinkAShave(int);
    void theWorstPiesInLondon(int);
    void arrive(int);
    void startSimulation();
    void createClients();
    void timeToPay();
    void changeGUI();
    ~FleetStreet();
};