#include "FleetStreet.h"

std::vector<std::thread> clients;
std::thread clientCreator;
std::thread barber;							// Sweeney Todd thread
std::thread baker;							// Mrs Lovett thread
std::thread GUI;							// Thread to refresh barber and bakery status
std::mutex bakery[bakeryCapacity];			// bakery queue as mutexes, pies are served at bakery[0], 1-3 is a queue
std::mutex waitingRoom[waitingRoomCapacity];// chairs in waiting room as mutexes
std::mutex razors[razorsCapacity];			// razors as mutexes. Need 2 to perform a shave. After using become bloodied and need to be cleaned before next use
std::mutex barberChair;		 				// barber's chair as mutex
std::mutex chute;							// chute as mutex
std::mutex myMutex; 						// mutex for keeping cout and some other operations safe
// 0 title, 1 client creator, 2 events, 4 title, 5 Sweeney, 6 Lovett, 8 title, 9-11 razors, 13 chair, 14-16 lounge, 
// 18 title, 19-22 bakery, 24 title, 25 meat, 26 pies, 27 money, 29 PAYDAY


FleetStreet::FleetStreet()
{
	stop = false;
	amIDead = false;
	amIFull = false;
	uniqueID = 0;
	meat = 0;
	meatPies = 0;
	money = 0;
	bloodiedRazors = 0;
	barberShopStatus.resize(waitingRoomCapacity + 1);
	for(int i = 0; i < barberShopStatus.size(); i++)
	{
		barberShopStatus[i] = -1;
	}
	bakeryStatus.resize(bakeryCapacity);
	for(int i = 0; i < bakeryStatus.size(); i++)
	{
		bakeryStatus[i] = -1;
	}
	priorityList.resize(4);
	for(int i = 0; i < priorityList.size(); i++)
	{
		priorityList[i] = i;
	}
	razorsStatus.resize(razorsCapacity);
	for(int i = 0; i < razorsStatus.size(); i++)
	{
		razorsStatus[i] = 0;
	}
}

void FleetStreet::barberFunction()
{
	while (!stop)
	{
		// barber is checking if waiting room is empty
		bool isEmpty = true;
		for(int i = 0; i < waitingRoomCapacity; i++)
		{
			if(waitingRoom[i].try_lock())
			{
				waitingRoom[i].unlock();
			}
			else 
				isEmpty = false;
		}
		if(isEmpty)
		{
			// if so, then barber is trying to sleep
			if (barberChair.try_lock())
			{
				myMutex.lock();
				barberShopStatus[0] = -2;
				myMutex.unlock();
				int randWait1 = (std::rand() % 1) + 15;
				float progressT1 = 0.0;
				for (int i = 1; i <= randWait1; i++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					progressT1 = (100 * i) / randWait1;
					myMutex.lock();
					move(5, 0);
					clrtoeol();
					printw("Sweeney Todd is sleeping in his chair\t\t\t%.0f\t%%", progressT1);
					refresh();
					myMutex.unlock();
				}
				barberChair.unlock();
			}
			else 
				isEmpty = false;
		}
		if(barberChair.try_lock())
			barberChair.unlock();
		else
		{	
			// pick up razors
			int pickedRazors = 0;
			for(int i = 0; i < razorsCapacity; i++)
			{
				if(razors[i].try_lock())
				{
					myMutex.lock();
					if(razorsStatus[i] == 0)
					{
						pickedRazors++;
						razorsStatus[i] = 1;	// this razor is now taken by barber
						myMutex.unlock();
						for(int j = 0; j < razorsCapacity; j++)
						{
							if(i == j)
								continue;
							if(razors[j].try_lock())
							{
								myMutex.lock();
								if(razorsStatus[j] == 0)
								{
									pickedRazors++;
									razorsStatus[j] = 1;	// this razor is now taken by barber
									myMutex.unlock();
								}
								else
								{
									razors[j].unlock();
									myMutex.unlock();
								}
							}
							if(pickedRazors == 2)
								break;
						}
					}
					else
					{
						razors[i].unlock();
						myMutex.unlock();
					}
				}
				if(pickedRazors == 2)
					break;
			}

			// if picked up only 1 clean razor, free it and try again later
			if(pickedRazors != 2)
			{
				myMutex.lock();
				move(5, 0);
				clrtoeol();
				printw("Sweeney Todd is waiting for clean razors");
				refresh();
				myMutex.unlock();
				for(int i = 0; i < razorsCapacity; i++)
				{
					myMutex.lock();
					if(razorsStatus[i] == 1)
					{
						razors[i].unlock();
						razorsStatus[i] = 0;
					}
					myMutex.unlock();
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				continue;
			}
			int randWait3 = (std::rand() % 1) + 25;
			float progressT3 = 0.0;
			for (int j = 1; j <= randWait3; j++)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				progressT3 = (100 * j) / randWait3;
				myMutex.lock();
				move(5, 0);
				clrtoeol();
				printw("Sweeney Todd is shaving Client[%d]\t\t\t%.0f\t%%", myName, progressT3);
				refresh();
				myMutex.unlock();
			}
			//kill
			myMutex.lock();
			amIDead = true;
			myMutex.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
			myMutex.lock();
			clients[myPos].join();
			clients.erase(clients.begin() + myPos);
			clientsIDs.erase(clientsIDs.begin() + myPos);
			myMutex.unlock();
			myMutex.lock();
			move(2, 0);
			clrtoeol();
			printw("Events: Sweeney Todd has killed Client[%d]", myName);
			refresh();
			myMutex.unlock();

			// unlock now bloodied razors
			for(int i = 0; i < razorsCapacity; i++)
			{
				myMutex.lock();
				if(razorsStatus[i] == 1)
				{
					razors[i].unlock();
					razorsStatus[i] = -1;
				}
				myMutex.unlock();
			}

			int randWait4 = (std::rand() % 1) + 15;
			float progressT4 = 0.0;
			chute.lock();
			for (int j = 1; j <= randWait4; j++)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				progressT4 = (100 * j) / randWait4;
				myMutex.lock();
				meat++;
				move(5, 0);
				clrtoeol();
				printw("Sweeney Todd is sending Client[%d] down the chute\t%.0f\t%%", myName, progressT4);
				refresh();
				myMutex.unlock();
			}
			myMutex.lock();
			barberShopStatus[0] = -1;
			myMutex.unlock();
			chute.unlock();
			barberChair.unlock();
		}
	}
}

