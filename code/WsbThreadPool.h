#pragma once

#include <windows.h>
#include <memory>
#include <functional>
using std::shared_ptr;
using std::function;

#ifdef DLL_IMPLEMENT_POOL
#define DLL_API_POOL __declspec(dllexport)
#else
#define DLL_API_POOL __declspec(dllimport)
#endif

/********************************************************
****     һ�����������̳߳�ʵ��
****     ������ԣ�C++
****     ���ƽ̨��win32
****     �� �� �ߣ�������
****     ʱ    �䣺20150628
**********************************************************/

namespace wsb
{
	typedef function< void (PVOID) > TaskFunction;//�������к���
	class CRealThread;
	
	//�߳����ȼ�,��������ͨ�͸�
	enum ThreadPriority
	{
		Normal,
		High
	};

	//������ӿڣ����ڽӿڱ�̣�����ά��
	class DLL_API_POOL CJob
	{
	public:
		virtual ~CJob();
		virtual void ExecuteTask()=0;          //ִ�й���
		virtual void WaitJobFinish() = 0;      //�ȴ���������
		virtual void ResetJob() = 0;           //���ù�������
		virtual ThreadPriority GetJobPri() = 0;//�����ҵ���ȼ�
		static shared_ptr<CJob> CreateJob(TaskFunction run, ThreadPriority pri=ThreadPriority::Normal, PVOID pParam = NULL);//���ù�����������һ����������
	};

	//�̳߳ؽӿڣ����ڽӿڱ�̣�����ά��
	class DLL_API_POOL CWsbThreadPool
	{
	public:
		virtual ~CWsbThreadPool();
		virtual void SwitchActiveThread(CRealThread* pThread)=0;//�߳����Job���ת������,�����ǽ���������ջ����ȡ��һ����������ִ��
		virtual bool SubmitJob(shared_ptr<CJob>& job)=0;        //�ύһ������
		virtual bool CloseThreadPool() = 0;                     //�ر��̳߳�
		static shared_ptr<CWsbThreadPool> CreateThreadPool(size_t minThreadNum, size_t maxThreadNum);//���ù�����������һ���̳߳ض���
	};
}


