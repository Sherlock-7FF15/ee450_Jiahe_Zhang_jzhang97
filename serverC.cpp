#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

int readAndCheck(char buff[]);

int main() {
    // define udp socket
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == -1)
    {
        perror("socket error!");
        exit(-1);
    }

    /* server congiguration */
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(21748);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind error!");
        exit(-1);
    }

    std::cout<<"The ServerC is up and running using UDP on port "<<ntohs(addr.sin_port)<<endl;

    while (1) {
        char buf[1024] = {0};
        char ipbuf[16]; //store IP

        struct sockaddr_in caddr;
        socklen_t len = sizeof(caddr);

        int num = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&caddr, &len);
        if (num < 0) {
            perror("UDP recv error!");
            continue;
        }
        std::cout<<"The ServerC received an authentication request from the Main Server."<<endl;

        inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ipbuf, sizeof(ipbuf));
        ntohs(caddr.sin_port);
        int snd[1];
        snd[0] = readAndCheck(buf); // read data from txt and compare it with message
        sendto(fd, snd, 1, 0,(struct sockaddr*)&caddr,sizeof(caddr));
        std::cout<<"The ServerC finished sending the response to the Main Server."<<endl;
        memset(buf, 0, sizeof(buf)); // clear buf
        memset(snd, 0, sizeof(snd)); // clear snd
        memset(ipbuf, 0, sizeof(ipbuf)); // clear ipbuf
    }
    close(fd);
    return 0;
}

/* 
buff: message from serverM
 */
int readAndCheck(char buff[]) {
    string username, password, target = buff;
    int count = 0, index = 0;
    for (int i = 0; i < target.length(); i++) {
        if (target[i] == ',') {
            index = i;
            break;
        }
    }
    username = target.substr(0, index);
    password = target.substr(index+1);

    /* read from txt and store the result in array */
    ifstream file("./cred.txt");
    if (file.is_open()) {
        string line;
        while (getline(file, line)) { //read from cred.txt line by line
            string c_username = line.substr(0, line.find(","));
            string c_password = line.substr(line.find(",")+1);

            if (int(c_password[c_password.length()-1]) == 13) { // If the ascii of last char in line is 13, remover it
                c_password.erase(c_password.length()-1);
            }
            if (c_password[c_password.length()-1] == '\n') { // If the ascii of last char in line is \n, remover it
                c_password.erase(c_password.length()-1);
            }
            if (c_password[c_password.length()-1] == '\r') { // If the ascii of last char in line is \r, remover it
                c_password.erase(c_password.length()-1);
            }

            // if find match one, return 0; return -1 if username doesn't match, return 1 if password doesn't match.
            if (username == c_username && password == c_password) {
                file.close();
                return 0;
            } else if (username == c_username && password != c_password) {
                file.close();
                return 1;
            } else if (username != c_username && password == c_password) {
                file.close();
                return -1;
            }
        }
        file.close();
    } else {
        cout<<"Unable to open file"<<endl;
    }
    return -1;
}