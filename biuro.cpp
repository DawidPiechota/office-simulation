#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <curses.h>
#include <mutex>
#include <queue>
#include <vector>

using namespace std;

const int w8Time = 200000, w8TimeRandom = 40000, w8TimeIterations = 40;              // czas czekania na akcje
const int numberOfWorkers = 10, numberOfCustomers = 20;                              // ilosc pracownikow i klientow
int endOfProgram = false;                                                            // zakonczenie programu
int peopleOnStrairs = 0;                                                             // ilosc ludzi na schodach
bool isComputerOccupied[4] = {false}, isVendingOccupied[4] = {false};                // stan zasobow
bool isWindowOccupiedByCustomer[4] = {false}, isWindowOccupiedByWorker[4] = {false}; // stan zasobow
bool isWorkerActive[4] = {false};                                                    // pomaganie klientowi
bool canCustomerGo[4] = {false};                                                     // klient zostal obsluzony i moze odejsc

vector<queue<int>> queues;                                                           // vector kolejek
bool pietro[numberOfWorkers + numberOfCustomers] = {0};                              // pietro danej osoby
string percent = "";                                                                 // zmienna uzywana do wyswietlania postepu

std::mutex screenLock;
std::mutex floorLock;
std::mutex stairsLock;
std::mutex vendingLock;
std::mutex windowLock;
std::mutex computerLock;

bool computer(int id, int computerId, string choice)
{
    std::lock_guard<std::mutex> lock(computerLock);
    if(choice == "in")
    {
        if(isComputerOccupied[computerId] == false)
        {
            isComputerOccupied[computerId] = true;
            return 1;
        }else return 0;
    }
    if(choice == "out")
    {
        isComputerOccupied[computerId] = false;
        return 1;
    }
    for(;;)cout << "blad w computer";
    exit(-1);
}

int floorChangeOrCheck(int id, string choice)
{
    std::lock_guard<std::mutex> lock(floorLock);
    if(choice == "check")
    {
        return pietro[id];
    }
    else if(choice == "change");
    {
        (pietro[id] == 1) ? pietro[id] = 0 : pietro[id] = 1;
        return pietro[id];
    }
    for(;;)cout << "blad w floorChangeOrCheck";
    exit(-1);
}

bool stairs(int id, string choice) // 2-osobowe schody z kolejka
{
    std::lock_guard<std::mutex> lock(stairsLock);
    if(choice == "stand in queue")
    {
        queues[8].push(id);
        return 1;
    }
    if(choice == "in") //chce wejsc na schody
    {
        if((queues[8].front() == id) && (peopleOnStrairs < 2))
        {
            peopleOnStrairs++;
            queues[8].pop();
            return 1;
        }
        else return 0;
    }
    if(choice == "out") //schodze ze schodow
    {
        peopleOnStrairs--;
        return 1;
    }
    for(;;)cout << "blad w stairs";
    exit(-1);
}

bool vendingMachine(int id, int vendingId, string choice) // automat vendingowy
{
    std::lock_guard<std::mutex> lock(vendingLock);
    if(choice == "stand in queue")
    {
        queues[vendingId+4].push(id);
        return 1;
    }
    if(choice == "in") //chce uzyc automatu
    {
        if((queues[vendingId+4].front() == id) && (isVendingOccupied[vendingId] == false))
        {
            isVendingOccupied[vendingId] = true;
            queues[vendingId+4].pop();
            return 1;
        }
        else return 0;
    }
    if(choice == "out") //odejscie od automatu
    {
        isVendingOccupied[vendingId] = false;
        return 1;
    }
    for(;;)cout << "blad w vendingMachine";
    exit(-1);
}

