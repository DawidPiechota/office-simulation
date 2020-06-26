# office-simulation

Compile using GNU C++ Compiler:
```bash
g++ -std=c++11 -pthread -o biuro biuro.cpp -lncurses
```

2-story office simulation done for "Systemy Operacyjne 2" course.  
The office is not visualised, instead everything is logged on the terminal in real time.  
People are implemented as threads, various places with things to do are considered resources.  

Tasks realised:
* Proper opening and closing of threads
* Thread and resources synchronization
* Starvation, Deadlock and Livelock prevention

Tasks were acomplished through the implementation of mutual exclusion and queues

---

__Threads__
* 10 office employees
* 20 customers  

__Resources__
* 4 customer service spots (2 for each floor)
* 4 computer stations (2 for each floor)
* 2 vending machines (2 for each floor)
* 2 sets of stairs (one for customers one for employees)

---

__Customer lifecycle__
1. Enters the building
2. Thinks for a while
3. 30% stands in line to stairs in orded to change floors
4. 70% stands in line to vending machine
5. Stands in line to customer service spot
6. Upon getting to it, does his buisness
7. Leaves building, remembers about somethning, goes back inside repeating the cycle

__Employee lifecycle__
1. Rests
2. 30% stands in line to stairs in orded to change floors
3. 50% works on computer, 50% works at customer service spot
4. Repeat

---

![run](/screenshot.png)
