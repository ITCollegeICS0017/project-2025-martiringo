#include <stdio.h>
#include <iostream>
#include <vector>
using namespace std;

// Type of service
enum ServiceType {
    PhotoPrinting,
    FilmDeveloping
};

// Order data
struct OrderData {
    int id;
    Client client;
    enum ServiceType type;
    string completionTime;
    bool isExpress;
    int price;
    bool done;
    bool paid;
};

struct OrderPhotographer {
    int id;
    enum ServiceType type;
    string completionTime;
    bool isExpress;
    bool done;
};

struct OrderClient {
    int id;
    Client client;
    enum ServiceType type;
    string completionTime;
    bool isExpress;
    int price;
};

// Client
class Client {
public:
    string name;
    string completionTime;
    OrderClient order;

    // Constructor
    Client(string n) : name(n) {}

    void placeOrder() {
        cout << "Client will choose which service type he want" << endl;
    }
};

// Receptionist
class Receptionist {
public:
    // Receptionist will keep track of all the order types
    vector<OrderData> mainOrders;
    vector<OrderPhotographer> ordersPh;
    vector<OrderClient> ordersCli;

    void createOrder() {
        cout << "The receptionist will input the client details and the completion time" << endl;
        cout << "It will create the main order, with the main template (OrderData)" << endl;
    }

    void orderPaid(OrderData order) {
        cout << "Order ID '" << order.id << "' is paid" << endl;
    }
    
    // I don't know if I need to create a struct or something similar for the reports, will see in the future
    void generateReport() {
        cout << "Receptionist generated the revenue report" << endl;
    }
};

// Order
class Order {
public:
    
    OrderData order;

    // Constructor
    Order(OrderData d) : order(d) {}
    
    void generateOrders() {
        cout << "From the main order it will create two types of order, one for client and the other for the photographer" << endl;
        cout << "INternally it will call calculatePrice() " << endl;
    }

    void calculatePrice() {
        cout << "It will calculate the price of the order depending on the service type and if it is express or not" << endl;
    }
};

// Photographer
// I suppose it will be only one photographer
class Photographer {
public:
    // The photographer will have all the orders from clients
    vector<OrderPhotographer> orders;

    void processOrder(OrderData order) {
        cout << "Once the photographer receives the orders, it process them based on the urgency and completion time" << endl;
        cout << "When finish with an order it will marked as done" << endl;
    }

    void submitReport() {
        cout << "Photographer submits all the consumables used to the administrator" << endl;
    }
};


class Administrator {
public:
    void trackConsumables() {
        cout << "Administrator is tracking the consumables materials" << endl;
    }
    
    // Don't know if this is necessary
    void replenishMaterials() {
        cout << "Administrator has replenished the materials" << endl;
    }
};

int main(void) {
    
}
