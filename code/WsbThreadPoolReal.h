#pragma once

#include "WsbThreadPool.h"
#include "stack"
#include "list"
#include "queue"
using std::list;
using std::stack;
using std::queue;

namespace wsb
{
	//Job�࣬�̳нӿ�CJob
	class CRealJob :public CJob
	{
	public:
		CRealJob(TaskFunction run,ThreadPriority pri,PVOID pParam);
		~CRealJob();
		void ExecuteTask();//ִ�й���
		void WaitJobFinish();//�ȴ���������
		void ResetJob();//���ù���
		ThreadPriority GetJobPri();//��ȡ��ҵ���ȼ�
	private:
		//��ֹ�����븳ֵ
		CRealJob(const CRealJob&);
		CRealJob& operator=(const CRealJob&);
	private:
		TaskFunction m_TaskFun;//����ִ����
		PVOID m_Param;//����ִ�в���
		HANDLE m_hFinishEvent;//�����¼�, ������������
		ThreadPriority m_Pri;//��ҵ���ȼ�
	};

	//�߳���
	class CRealThread
	{
	public:
		CRealThread(CWsbThreadPool* pool);
		~CRealThread();
		bool InitThread();//��ʼ���̣߳�������������߳�
		bool suspendThread();//�����߳�
		bool resumeThread();//�ָ��߳�
		bool AssignJob(shared_ptr<CJob>& job);//������ҵ
		bool RunJob();//������ҵ
		void notifyStartJob();//֪ͨ�̹߳�ʼ����
		void notifyThreadQuit();//���߳��˳��¼����ó����ź�״̬
	public:
		static unsigned int WINAPI threadFun(PVOID pParam);//�̺߳���
	private:
		HANDLE m_hThread;//�߳̾��
		unsigned int m_ThreadID;//�߳�ID
		bool m_bisExit;//���ڱ���߳��Ƿ��˳�
		HANDLE m_hWaitEvent;//�ȴ��¼��������̹߳���
		HANDLE m_hQuitEvent;//�����߳��˳��¼�
		shared_ptr<CJob> m_Job;//�̹߳���
		CWsbThreadPool* m_ThreadPoool;//�����̳߳�,�����̻߳���
	};

	//������
	class CMutex
	{
	public:
		CMutex();
		~CMutex();
		void waitMutex();
		void releaseMutex();
	private:
		HANDLE m_hMutex;
	private:
		CMutex(const CMutex&);
		CMutex& operator=(const CMutex&);
	};

	//��
	class CLock
	{
	public:
		explicit CLock(CMutex&);
		~CLock();
	private:
		CMutex& m_mutex;
	private:
		CLock(const CLock&);
		CLock& operator=(const CLock&);
	};

	//�����߳���
	class CIdleThreadStack
	{
	public:
		CIdleThreadStack();
		~CIdleThreadStack();
		size_t GetSize();//��ȡ��С
		bool isEmpty();//�Ƿ�Ϊ��
		void push(CRealThread* thread);//ѹ���߳�
		CRealThread* pop();//�����߳�
		void clear();//���
	private:
		stack<CRealThread*> m_ThreadStack;//�����߳�ջ
		CMutex m_mutex;//������
	};

	//������̶߳���
	class CActiveThreadList
	{
	public:
		CActiveThreadList();
		~CActiveThreadList();
		size_t GetSize();//��ȡ��С
		bool isEmpty();//�Ƿ�Ϊ��
		void addThread(CRealThread* thread);//����߳�
		void removeThread(CRealThread* thread);//�Ƴ��߳�
		void clear();//���
	private:
		list<CRealThread*> m_ActiveThread;//��߳�����
		CMutex m_mutex;//������
	};

	//����������
	class CJobQueue
	{
	public:
		CJobQueue();
		~CJobQueue();
		size_t GetSize();//��ȡ��С
		bool isEmpty();//�Ƿ�Ϊ��
		void pushJob(shared_ptr<CJob>& job);//ѹ��Job
		shared_ptr<CJob> popJop();//����Job
		void clear();//���
	private:
		queue<shared_ptr<CJob>> m_JobQueue;// ��������
		CMutex m_mutex;//������
	};

	//�����̳߳���
	class CRealThreadPool :public CWsbThreadPool
	{
	public:
		CRealThreadPool(size_t minNum,size_t maxNum);
		~CRealThreadPool();
		void SwitchActiveThread(CRealThread* pThread);//�߳����Job���ת������
		bool SubmitJob(shared_ptr<CJob>& job);//�ύһ������
		bool CloseThreadPool();//�ر��̳߳�
	private:
		CRealThreadPool(const CRealThreadPool&);
		CRealThreadPool& operator=(const CRealThreadPool&);
		void IncreaseCapacity();//�����߳���,����Ϊ��ǰ�߳���������������߳���
		void DecreaseCapacity();//���ٿ����߳���
		shared_ptr<CJob> GetJob();//���һ��Job
		static unsigned int WINAPI CheckIdleThread(PVOID pParam);//�������߳����Ƿ����ϴ�仯�����ǣ��򲻶Կ����߳̽��м��ݴ���
	private:
		CActiveThreadList m_ActiveThread;//��̶߳���
		CIdleThreadStack m_IdleThread;//�����߳�ջ
		CJobQueue m_NormalJob;;//�������ȼ�job����
		CJobQueue m_HighJob;//�����ȼ�job����
		const size_t m_minThreadNum;//��С���߳���
		const size_t m_maxThreadNum;//�����߳���
		size_t m_ThreadNum;//ʵ�ʵ��߳�����
		HANDLE m_hQuitEvent;//�����߳�������߳��˳��ź�
		CMutex m_mutex;//������
	};
}