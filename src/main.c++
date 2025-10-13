#include <cassert>
#include <chrono>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using namespace std;
using namespace std::chrono;

#define PHOTO_PRINTING_PRICE 20
#define FILM_DEVELOPING_PRICE 40
#define PASSPORT_PHOTO_PRICE 10
#define DIGITAL_EDITING_PRICE 30

// Type of service
enum ServiceType {
  PhotoPrinting,
  FilmDeveloping,
  PassportPhoto,
  DigitalEditing
};

// Increment variable for client IDs
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
struct OrderData {
  int id;
  Client *client;
  int studioId;
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
  int studioId;
  enum ServiceType type;
  chrono::system_clock::time_point completionTimestamp;
  bool isExpress;
  bool done;
};

struct OrderClient {
  int id;
  Client *client;
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

// Mappings for storing orders:
std::unordered_map<int, Order> mainOrders;

// Function to get the price of each service
int servicePrice(ServiceType service) {
  switch (service) {
  case PhotoPrinting:
    return PHOTO_PRINTING_PRICE;
  case FilmDeveloping:
    return FILM_DEVELOPING_PRICE;
  case PassportPhoto:
    return PASSPORT_PHOTO_PRICE;
  case DigitalEditing:
    return DIGITAL_EDITING_PRICE;
  }

  throw std::invalid_argument("Invalid ServiceType");
}

string serviceNameToString(ServiceType type) {
  switch (type) {
  case PhotoPrinting:
    return "PhotoPrinting";
  case FilmDeveloping:
    return "FilmDeveloping";
  case DigitalEditing:
    return "DigitalEditing";
  case PassportPhoto:
    return "PassportPhoto";
  default:
    return "Unknown service";
  }
}

// I create the interface so the receptionist and Photographer can access to
// functions of Studio without declaring it
class IStudio {
public:
  virtual ~IStudio() = default;

  virtual unordered_map<int, Client> &getClients() = 0;
  virtual vector<Order> getVisibleOrders() = 0;
  virtual bool offersService(ServiceType s) = 0;
  virtual int getId() = 0;
  virtual string getName() const = 0;
  virtual ConsumedMaterials &getConsumedMaterials() = 0;
};

// Order class
class Order {
private:
  OrderData orderData;

  void setPrice(int p) { orderData.price = p; }
  void setIsExpress(bool isEx) { orderData.isExpress = isEx; }
  void markDone() { orderData.done = true; }
  void markPaid() { orderData.paid = true; }
  void markReported() { orderData.reported = true; }
  void markTaken() { orderData.taken = true; }

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
    auto now = system_clock::now();
    auto expressTimeLimit = now + hours(24);
    auto deadline = now + hours(completionTime);

    int finalPrice = basePrice;
    bool isExpress = false;
    if (deadline <= expressTimeLimit) {
      finalPrice = basePrice + (basePrice * 25 / 100);
      isExpress = true;
    }

    return {finalPrice, isExpress};
  }

  OrderData getOrderData() { return orderData; }

  OrderClient getOrderClient() {
    OrderClient orderClient = {
        orderData.id,        orderData.client,
        orderData.type,      orderData.completionTimestamp,
        orderData.isExpress, orderData.price};

    return orderClient;
  }

  OrderPhotographer getOrderPhotographer() {
    OrderPhotographer orderPhotographer = {
        orderData.id,        orderData.studioId,
        orderData.type,      orderData.completionTimestamp,
        orderData.isExpress, orderData.done};

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
    cout << "\n\033[3;32m New order \033[0;33m" << orderId
         << "\033[3;32m added to the client \033[0;33m" << clientId << "\033[0m"
         << endl;
  }

  // Setters functions for the receptionist
  void setCompletionTime(system_clock::time_point com) { completionTime = com; }

  void setServiceType(ServiceType serv) { service = serv; }

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

  // Function to get last/current order of the client
  OrderClient getLastOrder() {
    int id = orderIds.back();
    return getOrder(id);
  }

  // Getter functions
  system_clock::time_point getCompletionTime() { return completionTime; }

  ServiceType getServiceType() { return service; }

  int getClientId() { return clientId; }

  vector<int> getAllOrderIds() { return orderIds; }

  string getName() { return name; }
};

