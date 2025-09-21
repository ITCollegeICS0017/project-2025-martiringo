# Class - Responabilities - Collaborators


## Client
| Responsabilities | Collaborators |
| -------- | ------- |
| Place an order (photo printing or film developing) <br> Pay for completed order  | Receptionist, Order | 

## Receptionist
| Responsabilities | Collaborators |
| -------- | ------- |
| Records the clients data <br> Passes orders to the photographer and clients <br> Collect payment upon completion* <br> Generate daily revenue report  | Client, Order, Photographer, Administrator|


## Order
| Responsabilities | Collaborators |
| -------- | ------- |
| Create client type order (Client, type, completion time, urgency) <br> Create a photographer type order (Num, type, completion time and urgency) <br> Calculate price (surcharge if express) | Client, Receptionist, Photographer |



## Photographer
| Responsabilities | Collaborators |
| -------- | ------- |
| Processes order and return results <br> Use consumables materials <br> Submits report on consumed materials  | Receptionist, Order, Administrator     |



## Administrator
| Responsabilities | Collaborators |
| -------- | ------- |
| Track consumables useage <br> Replenish materials as needed <br> Review reports from receptionist and materials  | Photographer, Receptionist   |