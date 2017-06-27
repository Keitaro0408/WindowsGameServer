#include "SceneManager/SceneManager.h"
#include <Windows.h>
#include <stdio.h>


int main()
{
	system("cls");
	printf("ServerBoot:\n");

	SceneManager* sceneMnager = new SceneManager();
	DWORD SyncOld = timeGetTime();	//	システム時間を取得
	DWORD SyncNow;
	timeBeginPeriod(1);


	while (1)
	{
		Sleep(1);
		SyncNow = timeGetTime();
		if (SyncNow - SyncOld >= 1000 / 60) //	1秒間に60回この中に入る
		{
			sceneMnager->Run();
			SyncOld = SyncNow;
		}
	}

	timeEndPeriod(1);

	return 0;
}