// Receptionist
class Receptionist {
private:
  // The studio they're tied to
  IStudio &studio;

public:
  Receptionist(IStudio &s) : studio(s) {}

  // Creation of the client
  // Separate the implementation and definition
  // @todo validation of the string?
  void createClient(string name) {
    auto &clients = studio.getClients();
    clients.emplace(clientIdInc, Client(name, clientIdInc));
    clientIdInc++;

    // Logs
    cout << "\033[3;32mClient created with id \033[0;33m" << clientIdInc - 1
         << "\033[3;32m and name \033[0;33m" << name << "\033[0m"
         << "\033[3;32m in studio \033[0;33m" << studio.getName() << "\033[0m"
         << endl;
  }

  // @dev the int completionTime needs to be in hours. If the order needs to be
  // completed in 1 day then input 24 (hours)
  // @dev clientId must exist, so createClient must be called first, before
  // createOrder
  // @dev the clientId is inserted by the receptionist in case one already
  // registered client wants to make another order

  int createOrder(int clientId, int completionTime, ServiceType service) {
    if (!studio.offersService(service)) {
      throw runtime_error("This studio does not offer this service");
    }
    // Not more than 1 year and a minimum of 1 hour
    if (completionTime > 8765 || completionTime < 1) {
      throw runtime_error(
          "Completion time must be 1 >= completionT <= 8765 (1 year in hours)");
    }
    auto &clients = studio.getClients();
    // Validation
    if (!validateClient(clientId, clients)) {
      throw runtime_error("Client don't exist or out of your studio");
    }
    // Automatically checks that the client is inside the clients of the studio,
    // but still, need to handle the error
    Client *client = &clients.at(clientId);
    system_clock::time_point completionT =
        system_clock::now() + hours(completionTime);

    // Set the client data to its class instance, dereference the pointer to
    // access the obejct itself (->)
    client->setCompletionTime(completionT);
    client->setServiceType(service);

    // Create the orderData Struct
    OrderData orderData = {
        orderId,
        client,         // client pointer
        studio.getId(), // Studio Id
        service,        // service type
        completionT, // The timestamp at which the order needs to be completed
        false,       // isExpress
        0,           // price
        false,       // done
        false,       // paid
        false        // reported
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
    int id = orderId;
    orderId++;

    // Logs
    cout << "\n\033[5;32mNew order created:\033[0m" << endl;
    cout << "Order ID: " << "\033[1;32m" << id << "\033[0m" << endl;
    cout << "Client ID: " << "\033[1;32m" << clientId << "\033[0m" << endl;
    cout << "Studio ID: " << "\033[1;32m" << studio.getId() << "\033[0m"
         << endl;
    cout << "Service: " << "\033[1;32m" << serviceNameToString(service)
         << "\033[0m" << endl;
    auto now = system_clock::to_time_t(completionT);
    string timeStr = ctime(&now);
    // The end line is included
    cout << "Completion Timestamp: " << "\033[1;32m" << timeStr << "\033[0m";
    cout << "Is express: " << "\033[1;32m" << boolalpha << isExpress
         << "\033[0m" << endl;
    cout << "Price: " << "\033[1;32m" << price << "\033[0m" << endl;

    return id;
  }

  bool validateClient(int clientId, unordered_map<int, Client> clients) {
    for (auto [id, client] : clients) {
      if (id == clientId) {
        return true;
      }
    }
    return false;
  }

  // Mark the order as paid
  // Only orders from the clients of the local studio
  void collectPayment(int orderId) {
    // Validation
    if (!checkOrder(orderId)) {
      throw runtime_error("Order don't exist or not from your studio");
    }
    Order &order = mainOrders.at(orderId);
    // Order needs to be done and not paid
    bool done = order.getOrderData().done;
    bool paid = order.getOrderData().paid;
    if (!done || paid) {
      throw runtime_error("Order is not done or is paid");
    }
    order.markPaid();

    int studioId = order.getOrderData().studioId;
    cout << "Order Id - " << orderId << " - has been paid" << endl;
    // The studio ids needs to match
    cout << "Studio Id of the order paid: " << studioId << endl;
    cout << "Local Studio Id: " << studio.getId() << endl;
  }

  // Checks that the order id they want to process is inside their clients list
  // of the local studio. I can not make it the same as the photographer, so the
  // receptionists can only work with orders inside their client list
  bool checkOrder(int orderId) {
    // A copy is okay, no need to make changes on the order
    Order order = mainOrders.at(orderId);
    int studioId = order.getOrderData().studioId;
    if (studioId == studio.getId()) {
      return true;
    }

    return false;
  }

  // Will get all the reports paid in a day and sum, everything
  void generateRevenueReport() {
    string name = "Daily_revenue_studio_" + to_string(studio.getId());
    // Append the new report to the same file
    ofstream file(name, ios::app);

    if (!file) {
      cerr << "Could not open revenue file!" << endl;
      return;
    }
    auto now = system_clock::to_time_t(system_clock::now());
    string timeStr = ctime(&now);
    timeStr.pop_back(); // Remove the "\n" new line
    file << "==== Report generated on " << timeStr << " ====" << endl;

    vector<int> ids = getPaidOrdersNotReported();
    int total = 0;
    for (int id : ids) {
      Order &o = mainOrders.at(id);
      OrderData d = o.getOrderData();
      total += d.price;
      file << "Order " << d.id << " - " << d.client->getName() << " - "
           << d.price << "€" << endl;
      o.markReported();
    }

    file << "Total Revenue: " << total << "€" << endl;
    file << "========================================" << endl << endl;
    file.close();

    cout << "Daily report generated for Studio" << studio.getId() << endl;
  }

  // Get the orders done and not paid so the clients can pay
  // Only orders from the local studio
  vector<int> getDoneOrdersNotPaid() {
    vector<int> orderIds;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders) {
      int studioId = order.getOrderData().studioId;
      // Get rid of all paid, reported and incomplete orders
      if (!order.orderData.done || order.orderData.paid ||
          order.orderData.reported || studioId != studio.getId()) {
        continue;
      } else if (order.orderData.done && !order.orderData.paid &&
                 studioId == studio.getId()) {
        orderIds.push_back(order.orderData.id);
      }
    }

    return orderIds;
  }

