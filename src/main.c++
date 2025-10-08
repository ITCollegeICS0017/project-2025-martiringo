#include <stdio.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <utility>
#include <cassert>
#include <fstream>
using namespace std;
using namespace std::chrono;

// Type of service
enum ServiceType
{
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
struct OrderData
{
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

struct OrderPhotographer
{
  int id;
  enum ServiceType type;
  chrono::system_clock::time_point completionTimestamp;
  bool isExpress;
  bool done;
};

struct OrderClient
{
  int id;
  Client *client;
  enum ServiceType type;
  chrono::system_clock::time_point completionTimestamp;
  bool isExpress;
  int price;
  bool paid;
};

// Struct and vector to store
struct ConsumedMaterials
{
  int paper;
  int developer;
  int toners;
  int fixer;
  int bleach;
};

// Mappings for storing orders:
std::unordered_map<int, Order> mainOrders;

// Function to get the price of each service
int servicePrice(ServiceType service)
{
  switch (service)
  {
  case PhotoPrinting:
    return 20;
  case FilmDeveloping:
    return 40;
  case PassportPhoto:
    return 10;
  case DigitalEditing:
    return 30;
  }

  throw std::invalid_argument("Invalid ServiceType");
}

// I create the interface so the receptionist and Photographer can access to functions of Studio without declaring it
class IStudio
{
public:
  virtual ~IStudio() = default;

  virtual unordered_map<int, Client> &getClients() = 0;
  virtual vector<Order> getVisibleOrders() = 0;
  virtual bool offersService(ServiceType s) = 0;
  virtual int getId() = 0;
  virtual ConsumedMaterials &getConsumedMaterials() = 0;
};

// Order class
class Order
{
private:
  OrderData orderData;

  void setPrice(int p)
  {
    orderData.price = p;
  }
  void setIsExpress(bool isEx)
  {
    orderData.isExpress = isEx;
  }
  void markDone()
  {
    orderData.done = true;
  }
  void markPaid()
  {
    orderData.paid = true;
  }
  void markReported()
  {
    orderData.reported = true;
  }
  void markTaken()
  {
    orderData.taken = true;
  }

  // Only the receptionist and the photographer can call this functions
  friend class Receptionist;
  friend class Photographer;

public:
  // Constructor
  Order(OrderData d) : orderData(d) {}

  // Calculates the prices and the express
  pair<int, bool> calculatePrice(ServiceType service, int completionTime)
  {
    int basePrice = servicePrice(service);
    // AN express order is until 24 hours
    auto expressTimeLimit = system_clock::now() + hours(24);
    auto deadline = system_clock::now() + hours(completionTime);

    int finalPrice = basePrice;
    bool isExpress = false;
    if (deadline <= expressTimeLimit)
    {
      finalPrice = basePrice + (basePrice * 25 / 100);
      isExpress = true;
    }

    return {finalPrice, isExpress};
  }

  OrderData getOrderData()
  {
    return orderData;
  }

  OrderClient getOrderClient()
  {
    OrderClient orderClient = {
        orderData.id,
        orderData.client,
        orderData.type,
        orderData.completionTimestamp,
        orderData.isExpress,
        orderData.price};

    return orderClient;
  }

  OrderPhotographer getOrderPhotographer()
  {
    OrderPhotographer orderPhotographer = {
        orderData.id,
        orderData.type,
        orderData.completionTimestamp,
        orderData.isExpress,
        orderData.done};

    return orderPhotographer;
  }
};

// Client
class Client
{
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
  // @todo check the that the order is valid? I think is already validated when creating the order, and this function can not be called isolated so I don't
  // think is necessary
  void setOrderToClient(int orderId)
  {
    orderIds.push_back(orderId);
  }

  // Setters functions for the receptionist
  // @todo validation of the completion time? I think is also done when creating the order
  void setCompletionTime(system_clock::time_point com)
  {
    completionTime = com;
  }

  void setServiceType(ServiceType serv)
  {
    service = serv;
  }

  friend class Receptionist;

public:
  // Constructor -> Used by receptionist
  // THe client ID needs to be unique
  Client(string name, int clientId) : name(name), clientId(clientId) {}

  // Function to get the specific order
  // Generate the order on demand
  OrderClient getOrder(int id)
  {
    Order order = mainOrders.at(id);
    OrderClient orderClient = order.getOrderClient();
    return orderClient;
  }

