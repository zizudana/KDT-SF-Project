#pragma comment(lib, "ws2_32.lib")
#include "client.h"

int main() {
    // 콘솔 채팅방 크기, 이름 설정
    system("mode con: cols=50 lines=40 | title CodingOnTalk");

    // 데이터 베이스 연결
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
            cout << "--------------------------------------------------" << endl;
            cout << "*                CHATTING PROGRAM                *" << endl;
            cout << "*           1: 로그인      2: 회원가입           *" << endl;
            cout << "*           3: 암호변경    4: 회원탈퇴           *" << endl;
            cout << "--------------------------------------------------" << endl;
            char user_input;
            cin >> user_input;
            if (user_input == '1') // 로그인하기
                is_login = login();
            else if (user_input == '2') // 회원가입하기
                join();
            else if (user_input == '3') //암호 변경
                change_pw();
            else if (user_input == '4') // 회원탈퇴
                delete_user();
            else {
                cout << "1 ~4 값을 입력해주세요" << endl << endl;
                continue;
            }
        }

        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN client_addr = {};
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(7777);
        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);

        while (1) {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) {
                cout << "Server Connect" << endl;
                string title = "title CodingOnTalk - " + user_name;
                Sleep(1000);
                system("cls");
                system("color 60");
                system(title.c_str());
                send(client_sock, user_name.c_str(), user_name.length(), 0);
                break;
            }
            cout << "Connecting..." << endl;
        }

        // 이전 채팅기록 보여주기
        show_before_chat();

        std::thread th2(chat_recv);

        while (1) {
            string text;
            std::getline(cin, text);
            if (text == "#EXIT") {
                cout << "종료" << endl;
                //th2.join();
                closesocket(client_sock);
                WSACleanup();
                exit(0);
            }
            else if (text == "#GAME") {
                game();
            }
            const char* buffer = text.c_str(); // string형을 char* 타입으로 변환
            send(client_sock, buffer, strlen(buffer), 0);
        }

        th2.join();
        closesocket(client_sock);
    }

    WSACleanup();
    // MySQL Connector/C++ 정리
    delete con;
    delete stmt;
    delete pstmt;
    delete result;

    return 0;
}

void textcolor(int foreground, int background)
{
    int color = foreground + background * 16;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

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
            if (user == "server" || user == "[공지]") {
                textcolor(4,6); // 서버에서 온 메세지는 빨간색으로 출력
                cout << buf << endl;
            }
            else if (user == "[DM]") {
                textcolor(1,6); // DM은 파란색으로 출력
                cout << buf << endl;
            }
            else {
                textcolor(0, 6);
                if (user != user_name)
                    cout << buf << endl; // 내가 보낸 게 아닐 경우에만 출력하도록.
            }
            textcolor(0, 6);
        }
        else {
            cout << "Server Off" << endl;
            return -1;
        }
    }
}

bool login() {
    cout << "name : ";
    cin >> user_name;
    cout << "password : ";
    cin >> user_pw;
    //select  
    pstmt = con->prepareStatement("SELECT * FROM user;");
    result = pstmt->executeQuery();
    bool is_login = false;
    while (result->next()) {
        if (user_name == result->getString(1) && user_pw == result->getString(2)) {
            is_login = true;
            break;
        }
    }
    if (is_login) {
        cout << "로그인 성공! 채팅방에 입장합니다" << endl << endl;
    }
    else {
        cout << "로그인 실패" << endl << endl;
    }
    return is_login;
}

bool join() {
    bool id_ok = false;
    bool is_join = false;
    bool pw_ok = false;
    while (id_ok == false) {        
        cout << "--------------------------------------------------" << endl;
        cout << "ID를 입력하세요(10자 이내) : ";
        cin >> user_name;        
            if (user_name.length() > 10) {
                cout << "10자 이내로 입력해주세요." << endl << endl;
                is_join = true;               
            }
            else {
                is_join == false;
                break;
            }
        
        pstmt = con->prepareStatement("SELECT * FROM user;");
        result = pstmt->executeQuery();
        //bool is_join = false;
        while (result->next()) {
            if (user_name == result->getString(1).c_str()) {
                cout << "이미 존재하는 ID입니다." << endl << endl;
                is_join = true;
                break;
            }
        }
        if (is_join == false)
            id_ok = true;
    }
    while (pw_ok == false) {
        cout << "--------------------------------------------------" << endl;
        cout << "password를 입력하세요(20자 이내) : ";
        cin >> user_pw;
        if (user_pw.length() > 20) {
            cout << "20자 이내로 입력해주세요." << endl << endl;
        }
        else {
            pw_ok = true;
            break;
        }
    }  
    pstmt = con->prepareStatement("INSERT INTO user(name, pw) VALUES(?,?)"); // INSERT

    pstmt->setString(1, user_name); // 첫 번째 컬럼에 id 삽입
    pstmt->setString(2, user_pw); // 두 번째 컬럼에 pwd 삽입
    pstmt->execute(); // 쿼리 실행
    cout << user_name << "님, ID가 정상적으로 생성되셨습니다." << endl;
    cout << "--------------------------------------------------" << endl << endl;
}

