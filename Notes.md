# Notes

## Flow order

1. Client starts the flow by choosing one service type (PhotoPrinting, FilmDeveloping).
2. After that, the receptionist creates an order with the client data and completion time. 
    - This will create an instance of Order class and store it in a mapping of OrderData that receptionist has (`int id --> OrderData order`).
    - Internally will generate two types of orders, one for the client and the other for the photographer. This orders will also be stored in each respective mapping that the receptionist will have (`int id --> OrderPhotographer orderPho`)(`int id --> OrderClient orderCli`), all this orders connected with the same id.
    - This two orders created will also be stored each one respectively to the photographer and teh client, inside their classes will have their own mappings as well.
    - Before generating the two orders will calculate the price and if it is express or not, depending on the completion time desired by the client.
3. Then, the photographer will process orders depending on the completion time (express first). 
    - When is done it will mark his order as done, that automatically will update the main order related as done as well. So the receptionist will see which orders are done and not paid.
4. The receptionist then will process the payment of the order. The order willbe marked as paid and can be keept as historical data and not delete it.



## To do

- Should the constructor of client contain the completion time as well? Nope, if the client has more than one order then needs to specify a completion time for the others. Will be passed in placeOrder().
- I need to know if in placeOrder() it calls directly the functions of the receptionist or only stores the data in a vector and then the receptionist creates the order from that vector called by herself.
- FOr deleting the pending orders from the vector I can use the pop_back, but for that I need to loop the vector from the end till  the start