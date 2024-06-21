---
author: "Nen"
date: "2024-06-21"
title: FLHook
---

# I.D. 

FLHook uses an id system with strong type aliases (ie they are unsigned integers but are unable to be implicitly converted to and from), these Ids are then used like typical objects with member functions in order to manipulate
the object it represents. 

Each of the various types mirrors Freelancers and FLServer's internal ids it uses for game objects.

The main ones you would want to know about are these 5

- ClientId, represents a "client" or person connected to the server, most functions related to player manipulation that are online are accessed via this.
- BaseId, which represents a base in the game, do note that Freelancer distinguishes between the physical base and the abstract concept of a base. Although they will always be linked
- ShipId, represents a specific instance of a ship, both players and NPCs have this. Manipulations related to the ship itself are accessed via this
- AccountId, This one is unique from the others as it is purely a FLHook datatype and is not related to anything Freelancer itself does. It is a handle for an account on the Mongo Database FLHook uses for accounts, characters, and plugin data. 
- SystemId, represents an in game system and has various methods to poll bases, solars, or ships/players that exist within the system.


The rest available are.

- EquipmentId
- GroupId
- RepGroupId
- ObjectId
- RepId

Most plugins though mainly use ClientId and AccountId.

# Using the Ids

Using them is rather simple, the functionality is basically just member functions on an object.

```
clientId.SetCash(3000); // calling a function that manipulates the active character of the client.

ShipId shipId = clientId.GetShipId();

```

A note is there are easy functions to "convert" between ids in a logical manner, in this example getShipId will get the ship id of the ship the player is currently flying. 