#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

int clientServ(char buff[], int ud); //UDP conmmunication with serverC for user login
void encryption(char x[]); //Username and password encryption function
bool clientLogin(int n_sock, int ud); //Process client login request
bool clientProcess(int n_sock, int ud, string target);//Process client request 
void clientServEE(char buff[], char stor[4096], int ud); //Send client query to EE server
void clientServCS(char buff[], char stor[4096], int ud); //Send client query to CS server
void multipleServ(char buff[], char que[1024], int ud); // Process multiple query

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0); //This fd is for TCP connection

    if(fd==-1)
    {
        perror("socket");
        exit(-1);
    }

    /* tcp server configuration */
    struct sockaddr_in ser;
    ser.sin_family = AF_INET;
    ser.sin_port = htons(atoi("25748")); //TCP port number
    ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t len = sizeof(struct sockaddr_in);

    /* TCP bind */
    if (bind(fd,(struct sockaddr*)&ser,len)<0)
    {
        perror("tcp bind error");
        close(fd);
        return -1;
    }

    if (listen(fd,5)<0)
    {
        perror("tcp listen error");
        close(fd);
        return -1;
    }

    int ud = socket(AF_INET, SOCK_DGRAM, 0); // This is for UDP message

    if (ud == -1)
    {
        perror("socket error!");
        exit(-1);
    }

    /* udp server configuration */
    struct sockaddr_in u_ser;
    u_ser.sin_family = AF_INET;
    int portNum = 24748; // UDP port number
    u_ser.sin_port = htons(portNum);
    u_ser.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = bind(ud, (struct sockaddr*)&u_ser, sizeof(u_ser));
    if (ret == -1)
    {
        perror("udp bind error!");
        exit(-1);
    }

    std::cout<<"The Main Server is up and running."<<endl;

    /* Process request */
    while(1) {
        struct sockaddr_in cli;

        int n_sock = accept(fd, (struct sockaddr*)&cli,&len); // TCP accept socket

        if (n_sock < 0) {
            perror("accept error!");
            continue;
        }

        bool flag = true;
        /* Proceed specific tcp request */
        while (flag) {
            flag = clientLogin(n_sock, ud);
        }
    }

    close(fd);
    return 0;
}

/* 
ud: UDP fd
n_sock: TCP acception number
Process client login request 
*/
bool clientLogin(int n_sock, int ud) {

    char buff[1024]={0};
    int ret = recv(n_sock, buff, 1023, 0); // Receive client login request
    if (ret < 0) {
        perror("recv error!");
        return false;
    } else if (ret > 0) {
        /* Split username from received message. Password and username are splited by /n */
        string target = buff;
        int index;
        int count = 0;
        for (int i = 0; i < target.length(); i++) {
            if (target[i] == ',') {
                index = i;
                break;
            }
        }
        string username;
        username = target.substr(0, index);
        std::cout<<"The main server received the authentication for "<<username<<" using TCP over port 25748."<<endl;

        /* Send message to serverC. If it is approved, call function clientProcess to process requests from client */
        int flag = clientServ(buff, ud);
        if (flag == 0) { //Login succeed
            int abc[1] = {0};
            send(n_sock, abc, sizeof(abc), 0);
            std::cout<<"The main server sent the authentication result to the client."<<endl;
            // process, not return
            bool flag1 = true;
            while (flag1) {
                flag1 = clientProcess(n_sock, ud, username);
            }
            return false;
        } else if (flag == 1) { //Username does not match
            int abc[1] = {1};
            send(n_sock, abc, sizeof(abc), 0);
            std::cout<<"The main server sent the authentication result to the client."<<endl;
        } else { //Password does not match
            int abc[1] = {-1};
            send(n_sock, abc, sizeof(abc), 0);
            std::cout<<"The main server sent the authentication result to the client."<<endl;
        }
        return true;
    } else {
        close(n_sock);
        return false;
    }
}

/* 
n_sock: TCP accept socket
ud: UDP Socket
target: client current username
If user is authenticated, we process his request. 
*/
bool clientProcess(int n_sock, int ud, string target) {
    char buff[1024] = {0}; //Store client Information
    int ret = recv(n_sock, buff, sizeof(buff)-1, 0);
    if (ret < 0) {
        cout<<"ret: "<<ret<<endl;
        perror("recv error!");
        return false;
    } else if (ret > 0) {
        char stor[4096];
        string rec = buff;

        /* If there is no " " in message, it is not multi query. Else it is normal query. */
        if (rec.find(" ") == -1) { // If space does not exist
            cout<<"The main server received from "<<target.c_str()<<" to query course "<<rec.substr(0, 5).c_str()<<" about "<<rec.substr(6).c_str()<<" using TCP over port 25748"<<endl;
            if (buff[0] == 'E') {
                clientServEE(buff, stor, ud); //send to serverEE
            } else{
                clientServCS(buff, stor, ud); //send to serverCS
            }
            send(n_sock, stor, sizeof(stor), 0); // Send query result to client.
        } else { // Multiple Query
            char que[1024]; // Store query result message
            multipleServ(buff, que, ud); //Call multi search function
            send(n_sock, que, 1024, 0); //Send query result to client.
            memset(que, 0, sizeof(que)); // clear que
        }
        cout<<"The main server sent the query information to the client."<<endl;
        memset(stor, 0, sizeof(stor)); // clear que
    } else {
        close(n_sock);
        return false;
    }
    memset(buff, 0, sizeof(buff)); // clear que
    return true;
}

