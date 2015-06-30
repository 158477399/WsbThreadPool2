#define DLL_IMPLEMENT_POOL

#include "WsbThreadPoolReal.h"
#include "process.h"

namespace wsb
{
	//�����ಿ�ֿ�ʼ  �޸�ʱ�䣺20150628
	CJob::~CJob()
	{

	}
	
	//************************************************************
	//********���ܣ�����һ��Job����
	//********������Job�����shared_ptrָ��
	//********����ֵ��
	//************************************************************
	shared_ptr<CJob> CJob::CreateJob(TaskFunction run, ThreadPriority pri, PVOID pParam)//���������ຯ��
	{
		return shared_ptr<CJob>(new CRealJob(run,pri,pParam));
	}

	//************************************************************
	//********���ܣ�CRealJob��Ĺ��캯��
	//********������
	//********����ֵ��
	//************************************************************
	CRealJob::CRealJob(TaskFunction run, ThreadPriority pri, PVOID pParam) :m_TaskFun(run), m_Pri(pri), m_Param(pParam)
	{
		m_hFinishEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	//************************************************************
	//********���ܣ�CRealJob�����������
	//********������
	//********����ֵ��
	//************************************************************
	CRealJob::~CRealJob()
	{
		if (m_hFinishEvent)
		{
			CloseHandle(m_hFinishEvent);
		}
	}

	//************************************************************
	//********���ܣ�Job����ִ�о��������
	//********������
	//********����ֵ��
	//************************************************************
	void CRealJob::ExecuteTask()
	{
		m_TaskFun(m_Param);
		SetEvent(m_hFinishEvent);//�����¼���֪ͨ��������
	}

	//************************************************************
	//********���ܣ��ȴ�Job�������
	//********������
	//********����ֵ��
	//************************************************************
	void CRealJob::WaitJobFinish()
	{
		WaitForSingleObject(m_hFinishEvent, INFINITE);
	}

	//************************************************************
	//********���ܣ�����Job����
	//********������
	//********����ֵ��
	//************************************************************
	void CRealJob::ResetJob()
	{
		ResetEvent(m_hFinishEvent);//�����¼��������¿�ʼ����
	}

	//************************************************************
	//********���ܣ���ȡJob��������ȼ�
	//********������
	//********����ֵ��������������ȼ�
	//************************************************************
	ThreadPriority CRealJob::GetJobPri()
	{
		return m_Pri;
	}
	//�����ಿ�ֽ���

	//�߳��ಿ�ֿ�ʼ   �޸�ʱ�䣺20150628
	//************************************************************
	//********���ܣ��߳���Ĺ��캯���������̣߳��������������̳߳�
	//********������
	//********����ֵ��
	//************************************************************
	CRealThread::CRealThread(CWsbThreadPool* pool) :
		m_hThread(NULL),
		m_ThreadID(0),
		m_bisExit(false),
		m_hWaitEvent(NULL),
		m_hQuitEvent(NULL),
		m_Job(NULL),
		m_ThreadPoool(pool)
	{
		m_hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		m_hThread = (HANDLE)_beginthreadex(NULL, 0, threadFun, this,0, &m_ThreadID);//����һ���߳�
	}

	//************************************************************
	//********���ܣ��߳���������������˳��߳�
	//********������
	//********����ֵ��
	//************************************************************
	CRealThread::~CRealThread()
	{
		if (!m_bisExit&&m_hThread)
		{
			m_bisExit = true;
			notifyStartJob();
			WaitForSingleObject(m_hQuitEvent, INFINITE);
			CloseHandle(m_hThread);
			m_hThread = NULL;
			m_ThreadID = 0;
		}
		if (m_hWaitEvent)
		{
			CloseHandle(m_hWaitEvent);
		}
		if (m_hQuitEvent)
		{
			CloseHandle(m_hQuitEvent);
		}
	}

	//************************************************************
	//********���ܣ��߳�ִ����
	//********�������̶߳���ָ��
	//********����ֵ��
	//************************************************************
	unsigned int WINAPI CRealThread::threadFun(PVOID pParam)//�̺߳���
	{
		CRealThread* pThread = static_cast<CRealThread*>(pParam);
		if (pThread == NULL)return 0;
		while (!pThread->m_bisExit)
		{
			DWORD ret = WaitForSingleObject(pThread->m_hWaitEvent, INFINITE);
			if (ret == WAIT_OBJECT_0)
			{
				pThread->RunJob();//���й���
			}
		}
		pThread->notifyThreadQuit();
		return 0;
	}

	//************************************************************
	//********���ܣ������������е��߳�
	//********������
	//********����ֵ��
	//************************************************************
	bool CRealThread::suspendThread()//�����߳�,�����̶߳�ջ�ڵ��߳�ȫ������
	{
		if (m_hThread)
		{
			SuspendThread(m_hThread);
		}
		return true;
	}

	//************************************************************
	//********���ܣ���������ָ̻߳�
	//********������
	//********����ֵ���ɹ����
	//************************************************************
	bool CRealThread::resumeThread()//�ָ��̣߳����ж�ջ���߳�ת�Ƶ��˶��߳�������ʱ���ã��Իָ��߳�����
	{
		if (m_hThread)
		{
			int ret = ResumeThread(m_hThread);
			if (ret > 1)return false;
			return true;
		}
		return false;
	}

	//************************************************************
	//********���ܣ�������߳�һ��Job,������Job
	//********������Job�����shared_ptrָ��
	//********����ֵ��
	//************************************************************
	bool CRealThread::AssignJob(shared_ptr<CJob>& job)
	{
		m_Job = job;
		m_Job->ResetJob();//������ҵ
		return true;
	}

	//************************************************************
	//********���ܣ����д���Job,������ɺ�Job�ÿղ������̳߳ص�ת������
	//********��������
	//********����ֵ��
	//************************************************************
	bool CRealThread::RunJob()
	{
		if (m_Job != NULL)
		{
			m_Job->ExecuteTask();
			m_Job = NULL;
			if (m_ThreadPoool)m_ThreadPoool->SwitchActiveThread(this);//���Job�󣬽���ת������ȡ��һ��������̣߳����߳�ѹ�����ջ��
		}
		return true;
	}

	//************************************************************
	//********���ܣ�֪ͨ�߳̿�ʼ����Job
	//********��������
	//********����ֵ��
	//************************************************************
	void CRealThread::notifyStartJob()//֪ͨ�̹߳�ʼ����
	{
		SetEvent(m_hWaitEvent);
	}

	//************************************************************
	//********���ܣ�֪ͨ�߳��Ѿ��˳�����
	//********��������
	//********����ֵ��
	//************************************************************
	void CRealThread::notifyThreadQuit()
	{
		SetEvent(m_hQuitEvent);
	}
	//�߳��ಿ�ֽ���   
	  

	//�����������ಿ�ֿ�ʼ  �޸�ʱ�䣺20150628
	CMutex::CMutex()
	{
		m_hMutex = CreateMutex(NULL, FALSE, NULL);
	}

	CMutex::~CMutex()
	{
		if (m_hMutex)
		{
			CloseHandle(m_hMutex);
		}
	}

	void CMutex::waitMutex()
	{
		if (m_hMutex == NULL)return;
		WaitForSingleObject(m_hMutex, INFINITE);
	}
	void CMutex::releaseMutex()
	{
		if (m_hMutex == NULL)return;
		ReleaseMutex(m_hMutex);
	}

	CLock::CLock(CMutex& mutex) :m_mutex(mutex)
	{
		m_mutex.waitMutex();
	}

	CLock::~CLock()
	{
		m_mutex.releaseMutex();
	}
	//�����������ಿ�ֽ���


	//�����̶߳�ջ�ಿ�ֿ�ʼ   �޸�ʱ�䣺20150628
	CIdleThreadStack::CIdleThreadStack()
	{

	}

	CIdleThreadStack::~CIdleThreadStack()
	{
		clear();//���
	}

	//************************************************************
	//********���ܣ���ȡ�����߳�ջ�Ĵ�С
	//********��������
	//********����ֵ�����ػ�߳������size
	//************************************************************
	size_t CIdleThreadStack::GetSize() const
	{
		return m_ThreadStack.size();
	}

	//************************************************************
	//********���ܣ���ȡ�����߳�ջ�Ƿ�Ϊ��
	//********��������
	//********����ֵ��true or false
	//************************************************************
	bool CIdleThreadStack::isEmpty() const
	{
		return m_ThreadStack.empty();
	}

	//************************************************************
	//********���ܣ�������߳�ջ�����һ���̶߳���ָ��
	//********�������̶߳���ָ��
	//********����ֵ��
	//************************************************************
	void CIdleThreadStack::push(CRealThread* thread)
	{
		if (thread != NULL)
		{
			CLock myLock(m_mutex);
			m_ThreadStack.push(thread);
		}
	}

	//************************************************************
	//********���ܣ����������߳�ջ�е�һ���̶߳���ָ��
	//********������
	//********����ֵ���̶߳���ָ��
	//************************************************************
	CRealThread* CIdleThreadStack::pop()
	{
		CLock myLock(m_mutex);
		if (m_ThreadStack.empty())return NULL;
		CRealThread* res = m_ThreadStack.top();
		m_ThreadStack.pop();
		return res;
	}

	//************************************************************
	//********���ܣ����һ�������߳�ջ
	//********��������
	//********����ֵ��
	//************************************************************
	void CIdleThreadStack::clear()
	{
		CLock myLock(m_mutex);
		while (!m_ThreadStack.empty())
		{
			CRealThread* res = m_ThreadStack.top();
			m_ThreadStack.pop();
			delete res;
		}
	}
	//�����̶߳�ջ�ಿ�ֽ���


	//������̶߳��в��ֿ�ʼ  �޸�ʱ�䣺20150628
	CActiveThreadList::CActiveThreadList()
	{

	}

	CActiveThreadList::~CActiveThreadList()
	{
		//while (!m_ActiveThread.empty())
		//{
		//	//*******************���������⣡����������������������
		//	CRealThread* p = m_ActiveThread.front();
		//	m_ActiveThread.pop_front();
		//	delete p;
		//}
		clear();
	}

	//************************************************************
	//********���ܣ���ȡ��߳�����Ĵ�С
	//********��������
	//********����ֵ�����ػ�߳������size
	//************************************************************
	size_t CActiveThreadList::GetSize() const
	{
		return m_ActiveThread.size();
	}

	//************************************************************
	//********���ܣ���ȡ��߳������Ƿ�Ϊ��
	//********��������
	//********����ֵ��true or false
	//************************************************************
	bool CActiveThreadList::isEmpty() const
	{
		return m_ActiveThread.empty();
	}

	//************************************************************
	//********���ܣ����߳����������һ���̶߳���ָ��
	//********�������̶߳���ָ��
	//********����ֵ��
	//************************************************************
	void CActiveThreadList::addThread(CRealThread* thread)
	{
		if (thread)
		{
			CLock myLock(m_mutex);
			m_ActiveThread.push_back(thread);
		}
	}

	//************************************************************
	//********���ܣ��Ƴ�һ���̶߳���ָ��
	//********��������
	//********����ֵ��
	//************************************************************
	void CActiveThreadList::removeThread(CRealThread* thread)
	{
		if (thread)
		{
			CLock myLock(m_mutex);
			m_ActiveThread.remove(thread);
		}
	}

	//************************************************************
	//********���ܣ����һ����߳�����
	//********��������
	//********����ֵ��
	//************************************************************
	void CActiveThreadList::clear()
	{
		CLock myLock(m_mutex);
		while (!m_ActiveThread.empty())
		{
			CRealThread* p = m_ActiveThread.front();
			m_ActiveThread.pop_front();
			delete p;
		}
	}
	//������̶߳��в��ֽ���
	

	//���������ಿ�ֿ�ʼ   �޸�ʱ�䣺20150628
	CJobQueue::CJobQueue()
	{

	}

	CJobQueue::~CJobQueue()
	{
		clear();
	}

	//************************************************************
	//********���ܣ���ȡ�������еĴ�С
	//********��������
	//********����ֵ�����ع������е�size
	//************************************************************
	size_t CJobQueue::GetSize() const
	{
		return m_JobQueue.size();
	}

	//************************************************************
	//********���ܣ���ȡ���������Ƿ�Ϊ��
	//********��������
	//********����ֵ��true or false
	//************************************************************
	bool CJobQueue::isEmpty() const
	{
		return m_JobQueue.empty();
	}

	//************************************************************
	//********���ܣ�����������ѹ��һ����������
	//********���������������shared_ptrָ��
	//********����ֵ��
	//************************************************************
	void CJobQueue::pushJob(shared_ptr<CJob>& job)
	{
		if (job != NULL)
		{
			CLock mLock(m_mutex);
			m_JobQueue.push(job);
		}
	}

	//************************************************************
	//********���ܣ�popһ����������
	//********��������
	//********����ֵ�����������shared_ptrָ��
	//************************************************************
	shared_ptr<CJob> CJobQueue::popJop()
	{
		CLock mLock(m_mutex);
		if (m_JobQueue.empty())return NULL;
		shared_ptr<CJob> job = m_JobQueue.front();
		m_JobQueue.pop();
		return job;
	}

	//************************************************************
	//********���ܣ����һ����������
	//********��������
	//********����ֵ��
	//************************************************************
	void CJobQueue::clear()
	{
		CLock mLock(m_mutex);
		while (!m_JobQueue.empty())
			m_JobQueue.pop();
	}
	//���������ಿ�ֽ���
	

	//�̳߳ز��ֿ�ʼ �޸�ʱ�䣺20150628
	CWsbThreadPool::~CWsbThreadPool()
	{
	}

	//************************************************************
	//********���ܣ������̳߳ض���
	//********�������̳߳����߳���������Сֵ�����ֵ
	//********����ֵ���̳߳�shared_ptrָ�룬����RAII�������
	//************************************************************
	shared_ptr<CWsbThreadPool> CWsbThreadPool::CreateThreadPool(size_t minThreadNum, size_t maxThreadNum)
	{
		return shared_ptr<CWsbThreadPool>(new CRealThreadPool(minThreadNum, maxThreadNum));
	}

	//************************************************************
	//********���ܣ������̳߳��๹�캯�����������ֵ����Сֵ��ֵ�����߳�
	//********�������̳߳����߳���������Сֵ�����ֵ
	//********����ֵ��
	//************************************************************
	CRealThreadPool::CRealThreadPool(size_t minNum, size_t maxNum) :m_minThreadNum(minNum), m_maxThreadNum(maxNum), m_ThreadNum((minNum + maxNum) / 2), m_hQuitEvent(NULL)
	{
		for (size_t i = 0; i < m_ThreadNum; i++)//�������ֵ����Сֵ��ֵ�����߳�
		{
			m_IdleThread.push(new CRealThread(this));
		}
		m_hQuitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, CheckIdleThread, this, 0, NULL);//���������߳�ջ����̣߳�ÿ��15s���һ��
		CloseHandle(hThread);
	}

