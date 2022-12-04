#include <iostream>
#include<arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>
#include<stdio.h>
using namespace std;
class inputType {
    public:
        string username;
        string password;
};

string passUserProcess(string password, string username); // Convert username and password
void courseOpera(int fd, inputType input); // Course operation after login

int main() {
    // define socket and connection
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    struct sockaddr_in saddr; //get server port number
    struct sockaddr_in caddr; //get client port number

    inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr.s_addr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(25748);
    int ret = connect(fd, (struct sockaddr *)&saddr, sizeof(saddr));

    socklen_t len = sizeof(caddr);
    int test = getsockname(fd, (sockaddr *)&caddr, &len);

    if(ret==-1)
    {
        perror("connet");
        exit(-1);
    } else {
        std::cout<<"The Client is up and running"<<endl;
    }
    
    int inputNum = 0;
    while(1)
    {
        
        if (inputNum >= 3) {
            std::cout<<"Authentication Failed for 3 attempts. Client will shut down."<<endl;
            close(fd);
            break;
        }
        int loginRecv[1];
        // user input their login information
        inputType input;

        std::cout<<"Please enter the username: ";
        getline(cin, input.username);
        std::cout<<"Please enter the password: ";
        getline(cin, input.password);
        
        // if (input.username.length() < 5) {
        //     std::cout<<"The length of username and password cannot less than 5 and more than 50 characters!"<<endl;
        //     inputNum++;
        //     continue;
        // }

        // Process username and password to send.
        string tar = passUserProcess(input.username, input.password);

        string *snd;
        snd = &tar;
        send(fd, snd->c_str(), snd->length(), 0);
        std::cout<<input.username<<" sent a authentication request to the main server."<<endl;
        int re = recv(fd, loginRecv, sizeof(loginRecv), 0);
        if (re < 0) {
            perror("receive error!");
            exit(1);
        }
        // cout<<"receive succeed!\n"<<endl;
        if(loginRecv[0] == 0) {
            // get port number.
            std::cout<<input.username<<" received the result of authentication using TCP over port "<<ntohs(caddr.sin_port)<<". Authentication is successful."<<endl;
            courseOpera(fd, input);
            break;
        } else {
            inputNum++;
            if (loginRecv[0] == -1) {
                std::cout<<input.username<<" received the result of authentication using TCP over port "<<ntohs(caddr.sin_port)<<". Authentication failed: Username does not exist.\nAttempts remaining: "<<3-inputNum<<endl;
            } else {
                std::cout<<input.username<<" received the result of authentication using TCP over port "<<ntohs(caddr.sin_port)<<". Authentication failed: Password does not match.\nAttempts remaining: "<<3-inputNum<<endl;
            }
            
        }
    }
    close(fd);
    return 0;
}

/* 
fd: TCP socket
input: username and password
Operation after login
 */
void courseOpera(int fd, inputType input) {
    struct caddr;
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int test = getsockname(fd, (sockaddr *)&caddr, &len);

    int count = 0;
    while(1) {
        if (count != 0) {
            std::cout<<"-----Start a new request-----"<<endl;
        }
        count++;
        std::cout<<"Please enter the course code to query: ";
        string code;
        getline(cin, code);

        string catal; //course catagory
        char que[200] = {0}; //send format: EE450,4
        if (code.find(" ") != -1) { // if there are multiple course codes
            strcpy(que, code.c_str());
        } else {
            std::cout<<"Please enter the category(Credit/Professor/Days/CourseName) to query: ";
            
            getline(cin, catal);
            strcat(que, code.c_str());
            strcat(que, ",");
            strcat(que, catal.c_str());
        }
        
        send(fd, que, strlen(que), 0);
        if (code.find(" ") == -1) {
            std::cout<<input.username<<" sent a request to the main server."<<endl;
        } else {
            std::cout<<input.username<<" sent a request with multiple CourseCode to the main server."<<endl;
        }
        char buff[10240] = {0}; //receive buffer
        int re = recv(fd, buff, sizeof(buff), 0);
        if (re < 0) {
            perror("receive error!");
            exit(1);
        }
        cout<<"The client received the response from the Main server using TCP over port "<<ntohs(caddr.sin_port)<<"."<<endl;
        if (code.find(" ") != -1) { //if the result is multi query
            cout<<"CourseCode: Credit, Professor, Days, CourseName"<<endl;
            std::cout<<buff<<endl;
        } else {
            string stra = buff;
            if (stra.substr(0, 6) == "Didn't") { //if didn't find the course code
                std::cout<<buff<<endl;
            } else {
                std::cout<<"The "<<catal<<" of "<<code<<" is "<<buff<<endl;
            }
            
        }
        memset(que, 0, sizeof(que)); // clear que
    }
}

/* convert the username and password */
string passUserProcess(string username, string password) {
    string ret = username+',';
    ret = ret+password;
    return ret;
}