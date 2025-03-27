/*Server file
* Tori Fischer 
* October 18, 2024
* 
* Implements a SOCKET API chat room server that handles a single client connection
* Allows users to log in, create new accounts, send messages, and logout
* Also interacts with text file that contains user credentials
* User data is stored in "users.txt", and the server listens on port 11234
* IP addrress is 127.0.0.1
*/



#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 11234
#define MAX_USERS 100
#define BUFFER_SIZE 1024

// Structure to store user information
typedef struct {
    char username[32];
    char password[8];
} User;

// Array to store the list of users
User users[MAX_USERS];
// Count of the current number of users
int user_count = 0;

// Function to load users from "users.txt" file
void load_users() {
    //open file and check if it has users already
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        printf("No users.txt found. Starting with no users.\n");
        return;
    }

    // Read each user from the file and add to the users array
    while (fscanf(file, "%s %s", users[user_count].username, users[user_count].password) == 2) {
        user_count++;
    }
    //close the file
    fclose(file);
}

// Function to save users to "users.txt" file
void save_users() {
    FILE* file = fopen("users.txt", "w");
    // Write all users to the file
    for (int i = 0; i < user_count; i++) {
        fprintf(file, "%s %s\n", users[i].username, users[i].password);
    }
    fclose(file);
}

// Function to find a user by username and password
int find_user(const char* username, const char* password) {
    // Iterate through users to check if there is a match
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1; // User found
        }
    }
    return 0; // User not found
}

//function to check if a username is being repeated
int find_user_create(const char* username) {
    // Iterate through users to check if there is a match
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 1; // User found
        }
    }
    return 0; // User not found
}

// Function to interact with client
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    char username[32], password[8], command[BUFFER_SIZE];
    int logged_in = 0; //variable that tracks if user is logged in
    

    while (1) {

        // Clear buffer and receive new input from client
        bzero(buffer, BUFFER_SIZE);
        int size_received = recv(client_socket, buffer, BUFFER_SIZE, 0);

        //if it is empty, go back to start for new prompt
        if (size_received <= 0) {
            send(client_socket, "Error: empty command\n", strlen("Error: empty command\n"), 0);
            break;
        }

        // Get command from the received buffer
        sscanf(buffer, "%s", command);

        // Handle "login" command
        if (strcmp(command, "login") == 0) { 
            if (logged_in)
            {
                send(client_socket, "Already logged in. Please log out to sign in to another profile\n", strlen("Already logged in. Please log out to sign in to another profile\n"), 0);
                //printf("Failed login\n");
            }
            else
            {
                sscanf(buffer, "%*s %s %s", username, password);  // read username and password
                //if credentials match, login and send confirmation
                if (find_user(username, password)) {
                    logged_in = 1;
                    send(client_socket, "login confirmed\n", strlen("login confirmed\n"), 0);
                    printf("%s login\n", username);
                }
                else {
                    //clear username and password fields and send error message
                    bzero(username, sizeof(username));
                    bzero(password, sizeof(password));
                    //print to client
                    send(client_socket, "Denied. User name or password incorrect.\n", strlen("Denied. User name or password incorrect.\n"), 0);
                    //printf("Failed login\n");
                }
            }
        }
        // Handle "newuser" command
        else if (strcmp(command, "newuser") == 0) {
            sscanf(buffer, "%*s %s %s", username, password);  // Read username and password
            if (find_user_create(username)) {
                //if username already exists, send error
                send(client_socket, "Denied. User account already exists.\n", strlen("Denied. User account already exists.\n"), 0);
                //printf("failed account creation\n");
            }
            else {
                // Create a new user and save it
                strcpy(users[user_count].username, username);
                strcpy(users[user_count].password, password);
                user_count++;
                save_users();
                //print to client
                send(client_socket, "New user account created. Please login.\n", strlen("New user account created. Please login.\n"), 0);
                printf("New user account created\n"); //print to server
            }
        }
        // Handle "send" command 
        else if (strcmp(command, "send") == 0) {
            if (logged_in)
            {
                char* message = buffer + 5;  // read message into string starting after the "send " command

                if (message != NULL && strlen(message) > 0) {
                    snprintf(buffer, BUFFER_SIZE, "%s: %s\n", username, message);  // Format the message
                    send(client_socket, buffer, strlen(buffer), 0);  // Send the formatted message back
                    printf("%s", buffer); //print to server
                }
                else {
                    //if message is empty, send error message
                    send(client_socket, "Message cannot be empty.\n", strlen("Message cannot be empty.\n"), 0);
                }
            }
            else {
                //if not logged in, send error message
                send(client_socket, "Denied. Please login first.\n", strlen("Denied. Please login first.\n"), 0);
            }
        }
        // Handle "logout" command
        else if (strcmp(command, "logout") == 0) {
            //format leave command
            char leave_message[BUFFER_SIZE];
            snprintf(leave_message, BUFFER_SIZE, "%s left.\n", username);
            //send leave message to client
            send(client_socket, leave_message, strlen(leave_message), 0);
            printf("%s logout\n", username); //print to server
            logged_in = 0;
        }
        // Handle invalid command
        else {
            //send error message to client if it does not match any of the valid commands
            send(client_socket, "Invalid command\n", strlen("Invalid command\n"), 0);
            //printf("Invalid command\n");
        }
    }

    // Close the client socket after handling communication
    close(client_socket);
}

int main() {
    // Load existing users from the file
    load_users();

    // Create a socket for the server
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    // Set server address information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified port and address
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    // Listen for incoming client connections
    listen(server_socket, 5);

    //printf("Server is running on port %d\n", PORT);
    printf("My chat room client. Version One.\n\n");

    // Accept and handle client connections in a loop
    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        // Handle the connected client
        handle_client(client_socket);
    }

    // Close the server socket
    close(server_socket);
    return 0;
}
