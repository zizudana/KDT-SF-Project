#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h> //Winsock ������� include. WSADATA �������.��
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mysql/jdbc.h>

#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "root"; // �����ͺ��̽� �����
const string password = "1234"; // �����ͺ��̽� ���� ��й�ȣ

SOCKET client_sock;
//string my_nick;
int user_input;
string user_name;
string user_pw;

int chat_recv() {
    char buf[MAX_SIZE] = { };
    string msg;

    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            msg = buf;
            std::stringstream ss(msg);  // ���ڿ��� ��Ʈ��ȭ
            string user;
            ss >> user; // ��Ʈ���� ����, ���ڿ��� ���� �и��� ������ �Ҵ�
            if (user != user_name) cout << buf << endl; // ���� ���� �� �ƴ� ��쿡�� ����ϵ���.
        }
        else {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}

int main() {

    // MySQL Connector/C++ �ʱ�ȭ
    sql::mysql::MySQL_Driver* driver; // ���� �������� �ʾƵ� Connector/C++�� �ڵ����� ������ ��
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

    // �����ͺ��̽� ����
    con->setSchema("chattingproject");

    // db �ѱ� ������ ���� ���� 
    stmt = con->createStatement();
    stmt->execute("set names euckr");
    if (stmt) { delete stmt; stmt = nullptr; }

    WSADATA wsa;

    // Winsock�� �ʱ�ȭ�ϴ� �Լ�. MAKEWORD(2, 2)�� Winsock�� 2.2 ������ ����ϰڴٴ� �ǹ�.
    // ���࿡ �����ϸ� 0��, �����ϸ� �� �̿��� ���� ��ȯ.
    // 0�� ��ȯ�ߴٴ� ���� Winsock�� ����� �غ� �Ǿ��ٴ� �ǹ�.
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        bool is_login = false;
        while (!is_login) {
            cout << "1: �α����ϱ� 2: ȸ�������ϱ�" << endl;
            cin >> user_input;
            if (user_input == 1) { // �α����ϱ�
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
                    cout << "�α��� ����! ä�ù濡 �����մϴ�" << endl;
                }
                else {
                    cout << "�α��� ����" << endl;
                }
                
            }
            else if (user_input == 2) { // ȸ�������ϱ�
                bool id_ok = false;
                while (id_ok == false) {
                    cout << "===============================" << endl;
                    cout << "ID�� �Է��ϼ���(10�� �̳�) : ";
                    cin >> user_name;
                    pstmt = con->prepareStatement("SELECT * FROM user;");
                    result = pstmt->executeQuery();
                    bool is_join = false;
                    while (result->next()) {
                        if (user_name == result->getString(1).c_str()) {
                            cout << "�̹� �����ϴ� ID�Դϴ�." << endl;
                            is_join = true;
                            break;
                        }
                    }
                    if (is_join == false)
                        id_ok = true;
                }
                cout << "password�� �Է��ϼ���(20�� �̳�) : ";
                cin >> user_pw;
                pstmt = con->prepareStatement("INSERT INTO user(name, pw) VALUES(?,?)"); // INSERT

                pstmt->setString(1, user_name); // ù ��° �÷��� id ����
                pstmt->setString(2, user_pw); // �� ��° �÷��� pwd ����
                pstmt->execute(); // ���� ����
                cout << user_name << "��, ID�� ���������� �����Ǽ̽��ϴ�." << endl;
                cout << "===============================" << endl << endl;
            }
            else {
                cout << "1 �Ǵ� 2�� �Է����ּ���" << endl;
            }
        }

        //cout << "����� �г��� �Է� >> ";
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
        pstmt = con->prepareStatement("SELECT sender, receiver, message FROM chatting;");
        result = pstmt->executeQuery();

        while (result->next()) {
            msg += result->getString(1) + " : " + result->getString(3) + "\n";
        }
        cout << msg;

        std::thread th2(chat_recv);

        while (1) {
            string text;
            std::getline(cin, text);
            const char* buffer = text.c_str(); // string���� char* Ÿ������ ��ȯ
            send(client_sock, buffer, strlen(buffer), 0);
        }
        th2.join();
        closesocket(client_sock);
    }

    WSACleanup();
    return 0;
}