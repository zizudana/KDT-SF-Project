#pragma once

#ifndef _SERVER_H
#define _SERVER_H

#include <WinSock2.h>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <random>
#include <functional>
#include <mysql/jdbc.h>

#define MAX_SIZE 1024
#define MAX_CLIENT 10

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::random_device;
using std::default_random_engine;
using std::uniform_int_distribution;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1234"; // 데이터베이스 접속 비밀번호

struct SOCKET_INFO { // 연결된 소켓 정보에 대한 틀 생성
    SOCKET sck;
    string user;
};

std::vector<SOCKET_INFO> sck_list; // 연결된 클라이언트 소켓들을 저장할 배열 선언.
SOCKET_INFO server_sock; // 서버 소켓에 대한 정보를 저장할 변수 선언.
int client_count = 0; // 현재 접속해 있는 클라이언트를 count 할 변수 선언.

void server_init(); // socket 초기화 함수. socket(), bind(), listen() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void add_client(); // 소켓에 연결을 시도하는 client를 추가(accept)하는 함수. client accept() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void send_msg(const char* msg); // send() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void recv_msg(int idx); // recv() 함수 실행됨. 자세한 내용은 함수 구현부에서 확인.
void del_client(int idx); // 소켓에 연결되어 있는 client를 제거하는 함수. closesocket() 실행됨. 자세한 내용은 함수 구현부에서 확인.
void show_user(int idx);
void calculator(int idx, int position, string sbuf);
void send_dm(int position, string sbuf, int idx);

// MySQL Connector/C++ 초기화
sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* result;

#endif