bool windowPoint(int id, int windowId, string position, string choice)
{
    std::lock_guard<std::mutex> lock(windowLock);
    
    if(choice == "stand in queue")
    {
        queues[windowId].push(id);
        return 1;
    }
    if(choice == "in" && position == "customer") //podejscie do okienka przez klienta
    {
        if((queues[windowId].front() == id) && (isWindowOccupiedByCustomer[windowId] == false))
        {
            isWindowOccupiedByCustomer[windowId] = true;
            queues[windowId].pop();
            return 1;
        }
        else return 0;
    }
    if(choice == "in" && position == "worker") //podejscie do okienka przez pracownika
    {
        if(isWindowOccupiedByWorker[windowId] == false && canCustomerGo[windowId] == false)
        {
            isWindowOccupiedByWorker[windowId] = true;
            return 1;
        }
        else return 0;
    }
    if(choice == "can i work now?" && position == "worker")
    {
        if(isWindowOccupiedByCustomer[windowId] == true && canCustomerGo[windowId] == false)
        {
            isWorkerActive[windowId] = true;
            return 1;
        }
        else
        {
            isWorkerActive[windowId] = false;
            return 0;
        }
    }
    if(choice == "is someone helping me?" && position == "customer")
    {
        if(isWindowOccupiedByWorker[windowId] == true && isWorkerActive[windowId] == true && canCustomerGo[windowId] == false)
        {
            return 1;
        }
        else return 0;
    }
    if(choice == "can i go now?" && position == "customer") //odejscie do okienka przez klienta
    {
        if(canCustomerGo[windowId] == true)
        {
            canCustomerGo[windowId] = false;
            return 1;
        }
        else return 0;
    }
    if(choice == "you can go now" && position == "worker")
    {
        canCustomerGo[windowId] = true;
        return 1;
    }
    if(choice == "out" && position == "customer") //odejscie od okienka przez klienta
    {
        isWindowOccupiedByCustomer[windowId] = false;
        return 1;
    }
    if(choice == "out" && position == "worker") //odejscie od okienka przez pracownika
    {
        isWindowOccupiedByWorker[windowId] = false;
        isWorkerActive[windowId] = false;
        return 1;
    }
    for(;;)cout << "blad w windowPoint";
    exit(-1);


}

void writeOnScreen(int id, std::string str)
{
    std::lock_guard<std::mutex> lock(screenLock);
    mvaddstr(id, 0, str.c_str());
    refresh();

}

void executeThreadWorker(int id)
{
    int floor = 0;
    while(true)
    {
////////////////////////////////////////////////  Odpoczynek
        for(int i = 0; i < w8TimeIterations; i++) // 2.5-3s
        {
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Odpoczywa                                 " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%                        ");
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
            if(endOfProgram) break;
        }
        if(endOfProgram) break;
////////////////////////////////////////////////  Zmiana pietra
        if((rand() % 2) == 0) //zmiana pietra 50%
        {
            stairs(id, "stand in queue");
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czeka przy schodach                               ");
            do{ 
                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;
            }while(!stairs(id, "in")); 
            if(endOfProgram) break;

            
            
            for(int i = 0; i < 10; i++)
            {
                (floorChangeOrCheck(id, "check") == 0)
                ? writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Wchodzi po schodach                       " + to_string(static_cast<int>(i/(10 * 1/100.0))) + "\%                               ")
                : writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Schodzi po schodach                       " + to_string(static_cast<int>(i/(10 * 1/100.0))) + "\%                        ");

                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;
            }
            if(endOfProgram) break; 
            stairs(id, "out");
            floor = floorChangeOrCheck(id, "change"); //zmiana pietra i zapisanie go
        }
        if(endOfProgram) break;
////////////////////////////////////////////////  Praca
        if(rand() % 2 == 0) // Obsluga klientow
        {
            int windowId = (2*floor) + (rand() % 2);
            if(windowPoint(id, windowId, "worker", "in"))
            {
                for(int i = 0; i < 4; i++)
                {
                    if(windowPoint(id, windowId, "worker", "can i work now?"))
                    {
                        for(int j = 0; j < w8TimeIterations; j++)
                        {
                            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Pracuje przy okienku nr: " + to_string(windowId) + "                " + to_string(static_cast<int>(j/(w8TimeIterations * 1/100.0))) + "\%               ");
                            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                            if(endOfProgram) break;
                        }
                        if(endOfProgram) break; 
                        if(i == 3) windowPoint(id, windowId, "worker", "out"); //odchodzi po 3 iteracjach
                        windowPoint(id, windowId, "worker", "you can go now");

                    }
                    else
                    {
                        for(int j = 0; j < w8TimeIterations; j++)
                        {
                            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czekam przy okienku nr: " + to_string(windowId) + "                 " + to_string(static_cast<int>(j/(w8TimeIterations * 1/100.0))) + "\%               ");
                            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                            if(endOfProgram) break;
                        }
                        if(endOfProgram) break; 
                        if(i == 3) windowPoint(id, windowId, "worker", "out"); //odchodzi po 3 iteracjach
                    }
                }

            }
        }
        else                // Praca na komputerze
        {
            int computerId = (floor*2)+(rand()%2);
            if(computer(id, computerId, "in"))
            {
                for(int i = 0; i < w8TimeIterations; i++)
                {
                    writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Pracuje przy komputerze nr: " + to_string(computerId) + "             " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%               ");
                    usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                    if(endOfProgram) break;
                }
                computer(id, computerId, "out");
            }
        }
        if(endOfProgram) break; 
        
    }
}

