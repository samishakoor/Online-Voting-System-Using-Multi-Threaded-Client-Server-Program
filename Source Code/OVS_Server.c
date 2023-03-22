/*
        TCP_Server. This Program will will create the Server side for TCP_Socket Programming.
*/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

struct voter_info
{
        char voterName[100];
        char voterCNIC[100];
};

char *capitalizeFirstLetter(char *str)
{
        static char result[1000];
        int i, j = 0;

        for (i = 0; i < strlen(str); i++)
        {
                if (i == 0 || str[i - 1] == ' ')
                {
                        result[j] = toupper(str[i]);
                }
                else
                {
                        result[j] = str[i];
                }
                j++;
        }

        result[j] = '\0';

        return result;
}
char *make_lower(char *s)
{
        int length = strlen(s);
        for (int i = 0; i < length; i++)
        {
                s[i] = tolower(s[i]);
        }
        return s;
}

void *check_valid_voter(void *arg)
{

        struct voter_info *obj = (struct voter_info *)arg;

        char *fileName = "Voters_List.txt";
        FILE *file = fopen(fileName, "r");

        int *valid = 0;

        if (!file)
        {
                printf("\n Unable to open : %s ", fileName);
                return -1;
        }
        else
        {
                char line[500];
                while (fgets(line, sizeof(line), file))
                {

                        int line_length = strlen(line);
                        int l = strcspn(line, "\n");
                        if (line[line_length - 1] == '\n')
                        {
                                line[l - 1] = '\0';
                        }

                        char *token = strtok(line, "/");
                        char *voter_name = token;
                        token = strtok(NULL, "/");
                        char *voter_CNIC = token;

                        if (strcmp(make_lower(obj->voterName), make_lower(voter_name)) == 0 && strcmp(obj->voterCNIC, voter_CNIC) == 0)
                        {
                                valid = 1;
                        }
                }
                fclose(file);
        }
        pthread_exit((void *)valid);
}

void *check_valid_casting(void *arg)
{

        struct voter_info *obj = (struct voter_info *)arg;

        char *fileName = "Results.txt";
        FILE *fp = fopen(fileName, "a+");

        int *valid_cast = 1;

        if (!fp)
        {
                printf("\n Unable to open : %s ", fileName);
                return -1;
        }
        else
        {
                long size;
                fseek(fp, 0, SEEK_END);
                size = ftell(fp);

                if (size == 0)
                {
                        // first voter will cast vote
                        valid_cast = 1;
                }
                else
                {
                        fseek(fp, 0, SEEK_SET);
                        char line[500];
                        while (fgets(line, sizeof(line), fp))
                        {
                                char *token = strtok(line, "/");
                                char *voter_name = token;
                                token = strtok(NULL, "/");
                                char *voter_CNIC = token;
                                token = strtok(NULL, "/");

                                if (strcmp(make_lower(obj->voterName), make_lower(voter_name)) == 0 && strcmp(obj->voterCNIC, voter_CNIC) == 0)
                                {
                                        valid_cast = 0;
                                }
                        }
                }

                fclose(fp);
        }
        pthread_exit((void *)valid_cast);
}

void *check_poll_symbol(void *arg)
{
        char *poll = (char *)arg;

        char *fileName = "Candidates_List.txt";
        FILE *file = fopen(fileName, "r");

        int *valid_poll = 0;
        if (!file)
        {
                printf("\n Unable to open : %s ", fileName);
                return -1;
        }
        else
        {
                char line[500];
                char *candidate_name;
                char *poll_symbol;
                while (fgets(line, sizeof(line), file))
                {

                        int line_length = strlen(line);
                        int l = strcspn(line, "\n");
                        if (line[line_length - 1] == '\n')
                        {
                                line[l] = '\0';
                        }

                        char *token = strtok(line, "-");
                        token = strtok(NULL, "-");
                        char *poll_symbol = token;

                        if (strcmp(make_lower(poll_symbol), make_lower(poll)) == 0)
                        {
                                valid_poll = 1;
                        }
                }
                fclose(file);
        }
        pthread_exit((void *)valid_poll);
}

