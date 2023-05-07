#pragma once

#ifndef _CLIENT_H
#define _CLIENT_H

#include <WinSock2.h> //Winsock 헤더파일 include. WSADATA 들어있음.ㄴ
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mysql/jdbc.h>
#include <windows.h>
#include <ctime>
#include <random>
#include <functional>

#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;

const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "root"; // 데이터베이스 사용자
const string password = "1234"; // 데이터베이스 접속 비밀번호

SOCKET client_sock;
//string my_nick;
string user_name;
string user_pw;

void textcolor(int foreground, int background);
int chat_recv();
bool login(); 
bool join();
void change_pw();
void delete_user();
void show_before_chat();
void game();
sql::mysql::MySQL_Driver* driver; // 추후 해제하지 않아도 Connector/C++가 자동으로 해제해 줌
sql::Connection* con;
sql::Statement* stmt;
sql::PreparedStatement* pstmt;
sql::ResultSet* result;

#endif