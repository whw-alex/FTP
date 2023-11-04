# UDP Documentation

 How to write a chat program (two **clients** chat with each other) with UDP?

Answer: 

As for the UDP server, it always listens for messages coming from the clients, keeps track of their addresses, and assigns a chat partner to the new clients. Once the connection is formed, the server receives a message from one client and then sends it to its corresponding chat partner by looking up the chat partner's address. If a client sends a goodbye message, the server will remove it from the clients' list and end the connection.

As for the clients, they can start a separate thread to receive messages coming from the server. At meanwhile, once they have input messages, they will send them to the server.