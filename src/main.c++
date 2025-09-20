#include <stdio.h>
#include <iostream>
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
};

// Client
class Client {
public:
    string name;
    string completionTime;

    // Constructor
    Client(string n) : name(n) {}

    // I think will not be any function attached to the client, as all the data input will do it the receptionist.
    // Maybe I will insert some informational functions, like when the order is created or is paid.
};

// Receptionist
class Receptionist {
public:
    void createOrder() {
        cout << "Receptionist created an order" << endl;
    }

    void orderPaid(OrderData order) {
        cout << "Order ID '" << order.id << "' is paid" << endl;
    }
    
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
        cout << "Order ID '" << order.id << "' has generated the client order" << endl;
        cout << "Order ID '" << order.id << "' has generated the photographer order" << endl;
    }

    void calculatePrice() {
        cout << "The price of the order '" << order.id << "' is: " << order.price << endl;
    }
};

// Photographer
class Photographer {
public:
    void processOrder(OrderData order) {
        cout << "Photographer processed order '" << order.id << "'" << endl;
    }

    void submitReport() {
        cout << "Photographer submitted consumables report report" << endl;
    }
};


class Administrator {
public:
    void trackConsumables() {
        cout << "Administrator is tracking the consumables materials" << endl;
    }

    void replenishMaterials() {
        cout << "Administrator has replenished the materials" << endl;
    }
};

int main(void) {
    
}
