#ifndef __RHI_H__
#define __RHI_H__


class RHI
{
public:
	RHI() {};
	virtual ~RHI() {};
	virtual void Init() = 0;
	virtual void Update() = 0;

protected:


protected:
	virtual void ClearScreen() = 0;
	

};

















#endif