int main(void)
{
        int socket_desc, client_sock, client_size;
        struct sockaddr_in server_addr, client_addr; // SERVER ADDR will have all the server address
        
        //creating a buffer that receive request from the client
        char buffer[1000];
        memset(buffer, '\0', sizeof(buffer));
        // server message will be sent in response to client request
        char server_message[2000];
        memset(server_message, '\0', sizeof(server_message));

        // Creating Socket
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_desc < 0)
        {
                printf("Could Not Create Socket. Error!!!!!\n");
                return -1;
        }


        // Binding IP and Port to socket

        server_addr.sin_family = AF_INET;                     /* Address family = Internet */
        server_addr.sin_port = htons(2000);                   // Set port number, using htons function to use proper byte order */
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Set IP address to localhost */

        // BINDING FUNCTION

        if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) // Bind the address struct to the socket.  /
                                                                                         // bind() passes file descriptor, the address structure,and the length of the address structure
        {
                printf("Bind Failed. Error!!!!!\n");
                return -1;
        }

        //the infinte number of clients can cast vote until the server is alive
        while (1)
        {
                // Put the socket into Listening State
                if (listen(socket_desc, 1) < 0) // This listen() call tells the socket to listen to the incoming connections.
                // The listen() function places all incoming connection into a "backlog queue" until accept() call accepts the connection.
                {
                        printf("Listening Failed. Error!!!!!\n");
                        return -1;
                }

                printf("\nWaiting for a Voter to Cast Vote .....\n\n");

                // Accept the incoming Connections

                client_size = sizeof(client_addr);

                client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size); // heree particular client k liye new socket create kr rhaa ha

                if (client_sock < 0)
                {
                        printf("Accept Failed. Error!!!!!!\n");
                        return -1;
                }

                //  printf("Client Connected with IP: %s and Port No: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                // Receive the message from the client
                if (recv(client_sock, buffer, sizeof(buffer), 0) < 0)
                {
                        printf("Receive Failed. Error!!!!!\n");
                        return -1;
                }

                char *token = strtok(buffer, "/");
                char *v_name = token;
                token = strtok(NULL, "/");
                char *v_CNIC = token;

                char *voterName = capitalizeFirstLetter(make_lower(v_name));
                struct voter_info v;
                memset(v.voterName, '\0', sizeof(v.voterName));
                memset(v.voterCNIC, '\0', sizeof(v.voterCNIC));

                strcpy(v.voterName, voterName);
                strcpy(v.voterCNIC, v_CNIC);

                printf("Voter Name: %s\n", v.voterName);
                printf("Voter CNIC: %s\n\n", v.voterCNIC);

                int *valid_voter;
                pthread_t t_id1;
                // a thread checking whether user enter the correct name and cnic.
                if (pthread_create(&t_id1, NULL, check_valid_voter, (void *)&v) != 0)
                {
                        printf("pthread_create failed!\n");
                }

                pthread_join(t_id1, (void **)&valid_voter);

                // if user enter the correct information about him then ,then we will check that the user has already casted the vote or not.
                if (valid_voter == 1)
                {
                        int *valid_casting = 0;
                        pthread_t t_id2; 
                        // a thread checking whether the user has already casted the vote or not.
                        if (pthread_create(&t_id2, NULL, check_valid_casting, (void *)&v) != 0)
                        {
                                printf("pthread_create failed!\n");
                        }
                        pthread_join(t_id2, (void **)&valid_casting);
                        // if the user has not already casted the vote , then server will send poll symbols to client 
                        if (valid_casting == 1)
                        {
                                printf("\n%s is a Valid Voter!\n\n", voterName);

                                strcpy(server_message, "1");
                                strcat(server_message, "/");
                                strcat(server_message, voterName);
                             
                                // getting poll symbols from Candidates_List.txt
                                char *fileName = "Candidates_List.txt";
                                FILE *file = fopen(fileName, "r");

                                if (!file)
                                {
                                        printf("\n Unable to open : %s ", fileName);
                                        return -1;
                                }
                                else
                                {
                                        char line[500];
                                        char *candidate_name;
                                        char *poll_symbol;
                                        while (fgets(line, sizeof(line), file))
                                        {
                                                char *token = strtok(line, "-");
                                                token = strtok(NULL, "-");
                                                char *poll_symbol = token;
                                                strcat(server_message, "/");
                                                strcat(server_message,poll_symbol);
                                        }
                                        fclose(file);
                                }
                                // sending poll symbols to client side
                                if (send(client_sock, server_message, strlen(server_message), 0) < 0)
                                {
                                        printf("Send Failed. Error!!!!!\n");
                                        return -1;
                                }
                                memset(buffer, '\0', sizeof(buffer));

                                if (recv(client_sock, buffer, sizeof(buffer), 0) < 0)
                                {
                                        printf("Receive Failed. Error!!!!!\n");
                                        return -1;
                                }

                                char poll_symbol[50];
                                strcpy(poll_symbol, buffer);
                                int *valid_symbol;
                                pthread_t t_id3;
                                // a thread checking the validity of the poll symbol that user has selected.
                                if (pthread_create(&t_id3, NULL, check_poll_symbol, (void *)&poll_symbol) != 0)
                                {
                                        printf("pthread_create failed!\n");
                                }

                                pthread_join(t_id3, (void **)&valid_symbol);
                                // if the user chooses a valid poll symbol from the given poll symbols , then server will save the user information against his selected poll symbol
                                if (valid_symbol == 1)
                                {
                                        printf("%s choose a valid Poll Symbol!\n\n", voterName);

                                        //storing the result in Results.txt file
                                        char result[1000];
                                        strcpy(result, voterName);
                                        strcat(result, "/");
                                        strcat(result, v.voterCNIC);
                                        strcat(result, "/");
                                        char *poll = capitalizeFirstLetter(make_lower(poll_symbol));
                                        strcat(result, poll);

                                        char *fileName = "Results.txt";
                                        FILE *file = fopen(fileName, "a");

                                        if (!file)
                                        {
                                                printf("\n Unable to open : %s ", fileName);
                                                return -1;
                                        }
                                        else
                                        {
                                                fprintf(file, "%s\n", result);
                                                fclose(file);
                                        }
                                        // passing the success message to the client that he successfully casted the vote.
                                        strcpy(server_message, "You Have Successfully Casted the Vote.");
                                }
                                else
                                {
                                        printf("\n%s did not choose a valid Poll Symbol!\n\n", voterName);
                                        strcpy(server_message, "Invalid Poll Symbol !");
                                }
                        }
                        else
                        {
                                printf("\n%s is a not Valid Voter!\n\n", voterName);
                                strcpy(server_message, "You have already casted the vote !");
                        }
                }
                else
                {
                        printf("\n%s is a not Valid Voter!\n\n", voterName);
                        strcpy(server_message, "Invalid Name or CNIC !");
                }
                
                //sending responses to client given by server
                if (send(client_sock, server_message, strlen(server_message), 0) < 0)
                {
                        printf("Send Failed. Error!!!!!\n");
                        return -1;
                }

                memset(server_message, '\0', sizeof(server_message));
                memset(buffer, '\0', sizeof(buffer));

                printf("\n\n*****************************************************\n\n");

                // Closing the Socket
        }
        close(client_sock);
        close(socket_desc);
        return 0;
}
