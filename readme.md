# Actor Framework

To correctly implement the actor pattern, we have implemented a framework entirely separate from our simulation. This code is located on a different folder to our main program, having a custom build objective on our *Makefile*.

This approach further increases the separation between our simulation and the framework, a separate example using our framework is provided on another folder, to prove its separation further.

Our framework supports multiple actors on each <span acronym-label="MPI" acronym-form="singular+abbrv">MPI</span> process, which allows running our simulation in any desired number of processes. However, a modern version of a compiler is required. On Cirrus we can set-up the environment with `module load gcc/8.2.0 mpt/2.18`.

C++ simplifies the use and implementation of our framework, allowing us to use high-level data structures like dynamic vectors and maps, which facilitates our performance. In order to simplify some map iterations, C++ 17 is used. Fortunately, most modern compilers already support this modern version.

## Design

Our framework is implemented on two main classes, `Actor` and `ExecutionUnit`. Two additional minor classes, `Message` and `Logger` were also created.

The class `Logger` handles the console output, a custom print function is provided to print text indicating the actor id and execution unit responsible for the message. Several log levels are provided, allowing to show internal messages if necessary.

The class message is responsible for storing the sent data. In order to identify the participating actors, its ids were added. An additional field is also used to store the type of message. We internally employ the message tag to identify the message function (create actor, communication between actors, etc.), the message type identifies the type of message (squirrel hop, new month, etc.)

We have a maximum message size for each message. However, we are still capable of sending shorter messages if desired. Finally, one must note that the sending and receiving of messages is done by the class `ExecutionUnit`.

Each rank must create its own execution unit. This class handles all our communications and most of our framework logic. It is responsible for creating and distributing the different actors, handling communications, killing and updating actors or finishing the communications.

The master will have additional data in order to know the load balance of each execution unit, allowing to distribute new process accordingly between the different processes. This causes that each time an execution unit creates a new actor, it must first send a message to the master.

The parameters used to create a new actor must be received as a message, which allows a smooth transmission between execution units. Finally, each time the master creates a new actor, it transmits its actor id and the rank of its execution unit to all the other units.

This additional communication is necessary in order to allow direct communications between actors across different execution units without always using the master. Finally, we have also implemented a custom logic for messages received on an incorrect unit, allowing its redirection to the correct execution unit, directly or with an extra redirection using the master.

Our framework uses asynchronous messages to handle the asynchronous nature of communications across actors. In order to avoid possible copy problems, each actor will have a buffer with many messages waiting to be sent. On a similar manner, internal messages will wait to be completed.

Our execution unit is continuously updated, on each update we first remove all finished actors with all its message sent. Secondly, we update all our active actors, and finally, we process any new messages.

Note that actors can be paused, something beneficial for actors that must not be updated while waiting to receive an answer, and those execution units without active actors, were optimised to wait until they receive a message.

## Framework Use

In order to use this framework, we need first to create an execution unit on each process. We then need to register all actors used by our program in all execution units with the same order. Each registered actor will have an id used to identify the type when we create a new actor.

Each actor needs to be created in only one execution unit to avoid duplication. It is possible to create one actor on any execution unit. However, a known limitation of our framework is that we can only know the id of the actor created on the master.

This inconvenient is not as significant as it seems since we only need the actor id to send messages to an actor. Is acceptable to solve it by creating all actor receiving messages from the master at the beginning of our simulation.

To create a new actor first, we need to create the message responsible for that. We need to set-up the message data to transmit of actor parameter, something quickly done with a `reinterpeter_cast`. We also need to set the new actor type to the correct id. Finally, we can create a new actor using the execution unit.

Once all actors are created, we must tell the execution unit to enter its main loop so that it can start updating all the new actors.

In order to create a new actor, we need first to have declared its corresponding class. C++ allows us to simplify this process, thanks to inheritance. A new actor type only needs to inherit from the class `Actor` and override its pure virtual functions, which makes its creation simpler than alternatives like function pointers.

The overridden functions specify the required functionality. For example, we need to specify how to process a message, update an actor and create a new actor of this type from the buffer of a message. Additional useful functions are already provided, like how to create a new message. One must remember that we are still responsible for pausing and finishing the desired actors.