void executeThreadCustomer(int id)
{
    int floor = 0;
////////////////////////////////////////////////  Wejście
    while(true)
    {
        for(int i = 0; i < w8TimeIterations; i++) // 2.5-3s
        {
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Wszedl do budynku                        " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%                        ");
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
            if(endOfProgram) break;
        }
        if(endOfProgram) break;
////////////////////////////////////////////////  Wejscie na pietro
        if((rand() % 2) == 0) //zmiana pietra 50%
        {
            stairs(id, "stand in queue");
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czeka przy schodach                               ");
            do{ 
                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;
            }while(!stairs(id, "in")); 
            if(endOfProgram) break;

            
            for(int i = 0; i < 10; i++)
            {
                (floorChangeOrCheck(id, "check") == 0)
                ? writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Wchodzi po schodach                      " + to_string(static_cast<int>(i/(10 * 1/100.0))) + "\%                               ")
                : writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Schodzi po schodach                      " + to_string(static_cast<int>(i/(10 * 1/100.0))) + "\%                        ");

                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;                
            }
            if(endOfProgram) break; 
            stairs(id, "out");
            floor = floorChangeOrCheck(id, "change"); //zmiana pietra i zapisanie go
        }
        if(endOfProgram) break;
////////////////////////////////////////////////  Przerwa
        for(int i = 0; i < w8TimeIterations; i++) // 2.5-3s
        {
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Mysli w budynku                          " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%                                      ");
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
            if(endOfProgram) break;
        }
        if(endOfProgram) break;
//////////////////////////////////////////////// Korzystanie z automatu vendingowego
        if((rand() % 3) > 0) //korzystanie z automatu 66%
        {
            int vendingId = (2*floor) + (rand() % 2); // Parter- 0,1. Pietro- 2,3.
            vendingMachine(id, vendingId, "stand in queue");
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czeka w kolejce do automatu nr " + to_string(vendingId) + "             ");
            do{ 
                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;
            }while(!vendingMachine(id, vendingId, "in"));
            if(endOfProgram) break;


            for(int i = 0; i < w8TimeIterations * 2; i++)
            {
                writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Uzywa automatu nr " + to_string(vendingId) + "                      " + to_string(static_cast<int>(i/(w8TimeIterations * 2/100.0))) + "\%             ");
                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;
            }
            if(endOfProgram) break;
            vendingMachine(id, vendingId, "out");
        }
        if(endOfProgram) break;
