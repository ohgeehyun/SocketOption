#pragma once
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

void HandleError(const char* cause)
{
	int32_t errCode = ::WSAGetLastError();
	cout << "ErrorCode : " << errCode << endl;
}

int main()
{

	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "start up 에러" << endl;
		return 0;
	}

	SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		HandleError("Socket");
		return 0;
	}
	//옵션을 해석하고 처리할 주체?
	//소켓 코드 ->SOL_SOCKET
	//IPv4->IPPROTO_IP
	//TCP 프로토콜 -> IPPROTO_TCP
	//어느 단계에서 설정한 옵션을 실행시킬것인가.(level)

	// SO_KEEPALIVE = 주기적으로 연결 상태 확인 여부(TCP Only)
	// 상대방이 소리소문없이 연결 끊을 경우 상대방이 의도적으로 보내지않는 것 인지 연결이 끊긴건지 애매모호
	// 주기적으로 TCP 프로토콜 연결 상태 확인 -> 끊어진 연결 감지 
	// 3번쨰 인자가 const char* 형식을받는다. 하지만 KEEPALIVE의 경우 문서에서 bool을 받기때문에 캐스팅 하여 적용

	bool enable = true;
	::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

	//SO_LINGER = 지연하다
	//송신 버퍼에 있는 데이터를 보낼 것인가? 날릴 것인가?
	//ex) send() 이후 바로 closesocket으로 닫혀버리면 커널 버퍼에는 send()가 들어가있는상태 일텐데?
	//on off = 0 이면 closesocket()이 바로 리턴 아니면 linger초만큼 대기(default 0)
	LINGER linger;
	linger.l_onoff = 1; //on_off
	linger.l_linger = 5;//대기시간
	::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&linger, sizeof(linger));

	//SO_SNDBUF = 송신 버퍼 크기
	//SO_RCVBUF = 수신 버퍼 크기 
	int32_t sendBufferSize;
	int32_t optionLen = sizeof(sendBufferSize);
	::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
	cout << "송신 버퍼 크기 : " << sendBufferSize << endl;

	int32_t recvBufferSize;
	optionLen = sizeof(recvBufferSize);
	::getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, &optionLen);
	cout << "수신 버퍼 크기 : " << recvBufferSize << endl;

	//SO_REUSEADDR
	//IP주소 및 port 재사용
	//이미 다른 곳에서 포트를 사용중이거나 다른 곳에서 강제종료등으로 찌꺼기를 남기고 종료하게되면 바로다시 socket bind를 할수가없다.
	{
		bool enable = true;
		::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
	}

	// IPPROTO_TCP
	// TCP_NODELAY = Nagle 네이글 알고리즘 작동 여부
	// 데이터가 충분히 크면 보내고 , 아니면 데이터가 충부닣 쌓일때 까지 대기. 회선낭비 줄이고 효율적이게 보내기
	// 나는 지금 당장 데이터를 보내야하는데 tcp레벨에서 데이터가 충분히쌓일떄까지 기다리면 큰일나니 알고리즘을 꺼주는것.
	// 장점: 작은 패킷이 불필요하게 많이 생성되는 일을 방지
	// 단점: 반응 시간 손해
	{
		bool enable = true;
		::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
	}




	//Half-Close
	//상대랑 통신중에 그냥 마음대로 소켓을 안쓸꺼라고 close떄리는건 좀 매너도없고 상대측의 동의를 구하지않고 일방적으로 끊어버리는게 문제
	//하지만 이론이라는 것. 온라인게임에서는 바로 close를 떄려도 크게 상관없다고한다.
	//SD_SEND : send막는다.
	//SD_RECEIVE : recv 막는다
	//SD_BOTH : 둘 다 막는다.
	//::shutdown(serverSocket, SD_SEND);
	//::closesocket(serverSocket);

	//윈속 종료
	::WSACleanup();

}