void change_pw() {
    bool id_ok = false;
    bool new_pw_ok = false;
    while (id_ok == false) {
        cout << "--------------------------------------------------" << endl;
        cout << "ID를 입력하세요(10자 이내) : ";
        cin >> user_name;
        cout << "--------------------------------------------------" << endl;
        cout << "암호를 입력하세요(20자 이내) : ";
        cin >> user_pw;
        pstmt = con->prepareStatement("SELECT * FROM user;");
        result = pstmt->executeQuery();
        while (result->next()) {
            if (user_name == result->getString(1) && user_pw == result->getString(2)) {
                string new_pw;
                while (new_pw_ok == false) {
                    cout << "--------------------------------------------------" << endl;
                    cout << "변경할 암호를 입력하세요(20자 이내) : ";
                    cin >> new_pw;
                    if (new_pw.length() > 20) {
                        cout << "20자 이내로 입력해주세요." << endl << endl;                        
                    }
                    else {
                        new_pw_ok = true;
                        break;
                    }
                }
                pstmt = con->prepareStatement("UPDATE user SET pw = ? WHERE name = ?");
                pstmt->setString(1, new_pw);
                pstmt->setString(2, user_name);
                pstmt->executeQuery();
                cout << "--------------------------------------------------" << endl;
                printf("암호가 정상적으로 변경되었습니다.\n");
                cout << endl;
                id_ok = true;
                break;
            }
        }
        if (id_ok == false) {
            cout << "올바른 회원정보가 아닙니다. " << endl << endl;
        }
    }
}

void delete_user() {
    cout << "--------------------------------------------------" << endl;
    cout << "ID를 입력하세요 : ";
    cin >> user_name;
    cout << "--------------------------------------------------" << endl;
    cout << "password를 입력하세요 : ";
    cin >> user_pw;
    //select  
    pstmt = con->prepareStatement("SELECT * FROM user;");
    result = pstmt->executeQuery();
    bool is_login = false;
    while (result->next()) {
        if (user_name == result->getString(1) && user_pw == result->getString(2)) {
            is_login = true;
            break;
        }
    }
    char delete_count;
    if (is_login) {
        cout << "--------------------------------------------------" << endl;
        cout << "정말 탈퇴하시겠습니까? (Y / N)";
        cin >> delete_count;
        if (delete_count == 'Y' || delete_count == 'y') {
            pstmt = con->prepareStatement("DELETE FROM user WHERE name = ?");
            pstmt->setString(1, user_name);
            result = pstmt->executeQuery();
            cout << "--------------------------------------------------" << endl;
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

void show_before_chat() {
    string msg = "";
    //select  
    pstmt = con->prepareStatement("SELECT sender, receiver, message, time from chatting limit 10");
    result = pstmt->executeQuery();

    while (result->next()) {
        string receiver = result->getString(2);
        if (receiver.empty() || receiver.compare(user_name) == 0) {
            msg += result->getString(1) + " : " + result->getString(3) + " [" + result->getString(4) + "]\n";
        }
    }
    cout << msg << endl;
}

void game() {
    //#GAME
    using std::random_device;
    using std::default_random_engine;
    using std::uniform_int_distribution;

    int min = 0;
    int max = 50;
    int chance = 5;
    int com = 0;
    int count = 0;
    int num;
    random_device rd;
    default_random_engine re(rd());
    uniform_int_distribution<int> dist(min, max);
    auto rand = bind(dist, re);
    count = 0;
    com = rand();

    cout << "***************** UP & DOWN GAME *****************" << endl;
    cout << "숫자의 범위 (" << min << "~" << max << ")" << endl;
    cout << "총 " << chance << "번의 기회가 있습니다." << endl;
    while (count <  chance) {
        cout << "숫자를 입력해주세요 : ";
        cin >> num;
        if (num < min || num > max) {
            cout << "범위 내의 숫자를 입력해주세요." << endl;
        }
        else {
            count++;
            if (num == com) {
                cout << count << "번 시도 끝에 맞추셨습니다! *게임종료*" << endl;
                return;
            }
            else
                cout << count << "번 시도 : " << (num > com ? "DOWN" : "UP") << endl;

        }
    }   
    cout << chance << "번 내에 숫자를 맞추지 못하였습니다.. 정답 : " << com << endl << endl;
    return;
}