/* 
buff: query request
que: final query reqult
ud: UDP socket
Process multi query
 */
void multipleServ(char buff[], char que[1024], int ud) {

    char tmp[1024]; //copy query request
    strcpy(tmp, buff);

    /* Store split course code. Slice query command with " ".*/
    const char *d = " ";
    char *x;
    x = strtok(tmp,d);
    int i = 0;
    char elem[10][1024]; // store temporary result
    while(x)
    {
        strcpy(elem[i], x);
        i++;
        x=strtok(NULL,d);
    }
    for (int j = 0; j < i; j++) {
        if (elem[j][0] == 'E') {
            clientServEE(elem[j], elem[j], ud);
        } else{
            clientServCS(elem[j], elem[j], ud);
        }
        strcat(que, elem[j]);
        strcat(que, "\n");
    }
}

/* 
buff: query message
stor: query result
Send client UDP query to CS server 
*/
void clientServCS(char buff[], char stor[4096], int ud) {

    /* ServerCS information */
    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(22748);
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);

    socklen_t len = sizeof(caddr);
    int test = getsockname(ud, (sockaddr *)&caddr, &len);

    //send data
    sendto(ud, buff, strlen(buff),0,(struct sockaddr*)&saddr,sizeof(saddr));
    cout<<"The main server sent a request to serverCS."<<endl;

    socklen_t slen = sizeof(saddr);
    //receive data
    int num = recvfrom(ud, stor, 4096, 0, (struct sockaddr*)&saddr, &slen);
    if (num < 0) {
        perror("serverM recv UDP from serverCS error!");
        exit(-1);
    }
    cout<<"The main server received the response from serverCS using UDP over port "<<ntohs(caddr.sin_port)<<"."<<endl;
    return ;
}

/* 
buff: query message
stor: query result
Send client UDP query to CS server 
*/
void clientServEE(char buff[], char stor[4096], int ud) {
    /* ServerCS information */
    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(23748);
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);

    socklen_t len = sizeof(caddr);
    int test = getsockname(ud, (sockaddr *)&caddr, &len);

    //send data
    sendto(ud, buff, strlen(buff),0,(struct sockaddr*)&saddr,sizeof(saddr));
    cout<<"The main server sent a request to serverEE."<<endl;
    socklen_t slen = sizeof(saddr);
    //receive data
    int num = recvfrom(ud, stor, 4096, 0, (struct sockaddr*)&saddr, &slen);
    if (num < 0) {
        perror("serverM recv UDP from serverEE error!");
        exit(-1);
    }
    cout<<"The main server received the response from serverEE using UDP over port "<<ntohs(caddr.sin_port)<<"."<<endl;
    return;
}


/* 
buff: client information
ud: UDP socket
UDP conmmunication with serverC */
int clientServ(char buff[], int ud) {
    //encryption algorithm
    encryption(buff);
    // serverC information
    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(21748);
    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);

    socklen_t len = sizeof(caddr);
    int test = getsockname(ud, (sockaddr *)&caddr, &len);

    bool buff1[1];
    // send data
    sendto(ud, buff, strlen(buff),0,(struct sockaddr*)&saddr,sizeof(saddr));
    std::cout<<"The main server sent an authentication request to ServerC."<<endl;

    //receive data
    int num = recvfrom(ud, buff1, 2, 0, NULL, NULL);
    if (num < 0) {
        perror("serverM recv UDP from serverC error!");
        exit(-1);
    }
    std::cout<<"The main server received the result of the authentication request from ServerC using UDP over port "<<ntohs(caddr.sin_port)<<"."<<endl;

    return buff1[0];
}

// Username and password encryption function
void encryption(char x[]) {
    //a: 97 z:122 A:65 Z:90 ASCII
    for (int i = 0; i < strlen(x); i++) {
        if (x[i] == ',') {
            continue;
        }
        if (x[i] >= 65 && x[i] <= 90) {
            char tmp;
            tmp = x[i] == 'V' ? 90:(x[i]-64+4)%26+64;
            // cout<<tmp<<endl;
            x[i] = tmp;
        } else if (x[i] >= 97 && x[i] <= 122) {
            char tmp;
            tmp = x[i] == 'v' ? 122:(x[i]-96+4)%26+96;
            // cout<<tmp<<endl;
            x[i] = tmp;
        } else if (x[i] > 47 && x[i] < 58) {
            char tmp = (int(x[i])-48+4)%10+48;
            x[i] = tmp;
        }
    }
}