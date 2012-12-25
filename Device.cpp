#include <dag_config_api.h>
#include <dagapi.h>
#include <dagutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <vcclr.h>
#include <winsock2.h>
#include <Windows.h>
#include "Device.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Threading::Tasks;
using namespace Dagnabnet;

#define ETHER_TYPE_IPV4 0x0800

Device::Device()
	:	_cts()
	,	_receive(nullptr)
{
}


void Device::Open(System::String^ name)
{
	char dagname_buf[DAGNAME_BUFSIZE];
	char dagname[DAGNAME_BUFSIZE];
	int dagstream;

	pin_ptr<const wchar_t> wch = PtrToStringChars(name);
	
	size_t size = DAGNAME_BUFSIZE;
	if(wcstombs_s(&size,dagname_buf,size,wch,size)!=0)
	{
		throw gcnew Exception();
	}	
	
	if(-1==dag_parse_name(dagname_buf,dagname,DAGNAME_BUFSIZE,&dagstream))
	{
		throw gcnew Exception();
	}

	if((_fd == dag_open(dagname)) < 0)
	{
		throw gcnew Exception();
	}

	_stream = dagstream;
}

void Device::Configure()
{
	Configure(System::String::Empty);
}

void Device::Configure(System::String^ parameters)
{
	pin_ptr<const wchar_t> wch = PtrToStringChars(parameters);
	size_t converted = 0;
	size_t size = ((parameters->Length + 1) * 2);

	char* params = (char*)malloc(size);

	if(wcstombs_s(&converted,params,size,wch,size)!=0)
	{
		throw gcnew Exception();
	}

	if(dag_configure(_fd,params)<0)
	{
		throw gcnew Exception();
	}
}

void Device::Reset()
{
}

void Device::Attach()
{
	Attach(0,0,0);
}

void Device::Attach(int stream, int flags, int extra)
{
	if(dag_attach_stream(_fd,stream,flags,extra)<0)
	{
		throw gcnew Exception();
	}
}

void Device::Start(){
	if(dag_start_stream(_fd,_stream)<0)
	{
		throw gcnew Exception();
	}

	Task::Factory->StartNew(gcnew Action(this, &Device::Poll),_cts.Token);
}

void Device::Close()
{
	_cts.Cancel();

	if(dag_close(_fd)<0)
	{
		throw gcnew Exception();
	}
}

void Device::Poll()
{
	timeval wait;
	timeval poll;
	dag_record_t* record = nullptr;
	eth_rec_t* eth = nullptr;
	array<Byte>^ buffer = gcnew array<Byte>(ONE_KIBI);

	timerclear(&wait);
	wait.tv_usec = 100*1000;/*100 ms timeout */

	timerclear(&poll);
	poll.tv_usec = 10*1000; /* 10ms poll interval */

	dag_set_stream_poll(_fd,0,32*ONE_KIBI,&wait,&poll);
	
	while(false == _cts.Token.IsCancellationRequested)
	{
		auto next = dag_rx_stream_next_record(_fd,0);
		record = (dag_record_t*)next;
		if((record) && (ERF_TYPE_ETH == record->type) && (ETHER_TYPE_IPV4 == ntohs(record->rec.eth.etype)))
		{			
			Marshal::Copy((IntPtr)record->rec.eth.pload,buffer,0,32);			
			Receive(buffer);
		}
	}
}