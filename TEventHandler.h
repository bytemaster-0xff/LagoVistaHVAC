#pragma once
class TEventHandler
{
public:
	virtual void Call(void) = 0;
};

template <class TClass> class TCEventHandler : public TEventHandler
{
private:
	void (TClass::*m_pFunc)(void);   // pointer to member function
	TClass* m_pObj;                  // pointer to object

public:

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	TCEventHandler(TClass* pObj, void(TClass::*pFunc)())
	{
		m_pObj = pObj;
		m_pFunc = pFunc;
	};

	// override function "Call"
	virtual void Call()
	{
		(*m_pObj.*m_pFunc)();
	};             // execute member function
};


