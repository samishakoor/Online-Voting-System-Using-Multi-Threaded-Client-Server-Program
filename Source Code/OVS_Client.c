/*
        TCP_Client. This Program will implement the Client Side for TCP_Socket Programming.
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr

int main(void)
{
        system("clear");
        int socket_desc;
        struct sockaddr_in server_addr;
        
        char server_message[2000];
        memset(server_message, '\0', sizeof(server_message));

        char buffer[1000];
        memset(buffer, '\0', sizeof(buffer));

        // Creating Socket

        socket_desc = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_desc < 0)
        {
                //               printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }

        // printf("Socket Created\n");

        // Specifying the IP and Port of the server to connect

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(2000);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Now connecting to the server accept() using connect() from client side

        if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
                printf("Connection Failed. Error!!!!!");
                return -1;
        }

        //     printf("Connected\n");

        printf("\n---------------------- Welcome to Online Voting System ----------------------\n\n");


        // asking the user about his information
        char name[1000];
        char cnic[1000];
        printf("Enter Name: ");
        gets(name);

        strcpy(buffer, name);
        strcat(buffer, "/");

        printf("Enter CNIC: ");
        gets(cnic);
        strcat(buffer, cnic);

        // Send the message to Server
        if (send(socket_desc, buffer, strlen(buffer), 0) < 0)
        {
                printf("Send Failed. Error!!!!\n");
                return -1;
        }

        // Receive the message from server whether user has given correct information or not
        if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0)
        {
                printf("Receive Failed. Error!!!!!\n");
                return -1;
        }

        char *received = server_message;

        char *token = strtok(received, "/");
        char *valid_casting = token;
        token = strtok(NULL, "/");
        char *voter_name = token;
        token = strtok(NULL, "/");
        char *pollSymbols=token;
        
        // client can only choose the poll symbol if he entered the correct information about him
        if (strcmp(valid_casting, "1") == 0)
        {

                printf("\n                    Welcome %s\n\n", voter_name);

                printf("Choose a Poll Symbol: \n");

                //Printing Poll Symbols On client-side terminal
                while (pollSymbols != NULL)
                {
                        printf("%s", pollSymbols);
                        pollSymbols = strtok(NULL, "/");
                }

                char p_symbol[50];
                printf("\nEnter Poll Symbol: ");
                gets(p_symbol);

                memset(buffer, '\0', sizeof(buffer));
                strcpy(buffer, p_symbol);

                //sending poll symbol to server after the user chooses it
                if (send(socket_desc, buffer, strlen(buffer), 0) < 0)
                {
                        printf("Send Failed. Error!!!!\n");
                        return -1;
                }

                memset(server_message, '\0', sizeof(server_message));

                //receiving message from server whether the selected poll symbol is valid or not
                if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0)
                {
                        printf("Receive Failed. Error!!!!!\n");
                        return -1;
                }

                printf("\nServer Message: %s\n\n", server_message);
        }
        else
        {
                printf("\nServer Message: %s\n\n", server_message);
        }

        memset(server_message, '\0', sizeof(server_message));
        memset(buffer, '\0', sizeof(buffer));

        // Closing the Socket

        close(socket_desc);

        return 0;
}