void FleetStreet::bakerFunction()
{
	while(!stop)
	{
		if(priorityList[0] == 0)
		{
			// make meatPies
			myMutex.lock();
			if(meat < 15)
			{
				myMutex.unlock();
				int tmp = priorityList[0];
				priorityList.erase(priorityList.begin());
				priorityList.push_back(tmp);
				continue;
			}
			else
				myMutex.unlock();

			if (chute.try_lock())
			{
				int randWait5 = (std::rand() % 1) + 15;
				float progressT5 = 0.0;
				for (int i = 1; i <= randWait5; i++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					progressT5 = (100 * i) / randWait5;
					myMutex.lock();
					meat--;
					move(6, 0);
					clrtoeol();
					printw("Mrs Lovett is baking a meat pie\t\t\t\t%.0f\t%%", progressT5);
					refresh();
					myMutex.unlock();
				}
				chute.unlock();
				myMutex.lock();
				meatPies++;
				myMutex.unlock();

				int tmp1 = priorityList[0];
				priorityList.erase(priorityList.begin());
				priorityList.push_back(tmp1);
			}
			int tmp2 = priorityList[0];
			priorityList.erase(priorityList.begin());
			priorityList.push_back(tmp2);
		}
		else if(priorityList[0] == 1)
		{
			// clean razors

			int tmp3 = priorityList[0];
			priorityList.erase(priorityList.begin());
			priorityList.push_back(tmp3);
		}
		else if(priorityList[0] == 2)
		{
			// serve meatPies
			if(bakery[0].try_lock()) 
			{
				bakery[0].unlock();
				int tmp4 = priorityList[0];
				priorityList.erase(priorityList.begin());
				priorityList.push_back(tmp4);
			}
			else
			{
				if(meatPies > 0)
				{
					int randWait7 = (std::rand() % 1) + 20;
					float progressT7 = 0.0;
					for (int i = 1; i <= randWait7; i++)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(200));
						progressT7 = (100 * i) / randWait7;
						myMutex.lock();
						move(6, 0);
						clrtoeol();
						printw("Mrs Lovett is serving a Client[%d]\t\t\t%.0f\t%%", myName2, progressT7);
						refresh();
						myMutex.unlock();
					}
					// leave
					myMutex.lock();
					amIFull = true;
					myMutex.unlock();
					/*move(3, 0);
					clrtoeol();
					printw("Events: Client[%d]:\t%d", myName2, myPos2);
					refresh();*/
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
					myMutex.lock();
					clients[myPos2].join();
					clients.erase(clients.begin() + myPos2);
					clientsIDs.erase(clientsIDs.begin() + myPos2);
					myMutex.unlock();
					myMutex.lock();
					move(2, 0);
					clrtoeol();
					printw("Events: Client[%d] ate a pie, paid and is going home", myName2);
					refresh();
					meatPies--;
					money += 5;
					myMutex.unlock();
				}
				int tmp5 = priorityList[0];
				priorityList.erase(priorityList.begin());
				priorityList.push_back(tmp5);
			}		
		}
		else
		{
			// sleep
			myMutex.lock();
			myMutex.unlock();
			int randWait6 = (std::rand() % 1) + 15;
			float progressT6 = 0.0;
			for (int i = 1; i <= randWait6; i++)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				progressT6 = (100 * i) / randWait6;
				myMutex.lock();
				move(6, 0);
				clrtoeol();
				printw("Mrs Lovett is taking a break\t\t\t\t%.0f\t%%", progressT6);
				refresh();
				myMutex.unlock();
			}
			int tmp0 = priorityList[0];
			priorityList.erase(priorityList.begin());
			priorityList.push_back(tmp0);
		}
	}
}

