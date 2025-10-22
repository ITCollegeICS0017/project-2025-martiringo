# Class - Responsibilities - Collaborators

## Client

| Responsibilities                                       | Collaborators               |
| ------------------------------------------------------ | --------------------------- |
| Be assigned an order, completion time and service type | Order, Receptionist, Studio |

## Receptionist

| Responsibilities                                                                                                                                                                                                                                                                                                          | Collaborators                                      |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------- |
| Creates clients based on input <br> Passes orders to the photographer and clients <br> Collect payment upon completion\* <br> Generate daily revenue report <br> Check order ID whether it is in client list <br> Get done orders that haven't been paid for <br> Get orders that have been paid for but not yet reported | Client, Order, Photographer, Administrator, Studio |

## Order

| Responsibilities                                                                                                                                                                             | Collaborators                              |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------ |
| Create client type order (Client, type, completion time, urgency) <br> Create a photographer type order (Num, type, completion time and urgency) <br> Calculate price (surcharge if express) | Client, Receptionist, Photographer, Studio |

## Photographer

| Responsibilities                                                                                                                       | Collaborators                              |
| -------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------ |
| Processes order and return when done <br> Mark orders as taken <br> Use consumable materials <br> Submits report on consumed materials | Receptionist, Order, Administrator, Studio |

## Administrator

| Responsibilities                                              | Collaborators                      |
| ------------------------------------------------------------- | ---------------------------------- |
| Review reports from receptionist and materials for its studio | Photographer, Receptionist, Studio |

## Studio

| Responsibilities                                                                                         | Collaborators                             |
| -------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| Have unique photographer, receptionist and client list for itself <br> Show which services are supported | Photographer, Receptionist, Client, Order |