  // Function to get last/current order
  OrderClient getLastOrder()
  {
    int id = orderIds.back();
    return getOrder(id);
  }

  // Getter functions
  system_clock::time_point getCompletionTime()
  {
    return completionTime;
  }

  ServiceType getServiceType()
  {
    return service;
  }

  int getClientId()
  {
    return clientId;
  }

  vector<int> getAllOrderIds()
  {
    return orderIds;
  }

  string getName()
  {
    return name;
  }
};

// Receptionist
class Receptionist
{
private:
  // The studio they're tied to
  IStudio &studio;

public:
  Receptionist(IStudio &s) : studio(s) {}

  // Creation of the client
  // Separate the implementation and definition
  // @todo validation of the string?
  void createClient(string name)
  {
    auto &clients = studio.getClients();
    clients.emplace(clientIdInc, Client(name, clientIdInc));
    clientIdInc++;
  }

  // @dev the int completionTime needs to be in hours. If the order needs to be completed in 1 day then input 24 (hours)
  // @dev clientId must exist, so createClient must be called first, before createOrder
  // @dev the clientId is inserted by the receptionist in case one already registered client wants to make another order
  int createOrder(int clientId, int completionTime, ServiceType service)
  {
    if (!studio.offersService(service))
    {
      throw runtime_error("This studio does not offer this service");
    }
    // Not more than 1 year and a minimum of 1 hour
    if (completionTime > 8765 || completionTime < 1)
    {
      throw runtime_error("Completion time must be 1 >= completionT <= 8765 (1 year in hours)");
    }
    // Automatically checks that the client is inside the clients of the studio
    // @ask is it enough error handling?
    auto &clients = studio.getClients();
    Client *client = &clients.at(clientId);
    system_clock::time_point completionT = system_clock::now() + hours(completionTime);

    // Set the client data to its class instance, dereference the pointer to access the obejct itself (->)
    client->setCompletionTime(completionT);
    client->setServiceType(service);

    // Create the orderData Struct
    OrderData orderData = {
        orderId,
        client,         // client pointer
        studio.getId(), // Studio Id
        service,        // service type
        completionT,    // The timestamp at which the order needs to be completed
        false,          // isExpress
        0,              // price
        false,          // done
        false,          // paid
        false           // reported
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

    return id;
  }

  // Mark the order as paid
  // Only orders from the clients of the local studio
  void collectPayment(int orderId)
  {
    if (!checkOrder(orderId))
    {
      throw runtime_error("Order don't exist or not from your studio");
    }
    Order &order = mainOrders.at(orderId);
    order.markPaid();
  }

  // Checks that the order id they want to process is inside their clients list of the local studio.
  // I can not make it the same as the photographer, so the receptionists can only work with orders inside their client list
  bool checkOrder(int orderId)
  {
    // A copy is okay, no need to make changes on the order
    Order order = mainOrders.at(orderId);
    int studioId = order.getOrderData().studioId;
    if (studioId == studio.getId())
    {
      return true;
    }

    return false;
  }

  // Will get all the reports paid in a day and sum, everything
  void generateRevenueReport()
  {
    string name = "Daily_revenue_studio_" + to_string(studio.getId());
    // Append the new report to the same file
    ofstream file(name, ios::app);

    if (!file)
    {
      cerr << "Could not open revenue file!" << endl;
      return;
    }
    auto now = system_clock::to_time_t(system_clock::now());
    string timeStr = ctime(&now);
    timeStr.pop_back(); // Remove the "\n" new line
    file << "==== Report generated on " << timeStr << " ====" << endl;

    vector<int> ids = getPaidOrdersNotReported();
    int total = 0;
    for (int id : ids)
    {
      Order &o = mainOrders.at(id);
      OrderData d = o.getOrderData();
      total += d.price;
      file << "Order " << d.id << " - " << d.client->getName() << " - " << d.price << "€" << endl;
      o.markReported();
    }

    file << "Total Revenue: " << total << "€" << endl;
    file << "========================================" << endl
         << endl;
    file.close();

    cout << "Daily report generated for Studio" << studio.getId() << endl;
  }

  // Get the orders done and not paid so the clients can pay
  // Only orders from the local studio
  vector<int> getDoneOrdersNotPaid()
  {
    vector<int> orderIds;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders)
    {
      int studioId = order.getOrderData().studioId;
      // Get rid of all paid, reported and incomplete orders
      if (!order.orderData.done || order.orderData.paid || order.orderData.reported || studioId != studio.getId())
      {
        continue;
      }
      else if (order.orderData.done && !order.orderData.paid && studioId == studio.getId())
      {
        orderIds.push_back(order.orderData.id);
      }
    }

    return orderIds;
  }

