#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <utility>
using namespace std;
using namespace std::chrono;

// Type of service
enum ServiceType {
    PhotoPrinting,
    FilmDeveloping
};

// Mapping of all clients with their uniques Ids
map<int, Client> clients;


// Mappings for storing orders:
std::map<int, Order > mainOrders;
std::map<int, OrderClient > ordersClient;
std::map<int, OrderPhotographer > ordersPhotographer;

// Mapping of all clients with their uniques Ids
map<int, Client> clients;


int orderId;
int clientId;

// Order data
struct OrderData {
    int id;
    Client client;
    enum ServiceType type;
    chrono::system_clock::time_point completionTimestamp;
    bool isExpress;
    int price;
    bool done;
    bool paid;
    bool reported;
};

// Struct and vector to store 
struct ConsumedMaterials {
    int paper;
    int developer;
    int toners;
    int fixer;
    int bleach;

};

struct OrderPhotographer {
    int id;
    enum ServiceType type;
    chrono::system_clock::time_point completionTimestamp;
    bool isExpress;
    bool done;
};

struct OrderClient {
    int id;
    Client client;
    enum ServiceType type;
    chrono::system_clock::time_point completionTimestamp;
    bool isExpress;
    int price;
};

// Function to get the price of each service
int servicePrice(ServiceType service) {
    switch (service) {
        case PhotoPrinting: return 20;
        case FilmDeveloping: return 40;
    }

    throw std::invalid_argument("Invalid ServiceType");
}



// Client
class Client {
private:
    int clientId;
    string name;
    // This two variables will keep changing while the client places more than one order
    system_clock::time_point completionTime;
    ServiceType service;

    vector<int> orderIds; // All the id's of the client, for accessing the order

public:
    // Constructor -> Used by receptionist
    // THe client ID needs to be unique
    Client(string name, int clientId) : name(name), clientId(clientId) {}

    // Function to get the specific order
    OrderClient getOrder(int id) {
        return ordersClient[id];
    }

    // Function to get last/current order
    OrderClient getLastOrder() {
        return ordersClient[orderIds.back()];
    }

    // Need it for the receptionist to set the orderID to the client
    void setOrderToClient(int orderId) {
        orderIds.push_back(orderId);
    }
    
    // Getter functions
    system_clock::time_point getCompletionTime() {
        return completionTime;
    }

    ServiceType getServiceType() {
        return service;
    }

    // Setters functions for the receptionist
    void setCompletionTime(system_clock::time_point com) {
        completionTime = com;
    }

    void setServiceType(ServiceType serv) {
        service = serv;
    }
};

// Receptionist
class Receptionist {
public:
    // Creation of the client
    void createClient(string name) {
        clients[clientId] = Client(name, clientId);
        clientId++;
    }

    // What about making a loop of all the pending orders?
    // Warning: the int completionTime needs to be in hours. If the order needs to be completed in 1 day then input 24 (hours)
    void createOrder(int clientId, int completionTime, ServiceType service) {
        Client client = clients[clientId];
        system_clock::time_point completionT = system_clock::now() + hours(completionTime);
        
        // Set the client data to its class instance
        client.setCompletionTime(completionT);
        client.setServiceType(service);

        // Create the orderData Struct
        OrderData orderData = {
            orderId,
            clients[clientId],
            service,
            completionT, // The timestamp at which the order needs to be completed
            false,
            0,
            false,
            false,
            false
        };
        
        // Create class Order
        Order order(orderData);

        // Get the price, with express included
        auto [price, isExpress] = order.calculatePrice(service, completionTime);
        
        // Set the price and the express to the order
        order.orderData.price = price;
        order.orderData.isExpress = isExpress;

        // Now I need to generate the two reports and add it into the right mappings
        order.generateOrders();

        // Set the order ID to the client vector, so the client can see his orders
        client.setOrderToClient(orderId);

        // Store the order into mapping
        mainOrders[orderId] = order;
        
        orderId++;
    
    }


    int getLastClientIdUsed() {
        return clientId - 1;
    }

    void collectPayment(int clientId, OrderData order) {
        cout << "Order ID '" << order.id << "' is paid" << endl;
    }
    
    // Will get all the reports paid in a day and sum, everything
    // Will give other characteristics, like the count of orders paid, the count of express orders, the count of clients, etc
    void generateReport() {
        cout << "Receptionist generated the revenue report" << endl;
    }
};

// Order
class Order {
public:
    
    OrderData orderData;
    //Should I put one instance of each order type?

    // Constructor
    Order(OrderData d) : orderData(d) {}

    // Generates orders for the photographer and the client
    std::pair<OrderClient, OrderPhotographer> generateOrders() {
        int id = orderData.id;

        // Create the order of the client
        OrderClient orderCli = {
            id,
            orderData.client,
            orderData.type,
            orderData.completionTimestamp,
            orderData.isExpress,
            orderData.price
        };

        // Create the order of the photographer
        OrderPhotographer orderPh = {
            id,
            orderData.type,
            orderData.completionTimestamp,
            orderData.isExpress,
            false
        };

        //Store the orders to the right mapping
        ordersClient[id] = orderCli;
        ordersPhotographer[id] = orderPh;

        return {orderCli, orderPh};
    }

    // Calculates the prices and the express
    std::pair<int, bool> calculatePrice(ServiceType service, int completionTime) {
        int basePrice = servicePrice(service);
        // AN express order is until 24 hours
        auto expressTimeLimit = system_clock::now() + hours(24);
        auto deadline = system_clock::now() + hours(completionTime);
        
        int finalPrice = basePrice;
        bool isExpress;
        if (deadline < expressTimeLimit) {
            finalPrice = basePrice + (basePrice * 25 / 100);
            isExpress = true;
        }

        return {finalPrice, isExpress};
    }
};

// Photographer
// I suppose it will be only one photographer
class Photographer {
public:
    // Inside here the photographer will need to input the consumables he spent for that order in specific, then at the end of the day 
    // the report can be made easily
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
    void viewReports() {
        cout << "Administrator is tracking the consumables materials" << endl;
    }
};

int main(void) {
    
}