  // Get paid orders and not reported to make the report calculations
  // Only orders from the local studio
  vector<int> getPaidOrdersNotReported() {
    vector<int> orderIds;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders) {
      int studioId = order.getOrderData().studioId;
      // Get rid of all not done, paid and reported orders
      if (!order.orderData.done || !order.orderData.paid ||
          order.orderData.reported || studioId != studio.getId()) {
        continue;
      } else if (order.orderData.done && order.orderData.paid &&
                 !order.orderData.reported && studioId == studio.getId()) {
        orderIds.push_back(order.orderData.id);
      }
    }

    return orderIds;
  }

  // To insert it
  int getLastClientIdUsed() { return clientIdInc - 1; }

  int getStudioId() { return studio.getId(); }
};

// Photographer
class Photographer {
private:
  IStudio &studio;

public:
  Photographer(IStudio &s) : studio(s) {}

  // Everytime an order is completed needs to be called by the photographer
  // @audit a photographer that sees this order and didn't take it, can process
  // the order. Maybe it can be allowed as the photographers are trusted, we
  // assume they won't misbehave.
  void processOrder(int orderId, ConsumedMaterials materials) {
    if (!checkOrder(orderId)) {
      throw invalid_argument("Invalid orderId");
    }
    // Extract the order
    Order &order = mainOrders.at(orderId);
    // The order has to be taken and not done
    bool taken = order.getOrderData().taken;
    bool done = order.getOrderData().done;
    if (!taken || done) {
      throw runtime_error("Order not taken or is already done");
    }
    // Set done to the order
    order.markDone();

    ConsumedMaterials &consumedDaily = studio.getConsumedMaterials();
    // Add the consumed materials to the global struct of the materials consumed
    // in that day
    consumedDaily.paper += materials.paper;
    consumedDaily.developer += materials.developer;
    consumedDaily.toners += materials.toners;
    consumedDaily.fixer += materials.fixer;
    consumedDaily.bleach += materials.bleach;

    cout << "Order - " << orderId << " - has been completed by studio - "
         << studio.getId() << endl;
    int studioId = order.getOrderData().studioId;
    cout << "Studio of the order completed: " << studioId;
  }

