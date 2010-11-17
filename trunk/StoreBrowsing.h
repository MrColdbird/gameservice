// ======================================================================================
// File         : StoreBrowsing.h
// Author       : sun xiao dong 
// Last Change  : 07/29/2010 | 15:48:22 PM | Thursday,July
// Description  : 
// ======================================================================================


#pragma once
#ifndef StoreBrowsing_H
#define StoreBrowsing_H
#ifdef _PS3
namespace GameService
{

class StoreBrowsing
{
public:
	StoreBrowsing();
	~StoreBrowsing();

	void Init();//called after np sign in()
	void Term();
	void Start();
	bool IsInited(){ return bInited; }
private:
	bool bInited;
};

} // namespace GameService
#endif

#endif // StoreBrowsing_H