  // Get paid orders and not reported to make the report calculations
  // Only orders from the local studio
  vector<int> getPaidOrdersNotReported()
  {
    vector<int> orderIds;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders)
    {
      int studioId = order.getOrderData().studioId;
      // Get rid of all not done, paid and reported orders
      if (!order.orderData.done || !order.orderData.paid || order.orderData.reported || studioId != studio.getId())
      {
        continue;
      }
      else if (order.orderData.done && order.orderData.paid && !order.orderData.reported && studioId == studio.getId())
      {
        orderIds.push_back(order.orderData.id);
      }
    }

    return orderIds;
  }

  // To insert it
  int getLastClientIdUsed()
  {
    return clientIdInc - 1;
  }

  int getStudioId()
  {
    return studio.getId();
  }
};

// Photographer
class Photographer
{
private:
  IStudio &studio;

public:
  Photographer(IStudio &s) : studio(s) {}

  // Everytime an order is completed needs to be called by the photographer
  void processOrder(int orderId, ConsumedMaterials materials)
  {
    if (!checkOrder(orderId))
    {
      throw invalid_argument("Invalid orderId");
    }
    // Extract the order
    Order &order = mainOrders.at(orderId);
    // Set done to the order
    order.markDone();

    ConsumedMaterials &consumedDaily = studio.getConsumedMaterials();
    // Add the consumed materials to the global struct of the materials consumed in that day
    consumedDaily.paper += materials.paper;
    consumedDaily.developer += materials.developer;
    consumedDaily.toners += materials.toners;
    consumedDaily.fixer += materials.fixer;
    consumedDaily.bleach += materials.bleach;
  }

  // Everytime the photographer starts an order
  void startOrder(int orderId)
  {
    if (!checkOrder(orderId))
    {
      throw invalid_argument("Invalid orderId");
    }

    Order &order = mainOrders.at(orderId);
    order.markTaken();
  }

  // Gets the orders that needs to be done by the next 24h, based on the visible orders of the studio
  // @dev We can not be based on express, as some orders will need to be completed that are not express, but express orders will be included

  vector<OrderPhotographer> getFirstOrders()
  {
    vector<OrderPhotographer> pendingOrders;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders)
    {
      // Get rid of all done orders or paid or reported
      if (order.orderData.done || order.orderData.paid || order.orderData.reported || order.orderData.taken)
      {
        continue;
      }
      else if (!order.orderData.done && !order.orderData.taken)
      {
        // Let's get the orders that need to be completed in the next 24h
        system_clock::time_point deadline = system_clock::now() + hours(24);
        if (order.orderData.completionTimestamp <= deadline)
        {
          pendingOrders.push_back(order.getOrderPhotographer());
        }
      }
    }
    return pendingOrders;
  }

  // In case the photographer completed all the necessary orders and wants to make more orders
  // @dev gets all the pending orders, the time does not matter
  vector<OrderPhotographer> getPendingOrders()
  {
    vector<OrderPhotographer> pendingOrders;
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders)
    {
      // Get rid of all done orders or paid or reported
      if (order.orderData.done || order.orderData.paid || order.orderData.reported || order.orderData.taken)
      {
        continue;
      }
      else if (!order.orderData.done && !order.orderData.taken)
      {
        pendingOrders.push_back(order.getOrderPhotographer());
      }
    }

    return pendingOrders;
  }

  // Checks that the order id they want to process is inside their visible orders, to avoid any mistake
  bool checkOrder(int orderId)
  {
    vector<Order> visibleOrders = studio.getVisibleOrders();
    for (Order order : visibleOrders)
    {
      if (order.getOrderData().id == orderId)
      {
        return true;
      }
    }

    return false;
  }

  // Reset the consumed materials struct to zero after reported
  void makeMaterialsReport()
  {
    string name = "Materials_report_studio_" + to_string(studio.getId());
    ofstream file(name, ios::app);

    if (!file)
    {
      cerr << "Error: Could not open materials report file" << endl;
    }

    auto now = system_clock::to_time_t(system_clock::now());
    string timeStr = ctime(&now);
    timeStr.pop_back(); // remove new line from ctime
    // Get the consumed materials of the studio, reference (&), if not I would modify only the copy not the storage itself
    ConsumedMaterials &consumedDaily = studio.getConsumedMaterials();

    file << "==== Materials Consumption Report (" << timeStr << ") ====" << endl;
    file << "Paper used: " << consumedDaily.paper << endl;
    file << "Developer used: " << consumedDaily.developer << endl;
    file << "Toners used: " << consumedDaily.toners << endl;
    file << "Fixer used: " << consumedDaily.fixer << endl;
    file << "Bleach used: " << consumedDaily.bleach << endl;
    file << "================================================" << endl
         << endl;

    file.close();

    // reset struct for the next day
    consumedDaily = {0, 0, 0, 0, 0};

    cout << "Photographer submitted materials report successfully." << endl;
  }
};