  // Everytime the photographer starts an order
  void startOrder(int orderId) {
    if (!checkOrder(orderId)) {
      throw invalid_argument("Invalid orderId");
    }

    Order &order = mainOrders.at(orderId);
    // The order cannot be taken already
    bool taken = order.getOrderData().taken;
    if (taken) {
      throw runtime_error("Order is taken already");
    }
    order.markTaken();

    cout << "Order - " << orderId << " - has been taken by studio - "
         << studio.getId() << endl;
    // See if the order has been taken by a different studio
    int studioId = order.getOrderData().studioId;
    cout << "Studio Id of the order taken: " << studioId << endl;
  }

  // Gets the orders that needs to be done by the next 24h, based on the visible
  // orders of the studio
  // @dev We can not be based on express, as some orders will need to be
  // completed that are not express, but express orders will be included

  vector<OrderPhotographer> getFirstOrders() {
    vector<OrderPhotographer> pendingOrders;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders) {
      // Get rid of all done orders or paid or reported
      if (order.orderData.done || order.orderData.paid ||
          order.orderData.reported || order.orderData.taken) {
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

  // In case the photographer completed all the necessary orders and wants to
  // make more orders
  // @dev gets all the pending orders, the time does not matter
  vector<OrderPhotographer> getPendingOrders() {
    vector<OrderPhotographer> pendingOrders;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders) {
      // Get rid of all done orders or paid or reported
      if (order.orderData.done || order.orderData.paid ||
          order.orderData.reported || order.orderData.taken) {
        continue;
      } else if (!order.orderData.done && !order.orderData.taken) {
        pendingOrders.push_back(order.getOrderPhotographer());
      }
    }

    return pendingOrders;
  }

  // Checks that the order id they want to process is inside their visible
  // orders, to avoid any mistake
  bool checkOrder(int orderId) {
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders) {
      if (order.getOrderData().id == orderId) {
        return true;
      }
    }

    return false;
  }

  // Reset the consumed materials struct to zero after reported
  void makeMaterialsReport() {
    string name = "Materials_report_studio_" + to_string(studio.getId());
    ofstream file(name, ios::app);

    if (!file) {
      cerr << "Error: Could not open materials report file" << endl;
    }

    auto now = system_clock::to_time_t(system_clock::now());
    // COnvert the time into string
    string timeStr = ctime(&now);
    timeStr.pop_back(); // remove new line from ctime
    // Get the consumed materials of the studio, reference (&), if not I would
    // modify only the copy not the storage itself
    ConsumedMaterials &consumedDaily = studio.getConsumedMaterials();

    file << "==== Materials Consumption Report (" << timeStr
         << ") ====" << endl;
    file << "Paper used: " << consumedDaily.paper << endl;
    file << "Developer used: " << consumedDaily.developer << endl;
    file << "Toners used: " << consumedDaily.toners << endl;
    file << "Fixer used: " << consumedDaily.fixer << endl;
    file << "Bleach used: " << consumedDaily.bleach << endl;
    file << "================================================" << endl << endl;

    file.close();

    // reset struct for the next day
    consumedDaily = {0, 0, 0, 0, 0};

    cout << "Photographer submitted materials report successfully." << endl;
  }
};

// @todo what do I do with this class?
class Administrator {
private:
  IStudio &studio;

public:
  Administrator(IStudio &s) : studio(s) {}

  void viewConsumedMaterialsReport() {
    string name = "Materials_report_studio_" + to_string(studio.getId());
    ifstream file(name);
    if (!file) {
      cerr << "No report found.\n";
      return;
    }

    cout << "=== Materials report for Studio " << studio.getId()
         << " ===" << endl;
    string line;
    while (getline(file, line)) {
      cout << line << endl;
    }
  }

  void viewDailyRevenueReport() {
    string filename = "Daily_revenue_studio_" + to_string(studio.getId());
    ifstream file(filename);
    if (!file) {
      cerr << "No report found.\n";
      return;
    }

    cout << "=== Revenue Report for Studio " << studio.getId()
         << " ===" << endl;
    string line;
    while (getline(file, line))
      cout << line << endl;
  }
};

class Studio : public IStudio {
private:
  int studioId;
  string name;

  unordered_map<int, Client> clients;
  Receptionist receptionist;
  Photographer photographer;
  Administrator administrator;