void FleetStreet::butFirstSirIThinkAShave(int clientID)
{
	int wrPointer = waitingRoomCapacity - 1;
	while(!stop)
	{
		if(waitingRoom[wrPointer].try_lock())
		{
			myMutex.lock();
			move(2, 0);
			clrtoeol();
			printw("Events: Client[%d] decided to have a shave", clientID);
			refresh();
			barberShopStatus[wrPointer + 1] = clientID;
			myMutex.unlock();
			break;
		}
		else 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}
	}
	while(!stop)
	{
		if(wrPointer == 0)
		{
			if(barberChair.try_lock())
			{
				myName = clientID;
				myMutex.lock();
				barberShopStatus[1] = -1;
				barberShopStatus[0] = clientID;
				myMutex.unlock();
				waitingRoom[0].unlock();
				while(!amIDead)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				amIDead = false;
				break;
			}
			else 
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		else 
		{
			if(waitingRoom[wrPointer-1].try_lock())
			{
				myMutex.lock();
				barberShopStatus[wrPointer + 1] = -1;
				barberShopStatus[wrPointer] = clientID;
				myMutex.unlock();
				waitingRoom[wrPointer].unlock();
				wrPointer--;
			}
			else 
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
	myPos = std::find(clientsIDs.begin(), clientsIDs.end(), clientID) - clientsIDs.begin();
}

void FleetStreet::theWorstPiesInLondon(int clientID)
{
	int bkrPointer = bakeryCapacity - 1;
	while(!stop)
	{
		if(bakery[bkrPointer].try_lock())
		{
			myMutex.lock();
			move(2, 0);
			clrtoeol();
			printw("Events: Client[%d] decided to eat a meat pie", clientID);
			refresh();
			bakeryStatus[bkrPointer] = clientID;
			myMutex.unlock();
			break;
		}
		else 
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue; 
		}
	}
	while(!stop)
	{
		if(bkrPointer == 1)
		{
			if(bakery[0].try_lock())
			{
				myName2 = clientID;
				myMutex.lock();
				bakeryStatus[1] = -1;
				bakeryStatus[0] = clientID;
				myMutex.unlock();
				bakery[1].unlock();
				while(!amIFull)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				amIFull = false;
				bakery[0].unlock();
				break;
			}
			else 
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		else 
		{
			if(bakery[bkrPointer-1].try_lock())
			{
				myMutex.lock();
				bakeryStatus[bkrPointer] = -1;
				bakeryStatus[bkrPointer - 1] = clientID;
				myMutex.unlock();
				bakery[bkrPointer].unlock();
				bkrPointer--;
			}
			else 
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
	myPos2 = std::find(clientsIDs.begin(), clientsIDs.end(), clientID) - clientsIDs.begin();
}

void FleetStreet::arrive(int clientID)
{
	int randDecision = (std::rand() % 99) + 1;
	if(randDecision > 50)
	{
		if(waitingRoom[waitingRoomCapacity - 1].try_lock())
		{
			waitingRoom[waitingRoomCapacity - 1].unlock();
			butFirstSirIThinkAShave(clientID);
		}
		else
			theWorstPiesInLondon(clientID);
	}
	else
	{
		if(bakery[bakeryCapacity - 1].try_lock())
		{
			bakery[bakeryCapacity - 1].unlock();
			theWorstPiesInLondon(clientID);
		}
		else
			butFirstSirIThinkAShave(clientID);
	}
}

void FleetStreet::createClients()
{
	while(!stop)
	{
		if(clients.size() < maxNoOfClients)
		{
			int randWait0 = (std::rand() % 1) + 15;
			float progressT0 = 0.0;
			for (int i = 1; i <= randWait0; i++)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				progressT0 = (100 * i) / randWait0;
				myMutex.lock();
				move(1, 0);
				clrtoeol();
				printw("Next Client is coming to Fleet Street\t\t\t%.0f\t%%", progressT0);
				refresh();
				myMutex.unlock();
			}
			myMutex.lock();
			clients.push_back(std::thread(&FleetStreet::arrive, this, uniqueID));
			clientsIDs.push_back(uniqueID);		  
			move(1, 0);
			clrtoeol();
			printw("Next Client is coming to Fleet Street\t\t\t%.0f\t%%", 0);
			refresh();
			myMutex.unlock();
			uniqueID++;
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
}

void FleetStreet::changeGUI()
{
	while(!stop)
	{
		myMutex.lock();
		for (int i = 0; i < razorsStatus.size(); i++)
		{
			move(i+9, 0);
			clrtoeol();
			if(razorsStatus[i] == -1)
				printw("Razor[%d]:\t\t\tbloodied", i);
			else if(razorsStatus[i] == -2)
				printw("Razor[%d]:\t\t\tcleaned by Mrs Lovett", i);
			else if(razorsStatus[i] == 1)
				printw("Razor[%d]:\t\t\tused by Sweeney Todd", i);
			else
				printw("Razor[%d]:\t\t\tclean", i);
		}
		move(13, 0);
		clrtoeol();
		if(barberShopStatus[0] == -1)
			printw("Barber's chair:\t\t\tempty");
		else if(barberShopStatus[0] == -2)
			printw("Barber's chair:\t\t\tSweeney Todd");
		else
			printw("Barber's chair:\t\t\tClient[%d]", barberShopStatus[0]);

		for (int i = 1; i < barberShopStatus.size(); i++)
		{
			move(i+13, 0);
			clrtoeol();
			if(barberShopStatus[i] == -1)
				printw("Lounge[%d]:\t\t\tempty", i - 1);
			else
				printw("Lounge[%d]:\t\t\tClient[%d]", i - 1, barberShopStatus[i]);
		}
		for (int i = 0; i < bakeryStatus.size(); i++)
		{
			move(i+19, 0);
			clrtoeol();
			if(bakeryStatus[i] == -1)
				printw("Bakery[%d]:\t\t\tempty", i);
			else
				printw("Bakery[%d]:\t\t\tClient[%d]", i, bakeryStatus[i]);
		}
		move(25, 0);
		clrtoeol();
		printw("Edible meat in the chute:\t%d\tdag", meat);
		move(26, 0);
		clrtoeol();
		printw("Meat pies ready for sale:\t%d\tportions", meatPies);
		move(27, 0);
		clrtoeol();
		printw("Amount of money:\t\t%d\tpounds", money);
		
		refresh();
		myMutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
}

void FleetStreet::startSimulation()
{
	initscr();	// try putting in constructor
	cbreak();
	//raw();
	//menuinit();

	// creating threads
	barber = std::thread(&FleetStreet::barberFunction, this);
	baker = std::thread(&FleetStreet::bakerFunction, this);
	clientCreator = std::thread(&FleetStreet::createClients, this);
	GUI = std::thread(&FleetStreet::changeGUI, this);

	std::cin.get(); 	// pauses main thread here, other threads still going
	stop = true;		// this breaks main loop
	amIDead = true;
	amIFull = true;

	// joining threads
	barber.join();
	baker.join();
	clientCreator.join();
	GUI.join();
	for (int i = 0; i < clients.size(); ++i)
	{
		clients[i].join();
	}
	endwin();	// try putting in destructor
}

FleetStreet::~FleetStreet() {  }