	//************************************************************
	//********���ܣ������̳߳�����������
	//********������
	//********����ֵ��
	//************************************************************
	CRealThreadPool::~CRealThreadPool()
	{
		if (m_hQuitEvent)
		{
			SetEvent(m_hQuitEvent);//�˳�����߳�
			CloseHandle(m_hQuitEvent);
		}
		CloseThreadPool();//�ر��̳߳�
	}

	//************************************************************
	//********���ܣ������߳�ջ����߳̾���ʵ��
	//********������ÿ��15s�ռ���ǰ�����߳�ջ���߳��������ռ���20������㷽�������С��1.5����˵������ջ�����߳�������ά�ֲ��䣬�ɽ��м��ݴ���
	//********�������̳߳�ָ��
	//********����ֵ��
	//************************************************************
	unsigned int WINAPI CRealThreadPool::CheckIdleThread(PVOID pParam)
	{
		CRealThreadPool* pool = static_cast<CRealThreadPool*>(pParam);
		if (pool == NULL)return 0;
		list<int> vThrNum;//�����߳�������
		list<int>::const_iterator iter;
		double flag1 = 0.0;//����
		double flag2 = 0.0;//��ֵ
		while (WaitForSingleObject(pool->m_hQuitEvent, 15000) == WAIT_TIMEOUT)
		{
			if (vThrNum.size() < 20)
			{
				vThrNum.push_back(pool->m_IdleThread.GetSize());
			}
			else
			{
				vThrNum.pop_front();
				vThrNum.push_back(pool->m_IdleThread.GetSize());
				//�����ֵ�뷽��
				for (iter = vThrNum.begin(); iter != vThrNum.end(); iter++)
				{
					flag2 += (*iter);
				}
				flag2 /= vThrNum.size();
				for (iter = vThrNum.begin(); iter != vThrNum.end(); iter++)
				{
					flag1 += ((*iter) - flag2)*((*iter) - flag2);
				}
				flag1 /= vThrNum.size();
				flag1 = sqrt(flag1);
				//������С��1.5������м��ݴ���
				if (flag1 < 1.5)
				{
					pool->DecreaseCapacity();//����
					vThrNum.clear();//��ռ�¼
				}
				flag1 = 0.0;
				flag2 = 0.0;
			}
		}
		return 0;
	}

