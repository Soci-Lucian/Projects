#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CLIENTS 5  // maximum number of clients

void error(const char *msg) {
    perror(msg);
    exit(1);
}

struct User {
    char username[20];
    char password[20];
    char tip[20];
};

struct User Users[10];

struct Championship {
    char name[20];
    int max_players;
    char date[20];
    char hour[10];
    int type; // 1 for 1v1, 2 for free for all
    struct User joined_users[10];
    int num_joined_users;
};

struct Championship Championships[10];
int num_users = 0;
int num_championships = 0;

void *handle_client(void *arg) {
    int newsockfd = *((int *)arg);
    int a = 0; // admin
    int o = 0; // obisnuit
    int l = -1; // logged in or not

    while (1) {
        char buffer[255];
        bzero(buffer, 255);
        int n = read(newsockfd, buffer, 255);
        if (n < 0)
            error("Error on reading");

        printf("Client: %s", buffer);

        int c1 = strncmp("Register", buffer, 8);
		if(c1 == 0){ 
                    n=write(newsockfd, "Username: ", 10);
                    n = read(newsockfd, buffer, 255);
                    buffer[strcspn(buffer, "\n")] = '\0';

                    int usernameTaken=0;

                    for (int i = 0; i < num_users; i++) {
                    if (strncmp(Users[i].username, buffer, strlen(Users[i].username)) == 0) 
                        usernameTaken = 1;
                    }

                    if(usernameTaken==1) n=write(newsockfd, "Username already taken", 22);
                        else{
                    strcpy(Users[num_users].username, buffer);

                    n=write(newsockfd, "Password: ", 10); 
                    n = read(newsockfd, buffer, 255);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    strcpy(Users[num_users].password, buffer); 

                    n=write(newsockfd, "obisnuit or admin: ", 19);
                    n = read(newsockfd, buffer, 255);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    strcpy(Users[num_users].tip, buffer); 

                    n=write(newsockfd, "Inregistrat cu succes ", 21); 
                    num_users++;
                        }            
                    }

        int c2 = strncmp("Login", buffer, 5);
		if(c2 == 0){
                    l=-1;
                    n=write(newsockfd, "username: ", 10);
                    n = read(newsockfd, buffer, 255);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    for(int i=0; i<num_users; i++){                    
                        if (strncmp(Users[i].username, buffer, strlen(Users[i].username))==0){ l=i;
                                                                                               break;}
                    }
                        if(l!=-1){
                            n=write(newsockfd, "password: ", 10); 
                            n = read(newsockfd, buffer, 255);
                            buffer[strcspn(buffer, "\n")] = '\0';
                            if (strncmp(Users[l].password, buffer, strlen(Users[l].password))==0){
                                if (strncmp("admin", Users[l].tip, 5)==0){
                                    n=write(newsockfd, "admincode: ", 11); // "0000"
                                    n = read(newsockfd, buffer, 255);
                                    if (strncmp("0000", buffer, 4)==0){ 
                                    a=1;
                                    o=1;
                                    n=write(newsockfd, "Logged in", 9);
                                    }
                                    else n=write(newsockfd, "cod gresit", 10);
                                 }
                                 else{ 
                                    o=1;
                                    n=write(newsockfd, "Logged in", 9);
                                    }                             
                            }                                    
                            else n=write(newsockfd, "Parola gresita", 14);
                        } else n=write(newsockfd, "Username gresit", 16);
        }
 
        int c3 = strncmp("Info", buffer, 4);
		if(c3 == 0){
            if (a == 1 || o == 1) {
                if (num_championships == 0) {
                    n = write(newsockfd, "No championships created yet", 28);
                } else {    
                            char infoMessage[255];
                            strcpy(infoMessage, "Championships:\n");
                            for (int i = 0; i < num_championships; i++) {
                            // Append the name of each championship
                            sprintf(infoMessage + strlen(infoMessage), "%d. %s\n", i + 1, Championships[i].name);
                            }

                            n = write(newsockfd, infoMessage, strlen(infoMessage)); //list of championship names to the client

                            n = write(newsockfd, "Enter the number of the championship for details: ", 50);
                            n = read(newsockfd, buffer, 255);
                            buffer[strcspn(buffer, "\n")] = '\0';
                            int nr=atoi(buffer);
                            if(nr>0 && nr<=num_championships){
                                    nr--;
                                    char infoMessage[255];
                                    sprintf(infoMessage, "Championship %d:\n"
                                                        "Name: %s\n"
                                                        "Max Players: %d\n"
                                                        "Date: %s\n"
                                                        "Hour: %s\n"
                                                        "Type: %s\n"
                                                        "Number of Joined Players: %d\n",
                                    nr + 1, Championships[nr].name, Championships[nr].max_players, Championships[nr].date,
                                    Championships[nr].hour, Championships[nr].type == 1 ? "1v1" : "Free for all",
                                    Championships[nr].num_joined_users);

                                    strcat(infoMessage, "Joined Users: ");
                                    for (int j = 0; j < Championships[nr].num_joined_users; j++) {
                                        strcat(infoMessage, Championships[nr].joined_users[j].username);
                                        strcat(infoMessage, ", ");
                                    }
                                    strcat(infoMessage, "\n");

                                    n = write(newsockfd, infoMessage, strlen(infoMessage));
                            }
                            else n=write(newsockfd, "Championship not found", 23);
                }       
            } else n = write(newsockfd, "Not currently logged in", 23);
                
        }

        int c4 = strncmp("Create Championship", buffer, 19); // only for admin
        if (c4 == 0) {
            if (a == 1) { // Check if logged in as admin
                
                n = write(newsockfd, "Championship Name: ", 20);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                strcpy(Championships[num_championships].name, buffer);

                n = write(newsockfd, "Max Players: ", 14);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                Championships[num_championships].max_players = atoi(buffer);

                n = write(newsockfd, "Date: ", 6);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                strcpy(Championships[num_championships].date, buffer);

                n = write(newsockfd, "Hour: ", 6);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                strcpy(Championships[num_championships].hour, buffer);

                n = write(newsockfd, "Type (1 for 1v1, 2 for free for all): ", 38);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                Championships[num_championships].type = atoi(buffer);

                n = write(newsockfd, "Championship created successfully", 33);
                num_championships++;
            } else {
               if(o==1) n = write(newsockfd, "Permission denied. Admin only.", 30);
                    else n = write(newsockfd, "Not currently logged in", 23);
            }
        } 
    
        int c5 = strncmp("Join", buffer, 4);
		if (c5 == 0) {
            if(a == 1 || o == 1){
                n = write(newsockfd, "Championship Name: ", 20);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                char championship_name[20];
                strcpy(championship_name, buffer);

                int championship_index = -1; // too look for the championship
                for (int i = 0; i < num_championships; i++) {
                    if (strcmp(Championships[i].name, championship_name) == 0) {
                      championship_index = i;
                      break;
                    }
                }

                if (championship_index != -1) {
                        int user_already_participating = 0;
                    for (int i = 0; i < Championships[championship_index].num_joined_users; i++) {
                        if (strcmp(Championships[championship_index].joined_users[i].username, Users[l].username) == 0) {
                            user_already_participating = 1;
                            break;
                        }
                    }

                    if (user_already_participating == 1) {
                        n = write(newsockfd, "Already participating in this Championship", 42);
                    } else if (Championships[championship_index].num_joined_users < Championships[championship_index].max_players) {
                        Championships[championship_index].joined_users[Championships[championship_index].num_joined_users] = Users[l]; // adds user to championship database
                        Championships[championship_index].num_joined_users++;
                        n = write(newsockfd, "Joined championship successfully", 33);
                    } else {
                        n = write(newsockfd, "Championship is full. Cannot join.", 35);
                    }    
                } else {
                    n = write(newsockfd, "Championship not found", 23);
                }
            } else n = write(newsockfd, "Not currently logged in", 23);
        }

        int c6 = strncmp("Leave Championship", buffer, 18);
		if(c6 == 0 && (a==1 || o==1)){
            n = write(newsockfd, "Championship Name: ", 20);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                char championship_name[20];
                strcpy(championship_name, buffer);

                int championship_index = -1; // too look for the championship
                for (int i = 0; i < num_championships; i++) {
                    if (strcmp(Championships[i].name, championship_name) == 0) {
                      championship_index = i;
                      break;
                    }
                }

                int pos=-1;
                if (championship_index != -1) {
                        int user_already_participating = 0;
                    for (int i = 0; i < Championships[championship_index].num_joined_users; i++) {
                        if (strcmp(Championships[championship_index].joined_users[i].username, Users[l].username) == 0) {
                            user_already_participating = 1;
                            pos=i;
                            break;
                        }
                    }

                    if (user_already_participating == 0) {
                        n = write(newsockfd, "Already not participating in this Championship", 46);
                    } else{ for(int i=pos; i<Championships[championship_index].num_joined_users; i++) {
                        Championships[championship_index].joined_users[i] = Championships[championship_index].joined_users[i+1]; // fixes championship database
                    }
                        Championships[championship_index].num_joined_users--;
                        n = write(newsockfd, "Left championship successfully", 31);
                    }    
                } else {
                    n = write(newsockfd, "Championship not found", 23);
                }
        } else if (c6==0 && a==0 && o==0) n = write(newsockfd, "Not currently logged in", 23);       

		int c7 = strncmp("Change Date", buffer, 11);
		if(c7 == 0){
            if(a==1){
            n = write(newsockfd, "Championship Name: ", 20);
                n = read(newsockfd, buffer, 255);
                buffer[strcspn(buffer, "\n")] = '\0';
                char championship_name[20];
                strcpy(championship_name, buffer);

                int championship_index = -1; // too look for the championship
                for (int i = 0; i < num_championships; i++) {
                    if (strcmp(Championships[i].name, championship_name) == 0) {
                      championship_index = i;
                      break;
                    }
                }

                if(championship_index!=-1){   
                       n = write(newsockfd, "New Date: ", 10);
                       n = read(newsockfd, buffer, 255);
                       buffer[strcspn(buffer, "\n")] = '\0';
                       strcpy(Championships[championship_index].date, buffer);

                       n = write(newsockfd, "New Hour: ", 10);
                       n = read(newsockfd, buffer, 255);
                       buffer[strcspn(buffer, "\n")] = '\0';
                       strcpy(Championships[championship_index].hour, buffer);

                       n = write(newsockfd, "Date and hour changed successfully", 34);
                } else {
                    n = write(newsockfd, "Championship not found", 23);
                }
            }else if (o==0) n = write(newsockfd, "Not currently logged in", 23);
                    else  n = write(newsockfd, "Permission denied. Admin only.", 30);
        }

        int c8 = strncmp("Logout", buffer, 6);
		if(c8 == 0){
            if (a==1 || o==1){
                a=0; o=0; l=-1;
                n = write(newsockfd, "Logged out successfully", 23);
            }
                else n = write(newsockfd, "Not currently logged in", 23);
        }

        int quit = strncmp("Quit", buffer, 4);
        if (quit == 0) {
            n = write(newsockfd, "Quit", 4);
            break;
        }

        if(c1!=0 && c2!=0 && c3!=0 && c4!=0 && c5!=0 && c6!=0 && c7!=0 &&c8!=0)
        n=write(newsockfd, "comanda invalida ", 17);
          
        if(n < 0)
		   error("Error on writing"); 
    }

    close(newsockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Port No. not provided. Program terminated\n");
        exit(1);
    }

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t threads[MAX_CLIENTS];
    int thread_count = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error opening Socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Binding failed");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (newsockfd < 0)
            error("Error on Accept");

        if (thread_count < MAX_CLIENTS) {
            int *arg = (int *)malloc(sizeof(*arg));
            *arg = newsockfd;

            if (pthread_create(&threads[thread_count], NULL, handle_client, arg) != 0) {
                perror("Error creating thread");
                free(arg);  // Free allocated memory in case of thread creation failure
            } else {
                thread_count++;
            }
        } else {
            fprintf(stderr, "Maximum number of clients reached. Connection rejected.\n");
            close(newsockfd);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    close(sockfd);
    return 0;
}
