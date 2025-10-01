#include <stdio.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <utility>
#include <cassert>
using namespace std;
using namespace std::chrono;

// Type of service
enum ServiceType {
    PhotoPrinting,
    FilmDeveloping,
    PassportPhoto,
    DigitalEditing
};

// Increment variable for clinet IDs
int clientIdInc = 0;
int orderId = 0;

// Declare classes before to avoid colisions
class Client;
class Order;
class Receptionist;
class Photographer;
class Administrator;
class Studio;

// Order data
// @todo Should I put an studio field to track the orders based on studio? So then the receptionist can only mark as paid the orders of his own studio?
struct OrderData {
    int id;
    Client* client;
    enum ServiceType type;
    chrono::system_clock::time_point completionTimestamp;
    bool isExpress;
    int price;
    bool taken; // to avoid making an order two times
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

// Function to get the price of each service
// @todo need to put price in new services
int servicePrice(ServiceType service) {
    switch (service) {
        case PhotoPrinting: return 20;
        case FilmDeveloping: return 40;
    }

    throw std::invalid_argument("Invalid ServiceType");
}

// I create the interface so the receptionist and Photographer can access to functions of Studio without declaring it
class IStudio {
public:
  virtual ~IStudio() = default;

  virtual unordered_map<int, Client>& getClients() = 0;
  virtual vector<Order>& getVisibleOrders() = 0;
  virtual bool offersService(ServiceType s) = 0;

};

// Order class
class Order {
private:
    
    OrderData orderData;

    void setPrice(int p) {
      orderData.price = p;
    }
    void setIsExpress(bool isEx) {
      orderData.isExpress = isEx;
    }
    void markDone() {
      orderData.done = true;
    }
    void markPaid() {
      orderData.paid = true;
    }
    void markReported() {
      orderData.reported = true;
    }
    void markTaken() {
      orderData.taken = true;
    }

    // Only the receptionist and the photographer can call this functions
    friend class Receptionist;
    friend class Photographer;

public:
    // Constructor
    Order(OrderData d) : orderData(d) {}

    // Calculates the prices and the express
    pair<int, bool> calculatePrice(ServiceType service, int completionTime) {
        int basePrice = servicePrice(service);
        // AN express order is until 24 hours
        auto expressTimeLimit = system_clock::now() + hours(24);
        auto deadline = system_clock::now() + hours(completionTime);
        
        int finalPrice = basePrice;
        bool isExpress = false;
        if (deadline <= expressTimeLimit) {
            finalPrice = basePrice + (basePrice * 25 / 100);
            isExpress = true;
        }

        return {finalPrice, isExpress};
    }

    OrderData getOrderData() {
      return orderData;
    }

    OrderClient getOrderClient() {
      OrderClient orderClient = {
            orderData.id,
            orderData.client,
            orderData.type,
            orderData.completionTimestamp,
            orderData.isExpress,
            orderData.price
        };

        return orderClient;
    }

    OrderPhotographer getOrderPhotographer() {
      OrderPhotographer orderPhotographer = {
        orderData.id,
        orderData.type,
        orderData.completionTimestamp,
        orderData.isExpress,
        orderData.done
      };

      return orderPhotographer;
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

    // Need it for the receptionist to set the orderID to the client
    void setOrderToClient(int orderId) {
        orderIds.push_back(orderId);
    }

    // Setters functions for the receptionist
    void setCompletionTime(system_clock::time_point com) {
        completionTime = com;
    }

    void setServiceType(ServiceType serv) {
        service = serv;
    }

    friend class Receptionist;

public:
    // Constructor -> Used by receptionist
    // THe client ID needs to be unique
    Client(string name, int clientId) : name(name), clientId(clientId) {}

    // Function to get the specific order
    // Generate the order on demand
    OrderClient getOrder(int id) {
        Order order = mainOrders.at(id);
        OrderClient orderClient = order.getOrderClient();
        return orderClient;
    }

    // Function to get last/current order
    OrderClient getLastOrder() {
        int id = orderIds.back();
        return getOrder(id);
    }
    
    // Getter functions
    system_clock::time_point getCompletionTime() {
        return completionTime;
    }

    ServiceType getServiceType() {
        return service;
    }
};


// Receptionist
class Receptionist {
private:
    // The studio they're tied to
    IStudio& studio;

