/*
server_init으로 서버측 소켓 활성화
max_client만큼 for문을 돌며
    클라이언트를 받을 수 있는 상태 만들기
    accept
    클라이언트가 앞으로 들어올 메세지들을 받을 준비 (recv)

서버는 listen까지 잘 실행되면 소켓 활성화
서버는 accept를 통해서 클라이언트의 요청을 받음. 
새로운 소켓을 만들어서 1:1연결(서버와 클라이언트를)
이때의 소켓 정보를 서버에서 sck_list vector 모두 저장

클라이언트는 connect를 통해서 내가 연결하고자 하는 서버에 연결 요청을 보냄(서버에서 새로 만든 소켓을 통해 나와 서버가 1:1 통신을 할 수 있는 상태가 됨)
send / recv 서로 통신

서버에 접속해 있는 모든 클라이언트에게 한명의 클라이언트가 보낸 메세지를 전송해야함. 
특정 클라이언트가 메세지를 보내면, 서버는 recv로 받고 서버에 연결되어 있는 모든 소켓 정보(sck_list)를 이용해 for문을 돌면서 모든 클라이언트에게 send를 하게 됨

서버가 send 하는 순간
1. accept를 할 때 (공지)
2. 서버 콘솔에서 메세지를 입력했을 때
3. 클라이언트가 보낸 메세지를 recv로 받았을 때
4. 클라이언트가 나갔을 때 (공지)

클라이언트가 send 하는 순간
1. 입장할 때 connect하고 바로 한번 실행 (내 이름을 서버에 알려주기 위해)
2. 콘솔에 메세지를 입력할 때
*/

#pragma comment(lib, "ws2_32.lib") // 명시적으로 라이브러리를 호출하는 방법 중 하나

#include <WinSock2.h> // 윈도우에서 소켓을 사용하기 위한 헤더
#include <string>
#include <iostream>
#include <thread> 
#include <vector>

// 상수선언
#define MAX_SIZE 1024
#define MAX_CLIENT 3

using std::cout;
using std::cin;
using std::endl;
using std::string;

// 구조체 정의
struct SOCKET_INFO { 
    SOCKET sck; // ctrl + 클릭 -> unsigned int 포인터
    string user; // 사람의 이름
};

std::vector<SOCKET_INFO> sck_list; // 서버에 연결된 client들을 저장할 벡터
SOCKET_INFO server_sock; // 서버 소켓의 정보를 저장할 변수
int client_count = 0; // 현재 접속된 클라이언트 수 카운트

void server_init(); // 서버용 소켓을 만드는 함수 ~listen()
void add_client(); // accept() 실행
void send_msg(const char* msg); // send() 실행
void recv_msg(int idx); // recv() 실행
void del_client(int idx); // 클라이언트와의 연결을 끊을 때

int main() {
    WSADATA wsa;

    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa); // 성공하면 0

    if (!code) { // code == 0
        server_init(); // 서버측 소켓 활성화

        // 크기가 MAX_CLIENT=3인 배열 생성. 배열에 담길 자료형은 std::thread
        // thread는 main함수가 종료되지 않도록 해줌 join 메소드
        std::thread th1[MAX_CLIENT]; 
        
        for (int i = 0; i < MAX_CLIENT; i++) {
            th1[i] = std::thread(add_client()); // 클라이언트를 받을 수 있는 상태 만들어줌 accept()
        }

        while (1) { 
            string text, msg = "";

            std::getline(cin, text);
            const char* buf = text.c_str();
            msg = server_sock.user + " : " + buf;
            send_msg(msg.c_str());
        }

        for (int i = 0; i < MAX_CLIENT; i++) { 
            th1[i].join(); 
        }

         closesocket(server_sock.sck);
    }
    else {
        cout << "프로그램 종료. (Error code : " << code << ")";
    }

    WSACleanup();

    return 0;
}

void server_init() {
    server_sock.sck = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); 
    // server_socket의 sck에 서버 소켓을 특정할 수 있는 int형 숫자를 담음
    // server_sock의 자료형은 SOCKET_INFO

    SOCKADDR_IN server_addr = {}; 
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(7777); // ~~~~:7777 정의
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 127.0.0.1:~~ 정의

    // server_sock.sck, 주소를 할당하고 싶은 socket
    // server_addr의 자료형 SOCKADDR_IN을 sockaddr* 형으로 변환
    bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr)); 
    
    // 서버를 열고싶은 socket
    listen(server_sock.sck, SOMAXCONN); 
    
    server_sock.user = "server";

    cout << "Server On" << endl;
}

void add_client() {
    // 클라이언트와 서버와 연결에 성공하면 생성되는 새로운 소켓 주소를 담을 변수
    SOCKADDR_IN addr = {};
    int addrsize = sizeof(addr);
    char buf[MAX_SIZE] = { }; // 메세지의 최대 길이 설정

    ZeroMemory(&addr, addrsize); // addr을 0으로 초기화

    // sck, user : 클라이언트의 소켓 정보를 저장
    SOCKET_INFO new_client = {};

    // connect()
    new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);

    // 클라이언트 측에서 바로 user이름을 담아서 send를 함. recv()로 받기 위해
    recv(new_client.sck, buf, MAX_SIZE, 0); // 클라이언트 connect(), send()
    new_client.user = string(buf); // char*형 buf를 string형으로 변환해서 user에 저장

    string msg = "[공지] " + new_client.user + " 님이 입장했습니다.";
    cout << msg << endl; // 서버 콘솔에 출력
    sck_list.push_back(new_client); // sck list에 추가함
    
    // 방금 생성된 new_client가 앞으로도 계속 메세지를 받을 수 있도록 대기상태로 만듬 recv()
    // client_cout를 인덱스로 사용
    std::thread th(recv_msg, client_count); 

    client_count++; // 한 명 들어올 때마다 증가
    cout << "[공지] 현재 접속자 수 : " << client_count << "명" << endl;
    send_msg(msg.c_str());

    th.join(); // thread 하나 당 join 하나 필요
}

void send_msg(const char* msg) {
    // 현재 접속중인 모든 클라이언트에게 메세지 보냄
    for (int i = 0; i < client_count; i++) {
        send(sck_list[i].sck, msg, MAX_SIZE, 0);
    }
}

void recv_msg(int idx) {
    char buf[MAX_SIZE] = { };
    string msg = "";

    while (1) { // 서버는 recv를 계속 실행해야함
        ZeroMemory(&buf, MAX_SIZE); // buf 0으로 초기화
        if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0) { // 메세지가 온 경우
            msg = sck_list[idx].user + " : " + buf;
            cout << msg << endl;
            send_msg(msg.c_str());
        }
        else { 
            msg = "[공지] " + sck_list[idx].user + " 님이 퇴장했습니다.";
            cout << msg << endl;
            send_msg(msg.c_str());
            del_client(idx); 
            return;
        }
    }
}

void del_client(int idx) {
    closesocket(sck_list[idx].sck);
    client_count--;
}
