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