// The reports can be queried by the administrators directly, no
class Administrator
{
private:
  IStudio &studio;

public:
  Administrator(IStudio &s) : studio(s) {}

  void viewConsumedMaterialsReport() {}

  void viewDailyRevenueReport() {}
};

class Studio : public IStudio
{
private:
  int studioId;
  string name;

  unordered_map<int, Client> clients;
  Receptionist receptionist;
  Photographer photographer;
  Administrator administrator;

  vector<ServiceType> supportedServices;
  // This will be the struct that will be keep adding materials everytime an order is completed.
  // Will be reset to zero every day at the end, when the report of materials has been done.
  ConsumedMaterials consumedDaily;

public:
  Studio(int id, const string n, vector<ServiceType> services)
      : studioId(id),
        name(n),
        supportedServices(services),
        receptionist(*this),
        photographer(*this),
        administrator(*this),
        consumedDaily{0, 0, 0, 0, 0} {} // Initialize the materials to zero

  string getName() const { return name; }

  int getId() { return studioId; }

  Receptionist &getReceptionist() { return receptionist; }
  Photographer &getPhotographer() { return photographer; }
  Administrator &getAdmin() { return administrator; }

  unordered_map<int, Client> &getClients() { return clients; }
  ConsumedMaterials &getConsumedMaterials() { return consumedDaily; }

  bool offersService(ServiceType s)
  {
    for (auto service : supportedServices)
    {
      if (service == s)
      {
        return true;
      }
    }
    return false;
  }

  // @dev I can only use this function for view purposes, not for changing the orders itself, as I think it would onyl make change  in the copy, not in storage
  vector<Order> getVisibleOrders()
  {
    vector<Order> visible;
    for (auto &[id, order] : mainOrders)
    {
      if (offersService(order.getOrderData().type))
      {
        visible.push_back(order);
      }
    }
    return visible;
  }
};

