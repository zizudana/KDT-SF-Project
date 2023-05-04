#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.ㄴ
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mysql/jdbc.h>
#include <ctime>



#define MAX_SIZE 1024



using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "cho337910!@@"; // 데이터베이스 접속 비밀번호

SOCKET client_sock;
//string my_nick;
int user_input;
string user_name;
string user_pw;
string new_pw;
char delete_count;

int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            msg = buf;
            std::stringstream ss(msg);  // 문자열을 스트림화
            string user;
            ss >> user; // 스트림을 통해, 문자열을 공백 분리해 변수에 할당
            if (user != user_name) cout << buf << endl; // 내가 보낸 게 아닐 경우에만 출력하도록.
        }
        else {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}

int main() {

    // MySQL Connector/C++ 초기화
    sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
    sql::Connection* con;
    sql::Statement* stmt;
    sql::PreparedStatement* pstmt;
    sql::ResultSet* result;



    try {
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException& e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        exit(1);
    }

    // 데이터베이스 선택
    con->setSchema("chattingproject");

    // db 한글 저장을 위한 셋팅 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    WSADATA wsa;

    // Winsock를 초기화하는 함수. MAKEWORD(2, 2)는 Winsock의 2.2 버전을 사용하겠다는 의미.
    // 실행에 성공하면 0을, 실패하면 그 이외의 값을 반환.
    // 0을 반환했다는 것은 Winsock을 사용할 준비가 되었다는 의미.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        bool is_login = false;
        while (!is_login) {
            cout << "----------------------------" << endl;
            cout << "*      CHATTING PROGRAM    *" << endl;
            cout << "*                          *" << endl;
            cout << "* 1: 로그인    2: 회원가입 *" << endl;
            cout << "* 3: 암호변경  4: 회원탈퇴 *" << endl;
            cout << "----------------------------" << endl;
            cin >> user_input;
            if (user_input == 1) { // 로그인하기
                cout << "name : ";
                cin >> user_name;
                cout << "password : ";
                cin >> user_pw;
                //select  
                pstmt = con->prepareStatement("SELECT * FROM user;");
                result = pstmt->executeQuery();

                while (result->next()) {
                    if (user_name == result->getString(1) && user_pw == result->getString(2)) {
                        is_login = true;
                        break;
                    }
                }
                if (is_login) {
                    cout << "로그인 성공! 채팅방에 입장합니다" << endl;
                }
                else {
                    cout << "로그인 실패" << endl;
                }

            }
            else if (user_input == 2) { // 회원가입하기
                bool id_ok = false;
                while (id_ok == false) {
                    cout << "===============================" << endl;
                    cout << "ID를 입력하세요(10자 이내) : ";
                    cin >> user_name;
                    pstmt = con->prepareStatement("SELECT * FROM user;");
                    result = pstmt->executeQuery();
                    bool is_join = false;
                    while (result->next()) {
                        if (user_name == result->getString(1).c_str()) {
                            cout << "이미 존재하는 ID입니다." << endl;
                            is_join = true;
                            break;
                        }
                    }
                    if (is_join == false)
                        id_ok = true;
                }
                cout << "password를 입력하세요(20자 이내) : ";
                cin >> user_pw;
                pstmt = con->prepareStatement("INSERT INTO user(name, pw) VALUES(?,?)"); // INSERT

                pstmt->setString(1, user_name); // 첫 번째 컬럼에 id 삽입
                pstmt->setString(2, user_pw); // 두 번째 컬럼에 pwd 삽입
                pstmt->execute(); // 쿼리 실행
                cout << user_name << "님, ID가 정상적으로 생성되셨습니다." << endl;
                cout << "===============================" << endl << endl;
            }
            else if (user_input == 3) { //암호 변경
                bool id_ok = false;
                while (id_ok == false) {
                    cout << "ID를 입력하세요(10자 이내) : ";
                    cin >> user_name;
                    pstmt = con->prepareStatement("SELECT * FROM user;");
                    result = pstmt->executeQuery();
                    while (result->next()) {
                        if (user_name == result->getString(1).c_str()) {
                            cout << "변경할 암호를 입력하세요(20자 이내) : ";
                            cin >> new_pw;
                            pstmt = con->prepareStatement("UPDATE user SET pw = ? WHERE name = ?");
                            pstmt->setString(1, new_pw);
                            pstmt->setString(2, user_name);
                            pstmt->executeQuery();
                            printf("암호가 정상적으로 변경되었습니다.\n");
                            id_ok = true;
                            break;
                        }
                    }
                    if (id_ok == false) {
                        cout << "올바른 ID가 아닙니다.(20자이내) : " << endl;
                    }
                }
            }
            else if (user_input == 4) { // 회원탈퇴
                cout << "ID를 입력하세요 : ";
                cin >> user_name;
                cout << "password를 입력하세요 : ";
                cin >> user_pw;
                //select  
                pstmt = con->prepareStatement("SELECT * FROM user;");
                result = pstmt->executeQuery();
                while (result->next()) {
                    if (user_name == result->getString(1) && user_pw == result->getString(2)) {
                        is_login = true;
                        break;
                    }
                }
                if (is_login) {
                    cout << "정말 탈퇴하시겠습니까? (Y / N)";
                    cin >> delete_count;
                    if (delete_count == 'Y' || delete_count == 'y') {
                        pstmt = con->prepareStatement("DELETE FROM user WHERE name = ?");
                        pstmt->setString(1, user_name);
                        result = pstmt->executeQuery();
                        printf("정상적으로 탈퇴되셨습니다.\n");
                        is_login = false;
                    }
                    else if (delete_count == 'N' || delete_count == 'n') {
                        is_login = false;

                    }
                }
                else {
                    cout << "정보가 올바르지 못합니다." << endl;
                }
            }

        }
        //cout << "사용할 닉네임 입력 >> ";
        //cin >> my_nick;

        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN client_addr = {};
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(7777);
        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);

        while (1) {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) {
                cout << "Server Connect" << endl;
                send(client_sock, user_name.c_str(), user_name.length(), 0);
                break;
            }
            cout << "Connecting..." << endl;
        }

        string msg = "";
       //select  
        pstmt = con->prepareStatement("SELECT sender, receiver, message, time FROM chatting;");
        result = pstmt->executeQuery();

        while (result->next()) {
            string receiver = result->getString(2);
            if (receiver.empty() || receiver.compare(user_name) == 0) {
                msg += result->getString(1) + " : " + result->getString(3) + " [" + result->getString(4) + "]\n";
            }
        }
        cout << msg << endl;
        

        std::thread th2(chat_recv);

        while (1) {
            string text;
            std::getline(cin, text);
            const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
            send(client_sock, buffer, strlen(buffer), 0);
        }
        th2.join();
        closesocket(client_sock);
    }


    WSACleanup();
    return 0;
}