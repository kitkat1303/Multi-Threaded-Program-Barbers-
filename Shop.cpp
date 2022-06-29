
#include "Shop.h"
/**
 * @brief
 * The shop class represents a Barber Shop with a certain amount of
 * barbers, waiting chairs, and customers.
 */

/**
 * @brief Destroy the Shop:: Shop object
 *
 */
Shop::~Shop()
{

   delete condCustomerServed;
   delete condBarberPaid;
   delete condBarberSleeping;

   condCustomerServed = nullptr;
   condBarberPaid = nullptr;
   condBarberSleeping = nullptr;
}

/**
 * init
 * this function initializes conditions
 */
void Shop::init()
{
   // mutex
   pthread_mutex_init(&mutex_, NULL);
   // customer waiting
   pthread_cond_init(&condCustomerWaiting, NULL);

   condCustomerServed = new pthread_cond_t[numberOfBarbers + 1]; // wait = not served, signal = served
   condBarberPaid = new pthread_cond_t[numberOfBarbers + 1];     // wait = not paid, signal = paid
   condBarberSleeping = new pthread_cond_t[numberOfBarbers + 1]; // wait = sleep, signal = wake up

   for (int i = 0; i <= numberOfBarbers; i++)
   {
      // customer served
      pthread_cond_init(&condCustomerServed[i], NULL);
      // barber paid
      pthread_cond_init(&condBarberPaid[i], NULL);
      // barber sleeping
      pthread_cond_init(&condBarberSleeping[i], NULL);
   }
}

/**
 * @brief int2string
 * this function converts integer to string
 *
 * @param i
 * @return string
 */
string Shop::int2string(int i)
{
   stringstream out;
   out << i;
   return out.str();
}

/**
 * @brief print prints either a customer if its a positive ID or
 * barber if is is a negative ID
 *
 * @param person: barber or customer to be printed
 * @param message: the message to be printed
 */
void Shop::print(int person, string message)
{
   cout << ((person > 0) ? "customer[" + to_string(person) : "barber  [" + to_string((person * -1))) << "]: " << message << endl;
}

/**
 * @brief returnes the number of dropped customers
 *
 * @return int: number of dropped customers
 */
int Shop::getNumDroppedCustomers() const
{
   return numDroppedCustomers;
}

/**
 * @brief vistitShop
 * takes in a customerID and either leaves the shop if there
 * are no waiting chairs, takes a waiting chair if no barbers are
 * available, or gets seated in barber chair for service
 *
 * @param customerID : customer entering the shop
 * @return int : barberID returned who is serving the customer
 */
int Shop::visitShop(int customerID)
{
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waitingCustomerIDs.size() == maxNumWaitingChairs)
   {
      print(customerID, "leaves the shop because of no available waiting chairs.");
      ++numDroppedCustomers;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }

   // if all barbers are busy and there are waiting customers
   if (barbersReady.empty() || !waitingCustomerIDs.empty())
   {
      // add id to waiting chair
      waitingCustomerIDs.push(customerID);
      print(customerID, "takes a waiting chair. # waiting seats available = " +
                            int2string(maxNumWaitingChairs - waitingCustomerIDs.size()));
      pthread_cond_wait(&condCustomerWaiting, &mutex_); // customer is waiting
      // signal is the byeCustomerMethod
      waitingCustomerIDs.pop();
   }

   print(customerID, "moves to the service chair. # waiting seats available = " + int2string(maxNumWaitingChairs - waitingCustomerIDs.size()));

   // get first barber ready
   int barberID = barbersReady.front();
   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&condBarberSleeping[barberID]);

   customerInChair[barberID] = customerID;
   barberChairFull[barberID] = true;

   // barber is now busy
   barbersReady.pop();
   pthread_mutex_unlock(&mutex_);

   return barberID;
}

/**
 * @brief leaveShop
 * takes in a customerID and barberID and waits for the barber to be
 * done servicing customer
 *
 * @param customerID
 * @param barberID
 */
void Shop::leaveShop(int customerID, int barberID)
{
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(customerID, "waits for barber[" +
                         int2string(barberID) + "]" + " to be done with hair-cut");
   while (barberChairFull[barberID] == true) // in service has to be for specific barber
   {
      pthread_cond_wait(&condCustomerServed[barberID], &mutex_);
   }

   // Pay the barber and signal barber appropriately
   moneyPaid[barberID] = true;
   pthread_cond_signal(&condBarberPaid[barberID]);
   print(customerID, "says good-bye to the barber[" + int2string(barberID) + "]");
   pthread_mutex_unlock(&mutex_);
}

/**
 * @brief helloCustomer
 * takes in barberID and adds to ready queue if they have not been
 * added already and then if there are no customers, barber sleeps,
 * or else starts a haircut
 *
 * @param barberID
 */
void Shop::helloCustomer(int barberID)
{
   pthread_mutex_lock(&mutex_);

   // check if barberId is in barbersReady queue
   // new
   if (barberReadytoService[barberID] == false)
   {
      // add barber to ready queue
      barbersReady.push(barberID);
   }

   // If no customers than barber can sleep
   if (waitingCustomerIDs.empty() && customerInChair[barberID] == 0)
   {
      print(0 - barberID, "sleeps because of no customers.");
      pthread_cond_wait(&condBarberSleeping[barberID], &mutex_);
   }

   if (customerInChair[barberID] == 0) // check if the customer, sit down.
   {
      pthread_cond_wait(&condBarberSleeping[barberID], &mutex_);
   }

   print(0 - barberID, "starts a hair-cut service for " + int2string(customerInChair[barberID]));
   pthread_mutex_unlock(&mutex_);
}

/**
 * @brief byeCustomer
 * takes in a barberID and gets customer to pay and then barber
 * is added back to the ready queue for servicing another customer
 *
 * @param barberID
 */
void Shop::byeCustomer(int barberID)
{
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   barberChairFull[barberID] = false;
   print(0 - barberID, "says he's done with a hair-cut service for " + int2string(customerInChair[barberID]));
   moneyPaid[barberID] = false;
   pthread_cond_signal(&condCustomerServed[barberID]);
   while (moneyPaid[barberID] == false)
   {
      pthread_cond_wait(&condBarberPaid[barberID], &mutex_);
   }

   // Signal to customer to get next one
   customerInChair[barberID] = 0;
   print(0 - barberID, "calls in another customer");
   barberReadytoService[barberID] = true;
   barbersReady.push(barberID);
   pthread_cond_signal(&condCustomerWaiting);
   pthread_mutex_unlock(&mutex_); // unlock
}
