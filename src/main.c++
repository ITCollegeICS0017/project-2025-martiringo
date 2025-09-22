#include <iostream>
#include <map>
#include <stdio.h>
#include <vector>
using namespace std;

// Client
class Client {
private:
  int clientId;
  string name;
  // This two variables will keep changing while the client places more than one
  // order
  string completionTime;
  ServiceType service;

  vector<int> orderIds; // All the id's of the client, for accessing the order

public:
  // Constructor -> Used by receptionist
  // The client ID needs to be unique
  Client(string name, int clientId) : name(name), clientId(clientId) {}

  // Function to get the specific order
  OrderClient getOrder(int id) { return ordersClient[id]; }

  // Function to get last/current order
  OrderClient getLastOrder() { return ordersClient[orderIds.back()]; }

  // Need it for the receptionist to set the orderID to the client
  void setOrderToClient(int idClient) { orderIds.push_back(idClient); }

  // Getters functions for the receptionist
  string getCompletionTime() { return completionTime; }

  void setCompletionTime(string com) { completionTime = com; }

  ServiceType getServiceType() { return service; }

  void setServiceType(ServiceType serv) { service = serv; }
};

// Receptionist
class Receptionist {
public:
  void createClient(string name, int clientId) {
    clients[clientId] = Client(name, clientId);
  }

  // What about making a loop of all the pending orders?
  void createOrder(int clientId, string completionTime, ServiceType service) {
    // I need to get the completion time and the service from the Client struct
    // and then create the order with it
  }

  void collectPayment(int clientId, OrderData order) {
    cout << "Order ID '" << order.id << "' is paid" << endl;
  }

  // Will get all the reports paid in a day and sum, everything
  // Will give other characteristics, like the count of orders paid, the count
  // of express orders, the count of clients, etc
  void generateReport() {
    cout << "Receptionist generated the revenue report" << endl;
  }
};

// Order
class Order {
public:
  OrderData order;
  // Should I put one instance of each order type?

  // Constructor
  Order(OrderData d) : order(d) {}

  void generateOrders() {
    cout << "From the main order it will create two types of order, one for "
            "client and the other for the photographer"
         << endl;
    cout << "Internally it will call calculatePrice() " << endl;
  }

  void calculatePrice() {
    cout << "It will calculate the price of the order depending on the service "
            "type and if it is express or not"
         << endl;
  }
};

// Photographer
// I suppose it will be only one photographer
class Photographer {
public:
  // The photographer will have all the orders from clients
  vector<OrderPhotographer> orders;

  void processOrder(OrderData order) {
    cout << "Once the photographer receives the orders, it process them based "
            "on the urgency and completion time"
         << endl;
    cout << "When finish with an order it will marked as done" << endl;
  }

  void submitReport() {
    cout << "Photographer submits all the consumables used to the administrator"
         << endl;
  }
};

class Administrator {
public:
  void viewReports() {
    cout << "Administrator is tracking the consumables materials" << endl;
  }
};



// Type of service
enum ServiceType { PhotoPrinting, FilmDeveloping };

// Mapping of all clients with their uniques Ids
map<int, class Client> clients;

// Order data
struct OrderData {
  int id;
  class Client client;
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
  class Client client;
  enum ServiceType type;
  string completionTime;
  bool isExpress;
  int price;
};


// Mappings for storing orders:
std::map<int, Order> mainOrders;
std::map<int, OrderClient> ordersClient;
std::map<int, OrderPhotographer> ordersPhotographer;

int main(void) {
  // Create receptionist that creates a client with a name and ID. The
  // receptionist then creates an order.
  Receptionist receptionist;
  receptionist.createClient("priit", 1);
  receptionist.createOrder(1, "23/09/2025-17:30", PhotoPrinting);
