﻿/*
 * WaitScene.cpp
 *
 *  Created on: 2017/06/14
 *      Author: kei
 */
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include "WaitScene.h"

WaitScene::WaitScene() :
SceneBase(SceneBase::SCENE_WAIT)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	FD_ZERO(&m_Readfds);
	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	m_Addr.sin_family = AF_INET;
	m_Addr.sin_port = htons(50000);
	m_Addr.sin_addr.s_addr = INADDR_ANY;

	bind(m_Socket, (sockaddr *)&m_Addr, sizeof(m_Addr));
	FD_SET(m_Socket, &m_Readfds);

	m_TimeOut.tv_sec = 2;
	m_TimeOut.tv_usec = 0;
	m_SendData.Id = 0;
	m_SendData.IsStart = false;

}

WaitScene::~WaitScene()
{
	closesocket(m_Socket);
	WSACleanup();
}

SceneBase::SceneID WaitScene::Update()
{
	memcpy(&m_Fds, &m_Readfds, sizeof(fd_set));
	select(0, &m_Fds, NULL, NULL, &m_TimeOut);

	if (FD_ISSET(m_Socket, &m_Fds))
	{
		RecvControl(m_Socket);
	}

	return m_SceneID;
}

void WaitScene::RecvControl(int _socket)
{
	auto CheckIsStart = [&]()
	{
		for (unsigned int i = 0; i < m_PlayerList.size(); i++)
		{
			if (!m_PlayerList[i].IsOk)
			{
				return false;
			}
		}
		return true;
	};

	auto SetIsOk = [&]()
	{
		for (unsigned int i = 0; i < m_PlayerList.size(); i++)
		{
			if (m_PlayerList[i].Id == m_RecvData.Id)
			{
				m_PlayerList[i].IsOk = m_RecvData.IsOk;
				break;
			}
		}
	};

	int len = (int)sizeof(sockaddr_in);
	recvfrom(_socket, reinterpret_cast<char*>(&m_RecvData), sizeof(RecvData), 0, (sockaddr*)&m_Addr, &len);
	if (m_PlayerList.size() == 0)
	{
		GameDataManager::PlayerData playerData;
		playerData.IsOk = false;
		playerData.Addr = m_Addr;
		playerData.Id = 1;
		m_PlayerList.push_back(playerData);
	}
	else
	{
		if (m_RecvData.Id == 0)
		{
			GameDataManager::PlayerData playerData;
			playerData.IsOk = false;
			playerData.Id = m_PlayerList.size() + 1;
			playerData.Addr = m_Addr;
			m_PlayerList.push_back(playerData);
		}
	}


	SetIsOk();
	std::system("clear");

	for (unsigned int i = 0; i < m_PlayerList.size(); i++)
	{
		printf("join from : %s\n", inet_ntoa(m_PlayerList[i].Addr.sin_addr));
		printf("       Id : %d\n", m_PlayerList[i].Id);
		if (m_PlayerList[i].IsOk)
		{
			printf("     IsOk : true\n\n");
		}
		else
		{
			printf("     IsOk : false\n\n");
		}

		m_SendData.Id = m_PlayerList[i].Id;
		m_Addr = m_PlayerList[i].Addr;
		m_SendData.IsStart = false;
		m_SendData.PlayerNum = m_PlayerList.size();
		if (CheckIsStart())
		{
			m_SendData.IsStart = true;
		}

		sendto(_socket, reinterpret_cast<char*>(&m_SendData),
			sizeof(SendData), 0, (struct sockaddr *)&m_Addr, sizeof(m_Addr));
	}

	if (CheckIsStart())
	{
		GameDataManager::GetInstance()->SetPlayerData(m_PlayerList);
		GameDataManager::GetInstance()->SetPlayerNum(m_PlayerList.size());
		m_SceneID = SceneBase::SCENE_GAME;
	}
}