//////////////////////////////////////////////// Przerwa
        for(int i = 0; i < w8TimeIterations; i++) // 2.5-3s
        {
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Mysli w budynku                          " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%                                      ");
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
            if(endOfProgram) break;
        }
        if(endOfProgram) break;
//////////////////////////////////////////////// Obsluga w okienku
        int windowId = (2*floor) + (rand() % 2); // Parter- 0,1. Pietro- 2,3.
        writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czeka w kolejce do okienka nr " + to_string(windowId) + "              ");
        windowPoint(id, windowId, "customer", "stand in queue");
        do{
           usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s 
           if(endOfProgram) break;
        }while(!windowPoint(id, windowId, "customer", "in"));
        if(endOfProgram) break;

        writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czeka na pracownika przy okienku nr " + to_string(windowId) + "              ");
        do{
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 );
            if(endOfProgram) break;
        }while(!windowPoint(id, windowId, "customer", "is someone helping me?"));
        if(endOfProgram) break;

        writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Jest obslugiwany przy okienku nr " + to_string(windowId) + "             ");
        do{
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 );
            if(endOfProgram) break;
        }while(!windowPoint(id, windowId, "customer", "can i go now?"));
        if(endOfProgram) break;

        windowPoint(id, windowId, "customer", "out");
////////////////////////////////////////////////////////////////////////// Jak na pietrze to musi zejsc
        if(floor == 1)
        {
            stairs(id, "stand in queue");
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Czeka przy schodach                               ");
            do{ 
                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;
            }while(!stairs(id, "in")); 
            if(endOfProgram) break;

            for(int i = 0; i < 10; i++)
            {
                (floorChangeOrCheck(id, "check") == 0)
                ? writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Wchodzi po schodach                      " + to_string(static_cast<int>(i/(10 * 1/100.0))) + "\%                               ")
                : writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Schodzi po schodach                      " + to_string(static_cast<int>(i/(10 * 1/100.0))) + "\%                          ");

                usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
                if(endOfProgram) break;                
            }
            if(endOfProgram) break; 
            stairs(id, "out");
            floor = floorChangeOrCheck(id, "change"); //zmiana pietra i zapisanie go
        }
        if(endOfProgram) break;
////////////////////////////////////////////////////////////// Wyjscie z budynku
        for(int i = 0; i < w8TimeIterations; i++) // 2.5-3s
        {
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Wychodzi z budynku                       " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%                                      ");
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
            if(endOfProgram) break;
        }
        if(endOfProgram) break;
///////////////////////////////////////////////////////////// Czekanie poza budynkiem
        for(int i = 0; i < w8TimeIterations; i++) // 2.5-3s
        {
            writeOnScreen(id, std::to_string(id) + " | " + std::to_string(floor) + "| Zmienia sie w inna osobe poza budynkiem  " + to_string(static_cast<int>(i/(w8TimeIterations * 1/100.0))) + "\%                           ");
            usleep(w8Time + w8TimeRandom * (rand() % 100)/100.0 ); //0.5- 0.6s
            if(endOfProgram) break;
        }
        if(endOfProgram) break;
    }
    
}


void checkForExit()
{
    while (1)
    {
        //cbreak();
        getch();
        endOfProgram = true;
        //writeOnScreen(numberOfCustomers + numberOfWorkers + 1, "checkForExit thread ended");
        return;
    }
    
}

