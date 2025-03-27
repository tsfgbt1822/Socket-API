/* Client file
* Tori Fischer - TSFGBT - 18221234
* October 18, 2024
*
*Implements a SOCKET API chat room client that connects to server on port 11234
* Allows users to log in, create new accounts, send messages, and logout
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 11234            // Define the port number for the connection
#define BUFFER_SIZE 1024      // Define the buffer size for message transfer

int main() {
    // Create a socket for the client
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;

    // Set up server address parameters
    server_addr.sin_family = AF_INET;                  // Set address family to IPv4
    server_addr.sin_port = htons(PORT);                // Set the port number 
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // Set the server address 

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");   // Print error message if connection fails
        close(client_socket);
        return 1;                       // Exit with error
    }

    char buffer[BUFFER_SIZE];            // Create a buffer for messages
    printf("My chat room client. Version One.\n"); 

    // Infinite loop to keep the client running until the user logs out
    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);            // Read input
        buffer[strcspn(buffer, "\n")] = 0;          // Remove the newline character

        // Send the message to the server
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            break;
        }
        
        bzero(buffer, BUFFER_SIZE);                   // Clear the buffer

        // Receive response from the server
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Receive failed");
            break;
        } else if (bytes_received == 0) {
            printf("Server disconnected\n");
            break;
        }

        buffer[bytes_received] = '\0';                // Null-terminate the received data
        printf("> %s", buffer);                      // Print the server's response
    }

    // Close the socket and end the connection
    close(client_socket);
    return 0;
}