	//************************************************************
	//********���ܣ��߳����Job���ת������
	//********������ÿ���̶߳�������һ���̳߳ض���ָ�룬�Ա�ʾ���߳����������̳߳أ����߳����Job���̳߳ض���Ҫ������߳̽���ת�����Ծ����Ƿ�������߳�ջ���Ǽ���������һ��Job
	//********�������̶߳���ָ��
	//********����ֵ����
	//************************************************************
	void CRealThreadPool::SwitchActiveThread(CRealThread* pThread)
	{
		if (m_NormalJob.isEmpty()&&m_HighJob.isEmpty())//�������ȼ�����ͨ���ȼ����ж�û�й��������򽫸��̴߳ӻ�߳�����ת�Ƶ������߳�ջ�У�����ͷ��乤�������߳�
		{
			m_ActiveThread.removeThread(pThread);//�ӻ�������Ƴ�
			m_IdleThread.push(pThread);//ѹ������߳�ջ
		}
		else
		{
			shared_ptr<CJob> job = NULL;
			//ȡ����
			if (!m_HighJob.isEmpty())
			{
				job = m_HighJob.popJop();
			}
			else
			{
				job = m_NormalJob.popJop();
			}
			//���乤��
			if (job != NULL)
			{
				pThread->AssignJob(job);
				pThread->notifyStartJob();
			}
		}
	}

