#ifndef PATCHMAN_H
#define PATCHMAN_H
#include <map>
#include <windows.h>
using namespace std;
#define MAX(X,Y) X>Y?X:Y
#define MIN(X,Y) X<Y?X:Y

class Patch
{
public:
	Patch():offset(0),length(0),patched(NULL),original(NULL) {}
	Patch(DWORD Offset,DWORD len,char * patch):offset(Offset),length(len)
	{
		patched= new char[length];
		memcpy(patched,patch,length);
		original=new char[length];
		memcpy(original,(void*)offset,length);
	}
	DWORD GetLength() const
	{
		return length;
	}
	DWORD GetOffset() const
	{
		return offset;
	}
	char * GetOriginal() const
	{
		return original;
	}
	char * GetPatched() const
	{
		return patched;
	}

	Patch( const Patch & obj):offset(0),length(0),patched(NULL),original(NULL)
	{
		CopyFrom(obj);
	}
	void CopyFrom(const Patch &obj)
	{
		delete [] patched;
		delete [] original;
		length = obj.length;
		offset = obj.offset;
		patched= new char[length];
		memcpy(patched,obj.patched,length);
		original=new char[length];
		memcpy(original,obj.original,length);
	}
	Patch &operator=(const Patch &obj)
	{
		if(this!=&obj)
		{
			CopyFrom(obj);
		}
		return *this;
	}
	~Patch()
	{
		delete [] patched;
		delete [] original;
	}

	private:
	DWORD offset;
	DWORD length;
	char *original;
	char *patched;
};
class PatchManager
{
public:
	~PatchManager()
	{
		RemoveAllPatches();
	}
	void RemoveAllPatches();
	void AddPatch(DWORD Offset,char *patchbytes,int len);
	BOOL DetourJMP(DWORD offset,DWORD to,int nopcount);
	BOOL DetourCALL(DWORD offset,DWORD to,int nopcount);
	void RemovePatchAt(map<DWORD,Patch>::iterator &iter);
	void RemovePatch(DWORD offset);
	void WriteMemory(DWORD offset,char * bytes,int len);
	BOOL GetPatchInfo(DWORD Offset,Patch &info);
	BOOL GetPatchInRange(DWORD Start,DWORD End,Patch &Info);
	private:
	map<DWORD,Patch> patchmap;

};
#endif 