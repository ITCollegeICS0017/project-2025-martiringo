| User Story  | CRC Responsability | Simple Class Diagram Element | C++ Element |
| -------- | ------- | ------- | ------- |
| Receptionist creates the order | **Receptionist**: Records the client data <br> **Order**: Calculates price     |         | `Receptionist::createOrder()` <br> `Order::calculatePrice()`   |
| Order generates two child orders    | **Order**: <br> Create client type order <br> Create photographer type order    |         | `Order::generateOrders()`    |
| Photographer process the orders | **Photographer**: <br> Processes order and return results <br> Use consumable materials |  | `Photographer::processOrder()` |
| Receptionist collects the payment | **Client**: Pay for completed order <br> Receptionist: Collect payment upon completion |  | `Receptionist::collectPayment()` |
| Receptionist generates a daily revenue report | **Receptionist**: Generates daily revenuew report | | `Receptionist::generateReport()` |
| Photographer submits a report on consumed materials | **Photographer**: Submit report on consumed materials | | `Photographer::submitReport` |