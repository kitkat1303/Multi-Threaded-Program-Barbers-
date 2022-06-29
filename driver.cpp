#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop.h"
using namespace std;

void *barber(void *);
void *customer(void *);

// ThreadParam class
// This class is used as a way to pass more
// than one argument to a thread.
class ThreadParam
{
public:
    ThreadParam(Shop *shop, int id, int service_time) : shop(shop), id(id), service_time(service_time){};
    Shop *shop;
    int id;
    int service_time;
};

int main(int argc, char *argv[])
{

    // Read arguments from command line
    if (argc != 5)
    {
        cout << "Usage: num_chairs num_customers service_time" << endl;
        return -1;
    }
    int num_barbers = atoi(argv[1]);   // number of barbers in chair
    int num_chairs = atoi(argv[2]);    // number of waiting chairs
    int num_customers = atoi(argv[3]); // number of customers
    int service_time = atoi(argv[4]);  // service time for barbers

    // 1-many barbers, one shop, many customers
    pthread_t barber_thread[num_barbers];
    pthread_t customer_threads[num_customers];
    Shop shop(num_barbers, num_chairs);

    // create barber threads
    for (int i = 0; i < num_barbers; i++)
    {
        // usleep(1000);
        int id = i + 1;
        ThreadParam *barber_param = new ThreadParam(&shop, id, service_time);
        pthread_create(&barber_thread[i], NULL, barber, barber_param);
    }

    // create customer threads
    for (int i = 0; i < num_customers; i++)
    {
        usleep(rand() % 1000);
        int id = i + 1;
        ThreadParam *customer_param = new ThreadParam(&shop, id, 0); // create unique thread id to pass into create thread function
        pthread_create(&customer_threads[i], NULL, customer, customer_param);
    }

    // Wait for customers to finish and cancel barber
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

    // Cancel barber threads
    for (int i = 0; i < num_barbers; i++)
    {
        pthread_cancel(barber_thread[i]);
    }

    cout << "# customers who didn't receive a service = " << shop.getNumDroppedCustomers() << endl;
    return 0;
}

void *barber(void *arg)
{
    ThreadParam *barber_param = (ThreadParam *)arg;
    Shop &shop = *barber_param->shop;
    int barberID = barber_param->id;
    int service_time = barber_param->service_time;
    delete barber_param;

    // keep working until terminated by main
    while (true)
    {
        // get customer and sleep for amount of service time
        shop.helloCustomer(barberID); // get customer and add to ready queue
        usleep(service_time);
        shop.byeCustomer(barberID); // release customer
    }
    return nullptr;
}

void *customer(void *arg)
{
    ThreadParam *customer_param = (ThreadParam *)arg;
    Shop &shop = *customer_param->shop;
    int customerID = customer_param->id;
    delete customer_param;

    // if assigned to barber i then wait for service to finish
    // -1 means did not get barber
    int barber = -1;

    if ((barber = shop.visitShop(customerID)) != -1)
    {
        shop.leaveShop(customerID, barber); // wait until my service is finished
    }

    return nullptr;
}
