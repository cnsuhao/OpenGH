#include "stdafx.h"
#include "PatchManager.h"
void PatchManager::AddPatch(DWORD Offset,char *patchbytes,int len)
{
	Patch object(Offset,len,patchbytes);
	patchmap[Offset]=object;
	WriteMemory(object.GetOffset(),object.GetPatched(),object.GetLength());
}
void PatchManager::RemovePatchAt(map<DWORD,Patch>::iterator &iter)
{
	Patch &object = iter->second;
	WriteMemory(object.GetOffset(),object.GetOriginal(),object.GetLength());
	patchmap.erase(iter);
}
void PatchManager::RemovePatch(DWORD offset)
{
	map<DWORD,Patch>::iterator iter= patchmap.find(offset);
	if(iter!=patchmap.end())
		RemovePatchAt(iter);
}
void PatchManager::WriteMemory(DWORD offset,char * bytes,int len)
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery((LPVOID)offset,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
	if(!VirtualProtect(mbi.BaseAddress,mbi.RegionSize,PAGE_EXECUTE_READWRITE,&mbi.Protect))
	{
		printf("VirtualProtect failed when trying to set PAGE_RW at %X\n",offset);
	}
	DWORD dwOldProtect;
	if(!WriteProcessMemory(GetCurrentProcess(),(LPVOID)offset,bytes,len,NULL))
	{	
		printf("WriteProcessMemory failed when trying to write %d bytes at %X\n",len,offset);
	}
	if(!VirtualProtect(mbi.BaseAddress,mbi.RegionSize,mbi.Protect,&dwOldProtect))
	{	
		printf("VirtualProtect failed when trying to restore old protection at %X\n",offset);
	}
	FlushInstructionCache(GetCurrentProcess(),(LPVOID)offset,len);
}
BOOL PatchManager::GetPatchInfo(DWORD Offset,Patch &info)
{
	map<DWORD,Patch>::iterator iter= patchmap.find(Offset);
	if(iter!=patchmap.end())
	{	
		info=iter->second;
		return true;
	}
	return false;
}
BOOL PatchManager::GetPatchInRange(DWORD Start,DWORD End,Patch &Info)
{
	map<DWORD,Patch>::iterator iter= patchmap.begin();
	while(iter!=patchmap.end())
	{	
		DWORD iterstart=(iter->second).GetOffset();
		DWORD iterend=iterstart+(iter->second).GetLength();
		DWORD intersmin= MAX(iterstart,Start);
		DWORD intersmax= MIN(iterend,End);
		if(!(intersmax<=intersmin))
		{
			Info=iter->second;
			return true;
		}
		++iter;
	}
	return false;
}
BOOL PatchManager::DetourJMP(DWORD offset, DWORD to, int nopcount)
{
	char *buff=new char[5+nopcount];
	buff[0]=0xE9;
	*(DWORD*)&buff[1]=to-(offset+5);
	for(int i=0;i<nopcount;i++)
		buff[5+i]=0x90;
	AddPatch(offset,buff,5+nopcount);
	delete []buff;
	return TRUE;
}
BOOL PatchManager::DetourCALL(DWORD offset,DWORD to,int nopcount)
{
	char *buff=new char[5+nopcount];
	buff[0]=0xE8;
	*(DWORD*)&buff[1]=to-(offset+5);
	for(int i=0;i<nopcount;i++)
		buff[5+i]=0x90;
	AddPatch(offset,buff,5+nopcount);
	delete []buff;
	return TRUE;
}
void PatchManager::RemoveAllPatches()
{
	map<DWORD,Patch>::iterator iter=patchmap.begin(),iter2=iter;
	while(iter!=patchmap.end())
	{
		iter2=iter++;
		RemovePatchAt(iter2);
	}
}