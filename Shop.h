#ifndef SHOP_H_
#define SHOP_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3 // default number of chairs = 3
#define kDefaultBarbers 1   // the default number of barbers = 1

/**
 * @brief
 * The shop class represents a Barber Shop with a certain amount of
 * barbers, waiting chairs, and customers.
 */
class Shop
{
public:
   Shop(int num_barbers, int num_chairs) : maxNumWaitingChairs((num_chairs > 0) ? num_chairs : kDefaultNumChairs), numDroppedCustomers(0), numberOfBarbers(num_barbers)
   {
      // initialize
      //  0 index will not be used, 1-numBarbers will be used
      for (int i = 0; i <= numberOfBarbers; i++)
      {
         customerInChair.push_back(0);
         barberChairFull.push_back(false);
         moneyPaid.push_back(false);
         barberReadytoService.push_back(false);
      }

      init();
   };
   Shop() : maxNumWaitingChairs(kDefaultNumChairs),
            numDroppedCustomers(0), numberOfBarbers(kDefaultBarbers)
   {
      // initialize the following
      //  0 index will not be used, 1-numBarbers will be used
      for (int i = 0; i <= numberOfBarbers; i++)
      {
         customerInChair.push_back(0);
         barberChairFull.push_back(false);
         moneyPaid.push_back(false);
         barberReadytoService.push_back(false);
      }

      init();
   };

   /**
    * @brief Destroy the Shop:: Shop object
    *
    */
   ~Shop();

   /**
    * @brief vistitShop
    * takes in a customerID and either leaves the shop if there
    * are no waiting chairs, takes a waiting chair if no barbers are
    * available, or gets seated in barber chair for service
    *
    * @param customerID : customer entering the shop
    * @return int : barberID returned who is serving the customer
    */
   int visitShop(int customerID); 

   /**
    * @brief leaveShop
    * takes in a customerID and barberID and waits for the barber to be
    * done servicing customer
    *
    * @param customerID
    * @param barberID
    */
   void leaveShop(int customerID, int barberID); 

   /**
    * @brief helloCustomer
    * takes in barberID and adds to ready queue if they have not been
    * added already and then if there are no customers, barber sleeps,
    * or else starts a haircut
    *
    * @param barberID
    */
   void helloCustomer(int barberID); 

   /**
    * @brief byeCustomer
    * takes in a barberID and gets customer to pay and then barber
    * is added back to the ready queue for servicing another customer
    *
    * @param barberID
    */
   void byeCustomer(int barberID); 

   /**
    * @brief returnes the number of dropped customers
    *
    * @return int: number of dropped customers
    */
   int getNumDroppedCustomers() const; 

private:
   const int maxNumWaitingChairs; // the max number of threads that can wait

   const int numberOfBarbers; // number of barbers

   vector<int> customerInChair; // customer ID of customer being serviced by barber[i]

   vector<bool> barberChairFull; // true if barber chair is in service

   vector<bool> moneyPaid; // true if money paid

   queue<int> waitingCustomerIDs; // includes the ids of all waiting threads

   queue<int> barbersReady; // includes id of barbers

   vector<bool> barberReadytoService;

   int numDroppedCustomers; // keeps track of customers turned away

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;

   // wait = cust waiting, signal = do something (single condition)
   pthread_cond_t condCustomerWaiting;

   // arrays for each barber
   pthread_cond_t *condCustomerServed; // wait = not served, signal = served
   pthread_cond_t *condBarberPaid;     // wait = not paid, signal = paid
   pthread_cond_t *condBarberSleeping; // wait = sleep, signal = wake up

   /**
    * init
    * this function initializes conditions
    */
   void init();

   /**
    * @brief int2string
    * this function converts integer to string
    *
    * @param i
    * @return string
    */
   string int2string(int i);

   /**
    * @brief print prints either a customer if its a positive ID or
    * barber if is is a negative ID
    *
    * @param person: barber or customer to be printed
    * @param message: the message to be printed
    */
   void print(int person, string message);
};
#endif
