// UseDll.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "WsbThreadPool.h"
#include "iostream"
#include "process.h"

shared_ptr<wsb::CWsbThreadPool> mypool = NULL;//线程池

void JobFun(PVOID p)
{
	static int i = 0;
	HANDLE mutex = static_cast<HANDLE>(p);
	WaitForSingleObject(mutex, INFINITE);
	i++;
	std::cout << "My Job:" << i << std::endl;
	ReleaseMutex(mutex);
	
}

unsigned int WINAPI JobSubmitThread(PVOID pParam)
{	
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, L"jobwaitabc");
	WaitForSingleObject(hEvent, INFINITE);
	shared_ptr<wsb::CJob> myjob = wsb::CJob::CreateJob(JobFun, wsb::ThreadPriority::Normal, pParam);
	for (int i = 0; i < 100; i++)
	{
		mypool->SubmitJob(myjob);
	}
	return 0;
}

unsigned int WINAPI JobSubmitThread2(PVOID pParam)
{
	shared_ptr<wsb::CJob> myjob = wsb::CJob::CreateJob(JobFun, wsb::ThreadPriority::Normal, pParam);
	for (int i = 0; i < 20; i++)
	{
		mypool->SubmitJob(myjob);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (mypool == NULL)
	{
		mypool = wsb::CWsbThreadPool::CreateThreadPool(2, 100);
	}
	HANDLE mutex = CreateMutex(NULL, false, L"MyJob");
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, L"jobwaitabc");
	for (int i = 0; i < 1500; i++)//第个进程开启的线程数是有限的,再加上线程池的200个线程，总共开启1600个线程
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, JobSubmitThread, mutex, 0, NULL);
		CloseHandle(hThread);
	}
	SetEvent(hEvent);//开始向线程池提交Job
	for (int i = 0; i < 10000; i++)//开启10万个线程，提交20万次作业，由于线程提交完成以后会退出，所以不存在线程数被限制的问题
	{
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, JobSubmitThread2, mutex, 0, NULL);
		CloseHandle(hThread);
	}
	system("pause");
	return 0;
}

