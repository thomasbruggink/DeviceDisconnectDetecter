// DeviceVIewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <SetupAPI.h>
#include <vector>

using namespace std;

#ifndef UNICODE  
  typedef std::string String; 
#else
  typedef std::wstring String; 
#endif

struct Device
{
	String GUID;
	String HID;
	String Description;
	String FriendlyName;
};

vector<Device> GetDeviceList()
{
	vector<Device> devices;

	PDWORD size = 0;
	PDWORD dataType = 0;
	SP_DEVINFO_DATA deviceInfoData;

    auto infoSet = SetupDiGetClassDevs(
                                    NULL,
                                    NULL,
                                    NULL,
                                    DIGCF_ALLCLASSES | DIGCF_PRESENT);

	ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	auto deviceIndex = 0;
    
	while (SetupDiEnumDeviceInfo(infoSet, deviceIndex, &deviceInfoData)) 
	{
		deviceIndex++;
		auto error = GetLastError();

		OLECHAR* guidString;
		StringFromCLSID(deviceInfoData.ClassGuid, &guidString);

		TCHAR description[1024], hardwareId[1024], friendlyName[1024], szHardwareIDs[4096];
		
		if(!SetupDiGetDeviceRegistryProperty(infoSet, &deviceInfoData, SPDRP_DEVICEDESC, dataType, (BYTE*)description, sizeof(szHardwareIDs), size))
			continue;
		if(!SetupDiGetDeviceRegistryProperty(infoSet, &deviceInfoData, SPDRP_HARDWAREID, dataType, (BYTE*)hardwareId, sizeof(szHardwareIDs), size))
			continue;
		if(!SetupDiGetDeviceRegistryProperty(infoSet, &deviceInfoData, SPDRP_HARDWAREID, dataType, (BYTE*)friendlyName, sizeof(szHardwareIDs), size))
			continue;
		Device device;
		device.GUID = String(guidString);
		device.HID = String(hardwareId);
		device.Description = String(description);
		device.FriendlyName = String(friendlyName);
		devices.push_back(device);
		::CoTaskMemFree(guidString);
	}

	if (infoSet) 
	{
		SetupDiDestroyDeviceInfoList(infoSet);
	}
	return devices;
}

int main()
{
	auto now = GetDeviceList();
	for(unsigned int i = 0; i < now.size(); i++)
	{
		auto d = now[i];
		printf("%S\n", d.Description.c_str());
	}

	while(true)
	{
		auto newList = GetDeviceList();

		vector<Device> leftList;
		vector<Device> rightList;
		if(now.size() > newList.size())
		{
			leftList = now;
			rightList = newList;
		}
		else
		{
			leftList = newList;
			rightList = now;
		}
		for(unsigned int i = 0; i < leftList.size(); i++)
		{
			auto found = false;
			auto leftDevice = leftList[i];
			for(unsigned int j = 0; j < rightList.size(); j++)
			{
				auto rightDevice = rightList[j];
				if(rightDevice.HID.compare(leftDevice.HID) == 0)
				{
					found = true;
					break;
				}
			}
			if(!found)
			{
				printf("Could not find HID %S in the new list, Description: %S\n", leftDevice.HID.c_str(), leftDevice.Description.c_str());
			}
		}
		printf("No diff found in %d/%d devices\n", now.size(), newList.size());
	}
}