void printResources()
{
    string str;
    queue <int> q;
    do{
        str = "Kolejka do schodów: ";
        q = queues[8];
        while(!q.empty())
        {
            str+= to_string(q.front()) + " ";
            q.pop();
        }
        str += "               ";
        writeOnScreen(numberOfCustomers + numberOfWorkers + 1, str);

        for(int i = 0; i < 4; i ++)
        {
            str = "Kolejka do okienka " + to_string(i) + ": ";
            q = queues[i];
            while(!q.empty())
            {
                str+= to_string(q.front()) + " ";
                q.pop();
            }
            str += "               ";
            writeOnScreen(numberOfCustomers + numberOfWorkers + 3 + i, str);
        }

        for(int i = 4; i < 8; i ++)
        {
            str = "Kolejka do automatu " + to_string(i-4) + ": ";
            q = queues[i];
            while(!q.empty())
            {
                str+= to_string(q.front()) + " ";
                q.pop();
            }
            str += "               ";
            writeOnScreen(numberOfCustomers + numberOfWorkers + 4 + i, str);

        }
        writeOnScreen(numberOfCustomers + numberOfWorkers + 13, "People on stairs: " + to_string(peopleOnStrairs));
        writeOnScreen(numberOfCustomers + numberOfWorkers + 14, "isVendingOccupied:  " + to_string(isVendingOccupied[0]) + " " + to_string(isVendingOccupied[1]) + " " + to_string(isVendingOccupied[2]) + " " + to_string(isVendingOccupied[3]));
        writeOnScreen(numberOfCustomers + numberOfWorkers + 15, "isComputerOccupied: " + to_string(isComputerOccupied[0]) + " " + to_string(isComputerOccupied[1]) + " " + to_string(isComputerOccupied[2]) + " " + to_string(isComputerOccupied[3]));
        writeOnScreen(numberOfCustomers + numberOfWorkers + 16, "isWindowOccupiedByCustomer " + to_string(isWindowOccupiedByCustomer[0]) + " " + to_string(isWindowOccupiedByCustomer[1]) + " " + to_string(isWindowOccupiedByCustomer[2]) + " " + to_string(isWindowOccupiedByCustomer[3]));
        writeOnScreen(numberOfCustomers + numberOfWorkers + 17, "isWindowOccupiedByWorker   " + to_string(isWindowOccupiedByWorker[0]) + " " + to_string(isWindowOccupiedByWorker[1]) + " " + to_string(isWindowOccupiedByWorker[2]) + " " + to_string(isWindowOccupiedByWorker[3]));
        writeOnScreen(numberOfCustomers + numberOfWorkers + 19, "canCustomerGo              " + to_string(canCustomerGo[0]) + " " + to_string(canCustomerGo[1]) + " " + to_string(canCustomerGo[2]) + " " + to_string(canCustomerGo[3]));
        writeOnScreen(numberOfCustomers + numberOfWorkers + 20, "isWorkerActive             " + to_string(isWorkerActive[0]) + " " + to_string(isWorkerActive[1]) + " " + to_string(isWorkerActive[2]) + " " + to_string(isWorkerActive[3]));
        usleep(200000);
    }while(!endOfProgram);
}

int main(int argc, char* argv[]) 
{
    for(int i = 0; i < 9; i++)
    {
        queues.push_back(queue<int>()); // 8 kolejek. 0-3 okienka, 4-7 automaty vendingowe, 8-schody.
    }
    // queues[0].push(1); // push 1 into queue number 0.

    WINDOW * mainwin;
    
    if ( (mainwin = initscr()) == NULL )
    {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(0);
    }

    srand (time(NULL));
    std::thread threads1[numberOfWorkers];
    std::thread threads2[numberOfCustomers];
    std::thread checkForExitThread = std::thread(checkForExit);
    std::thread printResourcesThread = std::thread(printResources);
    printResourcesThread.detach();
    checkForExitThread.detach();

    //Create
    for(int i = 0; i < numberOfWorkers; i++)
    {
        threads1[i] = std::thread(executeThreadWorker, i);
    }

    for(int i = 0; i < numberOfCustomers; i++)
    {
        threads2[i] = std::thread(executeThreadCustomer, i+numberOfWorkers);
    }
    // Join
    for(int i = 0; i < numberOfWorkers; i++)
    {
        threads1[i].join();
    }
    for(int i = 0; i < numberOfCustomers; i++)
    {
        threads2[i].join();
    }
delwin(mainwin);
endwin();
refresh();
std::cout<< "All threads ended\n";
return 0;
}