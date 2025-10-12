# Notes

## Flow

1. When a new client comes in, the receptionist needs to create a client object to assign its id and name.
    - The id will be autoincremented everytime a client is created.
    - Receptionist inputs the name.
2. After that the receptionist will create the order (createOrder()).
    - Needs to input the clientId (can query the getLastClientIdUsed()), completionTime and the service type.
    - It will create the main order struct with the inputs of the receptionist, with an orderId (autoincremented as well).
    - Create an obejct order by passing the struct we just created.
    - Calculates the prices and if it is express (<= 24h) and set them into the struct that is inside the obejct order.
    - It generates two types of orders (client and photographer) and sets them into structs (orderClient and orderPhotographer) into the order object, so we can access the three structs through the same object.
    - Adds the id of the order into the array of Ids inside the client object.
    - Push the order object into the mapping of mainOrders.

**NOTE**: at this point all the orders are created and accessed through an order object. Youc an access this order object inside `mainOrders` mapping by knowing the id.

3. Photographer process the orders (will have view functions to query the orders that need to be done)
    - Inputs the order id completed and the materials used for that order.
    - Marks the order as done on both structs (orderphotographer and orderdata) inside the order object.
    - There will be a global variable of type struct (consumedDaily), so the materials consumed for that order will be added into that variable (after will help us to make the report for the admin).

4. Once the orders are done, the receptionist can charge the clients and mark the order as paid (will have a getter for the orders done and not paid).
5. At the end of the day the receptionist will need to create a revenue report for the admin (will have a getter for the order paid and not reported).
    - The idea maybe is to have a global mapping with all the revenues of different days. The key would be the chrono (maybe create a getter for that) and the value the struct of the revenue itself.
6. The photographer needs to create as well his report of daily used materials.
    - The same as above, but easier on the logic, as we will have the global variable consumedMaterials already sum up, will need to take a snapshot of that a reset the variable for the next day.







## To resolve
1. Set const on getters.
2. Make sure you are writting on storage when making changes.

## Todo
- Error handling
- File mangement
- Each update needs to be changed in storage
- Terminal interactive


## Fix

Is failing on checkOrder() on the photographer class, not recognising the order is inside 


## Old main():

```c++
int main(void)
{
  // Create studios
  vector<ServiceType> s1 = {PhotoPrinting, FilmDeveloping, PassportPhoto};
  vector<ServiceType> s2 = {PhotoPrinting, DigitalEditing, PassportPhoto};

  Studio studioA(1, "Rotermanni", s1);
  Studio studioB(2, "Ulemiste", s2);

  // Get the receptionists
  auto &recepA = studioA.getReceptionist();
  auto &recepB = studioB.getReceptionist();

  // Create clients, two per studio
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

  // final checks clientsA
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

  // final check clientsB
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

```