int main(void)
{
  vector<ServiceType> s1 = {PhotoPrinting, FilmDeveloping, PassportPhoto};
  vector<ServiceType> s2 = {PhotoPrinting, DigitalEditing, PassportPhoto};

  Studio studioA(1, "Rotermanni", s1);
  Studio studioB(2, "Ulemiste", s2);

  auto &recepA = studioA.getReceptionist();
  auto &recepB = studioB.getReceptionist();

  // Create clients
  recepA.createClient("Alice");
  int aliceId = recepA.getLastClientIdUsed();
  recepA.createClient("Bob");
  int bobId = recepA.getLastClientIdUsed();
  recepB.createClient("Eve");
  int eveId = recepB.getLastClientIdUsed();
  recepB.createClient("John");
  int johnId = recepB.getLastClientIdUsed();

  // create orders
  int orderAlice = recepA.createOrder(aliceId, 12, PhotoPrinting);
  int orderBob = recepA.createOrder(bobId, 25, FilmDeveloping);
  int orderEve = recepB.createOrder(eveId, 48, DigitalEditing);
  int orderJohn = recepB.createOrder(johnId, 15, PassportPhoto);

  // Checks
  unordered_map<int, Client> clientsA = studioA.getClients();
  unordered_map<int, Client> clientsB = studioB.getClients();

  // Checks clientsA
  for (auto [id, client] : clientsA)
  {
    // Only alice and bob
    assert(client.getClientId() == aliceId || client.getClientId() == bobId);
    // Both clients needs to have exactly 1 order Id
    vector<int> ids = client.getAllOrderIds();
    assert(ids.size() == 1);
    Order order = mainOrders.at(ids[0]);
    OrderData data = order.getOrderData();
    assert(data.id == ids[0]);
    assert(data.studioId == recepA.getStudioId());
    if (client.getName() == "Alice")
    {
      int price = servicePrice(PhotoPrinting);
      assert(data.isExpress);
      assert(data.price == price + (price * 25 / 100));
    }
    else
    {
      assert(!data.isExpress);
      assert(data.price == servicePrice(FilmDeveloping));
    }
  }

  // Check clientsB
  for (auto [id, client] : clientsB)
  {
    // Only alice and bob
    assert(client.getClientId() == eveId || client.getClientId() == johnId);
    // Both clients needs to have exactly 1 order Id
    vector<int> ids = client.getAllOrderIds();
    assert(ids.size() == 1);
    Order order = mainOrders.at(ids[0]);
    OrderData data = order.getOrderData();
    assert(data.id == ids[0]);
    assert(data.studioId == recepB.getStudioId());
    if (client.getName() == "John")
    {
      int price = servicePrice(PassportPhoto);
      assert(data.isExpress);
      assert(data.price == price + (price * 25 / 100));
    }
    else
    {
      assert(!data.isExpress);
      assert(data.price == servicePrice(DigitalEditing));
    }
  }

  // Photographer from studio B takes Alice and Eve
  auto &phB = studioB.getPhotographer();
  // Use the same for simplicity
  ConsumedMaterials mat = {1, 1, 0, 0, 0};
  phB.startOrder(orderAlice);
  phB.startOrder(orderEve);
  phB.processOrder(orderAlice, mat);
  phB.processOrder(orderEve, mat);

  // Photographer from studio A takes BOb and john
  auto &phA = studioA.getPhotographer();
  // Use the same for simplicity
  ConsumedMaterials mat1 = {1, 1, 2, 1, 0};
  phA.startOrder(orderBob);
  phA.startOrder(orderJohn);
  phA.processOrder(orderBob, mat1);
  phA.processOrder(orderJohn, mat1);

  // Receptionist collects the payment
  // gets all the orders not paid
  vector<int> idsADone = recepA.getDoneOrdersNotPaid();
  vector<int> idsBDone = recepB.getDoneOrdersNotPaid();

  assert(idsADone.size() == 2);
  assert(idsBDone.size() == 2);

  for (auto id : idsADone)
  {
    recepA.collectPayment(id);
  }

  for (auto id : idsBDone)
  {
    recepB.collectPayment(id);
  }

  // Report the orders
  recepA.generateRevenueReport();
  recepB.generateRevenueReport();

  // Report consumed materials
  phA.makeMaterialsReport();
  phB.makeMaterialsReport();

  unordered_map<int, Client> clientsAFinal = studioA.getClients();
  unordered_map<int, Client> clientsBFinal = studioB.getClients();

  // Checks clientsA
  for (auto [id, client] : clientsAFinal)
  {
    // Only alice and bob
    assert(client.getClientId() == aliceId || client.getClientId() == bobId);
    // Both clients needs to have exactly 1 order Id
    vector<int> ids = client.getAllOrderIds();
    assert(ids.size() == 1);
    Order order = mainOrders.at(ids[0]);
    OrderData data = order.getOrderData();
    assert(data.id == ids[0]);
    // assert(data.studioId == recepA.getStudioId());
    if (client.getName() == "Alice")
    {
      int price = servicePrice(PhotoPrinting);
      assert(data.isExpress);
      assert(data.price == price + (price * 25 / 100));
    }
    else
    {
      assert(!data.isExpress);
      assert(data.price == servicePrice(FilmDeveloping));
    }
    assert(data.taken);
    assert(data.done);
    assert(data.paid);
    assert(data.reported);
  }

  // Check clientsB
  for (auto [id, client] : clientsBFinal)
  {
    // Only alice and bob
    assert(client.getClientId() == eveId || client.getClientId() == johnId);
    // Both clients needs to have exactly 1 order Id
    vector<int> ids = client.getAllOrderIds();
    assert(ids.size() == 1);
    Order order = mainOrders.at(ids[0]);
    OrderData data = order.getOrderData();
    assert(data.id == ids[0]);
    // assert(data.studioId == recepB.getStudioId());
    if (client.getName() == "John")
    {
      int price = servicePrice(PassportPhoto);
      assert(data.isExpress);
      assert(data.price == price + (price * 25 / 100));
    }
    else
    {
      assert(!data.isExpress);
      assert(data.price == servicePrice(DigitalEditing));
    }

    assert(data.taken);
    assert(data.done);
    assert(data.paid);
    assert(data.reported);
  }
}
