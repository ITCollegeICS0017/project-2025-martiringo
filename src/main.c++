#include <stdio.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <utility>
using namespace std;
using namespace std::chrono;

// Type of service
enum ServiceType {
    PhotoPrinting,
    FilmDeveloping
};

// Increment variable for clinet IDs
int clientIdInc = 0;
int orderId = 0;

// Declare classes before to avoid colisions
class Client;
class Order;

// Mapping of all clients with their uniques Ids
unordered_map<int, Client> clients;

// Order data
struct OrderData {
    int id;
    Client* client;
    enum ServiceType type;
    chrono::system_clock::time_point completionTimestamp;
    bool isExpress;
    int price;
    bool done;
    bool paid;
    bool reported;
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
    Client* client;
    enum ServiceType type;
    chrono::system_clock::time_point completionTimestamp;
    bool isExpress;
    int price;
    bool paid;
};

// Struct and vector to store 
struct ConsumedMaterials {
    int paper;
    int developer;
    int toners;
    int fixer;
    int bleach;

};
// This will be the struct that will be keep adding materials everytime an order is completed.
// Will be reset to zero every day at the end, when the report of materials has been done.
ConsumedMaterials consumedDaily;


// Mappings for storing orders:
std::unordered_map<int, Order > mainOrders;
// std::unordered_map<int, OrderClient > ordersClient;
// std::unordered_map<int, OrderPhotographer > ordersPhotographer;
// Mapping for historical data, processed order only

// Function to get the price of each service
int servicePrice(ServiceType service) {
    switch (service) {
        case PhotoPrinting: return 20;
        case FilmDeveloping: return 40;
    }

    throw std::invalid_argument("Invalid ServiceType");
}


// Order class
class Order {
public:
    
    OrderData orderData;
    OrderClient orderClient;
    OrderPhotographer orderPhotographer;

    // Constructor
    Order(OrderData d) : orderData(d) {}

    // Generates orders for the photographer and the client
    void generateOrders() {
        int id = orderData.id;

        // Set the order struct of the client
        orderClient = {
            id,
            orderData.client,
            orderData.type,
            orderData.completionTimestamp,
            orderData.isExpress,
            orderData.price
        };

        // Set the order struct of the photographer
        orderPhotographer = {
            id,
            orderData.type,
            orderData.completionTimestamp,
            orderData.isExpress,
            false
        };
    }

    // Calculates the prices and the express
    std::pair<int, bool> calculatePrice(ServiceType service, int completionTime) {
        int basePrice = servicePrice(service);
        // AN express order is until 24 hours
        auto expressTimeLimit = system_clock::now() + hours(24);
        auto deadline = system_clock::now() + hours(completionTime);
        
        int finalPrice = basePrice;
        bool isExpress = false;
        if (deadline < expressTimeLimit) {
            finalPrice = basePrice + (basePrice * 25 / 100);
            isExpress = true;
        }

        return {finalPrice, isExpress};
    }

    void markDone() {
      orderData.done = true;
      orderPhotographer.done = true;
    }
    void markPaid() {
      orderData.paid = true;
      orderClient.paid = true;
    }

    void markReported() {
      orderData.reported = true;
    }
};


// Client
class Client {
private:
    // Unique client id, not overwritten
    int clientId;
    string name;
    // This two variables will be overwritten every time an order is placed
    system_clock::time_point completionTime;
    ServiceType service;
    // All the order id's of the client, for accessing the order
    vector<int> orderIds;

public:
    // Constructor -> Used by receptionist
    // THe client ID needs to be unique
    Client(string name, int clientId) : name(name), clientId(clientId) {}

    // Function to get the specific order
    OrderClient getOrder(int id) {
        Order order = mainOrders.at(id);
        return order.orderClient;
    }

    // Function to get last/current order
    OrderClient getLastOrder() {
        int id = orderIds.back();
        Order order = mainOrders.at(id);
        return order.orderClient; 
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
        clients.emplace(clientIdInc, Client(name, clientIdInc));
        clientIdInc++;
    }