    // Creation of the client
    // Separate the implementation and definition
    void createClient(string name) {
        auto& clients = studio.getClients();
        clients.emplace(clientIdInc, Client(name, clientIdInc));
        clientIdInc++;
    }

    // What about making a loop of all the pending orders?
    // @dev the int completionTime needs to be in hours. If the order needs to be completed in 1 day then input 24 (hours)
    // @dev clientId must exist, so createClient must be called first, before createOrder
    // @dev the clientId is inserted by the receptionist in case one already registered client wants to make another order
    // @todo need to check if the client is in client list of the studio
    void createOrder(int clientId, int completionTime, ServiceType service) {
        if (!studio.offersService(service)) {
          throw runtime_error("This studio does not offer this service");
        }
        auto& clients = studio.getClients();
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
        order.setPrice(price);
        order.setIsExpress(isExpress);

        // Set the order ID to the client vector, so the client can see his orders
        client->setOrderToClient(orderId);

        // Store the order into mapping
        mainOrders.emplace(orderId, order);
        
        orderId++;
    
    }

    // Mark the order as paid
    // @audit-info should the receptionist can only mark as paid the orders made only by his studio? It can make sense
    // WHy? So the flow is, the photographer can  do an order of another studio but the receptionist can not mark as paid other orders that are outside of
    // his studio
    void collectPayment(int orderId) {
        Order &order = mainOrders.at(orderId);
        order.markPaid();
    }

    // Checks that the order id they want to process is inside their visible orders, to avoid any mistake
    bool checkOrder(int orderId) {
      vector<Order> visibleOrders = studio.getVisibleOrders();
      for (Order order : visibleOrders) {
        if (order.getOrderData().id == orderId){
          return true;
        }
      }  

      return false;
    }
    
    // Will get all the reports paid in a day and sum, everything
    // Will give other characteristics, like the count of orders paid, the count of express orders, the count of clients, etc
    // @todo the report, probably I will try to use the file handling
    void generateRevenueReport() {
        cout << "Receptionist generated the revenue report" << endl;
    }
public:
    Receptionist(IStudio& s) : studio(s) {}

    // Get the orders done and not paid so the clients can pay
    // @dev we can make it "const" as we are only going to read data, not change it. Efficient
    vector<int> getDoneOrdersNotPaid() {
      vector<int> orderIds;
      vector<Order> visibleOrders =  studio.getVisibleOrders();
      for (const Order order : visibleOrders) {
        // Get rid of all paid, reported and incomplete orders
        if (!order.orderData.done || order.orderData.paid || order.orderData.reported) {
          continue;
        } else if (order.orderData.done && !order.orderData.paid) {
          orderIds.push_back(order.orderData.id);
        }
      }

      return orderIds;
    }

    // Get paid orders and not reported to make the report calculations
    // @audit-info
    vector<int> getPaidOrdersNotReported() {
      vector<int> orderIds;
      vector<Order> visibleOrders =  studio.getVisibleOrders();
      for (const Order order : visibleOrders) {
        // Get rid of all not done, paid and reported orders
        if (!order.orderData.done || !order.orderData.paid || order.orderData.reported) {
          continue;
        } else if (order.orderData.done && order.orderData.paid && !order.orderData.reported) {
          orderIds.push_back(order.orderData.id);
        }
      }

      return orderIds;
    }

    // To insert it
    int getLastClientIdUsed() {
        return clientIdInc - 1;
    }
};

// Photographer
class Photographer {
private:
    IStudio& studio;

public:

    Photographer(IStudio& s) : studio(s) {}

    // Everytime an order is completed needs to be called by the photographer
    void processOrder(int orderId, ConsumedMaterials materials) {
      if (!checkOrder(orderId)) { throw invalid_argument("Invalid orderId");}
      // Extract the order  
      Order &order = mainOrders.at(orderId);
      // Set done to the order
      order.markDone();
      
      // Add the consumed materials to the global struct of the materials consumed in that day
      consumedDaily.paper += materials.paper;
      consumedDaily.developer += materials.developer;
      consumedDaily.toners += materials.toners;
      consumedDaily.fixer += materials.fixer;
      consumedDaily.bleach += materials.bleach;

    }

