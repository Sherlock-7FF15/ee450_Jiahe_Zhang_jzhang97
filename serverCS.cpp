#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

int readAndCheck(char buff[], char stor[3][200], char snd[]); //read and compare data

int main() {
    /* define udp socket and bind */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == -1)
    {
        perror("socket error!");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    int portNum = 22748; // port number
    addr.sin_port = htons(portNum);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind error!");
        exit(-1);
    }

    cout<<"The serverCS is up and running using UDP on port "<<portNum<<"."<<endl;

    while (1) {
        char buf[1024] = {0}; // buffer for receive
        char ipbuf[16]; //store IP address

        struct sockaddr_in caddr;
        socklen_t len = sizeof(caddr);

        //receive data from serverM
        int num = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&caddr, &len);
        if (num < 0) {
            perror("UDP recv error!");
            continue;
        }
        string code = buf; //store query message

        inet_ntop(AF_INET, &caddr.sin_addr.s_addr, ipbuf, sizeof(ipbuf));
        ntohs(caddr.sin_port);

        // if there is a comma in received data, it is a normal request. Else it is a multi query request
        if (code.find(",") == -1) {
            cout<<"The serverCS received a request from the Main Server about the course information of "<<code<<endl;
        } else {
            cout<<"The serverCS received a request from the Main Server about the "<<code.substr(6).c_str()<<" of "<<code.substr(0, 5).c_str()<<"."<<endl;
        }

        char stor[5][200]; // buffer for storing data read in txt
        char snd[1024] = {0}; // buffer for sending
        if (code.find(",") == -1) { // if it is multi query
            if (readAndCheck(buf, stor, snd) == 0) { //if we find course information
                cout<<"The course information has been found:\n"<<snd<<endl;
            } else {
                cout<<"Didn't find the course: "<<code.substr(0, 5).c_str()<<endl;
            }
        } else {
            if(readAndCheck(buf, stor, snd) == 0) { //if we find the course
            string target = code.substr(6);
            if (target == "Credit") {
                strcpy(snd, stor[1]);
            } else if (target == "Professor") {
                strcpy(snd, stor[2]);
            } else if (target == "Days") {
                strcpy(snd, stor[3]);
            } else if (target == "CourseName") {
                strcpy(snd, stor[4]);
            } else {
                string msg = "Catacory Error!";
                strcpy(snd,msg.c_str());
            }
            cout<<"The course information has been found: The "<<target.c_str()<<" of "<<code.substr(0, 5)<<" is "<<snd<<"."<<endl;
            } else {
                cout<<"Didn't find the course: "<<code.substr(0, 5).c_str()<<endl;
                string msg = "Didn't find the course: ";
                strcat(snd, msg.c_str());
                strcat(snd, code.substr(0, 5).c_str());
            }
        }
        //send data to serverM
        sendto(fd, snd, sizeof(snd), 0, (struct sockaddr*)&caddr,sizeof(caddr));
        cout<<"The ServerCS finished sending the response to the Main Server.\n"<<endl;
        memset(snd, 0, sizeof(snd)); // clear que
        memset(stor, 0, sizeof(stor)); // clear que
        memset(buf, 0, sizeof(buf)); // clear que
    }
    return 0;
}

/* 
buff: buffer for received data from serverM.
stor: buffer for splited data read in txt
snd: data for sending
 */
int readAndCheck(char buff[], char stor[5][200], char snd[]) {
    // read from txt and store the result in array
    ifstream file("./cs.txt");
    if (file.is_open()) {
        string line;
        int i = 0;

        //read data from txt
        while (getline(file, line)) {
            char *p;
            char tmp[1024]; //store read data
            string ori = buff; //ori: EE450,Credit
            if (ori.find(",") == -1) { // if it is multiple query
                if (line.find(ori) != -1) { //if match
                    line.replace(line.find(","), 1, ":");
                    strcpy(snd, line.c_str());
                    return 0;
                }
            } else if (ori.substr(0, 5) != line.substr(0, 5)) { // if doesn't match
                continue;
            } else { // if match
                //split data by ","
                const char *d = ",";
                char *x;
                strcpy(tmp, line.c_str());
                x = strtok(tmp,d);
                int i = 0;
                while(x)
                {
                    strcpy(stor[i], x);
                    i++;
                    x=strtok(NULL,d);
                }
                file.close();
                return 0;
            }
            memset(tmp, 0, sizeof(tmp)); // clear que
        }
        
    } else {
        cout<<"Unable to open file"<<endl;
    }
    file.close();
    return -1;
}