    // What about making a loop of all the pending orders?
    // @dev the int completionTime needs to be in hours. If the order needs to be completed in 1 day then input 24 (hours)
    // @dev clientId must exist, so createClient must be called first, before createOrder
    // @dev the clientId is inserted by the receptionist in case one already registered client wants to make another order
    void createOrder(int clientId, int completionTime, ServiceType service) {
        Client* client = &clients.at(clientId);
        system_clock::time_point completionT = system_clock::now() + hours(completionTime);
        
        // Set the client data to its class instance, dereference the pointer to access the obejct itself (->)
        client->setCompletionTime(completionT);
        client->setServiceType(service);

        // Create the orderData Struct
        OrderData orderData = {
            orderId,
            client,
            service,
            completionT, // The timestamp at which the order needs to be completed
            false, // isExpress
            0, // price
            false, // done
            false, // paid
            false // reported
        };
        
        // Create one instance of class Order with the struct just created
        Order order(orderData);

        // Get the price, with express included
        auto [price, isExpress] = order.calculatePrice(service, completionTime);
        
        // Set the price and the express to the order
        order.orderData.price = price;
        order.orderData.isExpress = isExpress;

        // Now I need to generate the two reports
        order.generateOrders();

        // Set the order ID to the client vector, so the client can see his orders
        client->setOrderToClient(orderId);

        // Store the order into mapping
        mainOrders.emplace(orderId, order);
        
        orderId++;
    
    }

    // Get the orders done and not paid so the clients can pay
    // @dev we can make it "const" as we are only going to read data, not change it. Efficient
    vector<int> getDoneOrdersNotPaid() {
      vector<int> orderIds;
      for (const auto &pair : mainOrders) {
        int id = pair.first;
        const Order &order = pair.second;
        // Get rid of all paid and reportedorders
        if (!order.orderData.done || order.orderData.paid || order.orderData.reported) {
          continue;
        } else if (order.orderData.done && !order.orderData.paid) {
          orderIds.push_back(id);
        }
      }

      return orderIds;
    }

    // Get paid orders and not reported to make the report calculations
    vector<int> getPaidOrdersNotReported() {
      vector<int> orderIds;
      for (const auto &pair : mainOrders) {
        int id = pair.first;
        const Order &order = pair.second;
        // Get rid of all not done, paid and reported orders
        if (!order.orderData.done || !order.orderData.paid || order.orderData.reported) {
          continue;
        } else if (order.orderData.done && order.orderData.paid && !order.orderData.reported) {
          orderIds.push_back(id);
        }
      }

      return orderIds;
    }

    // To insert it
    int getLastClientIdUsed() {
        return clientIdInc - 1;
    }
    
    // Mark the order as paid
    void collectPayment(int orderId) {
        Order order = mainOrders.at(orderId);
        order.markPaid();
    }
    
    // Will get all the reports paid in a day and sum, everything
    // Will give other characteristics, like the count of orders paid, the count of express orders, the count of clients, etc
    void generateRevenueReport() {
        cout << "Receptionist generated the revenue report" << endl;
    }
};

// Photographer
// I suppose it will be only one photographer
class Photographer {
public:

    // Everytime an order is completed needs to be called by the photographer
    void processOrder(int orderId, ConsumedMaterials materials) {
      // Extract the order  
      Order order = mainOrders.at(orderId);
      // Set done to the order
      order.markDone();
      
      // Add the consumed materials to the global struct of the materials consumed in that day
      consumedDaily.paper += materials.paper;
      consumedDaily.developer += materials.developer;
      consumedDaily.toners += materials.toners;
      consumedDaily.fixer += materials.fixer;
      consumedDaily.bleach += materials.bleach;

    }

    // Gets the orders that needs to be done by the next 24h
    // @dev We can not be based on express, as some orders will need to be completed that are not express, but express orders will be included
    vector<OrderPhotographer> getFirstOrders() {
      vector<OrderPhotographer> pendingOrders;
      for (const auto &pair : mainOrders) {
        int id = pair.first;
        const Order &order = pair.second;
        // Get rid of all done orders or paid or reported
        if (order.orderData.done || order.orderData.paid || order.orderData.reported) {
          continue;
        } else if (!order.orderData.done) {
          // Let's get the orders that need to be completed in the next 24h
          system_clock::time_point deadline = system_clock::now() + hours(24);
          if (order.orderData.completionTimestamp <= deadline) {
            pendingOrders.push_back(order.orderPhotographer);
          }
          
        }
      }

      return pendingOrders;
    }

    // In case the photographer completed all the necessary orders and wants to make more orders
    // @dev gets all the pending orders, the time does not matter
    vector<OrderPhotographer> getPendingOrders() {
      vector<OrderPhotographer> pendingOrders;
      for (const auto &pair : mainOrders) {
        int id = pair.first;
        const Order &order = pair.second;
        // Get rid of all done orders or paid or reported
        if (order.orderData.done || order.orderData.paid || order.orderData.reported) {
          continue;
        } else if (!order.orderData.done) {
            pendingOrders.push_back(order.orderPhotographer);          
        }
      }

      return pendingOrders;

    }


    void makeMaterialsReport() {
        cout << "Photographer submits all the consumables used to the administrator" << endl;
    }
};

// The reports can be queried by the administrators directly, no 
class Administrator {
public:
    void viewConsumedMaterialsReport() {}

    void viewDailyRevenueReport() {}
};

int main(void) {
    
}