    // Everytime the photographer starts an order
    void startOrder(int orderId) {
      if (!checkOrder(orderId)) { throw invalid_argument("Invalid orderId");}

      Order &order = mainOrders.at(orderId);
      order.markTaken();
    }

    // Gets the orders that needs to be done by the next 24h, based on the visible orders of the studio
    // @dev We can not be based on express, as some orders will need to be completed that are not express, but express orders will be included
    
    vector<OrderPhotographer> getFirstOrders() {
      vector<OrderPhotographer> pendingOrders;
      vector<Order> visibleOrders =  studio.getVisibleOrders();
      for (Order order : visibleOrders) {
        // Get rid of all done orders or paid or reported
        if (order.orderData.done || order.orderData.paid || order.orderData.reported || order.orderData.taken) {
          continue;
        } else if (!order.orderData.done && !order.orderData.taken) {
          // Let's get the orders that need to be completed in the next 24h
          system_clock::time_point deadline = system_clock::now() + hours(24);
          if (order.orderData.completionTimestamp <= deadline) {
            pendingOrders.push_back(order.getOrderPhotographer());
          }
          
        }
      }
      return pendingOrders;
    }

    // In case the photographer completed all the necessary orders and wants to make more orders
    // @dev gets all the pending orders, the time does not matter
    vector<OrderPhotographer> getPendingOrders() {
      vector<OrderPhotographer> pendingOrders;
      vector<Order> visibleOrders =  studio.getVisibleOrders();
      for (Order order : visibleOrders) {
        // Get rid of all done orders or paid or reported
        if (order.orderData.done || order.orderData.paid || order.orderData.reported || order.orderData.taken) {
          continue;
        } else if (!order.orderData.done && !order.orderData.taken) {
            pendingOrders.push_back(order.getOrderPhotographer());          
        }
      }

      return pendingOrders;

    }

    // Checks that the order id they want to process is inside their visible orders, to avoid any mistake
    bool checkOrder(int orderId) {
      vector<Order> visibleOrders = studio.getVisibleOrders();
      for (Order order : visibleOrders) {
        if (order.getOrderData().id == orderId){
          return true;
        }
      }  

      return false;
    }


    void makeMaterialsReport() {
        cout << "Photographer submits all the consumables used to the administrator" << endl;
    }
};

// The reports can be queried by the administrators directly, no 
class Administrator {
private:
    IStudio& studio;

public:
    Administrator(IStudio& s) : studio(s) {}

    void viewConsumedMaterialsReport() {}

    void viewDailyRevenueReport() {}
};


class Studio : public IStudio {
private:
    int studioId;
    string name;

    unordered_map<int, Client> clients;
    Receptionist receptionist;
    Photographer photographer;
    Administrator administrator;


    unordered_set<ServiceType> supportedServices;

public:
    Studio(int id, const string n, unordered_set<ServiceType> services) : studioId(id), name(n), supportedServices(services), receptionist(*this), photographer(*this), administrator(*this) {}

    string getName() const { return name; }

    int getId() const { return studioId; }

    Receptionist& getReceptionist() { return receptionist; }
    Photographer& getPhotographer() { return photographer; }
    Administrator& getAdmin() { return administrator; }

    unordered_map<int, Client>& getClients() { return clients; }
    
    bool offersService(ServiceType s) const {
      return supportedServices.count(s) > 0;
    }
    
    // @todo Check that this function is not utilized to make some storage changes.
    // @dev I can only use this function for view purposes, not for changing the orders itself, as I think it would onyl make change  in the copy, not in storage
    vector<Order>& getVisibleOrders() const {
      vector<Order> visible;
      for (auto& [id, order] : mainOrders) {
        if (offersService(order.getOrderData().type)) {
          visible.push_back(order);
        }
      }
      return visible;
    }
};

int main(void) {


    
}