	//************************************************************
	//********���ܣ����̳߳��ύ��������
	//********���������̳߳����п����߳����������ù��������̲߳�֪ͨ������������̳߳����ݻ򽫹����������ȼ�ѹ����Ӧ�Ĺ������У� ��/��ͨ��������
	//********��������������shared_ptrָ��
	//********����ֵ����
	//************************************************************
	bool CRealThreadPool::SubmitJob(shared_ptr<CJob>& job)//�ύһ������
	{
		if (m_IdleThread.isEmpty())//�����߳�ջΪ��
		{
			if (m_ThreadNum >= m_maxThreadNum)//��ǰ�߳������ڵ�������߳�����������ѹ�빤������
			{
				if (job->GetJobPri() == ThreadPriority::Normal)
					m_NormalJob.pushJob(job);//ѹ����ͨ��������
				else
					m_HighJob.pushJob(job);//ѹ������ȼ���������
				return true;
			}
			else
			{
				IncreaseCapacity();//����
			}
		}
		CRealThread* pThread=m_IdleThread.pop();//�ӿ���ջȡ��
		if (pThread == NULL)return false;

		m_ActiveThread.addThread(pThread);//��������
		pThread->AssignJob(job);//��������
		pThread->notifyStartJob();//��ʼ������

		return true;
	}