  vector<ServiceType> supportedServices;
  // This will be the struct that will be keep adding materials everytime an
  // order is completed. Will be reset to zero every day at the end, when the
  // report of materials has been done.
  ConsumedMaterials consumedDaily;

public:
  Studio(int id, const string n, vector<ServiceType> services)
      : studioId(id), name(n), supportedServices(services), receptionist(*this),
        photographer(*this), administrator(*this),
        consumedDaily{0, 0, 0, 0, 0} {} // Initialize the materials to zero

  string getName() const { return name; }

  int getId() { return studioId; }

  Receptionist &getReceptionist() { return receptionist; }
  Photographer &getPhotographer() { return photographer; }
  Administrator &getAdmin() { return administrator; }

  unordered_map<int, Client> &getClients() { return clients; }
  ConsumedMaterials &getConsumedMaterials() { return consumedDaily; }

  bool offersService(ServiceType s) {
    for (auto service : supportedServices) {
      if (service == s) {
        return true;
      }
    }
    return false;
  }

  // @dev I can only use this function for view purposes, not for changing the
  // orders itself, as I think it would onyl make change  in the copy, not in
  // storage
  vector<Order> getVisibleOrders() {
    vector<Order> visible;
    for (auto &[id, order] : mainOrders) {
      if (offersService(order.getOrderData().type)) {
        visible.push_back(order);
      }
    }
    return visible;
  }
};

// TESTS START
#define IS_TRUE(x)                                                             \
  {                                                                            \
    if (!(x))                                                                  \
      std::cout << __FUNCTION__ << " failed on line " << __LINE__              \
                << std::endl;                                                  \
    else {                                                                     \
      std::cout << __FUNCTION__ << " passed on line " << __LINE__              \
                << std::endl;                                                  \
    }                                                                          \
  }
class Tests {
private:
  vector<ServiceType> s1;
  vector<ServiceType> s2;
  Studio *studio;

  auto getReceptionst() {
    auto &recep = studio->getReceptionist();
    return recep;
  }
  auto getPhotographer() {
    auto &photo = studio->getPhotographer();
    return photo;
  }
  auto getClients() {
    auto &clients = studio->getClients();
    return clients;
  }