	//************************************************************
	//********���ܣ��ر��̳߳�
	//********��������ջ�̶߳��С���տ����߳�ջ����չ�������
	//********������
	//********����ֵ��
	//************************************************************
	bool CRealThreadPool::CloseThreadPool()//�ر��̳߳�
	{
		m_ThreadNum -= m_IdleThread.GetSize();
		m_IdleThread.clear();//��տ����߳�ջ
		m_ThreadNum -= m_ActiveThread.GetSize();
		m_ActiveThread.clear();//��ջ�߳�����
		m_NormalJob.clear();//�����ͨ���ȼ���������
		m_HighJob.clear();//��ո����ȼ���������
		return true;
	}

	//************************************************************
	//********���ܣ��̳߳����ݺ��������ӵ�ǰ�̳߳����߳�����
	//********���������ӵ���ǰ��߳������������߳������ֵ
	//********������
	//********����ֵ��
	//************************************************************
	void CRealThreadPool::IncreaseCapacity()
	{
		size_t nums = min(m_maxThreadNum, 2 * m_ThreadNum);
		while (m_ThreadNum < nums)
		{
			m_IdleThread.push(new CRealThread(this));
			m_ThreadNum++;
		}
	}

	//************************************************************
	//********���ܣ��̳߳ؼ��ݺ��������ٵ�ǰ�̳߳����߳�����
	//********���������ٵ�ǰ�����߳�����һ�룬�������������߳�����Ҫ������1�������߳����������߳�����Сֵ
	//********������
	//********����ֵ��
	//************************************************************
	void CRealThreadPool::DecreaseCapacity()
	{
		size_t nums = m_IdleThread.GetSize() / 2;
		while (m_IdleThread.GetSize()>1&&m_ThreadNum>m_minThreadNum&&nums > 0)
		{
			CRealThread* p = m_IdleThread.pop();
			delete p;
			m_ThreadNum--;
			nums--;
		}
	}
	//�̳߳ز��ֽ���
}