  void test_createClient() {
    auto recep = getReceptionst();
    recep.createClient("Krop");
    auto clients = getClients();

    string expect = "Krop";
    IS_TRUE(expect == clients.at(0).getName());
  }
  void test_servicePrice() {
    IS_TRUE(servicePrice(PhotoPrinting) == PHOTO_PRINTING_PRICE);
    IS_TRUE(servicePrice(FilmDeveloping) == FILM_DEVELOPING_PRICE);
    IS_TRUE(servicePrice(PassportPhoto) == PASSPORT_PHOTO_PRICE);
    IS_TRUE(servicePrice(DigitalEditing) == DIGITAL_EDITING_PRICE);
  }
  void test_createOrder() {}

public:
  Tests() {
    s1 = {PhotoPrinting, FilmDeveloping, PassportPhoto};
    s2 = {PhotoPrinting, DigitalEditing, PassportPhoto};
    studio = new Studio(1, "TestingStudio", s1);
  }
  void doTests() {
    test_createClient();
    test_servicePrice();
  }
}; // TESTS END // @todo add view functionality, view orders and so on. View
// reports? @todo Log the orders when created, see another logs that I could
// add
int main() {
  Tests tests;
  tests.doTests();
  // Create studios with supported services
  vector<ServiceType> s1 = {PhotoPrinting, FilmDeveloping, PassportPhoto};
  vector<ServiceType> s2 = {PhotoPrinting, DigitalEditing, PassportPhoto};

  Studio studioA(1, "Rotermanni", s1);
  Studio studioB(2, "Ulemiste", s2);

  bool running = true;
  while (running) {

    cout << "\033[1;31m\n=== PHOTO STUDIO SYSTEM ===\033[0m\n";
    cout << "1. Create client\n";
    cout << "2. Create order\n";
    // View functions for the photographer or receptionist
    cout << "3. View data\n";
    cout << "4. Start order\n";
    cout << "5. Process order (Photographer)\n";
    cout << "6. Collect payment\n";
    cout << "7. Generate revenue report\n";
    cout << "8. Generate materials report\n";
    cout << "9. View Reports\n";
    cout << "10. Exit\n";
    cout << "\033[3;32mSelect option: \033[0m";

    int choice;
    cin >> choice;

    char studioChoice = -1;
    while (studioChoice != '1' && studioChoice != '2') {
      cout << "Select studio (1 = Rotermanni, 2 = Ulemiste): ";
      cin >> studioChoice;
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      cin.clear();
    }

    Studio *studio = (studioChoice == '1') ? &studioA : &studioB;

    auto &recep = studio->getReceptionist();
    auto &photo = studio->getPhotographer();

    switch (choice) {
    // Create client
    case 1: {
      cout << "Enter client name: ";
      string name;
      // Used to input a name with spaces
      // cin.ignore();
      getline(cin, name);
      recep.createClient(name);
      break;
    }

    // Create order
    case 2: {
      cout << "Enter client ID: ";
      int clientId;
      cin >> clientId;

      cout << "Enter completion time (hours): ";
      int hours;
      cin >> hours;

      cout << "Select service:\n";
      cout << "0. Photo Printing\n";
      cout << "1. Film Developing\n";
      cout << "2. Passport Photo\n";
      cout << "3. Digital Editing\n";
      int s;
      // @todo need to validate that the input number is indeed between 0 and 3?
      cin >> s;
      ServiceType service = static_cast<ServiceType>(s);

      try {
        recep.createOrder(clientId, hours, service);
      } catch (const exception &e) {
        cerr << "Error creating order: " << e.what() << endl;
      }
      break;
    }

    // View data
    case 3: {
      cout << "\n=== VIEW DATA ===\n";
      cout << "1. View client info\n";
      cout << "2. View order info\n";
      cout << "Select option: ";
      int viewChoice;
      cin >> viewChoice;

      if (viewChoice == 1) {
        cout << "Enter client ID: ";
        int clientId;
        cin >> clientId;

        auto &clients = studio->getClients();
        if (clients.find(clientId) == clients.end()) {
          cout << "No client with that ID found in this studio.\n";
          break;
        }

        Client &client = clients.at(clientId);
        cout << "\n--- CLIENT INFO ---\n";
        cout << "Name: " << client.getName() << endl;
        cout << "Client ID: " << client.getClientId() << endl;
        cout << "Number of orders: " << client.getAllOrderIds().size() << endl;

        auto ids = client.getAllOrderIds();

        if (!ids.empty()) {
          cout << "All order ids: " << endl;
          for (int id : ids) {
            cout << "ID: " << id << endl;
          }
          cout << "Last order ID: " << ids.back() << endl;
          // In order client format?
          OrderClient lastOrder = client.getLastOrder();
          cout << "Last order service: " << serviceNameToString(lastOrder.type)
               << endl;
          cout << "Express: " << (lastOrder.isExpress ? "Yes" : "No") << endl;
          cout << "Price: " << lastOrder.price << "€" << endl;
          cout << "Paid: " << (lastOrder.paid ? "Yes" : "No") << endl;
        }
        cout << "--------------------\n";
      } else if (viewChoice == 2) {
        cout << "Enter order ID: ";
        int orderId;
        cin >> orderId;

        if (mainOrders.find(orderId) == mainOrders.end()) {
          cout << "No order with that ID found.\n";
          break;
        }

        Order &order = mainOrders.at(orderId);
        OrderData data = order.getOrderData();

        cout << "\n--- ORDER INFO ---\n";
        cout << "Order ID: " << data.id << endl;
        cout << "Studio ID: " << data.studioId << endl;
        cout << "Client Name: " << data.client->getName() << endl;
        cout << "Client Id: " << data.client->getClientId() << endl;
        cout << "Service: " << serviceNameToString(data.type) << endl;
        cout << "Express: " << (data.isExpress ? "Yes" : "No") << endl;
        cout << "Price: " << data.price << "€" << endl;
        cout << "Taken: " << (data.taken ? "Yes" : "No") << endl;
        cout << "Done: " << (data.done ? "Yes" : "No") << endl;
        cout << "Paid: " << (data.paid ? "Yes" : "No") << endl;
        cout << "Reported: " << (data.reported ? "Yes" : "No") << endl;
        cout << "------------------\n";
      } else {
        cout << "Returning to main menu.\n";
      }

      break;
    }

    // Start order
    case 4: {
      auto todayOrders = photo.getFirstOrders();
      auto pendingOrders = photo.getPendingOrders();
      if (pendingOrders.empty()) {
        cout << "No orders to take available.\n";
        break;
      }
      // To not leave the secondary menu
      while (true) {
        if (!todayOrders.empty()) {
          cout << "Orders to take for today:\n";
          for (OrderPhotographer order : todayOrders) {
            cout << "---- Order ID: " << order.id << "-------" << endl;
            cout << "- Service Type: " << serviceNameToString(order.type)
                 << endl;
            auto timestamp = system_clock::to_time_t(order.completionTimestamp);
            cout << "- Deadline: " << ctime(&timestamp);
            cout << "---------------------------\n";
          }
        }

        cout << "1. Process order\n";
        cout << "2. See all orders\n";
        cout << "3. Back to main menu\n";
        cout << "Select option: ";
        int choice;
        cin >> choice;

        if (choice == 2) {
          cout << "All orders available to take:\n";
          for (OrderPhotographer order : pendingOrders) {
            cout << "---- Order ID: " << order.id << "-------" << endl;
            cout << "- Service Type: " << serviceNameToString(order.type)
                 << endl;
            auto timestamp = system_clock::to_time_t(order.completionTimestamp);
            cout << "- Deadline: " << ctime(&timestamp) << endl;
            cout << "- Studio Id: " << order.studioId;
            cout << "---------------------------\n";
          }
        } else if (choice == 1) {
          cout << "Enter Order Id to process: ";
          int orderId;
          cin >> orderId;
          try {
            photo.startOrder(orderId);
          } catch (const exception &e) {
            cerr << "Error starting the order: " << e.what() << endl;
          }
          break;
        }
        // Any other choice goes back to the main menu
        else {
          cout << "Returning to main menu.\n";
          break;
        }
      }
      break;
    }

    // Process order
    case 5: {
      cout << "Enter order ID to process: ";
      int orderId;
      cin >> orderId;

      ConsumedMaterials materials;
      cout << "Enter paper used: ";
      cin >> materials.paper;
      cout << "Enter developer used: ";
      cin >> materials.developer;
      cout << "Enter toners used: ";
      cin >> materials.toners;
      cout << "Enter fixer used: ";
      cin >> materials.fixer;
      cout << "Enter bleach used: ";
      cin >> materials.bleach;

      try {
        photo.processOrder(orderId, materials);
      } catch (const exception &e) {
        cerr << "Error processing order: " << e.what() << endl;
      }
      break;
    }

    // Pay order
    case 6: {
      auto doneOrders = recep.getDoneOrdersNotPaid();
      if (doneOrders.empty()) {
        cout << "No completed unpaid orders available.\n";
        break;
      }

      cout << "Unpaid completed orders:\n";
      for (int id : doneOrders)
        cout << "- Order ID: " << id << endl;

      cout << "Enter order ID to mark as paid: ";
      int orderId;
      cin >> orderId;

      try {
        recep.collectPayment(orderId);
      } catch (const exception &e) {
        cerr << "Error collecting payment: " << e.what() << endl;
      }
      break;
    }

    case 7: {
      recep.generateRevenueReport();
      break;
    }

    case 8: {
      photo.makeMaterialsReport();
      break;
    }

    // View reports
    case 9: {
      cout << "\n=== ADMINISTRATOR REPORTS ===\n";
      cout << "1. View Daily Revenue Report\n";
      cout << "2. View Materials Report\n";
      cout << "3. Back to main menu\n";
      cout << "Select option: ";

      int adminChoice;
      cin >> adminChoice;

      auto &admin = studio->getAdmin();

      switch (adminChoice) {
      case 1:
        admin.viewDailyRevenueReport();
        break;
      case 2:
        admin.viewConsumedMaterialsReport();
        break;
      default:
        cout << "Returning to main menu.\n";
        break;
      }
      break;
    }

    case 10: {
      running = false;
      cout << "Exiting program. Bye!\n";
      break;
    }

    default:
      cout << "Invalid option, try again.\n";
    }
  }

  return 0;
}
